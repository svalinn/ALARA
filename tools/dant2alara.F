      program dant2alara


      implicit none

      integer maxInts, maxGrps

      parameter (maxInts = 10000, maxGrps=1000)

      character(6) junkString
      integer junkInt

      integer charpos

      integer ndim, ngroup, ninti, nintj, nintk, iter, nblock
      real    effk, power

      character(64) fluxFileName, outFluxFileName
      integer firstGroup, lastGroup

      integer bNum, grp, lGrp, uGrp
      integer int, nint, iint, jint, kint, lj, uj

      double precision flux(maxInts,maxGrps), fluxTmp(maxInts,maxGrps)

! read filename

 100  write(6,8500)
      read(5,*) fluxFileName

! open file (testing for existence)
      open(unit=2,file=fluxFileName,status='old',form='unformatted',
     &   err=9000)

! read header
      read(2) junkString, junkString, junkString, junkInt
      read(2) ndim, ngroup, ninti, nintj, nintk, iter, effk, power, 
     &        nblock

      nint = ninti*nintj*nintk

! write header info 
 110  write(6,8000) ngroup,ninti,nintj,nintk,nint,ndim

! exclude multi-d (for now)
      if (ndim > 2) then
         write(6,8701)
         close(2)
         goto 100
      end if

! prompt for first group
      write(6,8501)
      read(5,*) firstGroup

! test validity of input
      if (firstGroup < 1 .OR. firstGroup > ngroup) then
         write(6,8700)
         goto 110
      end if

! prompt for last group
 120  write(6,8502)
      write(6,8503)
      read(5,*) lastGroup

! accept special meaning of negative number
      if (lastGroup < 0) then
         lastGroup = -lastGroup
      else
         lastGroup = firstGroup + lastGroup -1
      end if

! test validitiy of input
      if (lastGroup < firstGroup .OR.lastGroup > ngroup) then
         write(6,8700)
         goto 120
      end if

! prompt for output filename
 130  write(6,8504)
      read(5,*) outFluxFileName

! open output filename (testing for overwrite)
      open(unit=3,file=outFluxFileName,status='new',err=9001)

! write task to user
      write(6,8001) firstGroup, lastGroup, outFluxFileName


      if (ndim == 1) then
         ! 1-D data
         do bNum = 1,nblock
            lGrp = (bNum-1) * ((ngroup-1)/nblock + 1) + 1
            uGrp =   bNum   * ((ngroup-1)/nblock + 1)
            uGrp = min(ngroup, uGrp)
            read(2) ((flux(int,grp), int=1,ninti), grp=lGrp,uGrp)
         end do
      else
         ! 2-D data
         do grp=1,ngroup
            int = 0
            do kint = 1,nintk
               do bNum = 1,nblock
                  lj = (bNum-1) * ((nintj-1) / nblock + 1) + 1
                  uj =   bNum   * ((nintj-1) / nblock + 1)
                  uj = min(nintj, uj)
                  read(2) ((fluxTmp(iint,grp),iint=1,ninti),jint=lj,uj)

                  do jint=lj,uj
                     do iint=1,ninti
                        int = int+1
                        flux(int,grp) = fluxTmp(iint,jint)
                     end do
                  end do
               end do
            end do

            if (int /= nint) then
               write(6,9003) int, nint
               stop
            end if
         end do

      end if

      close(2)

      do int=1,nint
         write(3,*)
         write(3,7500) (flux(int,grp),grp=firstGroup,lastGroup)
      end do
         
      close(3)

      stop

! data output
 7500 format(6(1pe13.5))

! information output
 8000 format('This file has ',I4,' groups in (',I4,'x',I4,'x',I4,')=',
     &  I4, ' intervals and ',I4,' dimensions.')
 8001 format('Exporting groups ',I4,' through ',I4,' to file: ',A)

! prompt output
 8500 format('Enter the rtflux/atflux filename: ')
 8501 format('Enter the first group number to export: ')
 8502 format('Enter the number of groups to export, OR')
 8503 format('   enter the last group as a negative number: ')
 8504 format('Enter the desired output filename: ')
 
! warning output
 8700 format('That group is not in the correct range.')
 8701 format('This tool currently only works for 1-D and 2-D data.')
 9000 write(6,*) 'No file with name ', fluxFileName, ' exists.'
      goto 100
 9001 write(6,*) 'The file ',outFluxFileName,' already exists.'
      goto 130
 9003 format('Found wrong number of intervals: ',I4,'. Shold be ',
     &  I4,'.')


      end
