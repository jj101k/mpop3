AC_INIT(mpop3, 1.0)
AC_ARG_WITH(openssl,
	[	--with-openssl	Use openssl md5/rand functions],
	[
		if test "x$withval" != "xno" ; then
			AC_DEFINE(USE_OPENSSL,1)
			WITH_OPENSSL=1
		fi
	]
)
AC_ARG_WITH(rendezvous,
	[	--with-rendezvous	Use rendezvous (apple zeroconf)],
	[
		if test "x$withval" != "xno" ; then
			AC_DEFINE(USE_RENDEZVOUS,1)
			WITH_RENDEZVOUS=1
		fi
	]
)
AC_CHECK_LIB(crypto,EVP_DigestInit, [
	if test "x$WITH_OPENSSL" == "x1" ; then
		LIBS="$LIBS -lcrypto"
	fi
], [AC_DEFINE(USE_OPENSSL,0)])
AC_CHECK_HEADERS([dns_sd.h], [], [AC_DEFINE(USE_RENDEZVOUS,0)])
AC_PROG_LEX
AC_PROG_CC
AC_CONFIG_FILES([Makefile defaults.mk module.mk])
AC_OUTPUT
