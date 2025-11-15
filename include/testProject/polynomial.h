#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>

/**
 * A single term in a polynomial: coefficient * x^exponent.
 */
typedef struct PolyTerm {
    double coefficient;
    int    exponent;
    struct PolyTerm *next;
} PolyTerm;

/**
 * Polynomial represented as a sorted (descending exponent) singly-linked list.
 */
typedef struct Polynomial {
    PolyTerm *head;
} Polynomial;

/**
 * Initialize a polynomial to the zero polynomial.
 */
void polynomial_init(Polynomial *poly);

/**
 * Return true if the polynomial is NULL or has no terms.
 */
bool polynomial_is_empty(const Polynomial *poly);

/**
 * Remove all terms from the polynomial and free associated memory.
 * After this call, the polynomial represents 0.
 */
void polynomial_clear(Polynomial *poly);

/**
 * Insert a term into the polynomial.
 *
 * - Keeps terms sorted by exponent (descending).
 * - Combines like terms (same exponent).
 * - If the resulting coefficient for an exponent becomes 0, the term is removed.
 *
 * Returns false on allocation failure, true otherwise.
 */
bool polynomial_insert_term(Polynomial *poly, double coefficient, int exponent);

/**
 * Copy src into dest (deep copy).
 *
 * Any existing contents of dest are cleared first.
 * Returns false on allocation failure, in which case dest is left as 0.
 */
bool polynomial_copy(const Polynomial *src, Polynomial *dest);

/**
 * result = a + b (as polynomials).
 *
 * Any existing contents of result are cleared first.
 * a, b, and result must be distinct (non-aliased) polynomials.
 * Returns false on allocation failure.
 */
bool polynomial_add(const Polynomial *a, const Polynomial *b, Polynomial *result);

/**
 * result = a * b (polynomial product).
 *
 * Any existing contents of result are cleared first.
 * a, b, and result must be distinct (non-aliased) polynomials.
 * Returns false on allocation failure.
 */
bool polynomial_multiply(const Polynomial *a, const Polynomial *b, Polynomial *result);

/**
 * Evaluate the polynomial at x.
 * If poly is NULL or empty, returns 0.0.
 */
double polynomial_evaluate(const Polynomial *poly, double x);

/**
 * Return the degree (largest exponent) of the polynomial.
 * Returns -1 for the zero polynomial.
 */
int polynomial_degree(const Polynomial *poly);

#ifdef __cplusplus
}
#endif

#endif /* POLYNOMIAL_H */
