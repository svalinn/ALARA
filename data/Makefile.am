#$Id: Makefile.am,v 1.7 2003-10-29 09:05:32 wilsonp Exp $

include $(top_srcdir)/Makefile.h


# fill in final data files here
dist_nonxs_DATA = ANS6_4_3.txt Data_list.txt elelib.std FetterC_hi.Ci_m3.wdr	\
          FetterC_lo.Ci_m3.wdr IAEA.clearance.Bq_kg matlib.sample	\
          NRCA.Ci_m3.wdr NRCC.Ci_m3.wdr

# create symbolic links to easily remebered filenames
install-data-hook:
	cd $(nonxsdir); \
	rm -f ANS6.4.3; ln -sf ANS6_4_3.txt ANS6.4.3; \
	rm -f FetterHi; ln -sf FetterC_hi.Ci_m3.wdr FetterHi; \
	rm -f FetterLo; ln -sf FetterC_lo.Ci_m3.wdr FetterLo; \
	rm -f IAEA.clearance; ln -sf IAEA.clearance.Bq_kg IAEA.clearance; \
	rm -f NRCA; ln -sf NRCA.Ci_m3.wdr NRCA; \
	rm -f NRCC; ln -sf NRCC.Ci_m3.wdr NRCC

install-exec-hook:
	mkdirhier $(xsdir)
	cd $(xsdir); \
	touch PlaceCrossSectionDataHere
