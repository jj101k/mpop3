#ifndef __storage_functions_h
#define __storage_functions_h

#include <stdlib.h>
#include "storage.h"

/*
 * int _storage_lock_mailbox(char const *)
 *
 * Open and lock the mailbox. Return 1 if successful, 0 (locked?) otherwise.
 */
extern int (*_storage_lock_mailbox)(char const *mailbox);

/*
 * enum arrayStyle _storage_array_style()
 *
 * Say how you have the 'struct pop3message's stored. If they're in a proper array (eg, if you could
 * cheaply count up how many were needed in advance), return asArray. Otherwise (the default) return
 * asLinkedList, and the 'next' pointers in each struct will be used instead.
 */
extern enum arrayStyle (*_storage_array_style)();

/*
 * unsigned long int _storage_message_count()
 *
 * Say how many messages there are (both deleted any undeleted). This gets called a lot, so you may
 * want to cache this value.
 */
extern unsigned long int (*_storage_message_count)();

/*
 * unsigned long int _storage_message_sum()
 *
 * (unused)
 */
extern unsigned long int (*_storage_message_sum)();

/*
 * struct pop3_message *_storage_first_message()
 *
 * Point to the first (real, whether deleted or not) message, ie. the start of the message array (or linked list)
 */
extern struct pop3_message *(*_storage_first_message)();

/*
 * struct pop3_message *_storage_message_number(unsigned long int)
 *
 * Point to a specific message, __where '1' means the first message__. The default implementation of this should be fine,
 * as it uses _storage_array_style() and _storage_first_message() to do the right thing.
 */
extern struct pop3_message *(*_storage_message_number)(unsigned long int);

/*
 * int _storage_uidl_supported()
 *
 * Return 1 if you have/will assign UIDL values to all messages. This only affects the UIDL POP3 command.
 */
extern int (*_storage_uidl_supported)();

/*
 * int _storage_dump_fragment(unsigned long int from_offset, unsigned long int to_offset, 
 *  unsigned long int skip_lines, unsigned long int max_lines, FILE * out)
 *
 * Dump some or all of a message to the given file pointer (ie, to the client). You need to byte-stuff leading full stops (.)
 * yourself, and you should guarantee that each line ends with \r\n.
 *
 * The offsets can be bytes, lines, or whatever. That's basically up to you, you just need to make sure that the
 * offsets in your 'struct pop3message's are of the same type. 'skip_lines' is there to help you skip any in-band
 * metadata (like the 'From ' line in mbox files); 'max_lines' is there to help with TOP.
 *
 * See below also.
 */
extern int (*_storage_dump_fragment)(unsigned long int, unsigned long int, unsigned long int, unsigned long int, FILE *);

/*
 * int _storage_dump_message(struct pop3_message *, FILE *)
 *
 * Dump an entire message. The default uses _storage_dump_fragment() and can be left alone unless you have
 * a specific optimisation to make.
 */
extern int (*_storage_dump_message)(struct pop3_message *, FILE *);

/*
 * int _storage_dump_headers(struct pop3_message *, FILE *)
 *
 * Dump just the message headers, as used in TOP. The default uses _storage_dump_fragment()
 */
extern int (*_storage_dump_headers)(struct pop3_message *, FILE *);

/* 
 * int _storage_dump_message_lines(struct pop3_message *, unsigned long int, FILE *)
 *
 * Dump the specified number of lines from the body of the message. Together with _storage_dump_headers(), this
 * forms more-or-less the entire TOP command. This also uses _storage_dump_fragment() by default.
 */
extern int (*_storage_dump_message_lines)(struct pop3_message *, unsigned long int, FILE *);

/*
 * int _storage_synch()
 *
 * Write out the mailbox, ie commit any message deletions to the storage system.
 */
extern int (*_storage_synch)();

/*
 * enum whichUser _storage_need_user()
 *
 * If any user will do, return wuAny; if you really want to be the default "safe" user, return wuDefault
 * otherwise, return wuMailbox to say you need to be the user which matches the mailbox name.
 *
 * Use this to specify which real user you need to be to access the mailbox. Default returns wuAny.
 */
extern enum whichUser (*_storage_need_user)() ;

#endif
