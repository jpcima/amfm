#include "amfm.h"
#include <cmath>

void AmFm::init(double sr)
{
    ph_ = 0.0;
    scal_ = sr / (2 * M_PI);
}

void AmFm::clear()
{
    ph_ = 0.0;
}

void AmFm::process(MYFLT *am, MYFLT *fm, const MYFLT *re, const MYFLT *im, uint32_t nsmps)
{
    double oph = ph_, f, ph;
    MYFLT scal = scal_;

    for (uint32_t n = 0; n < nsmps; n++) {
        am[n] = std::sqrt(re[n] * re[n] + im[n] * im[n]);
        ph = std::atan2(im[n], re[n]);
        f = ph - oph;
        oph = ph;
        if (f >= M_PI) f -= 2 * M_PI;
        else if (f < -M_PI) f += 2 * M_PI;
        fm[n] = f * scal;
    }
    this->ph_ = oph;
}
