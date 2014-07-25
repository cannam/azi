
#include "Azi.h"


Azi::Azi(float inputSampleRate) :
    Plugin(inputSampleRate)
    // Also be sure to set your plugin parameters (presumably stored
    // in member variables) to their default values here -- the host
    // will not do that for you
{
}

Azi::~Azi()
{
}

string
Azi::getIdentifier() const
{
    return "azi";
}

string
Azi::getName() const
{
    return "Stereo Plan";
}

string
Azi::getDescription() const
{
    // Return something helpful here!
    return "";
}

string
Azi::getMaker() const
{
    // Your name here
    return "";
}

int
Azi::getPluginVersion() const
{
    // Increment this each time you release a version that behaves
    // differently from the previous one
    return 1;
}

string
Azi::getCopyright() const
{
    // This function is not ideally named.  It does not necessarily
    // need to say who made the plugin -- getMaker does that -- but it
    // should indicate the terms under which it is distributed.  For
    // example, "Copyright (year). All Rights Reserved", or "GPL"
    return "";
}

Azi::InputDomain
Azi::getInputDomain() const
{
    return TimeDomain;
}

size_t
Azi::getPreferredBlockSize() const
{
    return 0; // 0 means "I can handle any block size"
}

size_t 
Azi::getPreferredStepSize() const
{
    return 0; // 0 means "anything sensible"; in practice this
              // means the same as the block size for TimeDomain
              // plugins, or half of it for FrequencyDomain plugins
}

size_t
Azi::getMinChannelCount() const
{
    return 1;
}

size_t
Azi::getMaxChannelCount() const
{
    return 1;
}

Azi::ParameterList
Azi::getParameterDescriptors() const
{
    ParameterList list;

    // If the plugin has no adjustable parameters, return an empty
    // list here (and there's no need to provide implementations of
    // getParameter and setParameter in that case either).

    // Note that it is your responsibility to make sure the parameters
    // start off having their default values (e.g. in the constructor
    // above).  The host needs to know the default value so it can do
    // things like provide a "reset to default" function, but it will
    // not explicitly set your parameters to their defaults for you if
    // they have not changed in the mean time.

    ParameterDescriptor d;
    d.identifier = "parameter";
    d.name = "Some Parameter";
    d.description = "";
    d.unit = "";
    d.minValue = 0;
    d.maxValue = 10;
    d.defaultValue = 5;
    d.isQuantized = false;
    list.push_back(d);

    return list;
}

float
Azi::getParameter(string identifier) const
{
    if (identifier == "parameter") {
        return 5; // return the ACTUAL current value of your parameter here!
    }
    return 0;
}

void
Azi::setParameter(string identifier, float value) 
{
    if (identifier == "parameter") {
        // set the actual value of your parameter
    }
}

Azi::ProgramList
Azi::getPrograms() const
{
    ProgramList list;

    // If you have no programs, return an empty list (or simply don't
    // implement this function or getCurrentProgram/selectProgram)

    return list;
}

string
Azi::getCurrentProgram() const
{
    return ""; // no programs
}

void
Azi::selectProgram(string name)
{
}

Azi::OutputList
Azi::getOutputDescriptors() const
{
    OutputList list;

    // See OutputDescriptor documentation for the possibilities here.
    // Every plugin must have at least one output.

    OutputDescriptor d;
    d.identifier = "output";
    d.name = "My Output";
    d.description = "";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = 1;
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool
Azi::initialise(size_t channels, size_t stepSize, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    // Real initialisation work goes here!

    return true;
}

void
Azi::reset()
{
    // Clear buffers, reset stored values, etc
}

Azi::FeatureSet
Azi::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    // Do actual work!
    return FeatureSet();
}

Azi::FeatureSet
Azi::getRemainingFeatures()
{
    return FeatureSet();
}

