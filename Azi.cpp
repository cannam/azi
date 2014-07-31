
#include "Azi.h"

#include <cmath>
#include <iostream>
#include <complex>

using std::vector;
using std::complex;
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
//    vector<complex<float> > left, right;
    vector<float> left, right;
    for (int i = 0; i <= m_blockSize/2; ++i) {
	left.push_back(std::abs
		       (complex<float>
			(inputBuffers[0][i*2], inputBuffers[0][i*2+1])));
	right.push_back(std::abs
			(complex<float>
			 (inputBuffers[1][i*2], inputBuffers[1][i*2+1])));
    }
    int n = left.size();

    vector<float> plan(m_width * 2 + 1, 0.f);
    vector<float> cancelled(m_width * 2 + 1);

    for (int i = 0; i < n; ++i) {

	for (int j = 0; j <= m_width * 2; ++j) {

	    float pan = float(j - m_width) / m_width;

	    float leftGain = 1.f, rightGain = 1.f;
	    if (pan > 0.f) leftGain *= 1.f - pan;
	    if (pan < 0.f) rightGain *= pan + 1.f;
	    
	    if (leftGain < rightGain) {
		float ratio = leftGain / rightGain;
		cancelled[j] = std::abs(left[i] - ratio * right[i]);
	    } else {
		float ratio = rightGain / leftGain;
		cancelled[j] = std::abs(right[i] - ratio * left[i]);
	    }
	}

	for (int j = 0; j <= m_width * 2; ++j) {

	    if ((j == 0 && cancelled[j] < cancelled[j+1]) ||
		(j == m_width*2 && cancelled[j] < cancelled[j-1]) ||
		(j > 0 && j < m_width*2 &&
		 cancelled[j] < cancelled[j-1] &&
		 cancelled[j] < cancelled[j+1])) {

		// local minimum

		plan[j] += 1;
	    }
	}
    }

    FeatureSet fs;
    Feature f;
    f.values = plan;

    fs[0].push_back(f);

//    cerr << "feature returning:" << endl;
//    for (int i = 0; i < fs[0][0].values.size(); ++i) {
//	cerr << fs[0][0].values[i] << " ";
//    }
//    cerr << endl;
    
    return fs;
}

Azi::FeatureSet
Azi::getRemainingFeatures()
{
    return FeatureSet();
}

