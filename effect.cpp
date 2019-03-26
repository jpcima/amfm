#include "effect.h"
#include <cmath>

namespace IF {

void Effect1to1::process(float *out, const float *in, unsigned nframes)
{
    while (nframes > 0) {
        unsigned curframes = (nframes < buffer_max) ? nframes : buffer_max;
        do_process(out, in, curframes);
        nframes -= curframes;
        in += curframes;
        out += curframes;
    }
}

void Effect2to1::process(float *out, const float *in1, const float *in2, unsigned nframes)
{
    while (nframes > 0) {
        unsigned curframes = (nframes < buffer_max) ? nframes : buffer_max;
        do_process(out, in1, in2, curframes);
        nframes -= curframes;
        in1 += curframes;
        in2 += curframes;
        out += curframes;
    }
}

}  // namespace IF

//------------------------------------------------------------------------------
namespace IF {

Colorizer::Colorizer()
{
    color_ = 2;
}

Colorizer::~Colorizer()
{
}

void Colorizer::init(double sr)
{
    hilb_.init(sr);
    amfm_.init(sr);
    oscph_ = 0;
    onedsr_ = 1 / sr;
}

void Colorizer::clear()
{
    oscph_ = 0;
    hilb_.clear();
    amfm_.clear();
}

void Colorizer::set_color(float c)
{
    color_ = c;
    clear();
}

void Colorizer::do_process(float *out, const float *in, unsigned nframes)
{
    float oscph = oscph_;
    float onedsr = onedsr_;
    float color = color_;
    float temp[buffer_max];

    float *hilb_re = out;
    float *hilb_im = temp;
    hilb_.process(hilb_re, hilb_im, in, nframes);

    float *am = out;
    float *fm = temp;
    amfm_.process(am, fm, hilb_re, hilb_im, nframes);

    for (unsigned i = 0; i < nframes; ++i) {
        float aam = am[i];
        float aif = fm[i];

        aif *= color;

        out[i] = aam * std::cos(float(2 * M_PI) * oscph);
        oscph += aif * onedsr;
        oscph -= (int)oscph;
    }

    oscph_ = oscph;
}

}  // namespace IF

//------------------------------------------------------------------------------
namespace IF {

Mutator::Mutator()
{
    mix_ = 0.5;
}

Mutator::~Mutator()
{
}

void Mutator::init(double sr)
{
    hilb1_.init(sr);
    hilb2_.init(sr);
    amfm1_.init(sr);
    amfm2_.init(sr);
    oscph_ = 0;
    onedsr_ = 1 / sr;
}

void Mutator::clear()
{
    oscph_ = 0;
    hilb1_.clear();
    hilb2_.clear();
    amfm1_.clear();
    amfm2_.clear();
}

void Mutator::set_mix(float m)
{
    mix_ = m;
}

void Mutator::do_process(float *out, const float *in1, const float *in2, unsigned nframes)
{
    float oscph = oscph_;
    float mix = mix_;
    float onedsr = onedsr_;
    float temp1[buffer_max], temp2[buffer_max], temp3[buffer_max];

    float *hilb_re1 = out;
    float *hilb_im1 = temp1;
    float *hilb_re2 = temp2;
    float *hilb_im2 = temp3;
    hilb1_.process(hilb_re1, hilb_im1, in1, nframes);
    hilb2_.process(hilb_re2, hilb_im2, in2, nframes);

    float *am1 = out;
    float *fm1 = temp1;
    float *am2 = temp2;
    float *fm2 = temp3;
    amfm1_.process(am1, fm1, hilb_re1, hilb_im1, nframes);
    amfm2_.process(am2, fm2, hilb_re2, hilb_im2, nframes);

    for (unsigned i = 0; i < nframes; ++i) {
        float aam = am1[i];
        float aif = (1 - mix) * fm1[i] + mix * fm2[i];

        out[i] = aam * std::cos(float(2 * M_PI) * oscph);
        oscph += aif * onedsr;
        oscph -= (int)oscph;
    }

    oscph_ = oscph;
}

}  // namespace IF

//------------------------------------------------------------------------------
#include "stk/DelayA.h"

namespace IF {

static const double chorus_delay[4] = {
    0.020453515, 0.020000000, 0.019546485, 0.019727891
};

static const unsigned chorus_lines =
    sizeof(chorus_delay) / sizeof(chorus_delay[0]);

static const int chorus_detune_mult[4] = {
    -2, -1, +1, +2
};

Chorus::Chorus()
{
    detune_ = 0.001;

    hilb_.reset(new Hilbert[chorus_lines + 1]);
    amfm_.reset(new AmFm[chorus_lines + 1]);
    oscph_.reset(new float[chorus_lines + 1]());
    lines_.reset(new stk::DelayA[chorus_lines]);
}

Chorus::~Chorus()
{
}

void Chorus::init(double sr)
{
    onedsr_ = 1 / sr;

    for (unsigned l = 0; l < chorus_lines + 1; ++l) {
        oscph_[l] = 0.0;
        hilb_[l].init(sr);
        amfm_[l].init(sr);
    }

    for (unsigned l = 0; l < chorus_lines; ++l) {
        stk::DelayA &line = lines_[l];
        line.clear();
        double delay = chorus_delay[l] * sr;
        line.setMaximumDelay((size_t)std::ceil(delay));
        line.setDelay(delay);
    }
}

void Chorus::clear()
{
    for (unsigned l = 0; l < chorus_lines + 1; ++l) {
        oscph_[l] = 0.0;
        hilb_[l].clear();
        amfm_[l].clear();
    }

    for (unsigned l = 0; l < chorus_lines; ++l)
        lines_[l].clear();
}

void Chorus::do_process(float *out, const float *in, unsigned nframes)
{
    float detune = detune_;
    float onedsr = onedsr_;
    float amgain = 1.0 / (chorus_lines + 1);
    float temp1[buffer_max], temp2[buffer_max];

    {
        float *hilb_re = temp1;
        float *hilb_im = temp2;
        hilb_[chorus_lines].process(hilb_re, hilb_im, in, nframes);

        float *am = temp1;
        float *fm = temp2;
        amfm_[chorus_lines].process(am, fm, hilb_re, hilb_im, nframes);

        float oscph = oscph_[chorus_lines];

        for (unsigned i = 0; i < nframes; ++i) {
            out[i] = amgain * am[i] * std::cos(float(2 * M_PI) * oscph);
            oscph += fm[i] * onedsr;
            oscph -= (int)oscph;
        }

        oscph_[chorus_lines] = oscph;
    }

    for (unsigned l = 0; l < chorus_lines; ++l) {
        float *hilb_re = temp1;
        float *hilb_im = temp2;
        hilb_[l].process(hilb_re, hilb_im, in, nframes);

        float *am = temp1;
        float *fm = temp2;
        amfm_[l].process(am, fm, hilb_re, hilb_im, nframes);

        lines_[l].tick(fm, nframes);

        float oscph = oscph_[l];
        float det = detune * chorus_detune_mult[l];

        for (unsigned i = 0; i < nframes; ++i) {
            out[i] += amgain * am[i] * std::cos((1 + det) * float(2 * M_PI) * oscph);
            oscph += fm[i] * onedsr;
            oscph -= (int)oscph;
        }

        oscph_[l] = oscph;
    }
}

}  // namespace IF

//------------------------------------------------------------------------------
namespace IF {

Wahwah::Wahwah()
{
    lforate_ = 0.5;   // joe 5.0 // ag 0.5
    lfodepth_ = 0.8;  // joe 0.5 // ag 0.8
}

Wahwah::~Wahwah()
{
}

void Wahwah::init(double sr)
{
    hilb_.init(sr);
    amfm_.init(sr);
    oscph_ = 0;
    output1_ = 0;
    output2_ = 0;
    onedsr_ = 1 / sr;
}

void Wahwah::clear()
{
    oscph_ = 0;
    output1_ = 0;
    output2_ = 0;
    hilb_.clear();
    amfm_.clear();
}

void Wahwah::do_process(float *out, const float *in, unsigned nframes)
{
    float oscph = oscph_;
    float fltprev1 = output1_;
    float fltprev2 = output2_;
    float lforate = lforate_;
    float lfodepth = lfodepth_;
    float lfophase = lfophase_;
    float onedsr = onedsr_;
    float temp[buffer_max];

    lforate *= onedsr;

    float *hilb_re = out;
    float *hilb_im = temp;
    hilb_.process(hilb_re, hilb_im, in, nframes);

    float *am = out;
    float *fm = temp;
    amfm_.process(am, fm, hilb_re, hilb_im, nframes);

    for (unsigned i = 0; i < nframes; ++i) {
        float aam = am[i];
        float aif = fm[i];

        // tick the LFO
        float wah = lfodepth * ((lfophase < 0.5f) ? (4 * lfophase) : (4 - 4 * lfophase));
        lfophase += lforate;
        lfophase -= (int)lfophase;

        // update time-variant filter
        double fltg = 0.1 * std::pow(4.0, wah);
        double fltfr = onedsr * 450.0 * std::exp2(2.3 * wah);
        double fltq = std::exp2(2.0 * (1.0 - wah) + 1.0);
        // update coefficients
        double fltrho = 1.0 - M_PI * fltfr / fltq;
        double flttheta = 2.0 * M_PI * fltfr;
        double flta1 = -2.0 * fltrho * std::cos(flttheta);
        double flta2 = fltrho * fltrho;

        aif = fltg * aif - flta1 * fltprev1 - flta2 * fltprev2;
        fltprev2 = fltprev1;
        fltprev1 = aif;

        out[i] = aam * std::cos(float(2 * M_PI) * oscph);
        oscph += aif * onedsr;
        oscph -= (int)oscph;
    }

    output1_ = fltprev1;
    output2_ = fltprev2;
    lfophase_ = lfophase;
}

}  // namespace IF
