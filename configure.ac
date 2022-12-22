AC_PREREQ(2.59)

AC_INIT([pnm2ppa], 1.13, [pnm2ppa-users@lists.sourceforge.net])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([ppa.c])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE([foreign no-exeext dist-bzip2])

CFLAGS="${CFLAGS} -Wall -Wsign-compare -Wfloat-equal -Wformat-security -g"

# Checks for programs.
AC_PROG_CC

AC_MSG_CHECKING([if cross compiling])
if test "x$cross_compiling" = "xno"; then
	CC_FOR_BUILD=${CC_FOR_BUILD-${CC-gcc}}
	AC_MSG_RESULT([no])
else
	CC_FOR_BUILD=${CC_FOR_BUILD-gcc}
	AC_MSG_RESULT([yes])
fi

AC_SUBST(CC_FOR_BUILD)

#
# do we need libm for 'sqrt' and 'pow'?
#
AC_SEARCH_LIBS(sqrt, m)
AC_SEARCH_LIBS(pow, m)

#
# other things we rely on
#
AC_CHECK_FUNCS([memmove] [memset] [strcasecmp] [strchr] [strdup])
AC_CHECK_HEADERS([sys/param.h])
AC_C_INLINE
AC_FUNC_MALLOC
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

#
# Some systems need a special library for 'getopt_long'
# (for example MacOS and FreeBSD).
#
AC_SEARCH_LIBS(getopt_long, [gnugetopt iberty])

#
# check if the syslog feature is present
# On some systems other than the libc is required to use this feature
#
AC_SEARCH_LIBS(syslog, [bsd socket inet])

# do we have a syslog feature?
if test "${ac_cv_search_syslog}" = "no"; then
	# disable its usage
	AC_DEFINE(__NO_SYSLOG__, 1, [no syslog])
else
	#
	# okay, 'syslog' is present. But should we use it?
	#
	AC_MSG_CHECKING([whether to enable syslog])
	AC_ARG_ENABLE(syslog,
		AS_HELP_STRING([--enable-syslog], [enable syslog @<:@default=yes@:>@]),
		[case "$enableval" in
			y | yes) CONFIG_SYSLOG=yes ;;
			*) CONFIG_SYSLOG=no ;;
		esac],
		[CONFIG_SYSLOG=yes])
	AC_MSG_RESULT([${CONFIG_SYSLOG}])

	if test "${CONFIG_SYSLOG}" = "yes"; then
		AC_DEFINE(USE_SYSLOG, 1 , [use the syslog feature])
	else
		AC_DEFINE(__NO_SYSLOG__, 1, [no syslog])
	fi
fi

#
# Some things have to be done in accordance to the operating system
#
AC_MSG_CHECKING([operating system])
case $host in
	*-linux*)
		AC_MSG_RESULT([Linux])
		;;
	*-solaris*)
		AC_MSG_RESULT([SOLARIS])
		AC_DEFINE(__NO_LOG_PERROR__, [], [absent])
		;;
	*-BeOS*)
		AC_MSG_RESULT([BeOS])
		# On BeOS the LDFLAGS should not be used.
		AM_LDFLAGS=""
		;;
	*-darwin*)
		AC_MSG_RESULT([Darwin/MacOS X])
		CFLAGS="${CFLAGS} -no-cpp-precomp -traditional-cpp"
		;;
	*-bsd*)
		AC_MSG_RESULT([FreeBSD])
		;;
	*)
		AC_MSG_RESULT([unsupported operating system, pnm2ppa may fail])
esac

#
# Debugging
#
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug], [enable debugging @<:@default=no@:>@]),
		[case "$enableval" in
		y | yes) CONFIG_DEBUG=yes ;;
		*) CONFIG_DEBUG=no ;;
	esac],
	[CONFIG_DEBUG=no])
AC_MSG_RESULT([${CONFIG_DEBUG}])
if test "${CONFIG_DEBUG}" = "yes"; then
	AC_DEFINE(DEBUG, 1, [debugging])
	AM_CONDITIONAL(PNM2PPA_DEBUG, [true])
	CFLAGS="${CFLAGS} -O1"
else
	CFLAGS="${CFLAGS} -O2 -DNDEBUG"
	AM_CONDITIONAL(PNM2PPA_DEBUG, [false])
fi

#
# define the compiled in language
#
AC_MSG_CHECKING([which language is to be used])
AC_ARG_WITH([language],
	AS_HELP_STRING([--with-language=EN | CZ | ES | FR | IT | NL],
	[Compiled in language]),
	[ppa_language="$withval"])

# english language is the default
if test -z "${ppa_language}"; then
	# default is english
	ppa_language=EN
fi

case "${ppa_language}" in
	EN | en)
		AC_DEFINE(LANG_EN, [], [english])
		used_language="english"
		;;
	CZ | en)
		AC_DEFINE(LANG_CZ, [], [cech])
		used_language="cech"
		;;
	ES | es)
		AC_DEFINE(LANG_ES, [], [espanol])
		used_language="espanol"
		;;
	FR | fr)
		AC_DEFINE(LANG_FR, [], [french])
		used_language="french"
		;;
	IT | it)
		AC_DEFINE(LANG_IT, [], [italian])
		used_language="italian"
		;;
	NL | nl)
		AC_DEFINE(LANG_NL, [], [dutch])
		used_language="dutch"
		;;
	*)
		AC_MSG_ERROR([Unsupported language])
		;;
esac

AC_MSG_RESULT([${used_language}])

AC_MSG_CHECKING([whether to build the 'parse_vlink' tool])
AC_ARG_ENABLE(vlink,
	AS_HELP_STRING([--enable-vlink], [enable building parse_vlink @<:@default=no@:>@]),
	[case "$enableval" in
		y | yes) CONFIG_VLINK=yes ;;
		*) CONFIG_VLINK=no ;;
	esac],
	[CONFIG_VLINK=no])
AC_MSG_RESULT([${CONFIG_VLINK}])

if test "${CONFIG_VLINK}" = "yes"; then
	AM_CONDITIONAL(BUILD_PARSE_VLINK, [true])
else
	AM_CONDITIONAL(BUILD_PARSE_VLINK, [false])
fi

AC_CONFIG_FILES(
	[Makefile]
	[ppa_protocol/Makefile]
)
AC_OUTPUT
