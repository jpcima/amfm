#pragma once
#include <cstdint>

typedef float MYFLT;

/* hilbert.h
 *
 * Copyright 1999, by Sean M. Costello
 *
 * hilbert is an implementation of an IIR Hilbert transformer.
 * The structure is based on two 6th-order allpass filters in
 * parallel, with a constant phase difference of 90 degrees
 * (+- some small amount of error) between the two outputs.
 * Allpass coefficients are calculated at i-time.
 */

class Hilbert
{
public:
    void init(double sr);
    void clear();
    void process(MYFLT *out1, MYFLT *out2, const MYFLT *in, uint32_t nsmps);
private:
    MYFLT xnm1[12], ynm1[12], coef[12];
};
