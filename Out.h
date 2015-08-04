
#ifndef AZI_OUT_H
#define AZI_OUT_H

#include <vector>

namespace breakfastquay {
class FFT;
class AudioWriteStream;
}

class Out
{
public:
    Out(int id, int blockSize, int stepSize, int sampleRate);
    ~Out();

    void put(int n, std::vector<float> leftSpec, std::vector<float> rightSpec);
    void flush(int n);
    
private:
    int m_id;
    int m_blockSize;
    int m_stepSize;
    int m_sampleRate;
    int m_nextn;
    std::vector<float> m_left;
    std::vector<float> m_right;
    std::vector<float> m_accumulator;
    std::vector<float> m_window;
    breakfastquay::FFT *m_fft;
    breakfastquay::AudioWriteStream *m_stream;
};

#endif
    
