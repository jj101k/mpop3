#ifndef __storage_functions_h
#define __storage_functions_h

#include <stdlib.h>
#include "storage.h"

/*
 * int (*_storage_lock_mailbox)(char const *)
 *
 * Opens and locks the mailbox. Returns 1 if successful, 0 (locked?) otherwise.
 */
extern int (*_storage_lock_mailbox)(char const *mailbox);


extern enum arrayStyle (*_storage_array_style)();
extern unsigned long int (*_storage_message_count)();
extern unsigned long int (*_storage_message_sum)();
extern struct pop3_message *(*_storage_first_message)();
extern struct pop3_message *(*_storage_message_number)(unsigned long int);
extern int (*_storage_uidl_supported)();
extern int (*_storage_dump_message)(struct pop3_message *, FILE *);
extern int (*_storage_dump_headers)(struct pop3_message *, FILE *);
extern int (*_storage_dump_message_lines)(struct pop3_message *, unsigned long int, FILE *);
extern int (*_storage_synch)();
extern int (*_storage_dump_fragment)(unsigned long int, unsigned long int, unsigned long int, unsigned long int, FILE *);

#endif
