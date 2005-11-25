AC_INIT(mpop3, 0.16)
AC_ARG_WITH(safe-user,
	AC_HELP_STRING([--with-safe-user=ARG], [SU to this user if you're root [[ARG=mail]]]),
	[
		AC_DEFINE_UNQUOTED(SAFE_USER, "$withval")
	],
	[
		AC_DEFINE(SAFE_USER,"mail")
	]
)
AC_LANG(C)
AC_RUN_IFELSE([
	AC_LANG_PROGRAM([[
#include <pwd.h>
]], [[ 
struct passwd *pwd=getpwnam(SAFE_USER);
if( !pwd || !(pwd->pw_uid) || !(pwd->pw_gid) ) return 1; 
]])
],
[],
[ AC_MSG_FAILURE([Bad fallback username, please use --with-safe-user=someuser]) ])
WITH_OPENSSL=1
AC_ARG_WITH(openssl,
	AC_HELP_STRING([--without-openssl],[Don't openssl md5/rand functions]),
	[
		if test "x$withval" == "xno" ; then
			WITH_OPENSSL=0
		fi
	]
)
AC_ARG_WITH(rendezvous,
	AC_HELP_STRING([--with-rendezvous],[Use rendezvous (apple zeroconf)]),
	[
		if test "x$withval" != "xno" ; then
			AC_DEFINE(USE_RENDEZVOUS,1)
			WITH_RENDEZVOUS=1
		fi
	]
)
AC_ARG_WITH(bonjour,
	AC_HELP_STRING([--with-bonjour],[Use rendezvous (apple zeroconf)]),
	[
		if test "x$withval" != "xno" ; then
			AC_DEFINE(USE_RENDEZVOUS,1)
			WITH_RENDEZVOUS=1
		fi
	]
)
AC_CHECK_LIB(crypto, EVP_DigestInit, [
	if test "x$WITH_OPENSSL" == "x1" ; then
		AC_DEFINE(USE_OPENSSL,1)
		LIBS="$LIBS -lcrypto"
	fi
], [
	AC_DEFINE(USE_OPENSSL,0)
])

AC_CHECK_FILE(/usr/lib/libc.dylib, [
	AC_SUBST(SHARED_EXT,".bundle")
	AC_SUBST(MAKE_SHARED,"-bundle")
], [
	AC_SUBST(SHARED_EXT,".so")
	AC_SUBST(MAKE_SHARED,"-shared")
])
extra_modules=""
AC_CHECK_HEADERS([dns_sd.h], [], [AC_DEFINE(USE_RENDEZVOUS,0)])
AC_CHECK_HEADERS([libpq-fe.h], [extra_modules="$extra_modules auth_postgresql"], [])
AC_SUBST(extra_modules)
AC_PROG_LEX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_CONFIG_FILES([Makefile defaults.mk module.mk])
AC_OUTPUT
