#### These things will be included in all necessary Makefile.am files
#### in this hierarchy

# where to put documentation, data and sample input/output
docdir = $(pkgdatadir)/doc
nonxsdir = $(pkgdatadir)/data
xsdir = $(pkglibdir)/$(OSTYPE)
sampledir = $(pkgdatadir)/sample
sampledatadir = $(pkgdatadir)/sample/data

AM_CPPFLAGS = -I$(top_srcdir)/src
