#include "polynomial.h"

#include <stdlib.h>
#include <math.h>

/* Internal helper to create a new PolyTerm node. */
static PolyTerm *polyterm_create(double coefficient, int exponent) {
    PolyTerm *term = (PolyTerm *)malloc(sizeof(PolyTerm));
    if (!term) {
        return NULL;
    }
    term->coefficient = coefficient;
    term->exponent    = exponent;
    term->next        = NULL;
    return term;
}

void polynomial_init(Polynomial *poly) {
    if (!poly) {
        return;
    }
    poly->head = NULL;
}

bool polynomial_is_empty(const Polynomial *poly) {
    return (!poly) || (poly->head == NULL);
}

void polynomial_clear(Polynomial *poly) {
    if (!poly) {
        return;
    }
    PolyTerm *current = poly->head;
    while (current) {
        PolyTerm *next = current->next;
        free(current);
        current = next;
    }
    poly->head = NULL;
}

bool polynomial_insert_term(Polynomial *poly, double coefficient, int exponent) {
    if (!poly) {
        return false;
    }

    /* Ignore pure zero terms. */
    if (coefficient == 0.0) {
        return true;
    }

    PolyTerm *new_term = polyterm_create(coefficient, exponent);
    if (!new_term) {
        return false;
    }

    PolyTerm *prev    = NULL;
    PolyTerm *current = poly->head;

    /* Find insertion point: keep list sorted by exponent (descending). */
    while (current && current->exponent > exponent) {
        prev    = current;
        current = current->next;
    }

    /* Combine like term if exponent already exists. */
    if (current && current->exponent == exponent) {
        current->coefficient += coefficient;
        free(new_term);

        /* If coefficient becomes zero, remove the node entirely. */
        if (current->coefficient == 0.0) {
            if (prev) {
                prev->next = current->next;
            } else {
                poly->head = current->next;
            }
            free(current);
        }
        return true;
    }

    /* Insert new term at the correct position. */
    if (!prev) {
        /* Insert at head. */
        new_term->next = poly->head;
        poly->head     = new_term;
    } else {
        new_term->next = current;
        prev->next     = new_term;
    }

    return true;
}

bool polynomial_copy(const Polynomial *src, Polynomial *dest) {
    if (!src || !dest) {
        return false;
    }

    polynomial_clear(dest);
    polynomial_init(dest);

    for (PolyTerm *t = src->head; t != NULL; t = t->next) {
        if (!polynomial_insert_term(dest, t->coefficient, t->exponent)) {
            polynomial_clear(dest);
            return false;
        }
    }

    return true;
}

bool polynomial_add(const Polynomial *a, const Polynomial *b, Polynomial *result) {
    if (!a || !b || !result) {
        return false;
    }

    polynomial_clear(result);
    polynomial_init(result);

    /* Add all terms from a. */
    for (PolyTerm *t = a->head; t != NULL; t = t->next) {
        if (!polynomial_insert_term(result, t->coefficient, t->exponent)) {
            polynomial_clear(result);
            return false;
        }
    }

    /* Add all terms from b. */
    for (PolyTerm *t = b->head; t != NULL; t = t->next) {
        if (!polynomial_insert_term(result, t->coefficient, t->exponent)) {
            polynomial_clear(result);
            return false;
        }
    }

    return true;
}

bool polynomial_multiply(const Polynomial *a, const Polynomial *b, Polynomial *result) {
    if (!a || !b || !result) {
        return false;
    }

    polynomial_clear(result);
    polynomial_init(result);

    /* Naïve O(n*m) multiplication. */
    for (PolyTerm *ta = a->head; ta != NULL; ta = ta->next) {
        for (PolyTerm *tb = b->head; tb != NULL; tb = tb->next) {
            double coeff = ta->coefficient * tb->coefficient;
            int    exp   = ta->exponent + tb->exponent;

            if (!polynomial_insert_term(result, coeff, exp)) {
                polynomial_clear(result);
                return false;
            }
        }
    }

    return true;
}

double polynomial_evaluate(const Polynomial *poly, double x) {
    if (!poly) {
        return 0.0;
    }

    double result = 0.0;
    for (PolyTerm *t = poly->head; t != NULL; t = t->next) {
        result += t->coefficient * pow(x, (double)t->exponent);
    }
    return result;
}

int polynomial_degree(const Polynomial *poly) {
    if (polynomial_is_empty(poly)) {
        return -1;
    }
    /* Head is always the largest exponent by construction. */
    return poly->head->exponent;
}
