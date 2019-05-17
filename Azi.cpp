
#include "Azi.h"

#include <cmath>
#include <iostream>
#include <complex>
#include <algorithm>
#include <climits>

using std::vector;
using std::complex;
using std::cerr;
using std::endl;

Azi::Azi(float inputSampleRate) :
    Plugin(inputSampleRate),
    m_width(128)
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
    return "Chris Cannam";
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
    return FrequencyDomain;
}

size_t
Azi::getPreferredBlockSize() const
{
    return 8192;
}

size_t 
Azi::getPreferredStepSize() const
{
    return 256;
}

size_t
Azi::getMinChannelCount() const
{
    return 1; // pointless, but supported
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
Azi::getParameter(string) const
{
    return 0;
}

void
Azi::setParameter(string, float) 
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
Azi::selectProgram(string)
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
    d.binCount = m_width * 2 + 3; // include a 1-bin "margin" at top and bottom
    d.hasKnownExtents = false;
    d.isQuantized = false;
    d.sampleType = OutputDescriptor::OneSamplePerStep;
    d.hasDuration = false;
    list.push_back(d);

    return list;
}

bool
Azi::initialise(size_t channels, size_t, size_t blockSize)
{
    if (channels < getMinChannelCount() ||
	channels > getMaxChannelCount()) return false;

    if (blockSize > INT_MAX) return false;

    m_channels = int(channels);
    m_blockSize = int(blockSize);

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
    return sqrtf(sum / float(buffer.size()));
}

Azi::FeatureSet
Azi::process(const float *const *inputBuffers, Vamp::RealTime)
{
    vector<float> left, right;

    int n = int(m_blockSize/2 + 1);

    vector<float> plan(m_width * 2 + 3, 0.f);

    const float *inleft = inputBuffers[0];
    const float *inright = (m_channels == 2 ? inputBuffers[1] : inleft);

    for (int i = 0; i < n; ++i) {

	int ri = i*2, ii = i*2 + 1;
	
	float lmag = sqrtf(inleft[ri] * inleft[ri] + inleft[ii] * inleft[ii]);
	float rmag = sqrtf(inright[ri] * inright[ri] + inright[ii] * inright[ii]);

	// lmag = 0.0, rmag = 1.0 -> min cancelled is at +1.0
	// lmag = 1.0, rmag = 0.0 -> at -1.0 [val at 0.0 = 1.0]
	// lmag = 0.5, rmag = 0.2 -> at -0.6 [val at 0.0 = 0.3]
	// lmag = 0.5, rmag = 1.0 -> at +0.5 [val at 0.0 = 0.5]
	// lmag = 1.0, rmag = 1.0 -> at +0.0 

	// if lmag == rmag -> 0.0
	// else if lmag > rmag -> negative
	// else -> positive

	// val at 0.0 = larger - smaller
	// mag of null = 1.0 - (smaller / larger)

	float larger = std::max(lmag, rmag);
	float smaller = std::min(lmag, rmag);

	float pan = 0.0;

	if (larger > smaller) {
	    float abspan = 1.f - (smaller / larger);
	    if (lmag > rmag) pan = -abspan;
	    else pan = abspan;
	}

	float leftGain = 1.f, rightGain = 1.f;
	if (pan > 0.f) leftGain *= 1.f - pan;
	if (pan < 0.f) rightGain *= pan + 1.f;

        float wid = float(m_width);
	float pos = -pan * wid + wid;
	float mag = leftGain * lmag + rightGain * rmag;

	float ipos = floorf(pos);
	
	plan[int(ipos) + 1] += mag * (1.f - (pos - ipos));
	plan[int(ipos) + 2] += mag * (pos - ipos);
    }

    FeatureSet fs;
    Feature f;
    f.values = plan;

    fs[0].push_back(f);

    return fs;
}

Azi::FeatureSet
Azi::getRemainingFeatures()
{
    return FeatureSet();
}

