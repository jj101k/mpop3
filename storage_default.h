#include "storage.h"
int _default_storage_uidl_supported() ;
enum arrayStyle _default_storage_array_style() ;
int _default_storage_dump_fragment(unsigned long int from, unsigned long int to, unsigned long int skip_lines, unsigned long int max_lines, FILE *out) ;
int _default_storage_dump_message(struct pop3_message *message, FILE *out) ;
int _default_storage_dump_headers(struct pop3_message *message, FILE *out) ;
int _default_storage_dump_message_lines(struct pop3_message *message, unsigned long int max_lines, FILE *out) ;
int _default_storage_lock_mailbox(char const *mailboxname) ;
unsigned long int _default_storage_message_count() ;
unsigned long int _default_storage_message_sum() ;
struct pop3_message *_default_storage_first_message() ;
struct pop3_message *_default_storage_message_number(unsigned long int index) ;
int _default_storage_synch() ;
