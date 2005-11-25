AC_INIT(mpop3, 1.0)
AC_ARG_WITH(openssl,
	[	--with-openssl	Use openssl md5/rand functions],
	[
		if test "x$withval" != "xno" ; then
			AC_DEFINE(USE_OPENSSL,1)
		fi
	]
)
AC_CHECK_LIB('crypto','EVP_DigestInit', [], [AC_DEFINE(USE_OPENSSL,0)])
AC_CONFIG_FILES([Makefile defaults.mk module.mk])
AC_OUTPUT
