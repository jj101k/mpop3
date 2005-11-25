#include <stdio.h>
#include "storage_functions.h"

enum whichUser _default_storage_need_user() {return wuAny;}

int _default_storage_uidl_supported() {return 0;}
enum arrayStyle _default_storage_array_style() {return asLinkedList;}

int _default_storage_dump_fragment(unsigned long int from, unsigned long int to, unsigned long int skip_lines, unsigned long int max_lines, FILE *out) {
	return 0;
}

int _default_storage_dump_message(struct pop3_message *message, FILE *out) {
	if(!message) return 1;
	return _storage_dump_fragment(message->header_offset, message->body_end_offset, 1, 0, out);
}

int _default_storage_dump_headers(struct pop3_message *message, FILE *out) {
	if(!message) return 1;
	return _storage_dump_fragment(message->header_offset, message->header_end_offset, 1, 0, out);
}
int _default_storage_dump_message_lines(struct pop3_message *message, unsigned long int max_lines, FILE *out) {
	if(!message) return 1;
	return _storage_dump_fragment(message->header_end_offset, message->body_end_offset, 0, max_lines+1, out);
}

int _default_storage_lock_mailbox(char const *mailboxname) {
	return 0;
}

unsigned long int _default_storage_message_count() {return 0;};
unsigned long int _default_storage_message_sum() {return 0;};
struct pop3_message *_default_storage_first_message() {return NULL;};
struct pop3_message *_default_storage_message_number(unsigned long int index) {
	unsigned long int j;
	struct pop3_message *current_message;

	if(index>_storage_message_count()) return NULL;
	if(index<1) return NULL;
	current_message=_storage_first_message();
	if(index==1) return current_message;
	if(_storage_array_style()==asArray) return current_message+index-1;
	for(j=1;j<index;j++, current_message=current_message->next) {
		if(!current_message) return NULL;
	}
	return current_message;
}
int _default_storage_synch() { return 0; }
