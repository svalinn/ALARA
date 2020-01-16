#$Id: Makefile.am,v 1.8 2007-02-28 23:24:51 phruksar Exp $

include $(top_srcdir)/Makefile.h

bin_PROGRAMS = alara

alara_SOURCES = alara.C alara.h PulseHistory.C RateCache.C VolFlux.C	\
        calcSchedule.C calcScheduleT.C topSchedule.C topScheduleT.C 	\
	Chain.C  MixCompRef.C  Node.C  NuclearData.C  Root.C  		\
	TreeInfo.C Component.C Dimension.C Geometry.C Input.C		\
        Mixture.C PulseLevel.C ScheduleItem.C Zone.C CoolingTime.C      \
        Flux.C History.C Loading.C Norm.C Schedule.C Volume.C           \
        istreamStack.C GammaSrc.C  OutputFormat.C  Result.C Matrix.C	\
	Statistics.C  debug.C  input.C  math.C  output.C  stub.C 	\
	#rt2al.F

LDADD = DataLib/libDataLib.a \
	DataLib/ASCIILib/EAFLib/libEAFLib.a DataLib/ASCIILib/IEAFLib/libIEAFLib.a 	\
	DataLib/ASCIILib/libASCIILib.a DataLib/ALARALib/libALARALib.a 		\
	DataLib/FEINDLib/libFEINDLib.a DataLib/FEINDLib/FEIND/libFEIND.a

SUBDIRS =  DataLib

noinst_HEADERS = PulseHistory.h RateCache.h VolFlux.h calcSchedule.h    \
        calcScheduleT.h topSchedule.h topScheduleT.h Chain.h  Node.h	\
	NuclearData.h  Root.h  TreeInfo.h  truncate.h Component.h 	\
	Dimension.h Geometry.h Input.h Loading.h Norm.h Schedule.h 	\
	Volume.h CoolingTime.h Flux.h History.h Input_def.h Mixture.h 	\
	PulseLevel.h ScheduleItem.h Zone.h GammaSrc.h  OutputFormat.h  	\
	Output_def.h  Result.h Matrix.h  Statistics.h  classes.h  	\
	debug.h  functions.h  input_tokens.h dflt_datadir.h

# Set the installation data directory
DISTCLEANFILES = dflt_datadir.h
BUILT_SOURCES = dflt_datadir.h
dflt_datadir.h: Makefile
	rm -f $@
	echo '#define DFLT_DATADIR "$(nonxsdir)"' > $@
	echo '#define DFLT_XSDIR   "$(xsdir)"' >> $@
