#include "DistrhoPlugin.hpp"
#include "../Defs.h"
#include "../../effect.h"

#ifndef PLUGIN_UNIQUE_ID
#define PLUGIN_UNIQUE_ID 'I', 'F', 'o', '1'
#endif
#ifndef PLUGIN_LABEL
#define PLUGIN_LABEL "AM/FM octaver"
#endif
#ifndef PLUGIN_DESCRIPTION
#define PLUGIN_DESCRIPTION "AM/FM octaver"
#endif

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

static_assert(DISTRHO_PLUGIN_NUM_INPUTS == DISTRHO_PLUGIN_NUM_OUTPUTS,
              "The number of inputs and outputs must be equal.");

class OctaverPlugin : public Plugin {
private:
    IF::Colorizer fOctaver[DISTRHO_PLUGIN_NUM_INPUTS];

public:
    enum ParameterId
    {
        P_Bypass,
        P_SemitoneShift,
        P_Wet,
        P_Dry,
        P_Count
    };

    OctaverPlugin()
        : Plugin(P_Count, 0, 0),
          bypass_(false),
          wet_(M_SQRT1_2),
          dry_(M_SQRT1_2)
    {
        sampleRateChanged(getSampleRate());
        setParameterValue(P_SemitoneShift, +12);
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
        case P_SemitoneShift:
            parameter.hints = kParameterIsInteger;
            parameter.name = "Semitone shift";
            parameter.symbol = "shift";
            parameter.ranges.min = -36;
            parameter.ranges.max = +36;
            parameter.ranges.def = +12;
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
        case P_SemitoneShift: {
            float k = fOctaver[0].color();
            value = 12 * std::log2(k);
            break;
        }
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
        case P_SemitoneShift: {
            float k = std::exp2(value * (1.0f / 12));
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
                fOctaver[c].set_color(k);
            break;
        }
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
            fOctaver[c].process(out, in, frames);
            for (unsigned i = 0; i < frames; ++i)
                out[i] = wet * out[i] + dry * in[i];
        }
    }

    void sampleRateChanged(double newSampleRate) override
    {
        for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS; ++c)
            fOctaver[c].init(newSampleRate);
    }

    // -------------------------------------------------------------------------------------------------------

private:
    bool bypass_;
    float wet_;
    float dry_;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaverPlugin)
};

Plugin *createPlugin()
{
    return new OctaverPlugin;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
