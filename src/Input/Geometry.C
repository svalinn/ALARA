/* (Potential) File sections:
 * Service: constructors, destructors
 * Input: functions directly related to input of data 
 * xCheck: functions directly related to cross-checking the input
 *         against itself for consistency and completeness
 * Preproc: functions directly related to preprocessing of input
 *          prior to solution 
 * Solution: functions directly related to the solution of a (sub)problem
 * Utility: advanced member access such as searching and counting 
 */


#include "Geometry.h"
#include "Dimension.h"

/***************************
 ********* Service *********
 **************************/

Geometry::Geometry(char *token)
{

  rMin = -1;
  rMaj = -1;

  switch (tolower(token[0]))
    {
    case 'p':
      type = GEOM_P;
      break;
    case 'r':
      type = GEOM_R;
      break;
    case 'c':
      type = GEOM_C;
      break;
    case 's':
      type = GEOM_S;
      break;
    case 't':
      type = GEOM_T;
      break;
    default:
      error(150,"Invalid geometry type: %s",token);
    }      

  verbose(2,"Set geometry type: %s.", token);

}

/***************************
 ********* xCheck **********
 **************************/


/* cross-check torus geometries for major and minor radii */
/* called by Input::xCheck(...) */
void Geometry::checkTorus(Dimension *dimListHead)
{

  if (rMaj < 0)
    error(350,"Toroidal problems with zone dimensions require a major radius.");
  else
    verbose(2,"Found major radius for torroidal geometry.");
  
  if(!dimListHead->find(DIM_R) && rMin < 0)
    error(351,"Toroidal problems with zone dimensions require either a minor radius or a radius dimension.");
  else
    verbose(2,"Found minor radius for torroidal geometry.");
}
    

