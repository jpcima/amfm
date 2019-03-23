#include "hilbert.h"
#include <cmath>

void Hilbert::init(double sr)
{
    double onedsr = 1 / sr;

    /* pole values taken from Bernie Hutchins, "Musical Engineer's Handbook" */
    double poles[12] = {0.3609, 2.7412, 11.1573, 44.7581, 179.6242, 798.4578,
                        1.2524, 5.5671, 22.3423, 89.6271, 364.7914, 2770.1114};
    double polefreq, rc, alpha, beta;
    /* calculate coefficients for allpass filters, based on sampling rate */
    for (int32_t j = 0; j < 12; j++) {
        /*      this->coef[j] = (1 - (15 * M_PI * pole[j]) / CS_ESR) /
                (1 + (15 * M_PI * pole[j]) / CS_ESR); */
        polefreq = poles[j] * 15.0;
        rc = 1.0 / (2.0 * M_PI * polefreq);
        alpha = 1.0 / rc;
        alpha = alpha * 0.5 * onedsr;
        beta = (1.0 - alpha) / (1.0 + alpha);
        this->xnm1[j] = this->ynm1[j] = 0.0;
        this->coef[j] = -(MYFLT)beta;
    }
}

void Hilbert::clear()
{
    for (int32_t j = 0; j < 12; j++)
        this->xnm1[j] = this->ynm1[j] = 0.0;
}

void Hilbert::process(MYFLT *out1, MYFLT *out2, const MYFLT *in, uint32_t nsmps)
{
    MYFLT *coef = this->coef;

    for (uint32_t n = 0; n < nsmps; n++) {
        MYFLT yn1, yn2;
        MYFLT xn1 = in[n];
        /* 6th order allpass filter for sine output. Structure is
         * 6 first-order allpass sections in series. Coefficients
         * taken from arrays calculated at i-time.
         */
        for (int32_t j = 0; j < 6; j++) {
            yn1 = coef[j] * (xn1 - this->ynm1[j]) + this->xnm1[j];
            this->xnm1[j] = xn1;
            this->ynm1[j] = yn1;
            xn1 = yn1;
        }
        MYFLT xn2 = in[n];
        /* 6th order allpass filter for cosine output. Structure is
         * 6 first-order allpass sections in series. Coefficients
         * taken from arrays calculated at i-time.
         */
        for (int32_t j = 6; j < 12; j++) {
            yn2 = coef[j] * (xn2 - this->ynm1[j]) + this->xnm1[j];
            this->xnm1[j] = xn2;
            this->ynm1[j] = yn2;
            xn2 = yn2;
        }
        out1[n] = yn2;
        out2[n] = yn1;
    }
}
