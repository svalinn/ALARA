#$Id: Makefile.am,v 1.5 2003-10-22 06:07:05 wilsonp Exp $

AM_CPPFLAGS = -I$(top_srcdir)/src

noinst_LIBRARIES = libInput.a

libInput_a_SOURCES = Component.C Dimension.C Geometry.C Input.C		\
	Mixture.C PulseLevel.C ScheduleItem.C Zone.C CoolingTime.C	\
	Flux.C History.C Loading.C Norm.C Schedule.C Volume.C		\
	istreamStack.C rt2al.F

noinst_HEADERS = Component.h Dimension.h Geometry.h Input.h Loading.h	\
	Norm.h Schedule.h Volume.h CoolingTime.h Flux.h History.h	\
	Input_def.h Mixture.h PulseLevel.h ScheduleItem.h Zone.h
