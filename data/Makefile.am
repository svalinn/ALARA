#$Id: Makefile.am,v 1.5 2003-10-22 06:07:00 wilsonp Exp $

include $(top_srcdir)/Makefile.h


# fill in final data files here
xs_DATA = ANS6_4_3.txt Data_list.txt elelib.std FetterC_hi.Ci_m3.wdr	\
          FetterC_lo.Ci_m3.wdr IAEA.clearance.Bq_kg matlib.sample	\
          NRCA.Ci_m3.wdr NRCC.Ci_m3.wdr

# create symbolic links to easily remebered filenames
install-data-hook:
	cd $(xsdir); \
	ln -s ANS6_4_3.txt ANS6.4.3; \
	ln -s elelib.std elelib; \
	ln -s FetterC_hi.Ci_m3.wdr FetterHi; \
	ln -s FetterC_lo.Ci_m3.wdr FetterLo; \
	ln -s IAEA.clearance.Bq_kg IAEA.clearance; \
	ln -s matlib.sample matlib; \
	ln -s NRCA.Ci_m3.wdr NRCA; \
	ln -s NRCC.Ci_m3.wdr NRCC

