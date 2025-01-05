// Incomplete gamma function and the chi-square distribution built on it.
#pragma once

namespace rc {

// Regularised lower incomplete gamma function P(a, x).
double gamma_p(double a, double x);

// Probability that a chi-square variable with `dof` degrees of freedom exceeds x.
double chi2_survival(double x, double dof);

}  // namespace rc
