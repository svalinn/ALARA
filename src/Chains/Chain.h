/* $Id: Chain.h,v 1.4 1999-08-24 22:06:14 wilson Exp $ */
#include "alara.h"

/* ******* Class Description ************

This class stores the information about a particular chain as the
chains are being created and solved.  Normally, a single chain object
will exist for each root isotope, exisiting only while that root is
being solved.

 *** Static Class Members ***

 truncLimit : double
    The tolerance limited to choose when to truncate chains.

 ignoreLimit : double
    The tolerance limited to choose when to ignore sub-trees.

 *** Class Members *** 

 chainLength : int
    A counter which simply tracks the length of the current chain.  By
    convention, it will always be 1 greater than the rank of the
    bottom isotope in the chain.

 newRank : int
    This indicates the rank of the first isotope which is different
    from the last chain.  This is important when manipulating the
    transfer and decay matrices; since they are lower triangular
    matrices, only the new elements, i.e. those corresponding to new
    isotopes, need to be changed.

 setRank : int
    This indicates the rank of the first isotope to be tallied in this
    chain.  A given isotope is tallied only immediately before it is
    removed from the chain.  Therefore, this isotope and all
    successive isotopes in the chain must currently be processing
    their last daughter isotope.  This is the condition which
    establishes which isotope is indicated by 'setRank'.

 solvingRef : int
    Boolean indicates whether or not we are solving a
    reference/truncation calculation or a full solution.

 loopRank : int*
    If an isotope is involved in a loop, the last iteration of the
    loop need not be treated with loop solutions.  This array tracks
    the beginning of the last loop iteration for each isotope.  In
    many (most?) situations this will be set to the default value, -1,
    indicating that this isotope is not involved in any loops.

 rates : double**
    A list of pointers to the various rate vectors related to the
    chain.  For each isotope, there is a pointer to the production
    rate vector and the total destruction rate vector for both
    transmutation and pure decay.  These are later collapsed with the
    fluxes as required.

 colRates : double * 
    An array of scalar rates, calculated by collapsing the rate
    vectors pointed to by 'rates' with the fluxes of a given interval.
    For each isotope in a chain and each flux in a problem, there is a
    scalar rate for production and destruction in both transmutation
    and pure decay.  A single 1-D array is used to store 3-D
    information to minimise memory allocation activity.  The
    organization is as follows:
       Block P[chainLength * numFluxes] : transmutation production rates
       Block d[chainLength * numFluxes] : transmutation destruction rates
       Block L[chainLength] : decay production rates
       Block l[chainLength] : decay destruction rates
    For Block's P and d, the rates are sorted in numFluxes segments,
    each of lengh chainLength.

 P, d, L, l : double *
    These pointers simply point into the colRates vector at the
    beginning of the blocks defined above to give 4 apparent rate
    arrays.  This is a simple convenience measure.

 root : Root*
    This points to the root isotope of this chain.  It is used
    primarily for knowing where to tally the results of the
    calculation.

 node : Node*
    This points to the last isotope in the chain.  This last node is
    the object through which the chain growth and truncation functions
    are accessed.

 reference : Volume*
    This is a reference interval which stores the maximum group-wise
    flux for truncation calculations.

 maxChainLength : int
    This simply indcates the current length of the various arrays and
    vectors.  To avoid reallocatin space with each change in the
    chain's length, these vectors are doubled and halved when
    appropriate.  When the vector is too small, it is doubled, and
    when the vector is larger than the default initial value
    INITMAXCHAINLENGTH and less than 1/4 full, it is halved. (see
    resizeRates())

 *** Protected Member Functions ***

 * - Chain - *

 void setState(topSchedule*)
    This function establishes the truncation state following the
    extension of a chain.  It has the primary responsibility for
    setting the initial truncation state.

 * - List - *

 void expandRates()
    This function performs the doubling of the rate vectors discussed
    in the description of 'maxChainLength'.

 void compressRates()
    This function performs the halving of the rate vectors discussed
    in the description of 'maxChainLength'.

 void resizeRates()
    This function determines whether to double, halve or leave the
    rates vectors following the extension or truncation of the chain.

 *** Static Public Member Functions ***

 * - Input - *

 void getTruncInfo(istream&)
    This function reads the truncation tolerance information from the
    input file attached to the stream reference passed in the first
    argument.

 *** Public Member Functions ***

 * - Constructors & Destructors - *

 Chain(Root*,topSchedule*);
    Default constructor establishes a chain with 'maxChainLength'
    equal to the default, INITMAXCHAINLENGTH, and the corresponding
    storage for 'loopRank' and 'rates'.  When called with no
    arguments, all other members are set to 0 or NULL.  Otherwise, the
    'root' and 'node' pointers are set to the first argument.

 Chain(const Chain&)
    The copy constructor copies all the scalar members directly and
    copies the vectors on an element-by-element basis.  The sizes of
    the vectors are (obviously?) based on the copied 'maxChainLength'
    member.

 ~Chain()
    Destructor deletes all the vector storage associated with this
    class and the reference volume.

 Chain operator=(const Chain&)
    The correct implementation of this operator must ensure that
    previously allocated space is returned to the free store before
    allocating new space into which to copy the object.

 * - Chain - *

 int build()
    This recursive function hold the primary responsibility for
    managing the growth and pruning of the tree/chains.  For each
    newly added node, it does some initialization and sets up the rate
    pointers.  It then queries the truncation state, and acts
    accordingly.  For example, when continuing the chain, it calls on
    the 'node' to add a daughter, and calls itself recursively.

 * - Solution - *

 void setupColRates()
    This function initializes the 'colRates' vector and sets the
    non-flux dependent values.

 void collapseRates(VolFlux*)
    This function folds the vector rates pointed to by the 'rates'
    vector with the fluxes contained in the VolFlux list and fills
    'colRates' with the scalar results.

 void fillTMat(Matrix&, double, int)
    This function is the workhorse of the solution phase.  It fills
    the necessary elements of a transfer matrix by adaptively choosing
    the appropriate method: Bateman, Laplace inversion or Laplace
    expansion.  The matrix referenced in the first argument is filled
    using the irradiation time specified in the second argument and
    the flux number specified in the third argument.  *** This is a
    member of class Chain to take advantage of the chain parameters
    such as 'chainLength', 'newRank', and 'colRates'.  This minimizes
    extra computation by knowing which parts of the resultant matrix
    have already been calcualted and which must be
    newly(re?)-calculated.  Be sure that the matrix referenced in the
    first argument is consistent with this. ***

 void mult(Matrix&, Matrix, Matrix) 
    This function multiplies the two lower triangular matrices
    specified in the second and third arguments and assigns the result
    to the first argument.  *** This is a member of class Chain to
    take advantage of the chain parameters such as 'chainLength' and
    'newRank'. This minimizes extra computation by knowing which parts
    of the resultant matrix have already been calcualted and which
    must be newly(re?)-calculated.  Be sure that the matrix referenced
    in the first argument is consistent with this. ***

 void setDecay(Matrix&,double)
    This function fills the necessary elements of a decay matrix,
    referenced by the first argument, using the decay time of the
    second argument.  It always calls a Bateman method since pure
    decay cannot have loops.  *** This is a member of class Chain to
    take advantage of the chain parameters such as 'chainLength',
    'newRank', and 'colRates'.  This minimizes extra computation by
    knowing which parts of the resultant matrix have already been
    calcualted and which must be newly(re?)-calculated.  Be sure that
    the matrix referenced in the first argument is consistent with
    this. ***

 * - Utility - *

 int getRoot()
    Calls interface routine of object pointed to by 'root' to get, and
    return, its KZA value
 
 int getKza(int)
    Function steps through chain to rank passed in argument and calls
    the interface function of that Node object to get, and return, its
    KZA value.  If the specified rank is greater than the
    'chainLength', it will return 0.

 int getSetRank()
    Inline function provides access to the 'setRank' variable.  */


#ifndef _CHAIN_H 
#define _CHAIN_H

#define INITMAXCHAINLENGTH 25

class Chain
{
protected:
  static double truncLimit;
  static double ignoreLimit;
  static int mode;

  int chainLength, newRank, setRank, solvingRef;
  int *loopRank;
  double **rates;
  double *colRates, *P, *d, *L, *l;
  Root *root;
  Node *node;
  Volume *reference;

  int maxChainLength;

  /* Chain */
  void setState(topSchedule*);
  
  /* List */
  void expandRates();
  void compressRates();
  void resizeRates();

public:
  /* Input */
  static void getTruncInfo(istream&);
  static void modeReverse();

  /* Service */
  Chain(Root *newRoot=NULL,topSchedule* top=NULL);
  Chain(const Chain&);
  ~Chain();

  Chain& operator=(const Chain&);

  /* Chain */
  int build(topSchedule*);

  /* Solution */
  void setupColRates();
  void collapseRates(VolFlux*);
  void fillTMat(Matrix&, double, int);
  void mult(Matrix&, Matrix&, Matrix&);
  void setDecay(Matrix&,double);

  /* Utility */
  int getRoot();
  int getKza(int);
  int getSetRank() { return setRank; };
  int getChainLength() { return chainLength; };
};

#endif
