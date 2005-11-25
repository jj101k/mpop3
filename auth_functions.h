/*
 * char const *(*_auth_attempt_login)(char const *username, char const *password)
 *
 * If login succeeds, returns the mailbox name. This would be used by USER/PASS, and is suitable for systems without verbatim
 * stored passwords (eg most system logins).
 */
extern char const *(*_auth_attempt_login)(char const *, char const *);

/*
 * char const *(*_auth_timestamp)()
 * char const *(*_auth_password)(char const *username)
 * char const *(*_auth_login)(char const *username)
 *
 * These are the APOP counterparts of the above. APOP support will not be provided at all if (*_auth_timestamp)() returns NULL.
 * (*_auth_password)() is intended to return the user's password (for hashing); _auth_login() does the same as _auth_attempt_login()
 * except without verifying the password, and it'd be run only *after* the login has been verified.
 */
extern char const *(*_auth_timestamp)();
extern char const *(*_auth_password)(char const *);
extern char const *(*_auth_login)(char const *);

/*
 * int (*_auth_login_delay_needed)(char const *)
 * int (*_auth_default_login_delay)()
 *
 * These two perform complementary, but not directly related, functions. The first is run just before actually opening a mail spool
 * to see if your login throttling (if you have it) intends to deny the user access at this point. The second is run in response
 * to CAPA to find out if you're generally enforcing login throttling. You can have one without the other, but well-behaved email
 * clients should handle things better if they see both, or neither.
 */
extern int (*_auth_login_delay_needed)(char const *);
extern int (*_auth_default_login_delay)();
