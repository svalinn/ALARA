! ===========================
! This lightweight NJOY wrapper accesses the NJOY ENDF module's utility
! function endf.terpa() to make use of NJOY's built-in interpolation 
! functionality to apply the appropriate interpolation schemes. The
! designation for these schemes are encoded within the TAB1 record header 
! variable INT, representing one of the following interpolation schemes, as
! laid out in the ENDF-6 manual
! (https://www.nndc.bnl.gov/endfdocs/ENDF-102-2023.pdf), under section 0.5.2
! ("Interpolation Laws"):
!
!     INT   | Interpolation Scheme
!     1     | y is constant in x (constant, histogram)
!     2     | y is linear in x (linear-linear)
!     3     | y is linear in ln(x) (linear-log)
!     4     | ln(y) is linear in x (log-linear)
!     5     | ln(y) is linear in ln(x) (log-log)
!     6     | special one-dimensional interpolation law, used for charged-
!           | particle cross sections only
!     11-25 | method of corresponding points (follow interpolation laws 1-5)
!     21-25 | unit base interpolation (follow interpolation laws of 1-5)
!
! This module's incorporation and usage within ALARAJOYWrapper is managed by
! njoy_tools.import_njoy_endf_wrapper(), which conditionally compiles this
! Fortran file to an executable using numpy.f2py (if such an executable has
! not already been created) and importing njoy_endf_wrapper as a Python
! package. This allows the subroutine interpolate_tab1() to be callable within
! ALARAJOYWrapper, which is necessary for the construction of pathway-specific
! reaction cross-section from MF9 multiplicities multiplied by MF3 cumulative
! cross-sections (see xs_plotting.extract_continuous_data() for specific use-
! case implementation).
! ===========================

module njoy_endf_wrapper

   use endf
   implicit none

contains

   subroutine interpolate_tab1(tab1, x, y)

      real(kind=8), intent(in) :: tab1(:)
      real(kind=8), intent(in) :: x(:)
      real(kind=8), intent(out) :: y(size(x))

      integer :: i
      integer :: ip, ir, idis
      real(kind=8) :: xnext

      ip = 2
      ir = 1

      do i = 1, size(x)
         call terpa(y(i), x(i), xnext, idis, tab1, ip, ir)
      end do

   end subroutine interpolate_tab1

end module njoy_endf_wrapper