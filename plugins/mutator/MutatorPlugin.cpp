#include "DistrhoPlugin.hpp"
#include "../Defs.h"
#include "../../effect.h"

#ifndef PLUGIN_UNIQUE_ID
#define PLUGIN_UNIQUE_ID 'I', 'F', 'm', '1'
#endif
#ifndef PLUGIN_LABEL
#define PLUGIN_LABEL "AM/FM mutator"
#endif
#ifndef PLUGIN_DESCRIPTION
#define PLUGIN_DESCRIPTION "AM/FM mutator"
#endif

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

static_assert(DISTRHO_PLUGIN_NUM_INPUTS == DISTRHO_PLUGIN_NUM_OUTPUTS * 2,
              "The number of inputs must be equal to twice the number of outputs.");

class MutatorPlugin : public Plugin {
private:
    IF::Mutator fMutator[DISTRHO_PLUGIN_NUM_OUTPUTS];

public:
    enum ParameterId
    {
        P_Bypass,
        P_Mix12,
        P_Count
    };

    MutatorPlugin()
        : Plugin(P_Count, 0, 0),
          bypass_(false)
    {
        sampleRateChanged(getSampleRate());
        setParameterValue(P_Mix12, 0.5);
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
        case P_Mix12:
            parameter.hints = kParameterIsAutomable;
            parameter.name = "In1/In2 mix";
            parameter.symbol = "mix12";
            parameter.ranges.min = 0.0;
            parameter.ranges.max = 1.0;
            parameter.ranges.def = 0.5;
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
        case P_Mix12: {
            value = fMutator[0].mix();
            break;
        }
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
        case P_Mix12: {
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_OUTPUTS; ++c)
                fMutator[c].set_mix(value);
            break;
        }
        default:
            DISTRHO_SAFE_ASSERT_RETURN(false, );
        }
    }

    void run(const float **inputs, float **outputs, uint32_t frames) override
    {
        if (bypass_) {
            for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_OUTPUTS; ++c)
                std::copy(inputs[c], inputs[c] + frames, outputs[c]);
            return;
        }

        for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_OUTPUTS; ++c)
            fMutator[c].process(outputs[c], inputs[c], inputs[c + DISTRHO_PLUGIN_NUM_OUTPUTS], frames);
    }

    void sampleRateChanged(double newSampleRate) override
    {
        for (unsigned c = 0; c < DISTRHO_PLUGIN_NUM_INPUTS / 2; ++c)
            fMutator[c].init(newSampleRate);
    }

    // -------------------------------------------------------------------------------------------------------

private:
    bool bypass_;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MutatorPlugin)
};

Plugin *createPlugin()
{
    return new MutatorPlugin;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
