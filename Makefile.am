## Top level Makefile.am for ALARA project
AUTOMAKE_OPTIONS = gnu

include $(top_srcdir)/Makefile.h

EXTRA_DIST = Makefile.h
SUBDIRS = src tools doc data sample developer-info  # tests

dist-hook:
	mkdir $(distdir)/sample/dump_files


