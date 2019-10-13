#### These things will be included in all necessary Makefile.am files
#### in this hierarchy

# where to put documentation, data and sample input/output
docdir = $(prefix)/doc
ugdir = $(docdir)/usersguide
nonxsdir = $(prefix)/data
xsdir = $(prefix)/data
sampledir = $(prefix)/sample
sampledatadir = $(prefix)/sample/data

AM_CPPFLAGS = -I$(top_srcdir)/src
