#$Id: Makefile.am,v 1.3 2003-10-22 06:07:07 wilsonp Exp $


AM_CPPFLAGS = -I$(top_srcdir)/src

### define processes required for building & installing tools

bin_PROGRAMS = dant2alara 
bin_SCRIPTS = extract_pathways summary

dant2alara_SOURCES = dant2alara.F

#### Exclude F90 for now since I can't get automake/autoconf to work
# dant2alara_SOURCES = dant2alara.F90

#.F90.o:
#	f90 .....
