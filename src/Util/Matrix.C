#include "Matrix.h"


Matrix& Matrix::operator*=(const Matrix& B)
{
  Matrix result = *this*B;
  
  *this = result;
  
  return *this;
}

Matrix Matrix::operator*(const Matrix& B)
{

  if (size == 0)
    return B;
  
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

/* raise a matrix to a power */
Matrix Matrix::operator^(int power)
{
  /* initialize matrices */
  Matrix answer = Matrix::Identity(size);
  Matrix accumulator = *this;

  /* while the exponent counter is still > 0 */
  while (power != 0)
    {
      /* if exponent is odd */
      if (power%2 == 1)
	answer = answer*accumulator;

      /* time saver */
      if (power > 1)
	accumulator = accumulator*accumulator;

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
      memCheck(data,"Matrix::operator=(...): data");
      
      for (int idx=0;idx<size*(size+1)/2;idx++)
	data[idx] = m.data[idx];
    }

  return *this;

}


Matrix Matrix::Identity(int sz)
{
  Matrix result(sz);

  while (sz-->0)
    result.data[sz*(sz+3)/2] = 1;

  return result;
}
      
Matrix Matrix::Triangle(double *d, int sz)
{
  Matrix result(sz);
  int row,col,idx;
  
  row = 0;
  col = 0;
  for (idx=0;idx<sz*(sz+1)/2;idx++)
    {
      result.data[idx] = d[col++];
      if (col > row)
	{
	  row++;
	  col=0;
	}
    }

  return result;
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

Matrix::Matrix(const Matrix& m)
{
  size = m.size;
  data = NULL;
  if (size>0)
    {
      data = new double[size*(size+1)/2];
      memCheck(data,"Matrix::Matrix(...) copy constructor: data");
      
      for (int idx=0;idx<size*(size+1)/2;idx++)
	data[idx] = m.data[idx];
    }

}

Matrix::Matrix(int siz)
{
  size = siz;
  data = NULL;

  if (size>0)
    {
      data = new double[size*(size+1)/2];
      memCheck(data,"Matrix::Matrix(...) constructor: data");

      for (siz=0;siz<size*(size+1)/2;siz++)
	data[siz] = 0;
    }

}


Matrix::~Matrix()
{
  delete data;
  data = NULL;
}
