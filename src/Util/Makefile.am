#$Id: Makefile.am,v 1.7 2003-10-28 22:11:39 wilsonp Exp $

include $(top_srcdir)/Makefile.h

noinst_LIBRARIES = libUtil.a

libUtil_a_SOURCES = Matrix.C  Statistics.C  debug.C  input.C  math.C  output.C  stub.C

noinst_HEADERS = Matrix.h  Statistics.h  classes.h  debug.h  functions.h  input_tokens.h dflt_datadir.h

# Set the installation data directory
DISTCLEANFILES = dflt_datadir.h
dflt_datadir.h: Makefile
	rm -f $@
	echo '#define DFLT_DATADIR "$(nonxsdir)"' > $@
	echo '#define DFLT_XSDIR   "$(xsdir)"' >> $@

