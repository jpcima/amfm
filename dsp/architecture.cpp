#include "DistrhoPlugin.hpp"
#include <vector>
#include <cstring>

template <class T> T min(T a, T b) { return (a < b) ? a : b; }
template <class T> T max(T a, T b) { return (a > b) ? a : b; }

class Meta {
public:
    void declare(const char *label, const char *value);
};

class UI {
public:
    void openHorizontalBox(const char *label);
    void openVerticalBox(const char *label);
    void closeBox();
    void declare(float *zone, const char *label, const char *value);
    void addButton(const char *label, float *zone);
    void addCheckButton(const char *label, float *zone);
    void addVerticalSlider(const char *label, float *zone, float init, float min, float max, float step);
    void addHorizontalSlider(const char *label, float *zone, float init, float min, float max, float step);

    struct Parameter { float *zone; DISTRHO::Parameter param; };
    std::vector<Parameter> fParameters;

private:
    Parameter &parameterFor(float *zone);
};

class dsp {
public:
    static const unsigned gParameterCount;

private:
    template <class D> static unsigned staticParameterCount();
};

//------------------------------------------------------------------------------
void Meta::declare(const char *label, const char *value)
{
    (void)label;
    (void)value;
}

//------------------------------------------------------------------------------
void UI::openHorizontalBox(const char *label)
{
    (void)label;
}

void UI::openVerticalBox(const char *label)
{
    (void)label;
}

void UI::closeBox()
{
}

void UI::declare(float *zone, const char *label, const char *value)
{
    Parameter &p = parameterFor(zone);
    if (!std::strcmp(label, "tooltip"))
        p.param.name = value;
    else if (!std::strcmp(label, "unit"))
        p.param.unit = value;
    else if (!std::strcmp(label, "scale") && !std::strcmp(value, "log"))
        p.param.hints |= kParameterIsLogarithmic;
}

void UI::addButton(const char *label, float *zone)
{
    Parameter p = parameterFor(zone);
    p.param.hints |= kParameterIsTrigger;
    p.param.symbol = label;
    p.param.ranges.min = 0;
    p.param.ranges.max = 1;
    p.param.ranges.def = 0;
    fParameters.push_back(p);
}

void UI::addCheckButton(const char *label, float *zone)
{
    Parameter p = parameterFor(zone);
    p.param.hints |= kParameterIsBoolean;
    p.param.symbol = label;
    p.param.ranges.min = 0;
    p.param.ranges.max = 1;
    p.param.ranges.def = 0;
    fParameters.push_back(p);
}

void UI::addVerticalSlider(const char *label, float *zone, float init, float min, float max, float step)
{
    (void)step;
    Parameter p = parameterFor(zone);
    p.param.symbol = label;
    p.param.ranges.min = min;
    p.param.ranges.max = max;
    p.param.ranges.def = init;
    fParameters.push_back(p);
}

void UI::addHorizontalSlider(const char *label, float *zone, float init, float min, float max, float step)
{
    (void)step;
    Parameter p = parameterFor(zone);
    p.param.symbol = label;
    p.param.ranges.min = min;
    p.param.ranges.max = max;
    p.param.ranges.def = init;
    fParameters.push_back(p);
}

UI::Parameter &UI::parameterFor(float *zone)
{
    for (Parameter &p : fParameters) {
        if (p.zone == zone)
            return p;
    }
    Parameter p;
    p.zone = zone;
    fParameters.push_back(p);
    return fParameters.back();
}

class mydsp;
const unsigned dsp::gParameterCount = staticParameterCount<mydsp>();

template <class D> unsigned dsp::staticParameterCount()
{
    UI ui;
    D().buildUserInterface(&ui);
    return (unsigned)ui.fParameters.size();
}
