#include "Matrix.h"


Matrix::Matrix(int siz)
{
  size = siz;
  data = NULL;

  if (size>0)
    {
      data = new double[size*(size+1)/2];

      for (siz=0;siz<size*(size+1)/2;siz++)
	data[siz] = 0;
    }

  /* all matrices are made as identiy matrices by default */
  siz = size;
  while (siz-->0)
    data[siz*(siz+3)/2] = 1;

}


Matrix::Matrix(const Matrix& m)
{
  size = m.size;
  data = NULL;

  if (size>0)
    {
      data = new double[size*(size+1)/2];
      
      for (int idx=0;idx<size*(size+1)/2;idx++)
	data[idx] = m.data[idx];
    }

}

/* make a triangular matrix from the array d,
 * where all elements are d[col] (regardless of row) */
Matrix::Matrix(double *d, int sz, int ecol)
{
  size = sz;
  data = NULL;

  int row,col,idx;

  if (size>0)
    {
      
      data = new double[size*(size+1)/2];
      row = 0;
      col = 0;
      for (idx=0;idx<sz*(sz+1)/2;idx++)
	{
	  data[idx] = d[ecol+(col++)];
	  if (col > row)
	    {
	      row++;
	      col=0;
	    }
	}
    }

}

Matrix::~Matrix()
{
  delete data;
  data = NULL;
}


Matrix& Matrix::operator*=(const Matrix& B)
{
  /* if the to matrices are equal, go straight
   * to square() */
  if (this == &B)
    square();

  /* if this matrix is empty, 
   * simply assign */
  else if (size == 0)
    *this = B;

  /* otherwise, if B is not zero, do math */
  else if (B.size > 0)
    {
      double *A_data = data;
      data = new double[size*(size+1)];
      
      int row=0,col=0,idx,term, idxA=0;
      
      for (idx=0;idx<size*(size+1)/2;idx++)
	{
	  if (col > row)
	    {
	      row++;
	      col=0;
	      idxA = idx;
	    }
	  data[idx] = 0;
	  for (term=col;term<=row;term++)
	    data[idx] += A_data[idxA+term]*B.data[term*(term+1)/2+col];
	  col++;
	}
      
      delete A_data;
    }

  return *this;
}

Matrix Matrix::operator*(const Matrix& B)
{
  if (size == 0)
    return B;
  else if (B.size == 0)
    return *this;

  Matrix result(size);
  int row=0,col=0,idx,term, idxA=0;

  for (idx=0;idx<size*(size+1)/2;idx++)
    {
      if (col > row)
	{
	  row++;
	  col=0;
	  idxA = idx;
	}
      result.data[idx] = 0;
      for (term=col;term<=row;term++)
	result.data[idx] += data[idxA+term]*B.data[term*(term+1)/2+col];
      col++;
    }
      
 return result;
}

void Matrix::square()
{
  if (size==0)
    return;

  /* save old data */
  double *old_data = data;

  /* allocate new data */
  data = new double[size*(size+1)];

  /* do multiplication */
  int row=0,col=0,idx,term, idxA=0;
  for (idx=0;idx<size*(size+1)/2;idx++)
    {
      if (col > row)
	{
	  row++;
	  col=0;
	  idxA = idx;
	}
      data[idx] = 0;
      for (term=col;term<=row;term++)
	data[idx] += old_data[idxA+term]*old_data[term*(term+1)/2+col];
      col++;
    }

  delete old_data;
}

/* raise a matrix to a power */
Matrix Matrix::operator^(int power)
{
  if (size == 0)
    return *this;

  /* initialize matrices */
  Matrix answer(size);
  Matrix accumulator(*this);

  /* while the exponent counter is still > 0 */
  while (power != 0)
    {
      /* if exponent is odd */
      if (power%2 == 1)
	answer *= accumulator;

      /* time saver */
      if (power > 1)
	accumulator.square();

      power = power/2;
    }

  return answer;
}

Matrix& Matrix::operator=(const Matrix& m)
{
  if (this == &m)
    return *this;

  size = m.size;

  delete data;
  data = NULL;

  if (size>0)
    {
      data = new double[size*(size+1)/2];
      
      for (int idx=0;idx<size*(size+1)/2;idx++)
	data[idx] = m.data[idx];
    }

  return *this;

}


double Matrix::rowSum(int row)
{
  double result = 0;
  int idx;

  if (row < size)
    for (idx=row*(row+1)/2;idx<(row+1)*(row+2)/2;idx++)
      result += data[idx];

  return result;
}

