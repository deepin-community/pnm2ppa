#
# What is to be built
#
bin_PROGRAMS = \
	pnm2ppa \
	calibrate_ppa

# build the parse_vlink tool on demand only
if BUILD_PARSE_VLINK
SUBDIRS = ppa_protocol
endif

#
# Where to find the config files at runtime
#
AM_CPPFLAGS = -DSYSCONFDIR='"$(sysconfdir)"'

#
# Distribute these directories and files
#
EXTRA_DIST = \
	Changelog \
	README \
	README.security \
	pdq \
	lpd \
	sample_scripts \
	stamp-h1 \
	test.ps \
	testpage-a4.ps \
	testpage.ps \
	testpages \
	utils \
	rhs-printfilters \
	rpm \
	docs \
	make_hash_ink.c

#
# distribute and install the manpage
#
dist_man1_MANS = docs/en/pnm2ppa.1

MAINTAINERCLEANFILES = \
	configure \
	aclocal.m4 \
	config.h.in \
	Makefile.in

#
# distribute, but do not install
#
noinst_HEADERS = \
	global.h \
	image.h \
	defaults.h \
	pnmtools.h \
	debug.h \
	ppa.h \
	ppa_syslog.h \
	palette.h \
	fscoeff.h \
	lang.h \
	text-cz.h \
	text-en.h \
	text-es.h \
	text-fr.h \
	text-it.h \
	text-nl.h \
	gamma.h \
	dither.h \
	hash_ink.h \
	pnm2ppa.h \
	enh-curve.h

#
# Other things to install
#
dist_sysconf_DATA = \
	pnm2ppa.conf

#
# main filter program
#
pnm2ppa_SOURCES = \
	pnm2ppa.c \
	syslog.c \
	ppa.c \
	image.c \
	dither.c \
	cutswath.c \
	gamma.c

# part of pnm2ppa, but generated at build time
nodist_pnm2ppa_SOURCES = \
	hash_ink.c

# part of pnm2ppa, but only if DEBUG is enabled
if PNM2PPA_DEBUG
pnm2ppa_SOURCES += debug.c
endif

#
# calibration tool
#
calibrate_ppa_SOURCES = \
	calibrate_ppa.c \
	pnmtools.c \
	gamma.c

# create this file 'on the fly'
hash_ink.c: make_hash_ink
	./make_hash_ink 4 3 > hash_ink.c
	./make_hash_ink 1 5 >> hash_ink.c

CLEANFILES = hash_ink.c make_hash_ink

# ------------------- cross compile relevant stuff ---------------------------

#
# We need to create a tool to generate the hash table.
# This tool creates the 'hash_ink.c' source file (and must
# run on the build host in case of cross compiling)
#
CC_FOR_BUILD = @CC_FOR_BUILD@
CFLAGS_FOR_BUILD = -O2
CPPFLAGS_FOR_BUILD = -I$(top_srcdir)
LDFLAGS_FOR_BUILD =
LDLIBS_FOR_BUILD =

make_hash_ink: make_hash_ink.c
	$(CC_FOR_BUILD) $(CPPFLAGS_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) -o $@ $< $(LDLIBS_FOR_BUILD)
