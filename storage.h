#ifndef __storage_h
#define __storage_h

#include <stdlib.h>

struct pop3_message {
	struct pop3_message * next;
	char *uidl;
	// This is the size it will be on output, with all newlines normalised to \r\n
	unsigned long int size;
	// Where the header starts. It's up to you what this means.
	unsigned long int header_offset;
	// Where the header ends (including the last empty line)...
	unsigned long int header_end_offset;
	// ...and where the body (ie, whole message) ends.
	unsigned long int body_end_offset;
	// A count of how many lines in this message match /^\./, which you may find useful
	char deleted;
};

/*
 * int _storage_lock_mailbox(char const *)
 *
 * Opens and locks the mailbox. Returns 1 if successful, 0 (locked?) otherwise.
 */
int _storage_lock_mailbox(char const *mailbox);
unsigned long int _storage_message_count();
unsigned long int _storage_message_sum();
struct pop3_message *_storage_first_message();
struct pop3_message *_storage_message_number(unsigned long int);
int _storage_uidl_supported();
int _storage_dump_message(struct pop3_message *, FILE *);
int _storage_dump_message_lines(struct pop3_message *, unsigned long int, FILE *);
int _storage_dump_headers(struct pop3_message *, FILE *);
int _storage_synch();

#endif
