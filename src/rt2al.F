c $Id: rt2al.F,v 1.1 2002-08-28 20:23:21 fateneja Exp $
      subroutine rt2al(AfluxMat,AnumInt,AnumGrps,Askip,Afname,
     *                 Afnamelength,Aerr)
c Arguments:
c   AfluxMat: Matrix to store flux values for ALARA
c   AnumInt: Number of intervals to store
c   AnumGrps: Number of groups to store
c   Askip: Number of intervals to skip
c   Afname: Name of binary file to be read      character 
c   Afnamelength: Length of the string Afname
c   Aerr: Stores an error code

c Error Codes:
c   0: No Errors
c   1: Flux data not 1d
c   2: not enough data in binary flux file

      implicit none

      integer ntotal
      parameter (ntotal = 1000*1000)

      integer i,j,ii,jj,kk,jl,jup,ju,m,nin
      integer ndim, ngrp, ninti, nintj, nintk, iter, nblok, iver
      real effk, power

c  for 1_D flux file

      integer AnumGrps,AnumInt

      double precision  freg(1000,300), AfluxMat(AnumGrps,AnumInt)

      character hname*6, huse(2)*6, aint*3

      integer Askip, Afnamelength, Aerr
      character*256 Afname
      character*256 filename
      filename(1:Afnamelength) = Afname

      Aerr = 0

c     Open input file

      open(unit=2,file=filename,status='old',form='unformatted')

c  read file identification card

      read(2) hname, (huse(2), i=1,2), iver

c  read specifications

      read(2)    ndim, ngrp, ninti, nintj, nintk, iter, effk,
     *             power, nblok

c     Make sure there are enough groups and Intervals

      if (AnumInt+Askip .gt. ninti) then
         Aerr = 2
         goto 999
      endif

      if (ngrp-42 .lt. AnumGrps) then
         Aerr = 2
         goto 999
      endif

      if (ndim .eq. 1) then

c          processing 1-D flux file ......

           do 10 m=1,nblok
               jl = (m-1) * ((AnumGrps-1) / nblok + 1) + 1
               jup = m * ((AnumGrps-1) / nblok + 1)
               ju = min( AnumGrps , jup )
               read(2) ((freg(i,j),i=1,ninti),j=jl,ju)
   10      continue

c          copy data into matrix for ALARA

           do 32 ii=1,AnumGrps
              do 32 jj=1,AnumInt
                 AfluxMat(ii,jj) = freg(jj+Askip,ii)
   32      continue

      else
         Aerr = 1
  
      endif

  999 continue
      close( unit=nin, status='keep' )

      return
      end
