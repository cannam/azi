
#include "Azi.h"

#include <cmath>
#include <iostream>

using std::vector;
using std::cerr;
using std::endl;

Azi::Azi(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_width(32)
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
    return 2048;
}

size_t 
Azi::getPreferredStepSize() const
{
    return 256;
}

size_t
Azi::getMinChannelCount() const
{
    return 2;
}

size_t
Azi::getMaxChannelCount() const
{
    return 2;
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

    return list;
}

float
Azi::getParameter(string identifier) const
{
    return 0;
}

void
Azi::setParameter(string identifier, float value) 
{
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
    d.identifier = "plan";
    d.name = "Plan";
    d.description = "";
    d.unit = "";
    d.hasFixedBinCount = true;
    d.binCount = m_width * 2 + 1;
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

    m_blockSize = blockSize;

    return true;
}

void
Azi::reset()
{
    // Clear buffers, reset stored values, etc
}

float
Azi::rms(const vector<float> &buffer)
{
    float sum = 0;
    for (int i = 0; i < int(buffer.size()); ++i) {
	sum += buffer[i] * buffer[i];
    }
    return sqrtf(sum / buffer.size());
}

Azi::FeatureSet
Azi::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    vector<float> left, right, mixed;
    for (int i = 0; i < m_blockSize; ++i) {
	left.push_back(inputBuffers[0][i]);
	right.push_back(inputBuffers[1][i]);
	mixed.push_back((left[i] + right[i]) / 2.f);
    }

    FeatureSet fs;
    Feature f;
    f.values = vector<float>(m_width * 2 + 1, 0.f);

    float threshold = 0.001;
    float diffThreshold = 0.000001;

    float mixedRms = 0.f;

    vector<float> cancelled(m_blockSize, 0.f);
    vector<float> bestRemainder(m_blockSize, 0.f);

    while (1) {

	float prevMixedRms = mixedRms;
	mixedRms = rms(mixed);

	if (mixedRms < threshold ||
	    fabsf(mixedRms - prevMixedRms) < diffThreshold) {
	    break;
	}

	float bestRmsDiff = 0.f;
	int bestAzi = 0;
	float bestLeftGain = 0.f;
	float bestRightGain = 0.f;

	for (int j = -m_width; j <= m_width; ++j) {

	    float pan = float(j) / m_width;

	    float leftGain = 1.f, rightGain = 1.f;
	    if (pan > 0.f) leftGain *= 1.f - pan;
	    if (pan < 0.f) rightGain *= pan + 1.f;

	    if (leftGain < rightGain) {
	
		float ratio = leftGain / rightGain;
		for (int i = 0; i < m_blockSize; ++i) {
		    cancelled[i] = left[i] - ratio * right[i];
		}

	    } else {
	
		float ratio = rightGain / leftGain;
		for (int i = 0; i < m_blockSize; ++i) {
		    cancelled[i] = right[i] - ratio * left[i];
		}
	    }

	    float cancelledRms = rms(cancelled);
	    float diff = mixedRms - cancelledRms;

//	    cerr << "for j = " << j << " diff = " << diff << endl;

	    if (diff > bestRmsDiff) {
		bestRmsDiff = diff;
		bestAzi = j;
		bestLeftGain = leftGain;
		bestRightGain = rightGain;
		for (int i = 0; i < m_blockSize; ++i) {
		    bestRemainder[i] = (mixed[i] - cancelled[i]);
		}
	    }
	}

	cerr << "mixedRms = " << mixedRms << ", bestAzi = " << bestAzi << ", bestRmsDiff = " << bestRmsDiff << endl;

	f.values[bestAzi + m_width] = bestRmsDiff;
	    
	for (int i = 0; i < m_blockSize; ++i) {
	    left[i] -= bestLeftGain * bestRemainder[i];
	    right[i] -= bestRightGain * bestRemainder[i];
	    mixed[i] = (left[i] + right[i]) / 2.f;
	}
    }

    cerr << "mixedRms after = " << mixedRms << endl;

    fs[0].push_back(f);

    return fs;
}

Azi::FeatureSet
Azi::getRemainingFeatures()
{
    return FeatureSet();
}

