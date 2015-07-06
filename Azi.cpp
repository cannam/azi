
#include "Azi.h"

#include <cmath>
#include <iostream>
#include <complex>
#include <numeric>
#include <algorithm>

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
    d.binCount = m_width * 2 + 3; // include a 1-bin "margin" at top and bottom
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

static double LONG_DISTANCE = 1e8;

double
Azi::distance(const vector<float> &vv1, const vector<float> &vv2)
{
    float thresh = 1e-6;
    float sum1 = accumulate(vv1.begin(), vv1.end(), 0.f);
    float sum2 = accumulate(vv2.begin(), vv2.end(), 0.f);
    if (sum1 < thresh || sum2 < thresh) {
	return LONG_DISTANCE;
    }
    double dist = 0.0;
    for (int i = 0; i < int(vv1.size()); ++i) {
	float v1 = vv1[i] / sum1;
	float v2 = vv2[i] / sum2;
	dist += fabs(v1 - v2);
    }
    return dist;
}

Azi::FeatureSet
Azi::process(const float *const *inputBuffers, Vamp::RealTime timestamp)
{
    vector<float> left, right;

    int n = int(m_blockSize/2 + 1);

    const float *inleft = inputBuffers[0];
    const float *inright = inputBuffers[1];

    vector<vector<float> > planSpec(m_width * 2 + 1, vector<float>(n, 0.f));

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
	    float abspan = 1.0 - (smaller / larger);
	    if (lmag > rmag) pan = -abspan;
	    else pan = abspan;
	}

	float leftGain = 1.f, rightGain = 1.f;
	if (pan > 0.f) leftGain *= 1.f - pan;
	if (pan < 0.f) rightGain *= pan + 1.f;

	float pos = -pan * m_width + m_width;
	float mag = leftGain * lmag + rightGain * rmag;

	float ipos = floorf(pos);
	if (ipos < 0) ipos = 0;
	if (ipos >= m_width * 2) ipos = m_width * 2 - 1;

	planSpec[int(ipos)][i] = mag * (1.f - (pos - ipos));
	planSpec[int(ipos) + 1][i] = mag * (pos - ipos);
    }

    vector<int> labels(m_width * 2 + 1, 0);

    if (m_prevPlanSpec.empty()) {
	m_prevPlanSpec = planSpec;
	m_labels = labels;
	return FeatureSet();
    }

    vector<float> plan(m_width * 2 + 3, 0.f);

    int maxoff = m_width / 16;
    
    double thresh = 40.0;

    for (int j = 0; j < m_width * 2 + 1; ++j) {

	int bestk = j;
	double bestdist = LONG_DISTANCE;

	double sum = accumulate(planSpec[j].begin(), planSpec[j].end(), 0.0);
	if (sum < thresh) {
	    labels[j] = 0;
	    continue;
	}
	
	for (int off = -maxoff; off <= maxoff; ++off) {
	    int k = j + off;
	    if (k < 0 || k >= m_width * 2 + 1) {
		continue;
	    }
	    double dist = distance(m_prevPlanSpec[k], planSpec[j]);
	    if (dist < bestdist) {
		bestdist = dist;
		bestk = k;
	    }
	}

	if (bestdist < LONG_DISTANCE) {
	    if (m_labels[bestk] == 0) {
		int nextLabel = 1;
		while (find(m_labels.begin(), m_labels.end(), nextLabel)
		       != m_labels.end()) {
		    nextLabel = (rand() >> 8) % (m_width * 2);
		}
		labels[j] = m_labels[bestk] = nextLabel;
	    } else {
		labels[j] = m_labels[bestk];
	    }
	}

	plan[j] = float(labels[j]);
    }
    
    FeatureSet fs;
    Feature f;
    f.values = plan;

    fs[0].push_back(f);

    m_prevPlanSpec = planSpec;
    m_labels = labels;
    
    return fs;
}

Azi::FeatureSet
Azi::getRemainingFeatures()
{
    return FeatureSet();
}

