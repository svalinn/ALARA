#include "alara.h"

#ifndef _MATRIX_H
#define _MATRIX_H

#include "Chains/Chain.h"

class Matrix
{
  friend void Chain::fillTMat(Matrix&, double, int);
  friend void Chain::setDecay(Matrix&, double);
  friend void Chain::mult(Matrix&, Matrix&, Matrix&);

protected:
  int size;
  double *data;

  void square();

public:
  Matrix() { size = 0; data = NULL; };
  Matrix(int);
  Matrix(const Matrix&);
  Matrix(double*,int);
  ~Matrix();

  Matrix operator*(const Matrix&);
  Matrix& operator*=(const Matrix&);
  Matrix operator^(int);
  Matrix& operator=(const Matrix&);
  double& operator[](int idx) { return data[idx]; };

  int getSize() { return size; };

  double rowSum(int);

};

#endif
