#ifndef AZI_H
#define AZI_H

#include <vamp-sdk/Plugin.h>

using std::string;


class Azi : public Vamp::Plugin
{
public:
    Azi(float inputSampleRate);
    virtual ~Azi();

    string getIdentifier() const;
    string getName() const;
    string getDescription() const;
    string getMaker() const;
    int getPluginVersion() const;
    string getCopyright() const;

    InputDomain getInputDomain() const;
    size_t getPreferredBlockSize() const;
    size_t getPreferredStepSize() const;
    size_t getMinChannelCount() const;
    size_t getMaxChannelCount() const;

    ParameterList getParameterDescriptors() const;
    float getParameter(string identifier) const;
    void setParameter(string identifier, float value);

    ProgramList getPrograms() const;
    string getCurrentProgram() const;
    void selectProgram(string name);

    OutputList getOutputDescriptors() const;

    bool initialise(size_t channels, size_t stepSize, size_t blockSize);
    void reset();

    FeatureSet process(const float *const *inputBuffers,
                       Vamp::RealTime timestamp);

    FeatureSet getRemainingFeatures();

protected:
    int m_width;
    int m_blockSize;

    std::vector<std::vector<float> > m_prevPlanSpec;
    std::vector<int> m_labels;

    double distance(const std::vector<float> &vv1, const std::vector<float> &vv2);
    
    float rms(const std::vector<float> &);
};



#endif
