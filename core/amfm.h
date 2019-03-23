#pragma once
#include <cstdint>

typedef float MYFLT;

class AmFm
{
public:
    void init(double sr);
    void clear();
    void process(MYFLT *am, MYFLT *fm, const MYFLT *re, const MYFLT *im, uint32_t nsmps);
private:
  double ph_;
  double scal_;
};
