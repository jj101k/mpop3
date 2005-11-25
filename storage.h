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

enum arrayStyle {asArray, asLinkedList};

#endif
