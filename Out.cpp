
#include "Out.h"

#include <bqfft/FFT.h>
#include <bqaudiostream/AudioWriteStream.h>
#include <bqaudiostream/AudioWriteStreamFactory.h>
#include <iostream>
#include <vector>
#include <string>

#include <cstdio>
#include <cmath>

#include "bqvec/VectorOps.h"

using namespace breakfastquay;
using namespace std;

Out::Out(int id, int blockSize, int stepSize, int sampleRate) :
    m_id(id),
    m_blockSize(blockSize),
    m_stepSize(stepSize),
    m_sampleRate(sampleRate)
{
    m_nextn = 0;
    m_left.resize(m_blockSize);
    m_right.resize(m_blockSize);
    m_accumulator.resize(2 * (m_blockSize + m_stepSize));
    m_fft = new FFT(m_blockSize);
    m_window.resize(m_blockSize);
    for (int i = 0; i < m_blockSize; ++i) {
	m_window[i] = 0.5 - 0.5 * cos((2.0 * M_PI * i) / m_blockSize);
    }
    char buffer[50];
    sprintf(buffer, "stream-%d.wav", id);
    m_stream = AudioWriteStreamFactory::createWriteStream
	(buffer, 2, m_sampleRate);
}

Out::~Out()
{
    //!!! + flush
    delete m_fft;
    delete m_stream;
}

void
Out::put(int n, vector<float> leftSpec, vector<float> rightSpec)
{
    cerr << "put: id = " << m_id << ", n = " << n << endl;
    
    if (n > m_nextn) {
	vector<float> empty(m_blockSize + 2, 0.f);
	for (int i = 0; i < n; ++i) {
	    put(i, empty, empty);
	}
    }

    m_stream->putInterleavedFrames(m_stepSize, m_accumulator.data());
    m_accumulator = vector<float>(m_accumulator.begin() + m_stepSize * 2,
				  m_accumulator.end());
    m_accumulator.resize(2 * (m_blockSize + m_stepSize));
	
    m_fft->inverseInterleaved(leftSpec.data(), m_left.data());
    m_fft->inverseInterleaved(rightSpec.data(), m_right.data());

    v_fftshift(m_left.data(), m_blockSize);
    v_fftshift(m_right.data(), m_blockSize);
    
    v_scale(m_left.data(), 1.0 / m_blockSize, m_blockSize);
    v_scale(m_right.data(), 1.0 / m_blockSize, m_blockSize);

    v_multiply(m_left.data(), m_window.data(), m_blockSize);
    v_multiply(m_right.data(), m_window.data(), m_blockSize);

    for (int i = 0; i < m_blockSize; ++i) {
	m_accumulator[m_stepSize * 2 + i * 2] += m_left[i];
	m_accumulator[m_stepSize * 2 + i * 2 + 1] += m_right[i];
    }
    
    m_nextn = n + 1;
}
