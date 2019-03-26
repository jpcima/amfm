#include "DistrhoPlugin.hpp"
#include "../Defs.h"
#include "../../effect.h"

#ifndef PLUGIN_UNIQUE_ID
#define PLUGIN_UNIQUE_ID 'I', 'F', 'w', '1'
#endif
#ifndef PLUGIN_LABEL
#define PLUGIN_LABEL "AM/FM wah-wah"
#endif
#ifndef PLUGIN_DESCRIPTION
#define PLUGIN_DESCRIPTION "AM/FM wah-wah"
#endif

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

static_assert(DISTRHO_PLUGIN_NUM_INPUTS == DISTRHO_PLUGIN_NUM_OUTPUTS,
              "The number of inputs and outputs must be equal.");

class WahwahPlugin : public Plugin {
private:
    IF::Wahwah fWahwah[DISTRHO_PLUGIN_NUM_INPUTS];

public:
    enum ParameterId
    {
        P_Bypass,
        P_LfoRate,
        P_LfoDepth,
        P_Wet,
        P_Dry,
        P_Count
    };

    WahwahPlugin()
        : Plugin(P_Count, 0, 0),
          bypass_(false),
          wet_(M_SQRT1_2),
          dry_(M_SQRT1_2)
    {
        sampleRateChanged(getSampleRate());
    }

protected:
    const char *getLabel() const override
    {
        return PLUGIN_LABEL;
    }

    const char *getDescription() const override
    {
        return PLUGIN_DESCRIPTION;
    }

    const char *getMaker() const override
    {
        return PLUGIN_MAKER;
    }

    const char *getHomePage() const override
    {
        return PLUGIN_HOMEPAGE;
    }

    const char *getLicense() const override
    {
        return PLUGIN_LICENSE;
    }

    uint32_t getVersion() const override
    {
        return d_version(PLUGIN_VERSION);
    }

    int64_t getUniqueId() const override
    {
        return d_cconst(PLUGIN_UNIQUE_ID);
    }

    void initParameter(uint32_t index, Parameter &parameter) override
    {
        switch (index) {
        case P_Bypass:
            parameter.designation = kParameterDesignationBypass;
            break;
        case P_LfoRate:
            parameter.hints = kParameterIsAutomable;
            parameter.name = "LFO rate";
            parameter.symbol = "lforate";
            parameter.ranges.min = 0.1;
            parameter.ranges.max = 5.0;
            parameter.ranges.def = 0.5;
            break;
        case P_LfoDepth:
            parameter.hints = kParameterIsAutomable;
            parameter.name = "LFO depth";
            parameter.symbol = "lfodepth";
            parameter.ranges.min = 0.0;
            parameter.ranges.max = 1.0;
            parameter.ranges.def = 0.8;
            break;
        case P_Wet:
            parameter.hints = kParameterIsLogarithmic|kParameterIsAutomable;
            parameter.name = "Wet gain";
            parameter.symbol = "wet";
            parameter.ranges.min = 0.001;
            parameter.ranges.max = 10.0;
            parameter.ranges.def = M_SQRT1_2;
            break;
        case P_Dry:
            parameter.hints = kParameterIsLogarithmic|kParameterIsAutomable;
            parameter.name = "Dry gain";
            parameter.symbol = "dry";
            parameter.ranges.min = 0.001;
            parameter.ranges.max = 10.0;
            parameter.ranges.def = M_SQRT1_2;
            break;
        default:
            DISTRHO_SAFE_ASSERT_RETURN(false, );
        }
    }

    float getParameterValue(uint32_t index) const override
    {
        float value = 0;
        switch (index) {
        case P_Bypass:
            value = bypass_;
            break;
        case P_LfoRate:
            value = fWahwah[0].lfo_rate();
            break;
        case P_LfoDepth:
            value = fWahwah[0].lfo_depth();
            break;
        case P_Wet:
            value = wet_;
            break;
        case P_Dry:
            value = dry_;
            break;
        default:
            DISTRHO_SAFE_ASSERT_RETURN(false, 0);
        }
        return value;
    }

    void setParameterValue(uint32_t index, float value) override
    {
        switch (index) {
        case P_Bypass:
            bypass_ = value > 0.5f;
            break;
        case P_LfoRate:
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
                fWahwah[0].set_lfo_rate(value);
            break;
        case P_LfoDepth:
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
                fWahwah[0].set_lfo_depth(value);
            break;
        case P_Wet:
            wet_ = value;
            break;
        case P_Dry:
            dry_ = value;
            break;
        default:
            DISTRHO_SAFE_ASSERT_RETURN(false, );
        }
    }

    void run(const float **inputs, float **outputs, uint32_t frames) override
    {
        if (bypass_) {
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
                std::copy(inputs[c], inputs[c] + frames, outputs[c]);
            return;
        }

        float wet = wet_;
        float dry = dry_;

        for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c) {
            const float *in = inputs[c];
            float *out = outputs[c];
            fWahwah[c].process(out, in, frames);
            for (unsigned i = 0; i < frames; ++i)
                out[i] = wet * out[i] + dry * in[i];
        }
    }

    void sampleRateChanged(double newSampleRate) override
    {
        for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
            fWahwah[c].init(newSampleRate);
    }

    // -------------------------------------------------------------------------------------------------------

private:
    bool bypass_;
    float wet_;
    float dry_;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WahwahPlugin)
};

Plugin *createPlugin()
{
    return new WahwahPlugin;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
