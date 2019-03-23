#pragma once
#include "core/hilbert.h"
#include "core/amfm.h"
#include <memory>
namespace stk { class DelayA; }

namespace IF {

class Effect
{
public:
    virtual ~Effect() {}
    virtual void init(double sr) = 0;
    virtual void clear() = 0;
};

//------------------------------------------------------------------------------
class Effect1to1 : public Effect
{
public:
    virtual ~Effect1to1() {}
    void process(float *out, const float *in, unsigned nframes);
protected:
    static constexpr unsigned buffer_max = 256;
    virtual void do_process(float *out, const float *in, unsigned nframes) = 0;
};

class Effect2to1 : public Effect
{
public:
    virtual ~Effect2to1() {}
    void process(float *out, const float *in1, const float *in2, unsigned nframes);
protected:
    static constexpr unsigned buffer_max = 256;
    virtual void do_process(float *out, const float *in1, const float *in2, unsigned nframes) = 0;
};

//------------------------------------------------------------------------------
class Colorizer : public Effect1to1
{
public:
    void init(double sr) override;
    void clear() override;
    float color() const { return color_; }
    void set_color(float c);
protected:
    void do_process(float *out, const float *in, unsigned nframes) override;
private:
    Hilbert hilb_;
    AmFm amfm_;
    float oscph_;
    float onedsr_;
    float color_;
};

//------------------------------------------------------------------------------
class Mutator : public Effect2to1
{
public:
    void init(double sr) override;
    void clear() override;
    float mix() const { return mix_; }
    void set_mix(float m);
protected:
    void do_process(float *out, const float *in1, const float *in2, unsigned nframes) override;
private:
    Hilbert hilb1_;
    Hilbert hilb2_;
    AmFm amfm1_;
    AmFm amfm2_;
    float oscph_;
    float mix_;
    float onedsr_;
};

//------------------------------------------------------------------------------
class Chorus : public Effect1to1
{
public:
    Chorus();
    ~Chorus();
    void init(double sr) override;
    void clear() override;
    float detune() const { return detune_; }
    void set_detune(float d) { detune_ = d; }
protected:
    void do_process(float *out, const float *in, unsigned nframes) override;
private:
    float detune_;
    float onedsr_;
    std::unique_ptr<Hilbert[]> hilb_;
    std::unique_ptr<AmFm[]> amfm_;
    std::unique_ptr<float[]> oscph_;
    std::unique_ptr<stk::DelayA[]> lines_;
};

}  // namespace IF
