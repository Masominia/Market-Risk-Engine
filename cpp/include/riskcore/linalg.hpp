// Factorisations of symmetric matrices.
#pragma once

#include "riskcore/matrix.hpp"

namespace rc {

// Lower-triangular factor of a symmetric positive definite matrix. Returns false if it is not positive definite.
bool cholesky(const Matrix& a, Matrix& lower);

// Solves a x = b given the Cholesky factor of a.
Vector cholesky_solve(const Matrix& lower, const Vector& b);

// Eigenvalues of a symmetric matrix by cyclic Jacobi rotations, ascending.
// If `vectors` is given, its columns receive the eigenvectors in the same order.
Vector symmetric_eigenvalues(Matrix a, Matrix* vectors = nullptr);

// Largest eigenvalue over smallest; infinite if the smallest is not positive.
double condition_number(const Matrix& a);

}  // namespace rc
