#include <unistd.h>
#include <regex.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "pop3.h"
#include "storage.h"

#define MBOX_ROOT "spools"

int spoolfd;
FILE *spoolfp;
char const *g_mailboxname;

#ifndef USE_OPENSSL
#define USE_OPENSSL 0
#endif

#if USE_OPENSSL
#include <openssl/evp.h>
#endif

#define LOCK_FLOCK 1
#define LOCK_MODE LOCK_FLOCK

int _storage_uidl_supported() {return USE_OPENSSL;}

#define IS_MESSAGE_START_LINE(line) (strncmp("From ", line, 5)==0)

struct pop3_message * first_message;
unsigned long int message_count;
unsigned long int total_size;
unsigned long int total_spool_size;

int cache_metadata() {
	char in_body=0, started=0;
	struct pop3_message * current_message=NULL;
	spoolfp=fdopen(spoolfd, "r");
	char current_line[RFC_MAX_OUTPUT_LENGTH];
	unsigned long int current_offset=0;
#if USE_OPENSSL
  EVP_MD_CTX *ssl_context;
  char digest_raw[EVP_MAX_MD_SIZE];
  
  ssl_context=EVP_MD_CTX_create();
	EVP_MD_CTX_init(ssl_context);
#endif

	while(fgets(current_line, RFC_MAX_OUTPUT_LENGTH-1, spoolfp)) {
		super_chomp_n(current_line, ftell(spoolfp)-current_offset);
		if(!started) {
			if(!IS_MESSAGE_START_LINE(current_line)) {
				
				//corrupt. Bail out.
				rewind(spoolfp);
				return 0;
			} else {
				first_message=calloc(1, sizeof(struct pop3_message));
				message_count++;
				first_message->header_offset=current_offset;
				current_message=first_message;
#if USE_OPENSSL
				EVP_DigestInit_ex(ssl_context, EVP_md5(), NULL);
#endif
			}
			started=1;
		} else if(IS_MESSAGE_START_LINE(current_line)) {
			in_body=0;
			if(!current_message->header_end_offset) current_message->header_end_offset=current_offset;
			current_message->body_end_offset=current_offset;
			current_message->next=calloc(1, sizeof(struct pop3_message));
			message_count++;
			total_size+=current_message->size;
#if USE_OPENSSL
			EVP_DigestFinal_ex(ssl_context, digest_raw, NULL);
			current_message->uidl=malloc((EVP_MD_size(EVP_md5())*2) + 1);
			hex_from_binary(current_message->uidl, digest_raw, EVP_MD_size(EVP_md5()));

			EVP_DigestInit_ex(ssl_context, EVP_md5(), NULL);
#endif

			current_message=current_message->next;
			current_message->header_offset=current_offset;
		} else {
			if((!in_body) && current_line[0]==0 ) {
				in_body=1;
				current_message->header_end_offset=current_offset;
			}
#if USE_OPENSSL
			EVP_DigestUpdate(ssl_context, current_line, ftell(spoolfp)-current_offset);
#endif
			current_message->size+=strlen(current_line)+2;
		}

		current_offset=ftell(spoolfp);
	}
	if(current_message) {
		if(!current_message->header_end_offset) current_message->header_end_offset=current_offset;
		current_message->body_end_offset=current_offset;
		total_size+=current_message->size;
#if USE_OPENSSL
		EVP_DigestFinal_ex(ssl_context, digest_raw, NULL);
		current_message->uidl=malloc((EVP_MD_size(EVP_md5())*2) + 1);
		hex_from_binary(current_message->uidl, digest_raw, EVP_MD_size(EVP_md5()));
#endif
	}
	total_spool_size=current_offset;
	rewind(spoolfp);
	return 1;
}
int _storage_dump_fragment(unsigned long int from, unsigned long int to, unsigned long int skip_lines, unsigned long int max_lines, FILE *out) {
	char current_line[RFC_MAX_OUTPUT_LENGTH];
	if(to<=from) return 1;
	unsigned long int i;
	unsigned long int line_count=0;
	char last_removed_char;
	char *cursor;
	unsigned long int file_offset;
	unsigned long int bytes_read;

	fseek(spoolfp, from, SEEK_SET);
	for(i=0;i<skip_lines;i++) {
		fgets(current_line, RFC_MAX_OUTPUT_LENGTH-1, spoolfp); // 'From ' line
	}
	if(ftell(spoolfp)>=to) return 1; // Shouldn't happen, of course...
	while(ftell(spoolfp)<to) {
		file_offset=ftell(spoolfp);
		fgets(current_line, RFC_MAX_OUTPUT_LENGTH-1, spoolfp);
		bytes_read=ftell(spoolfp)-file_offset;
		// This drop of magic is needed to re-attach lines broken in the middle of a \r\n pair.
		if(last_removed_char=='\r' && current_line[0]=='\n') {
			cursor=current_line+1;
			bytes_read--;
		} else {
			cursor=current_line;
		}
		last_removed_char=super_chomp_n(cursor, bytes_read);
		// RFC 2449 implicitly allows any length on multiline responses....
		if(!last_removed_char) {
			// Caught in mid-line.
			fwrite(cursor, bytes_read, 1, out);
		} else {
			if(cursor[bytes_read-1]==0) {
				bytes_read--;
			} else {
				bytes_read-=2;
			}
			if(cursor[0]=='.') {
				fwrite(".", 1, 1, out);
			}
			fwrite(cursor, bytes_read, 1, out);
			fwrite("\r\n", 2, 1, out);
			line_count++;
			if(max_lines && (line_count>=max_lines) ) break;
		}
	}
	return 1;
}

int _storage_lock_mailbox(char const *mailboxname) {
	regex_t preg;
	if(regcomp(&preg, "^[[:alnum:]_]+$", REG_EXTENDED|REG_NOSUB)) {
		return 0;
	}
	
	if(chdir(MBOX_ROOT)!=0) {
		syslog(LOG_ERR, "Can't enter mailbox root '%s'", MBOX_ROOT);
		return 0;
	}
	if(!mailboxname || !mailboxname[0]) {
		syslog(LOG_WARNING, "Bad mailboxname: '%s'", mailboxname);
		return 0;
	}
	if(regexec(&preg, mailboxname, 0, NULL, 0)!=0) {
		syslog(LOG_WARNING, "Bad mailboxname format: '%s'", mailboxname);
		return 0;
	}
	regfree(&preg);
#if LOCK_MODE == LOCK_FLOCK
	spoolfd=open(mailboxname, O_RDWR|O_CREAT|O_EXLOCK|O_NONBLOCK, 0660);
#else
	spoolfd=open(mailboxname, O_RDWR|O_CREAT, 0660);
#endif
	if(spoolfd<0) return 0;
	cache_metadata();
	g_mailboxname=mailboxname;
	return 1;
}

unsigned long int _storage_message_count() {return message_count;};
unsigned long int _storage_message_sum() {return total_size;};
struct pop3_message *_storage_first_message() {return first_message;};
int _storage_synch() {
	char *spool_copy=malloc(total_spool_size);
	struct pop3_message *current_message;
	unsigned long byte_pos=0;
	unsigned long to_read;

	for(current_message=first_message;current_message;current_message=current_message->next) {
		if(!current_message->deleted) {
			fseek(spoolfp, current_message->header_offset, SEEK_SET);
			to_read=current_message->body_end_offset-current_message->header_offset;
			fread(spool_copy+byte_pos, to_read, 1, spoolfp);
			byte_pos+=to_read;
		}
	}
	
	FILE *spool_w_fp=fopen(g_mailboxname, "w");
	if(!spool_w_fp) return 0;
	fwrite(spool_copy, byte_pos, 1 ,spool_w_fp);
	fclose(spool_w_fp);
	fclose(spoolfp);
	return 1;
}
