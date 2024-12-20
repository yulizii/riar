#include <Rcpp.h>
#include <cmath>

using namespace Rcpp;

//' @title FORM Algorithm using Rcpp
//' @name form
//' @description Calculates the failure probability and reliability index using the FORM algorithm
//' @param g A function representing the limit state function
//' @param mean A NumericVector of the mean values of the input variables
//' @param sigma A NumericVector of the standard deviations of the input variables
//' @return A List with failure probability \code{pf} and reliability index \code{beta}
//' @examples
//' \dontrun{
//' g <- function(x) { x[1] + x[2] - 3 }
//' mean <- c(2, 2)
//' sigma <- c(1, 1)
//' result <- form(g, mean, sigma)
//' print(result)
//' }
//' @export
// [[Rcpp::export]]
 List form(Function g, NumericVector mean, NumericVector sigma) {
   int n = mean.size(); // Number of variables
   NumericVector u = mean / sigma; // Initial guess in standard normal space
   NumericVector grad(n);          // Gradient vector
   double beta = 0.0, beta_prev;   // Reliability index
   double tol = 1e-6;              // Convergence tolerance
   int max_iter = 100;             // Maximum iterations

   // Iterative optimization
   for (int iter = 0; iter < max_iter; ++iter) {
     // Compute gradient and g value
     NumericVector x = mean + sigma * u; // Map back to original space
     double g_val = as<double>(g(x));

     // Numerical gradient approximation
     for (int i = 0; i < n; ++i) {
       NumericVector x_eps = clone(x);
       double eps = 1e-5; // Small perturbation
       x_eps[i] += eps;
       grad[i] = (as<double>(g(x_eps)) - g_val) / eps;
     }

     // Update beta and u
     double grad_norm = sum(pow(grad, 2.0));
     if (grad_norm == 0) stop("Gradient norm is zero, optimization cannot proceed.");
     grad = grad / std::sqrt(grad_norm); // Normalize gradient
     beta_prev = beta;
     beta = -g_val / std::sqrt(grad_norm); // Update reliability index
     u = -beta * grad; // Update u

     // Check for convergence
     if (std::abs(beta - beta_prev) < tol) break;
   }

   // Compute failure probability
   double pf = R::pnorm(-beta, 0.0, 1.0, 1, 0); // Standard normal CDF

   // Return results
   return List::create(
     Named("pf") = pf,
     Named("beta") = beta
   );
 }
