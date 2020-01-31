#$Id: Makefile.am,v 1.5 2006-01-17 22:55:49 phruksar Exp $

AM_CPPFLAGS = -I$(top_srcdir)/src

noinst_LIBRARIES = libDataLib.a

libDataLib_a_SOURCES = DataLib.C ADJLib.C  ALARALib.C  DaugItem.C  LibIdx.C 	\
	ParItem.C ASCIILib.C EAFLib.C IEAFLib.C FEINDLib.C FEIND.cpp  	\
	Parent.cpp  RamLib.cpp XSec.cpp Cinder.cpp  DecayEndf6.cpp  	\
	Eaf41.cpp  EndfIeaf.cpp Exception.cpp ExEmptyXSec.cpp  	\
	ExFormat.cpp ExXsecSize.cpp ExDecayMode.cpp  		\
	ExFileOpen.cpp ExInvalidOption.cpp


noinst_HEADERS = DataLib.h ADJLib.h  ALARALib.h  ALARALib_def.h ASCIILib.h	\
	EAFLib.h IEAFLib.h FEINDLib.h Consts.h FeindNs.h Parent.h RamLib.h	\
        XSec.h ClassDec.h Elements.h FEIND.h LibDefine.h Parser.h       \
        Typedefs.h Cinder.h DecayEndf6.h Eaf41.h EndfIeaf.h             \
        Exception.h    ExEmptyXSec.h  ExFormat.h   ExInvalidOption.h    \
        ExDecayMode.h  ExFileOpen.h   ExInclude.h  ExXsecSize.h


SUBDIRS = swig

