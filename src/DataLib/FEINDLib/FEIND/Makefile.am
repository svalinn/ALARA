#$Id: Makefile.am,v 1.1 2007-04-18 14:12:07 wilsonp Exp $
AM_CPPFLAGS = -I$(top_srcdir)/src

noinst_LIBRARIES = libFEIND.a

libFEIND_a_SOURCES = FEIND.cpp  Parent.cpp  RamLib.cpp  XSec.cpp \
	Cinder.cpp  DecayEndf6.cpp  Eaf41.cpp  EndfIeaf.cpp  \
	exception/Exception.cpp    exception/ExEmptyXSec.cpp  exception/ExFormat.cpp \
	exception/ExXsecSize.cpp   exception/ExDecayMode.cpp  exception/ExFileOpen.cpp \
	exception/ExInvalidOption.cpp

noinst_HEADERS = Consts.h  FeindNs.h  Parent.h  RamLib.h  XSec.h ClassDec.h \
	Elements.h  FEIND.h     LibDefine.h  Parser.h  Typedefs.h \
	Cinder.h     DecayEndf6.h  Eaf41.h     EndfIeaf.h \
	exception/Exception.h    exception/ExEmptyXSec.h  exception/ExFormat.h   exception/ExInvalidOption.h \
	exception/ExDecayMode.h  exception/ExFileOpen.h   exception/ExInclude.h  exception/ExXsecSize.h
