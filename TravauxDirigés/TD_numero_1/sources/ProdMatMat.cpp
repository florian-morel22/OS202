#include "Matrix.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace {
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix &A, const Matrix &B, Matrix &C) {

#pragma omp parallel for
  for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)
    for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++)
      for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
        C(i, j) += A(i, k) * B(k, j);
}

} // namespace

Matrix operator*(const Matrix &A, const Matrix &B) {
  Matrix C(A.nbRows, B.nbCols, 0.0);

  /* ------------------ Produit Block Block ------------------ */

  const int block_size = 512;

  // Ce code découpe la matrice A et B en sous-matrices de taille block_size,
  // puis utilise la fonction "prodSubBlocks" pour calculer le produit des
  // sous-matrices et stocker le résultat dans la matrice C.

  for (int iRowBlkA = 0; iRowBlkA < A.nbRows; iRowBlkA += block_size) {
    for (int iColBlkB = 0; iColBlkB < B.nbCols; iColBlkB += block_size) {
      for (int iColBlkA = 0; iColBlkA < A.nbCols; iColBlkA += block_size) {
        prodSubBlocks(iRowBlkA, iColBlkB, iColBlkA, block_size, A, B, C);
      }
    }
  }

  /* ------------------ Produit Scalaire Scalaire ------------------ */
  // prodSubBlocks(0, 0, 0, std::max({A.nbRows, B.nbCols, A.nbCols}), A, B, C);

  return C;
}
