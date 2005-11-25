#ifndef __POP3_H
#define __POP3_H
#include <stdio.h>
enum pop3_state {p3Dead, p3Authorisation, p3Transaction, p3Update, p3EndMarker};

#define BIT(a) (1<<(a))
#define ALL_POP3_STATES BIT(p3EndMarker)-1

enum pop3_state command_loop(FILE *ifp, FILE *ofp, enum pop3_state current_state);
int handle_connection(FILE *ifp, FILE *ofp);
int _send_misc(FILE *ofp, char *prefix, char *message, char *extended_code);

#define POP3_SUCCESS "+OK"
#define POP3_FAILURE "-ERR"

#define _send_ERR(ofp, message, extended_error) _send_misc(ofp, POP3_FAILURE, message, extended_error)
#define _send_OK(ofp, message) _send_misc(ofp, POP3_SUCCESS, message, NULL)

#define DEFAULT_PORT htons(110)

// POP3 extended error codes follow.
#define P3EXT_LOGIN_DELAY "LOGIN-DELAY"
#define P3EXT_IN_USE "IN-USE"

// rfc 1939
#define RFC_MAX_ARG_LENGTH 40
#define RFC_MAX_OUTPUT_LENGTH 512
#define RFC_MIN_INACTIVITY_TIMER 600
// rfc 2449
#define RFC_MAX_INPUT_LENGTH 255
#define RFC_MIN_ARG_LENGTH 1


#define DEFAULT_TIMEOUT RFC_MIN_INACTIVITY_TIMER+0

/*
 * Utility functions
 * =================
 */

/* 
 * super_chomp(string)
 * super_chomp_n(string, length)
 *
 * s/[\r\n]/\x00/ (Starting from the end and working *backwards*)
 *
 * This returns a character if [\r\n] was found.
 * MS-DOS (\r\n): \n
 * Olde-worlde Mac (\r): \r
 * UNIX (\n): \n
 *
 * The reason for this is so that you have some hope of detecting \r<line length exceeded>\n
 *
 * This is only intended to trim off a trailing \r, \r\n or \n left by (eg) fgets(). If there are such characters in the middle,
 * then you're done for anyway...
 *
 */
char super_chomp(char *);
char super_chomp_n(char *, unsigned long int);


/*
 * _pop3_fprintf(fp, format, ...)
 *
 * Wraps around fprintf, with the exceptions that:
 * - it guarantees no more than 512 characters will be written
 * - it guarantees that it will end with \r\n
 *
 */
int _pop3_fprintf(FILE *out, char const *format, ...);

/*
 * hex_from_binary(out_string, in_string, bytes)
 *
 * Turns 'bytes' bytes of 'in_string' into 'bytes'*2 hex characters in 'out_string', followed by a "\0" character.
 *
 */
int hex_from_binary(char *outstring, char const *instring, unsigned long bytes);
#endif
