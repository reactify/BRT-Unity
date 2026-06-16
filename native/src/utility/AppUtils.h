
#include <string>
#include "Logging.h"

#ifndef _APP_UTILS_HPP_
#define _APP_UTILS_HPP_

#define HRTFRESAMPLINGSTEP 15

class AppUtils
{
public:
    static bool LoadHRTFSofaFile(const std::string & _filePath, std::shared_ptr<BRTServices::CSphericalInterpolatedFIRTable> hrtf) {
                
        BRTReaders::CSOFAReader sofaReader;
        Common::CGlobalParameters globalParameters;

        int sampleRateInSOFAFile = sofaReader.GetSampleRateFromSofa(_filePath);
        if (sampleRateInSOFAFile == -1) {
            BRT_Log (2, "No sample rate in HRTF SOFA file");
            return false;
        }
        if (globalParameters.GetSampleRate() != sampleRateInSOFAFile)
        {
            BRT_Log (2, "The sample rate in HRTF SOFA file doesn't match the configuration.");
            return false;
        }
        BRT_Log (0, "Loading HRTF SOFA File.....");
        bool result = sofaReader.ReadHRTFFromSofa (_filePath, hrtf, HRTFRESAMPLINGSTEP, BRTServices::TEXTRAPOLATION_METHOD::nearest_point);
        if (result) {
            BRT_Log (0, "HRTF SOFA file loaded successfully.");
            return true;
        }
        else {
            BRT_Log (2, "Error loading HRTF SOFA file");
            return false;
        }
    }

    static bool LoadBRIRSofaFile(const std::string& _filePath, std::shared_ptr<BRTServices::CSphericalFIRTable> brir
        , float _fadeWindowThreshold, float _fadeInWindowRiseTime
        , float _fadeOutWindowThreshold, float _fadeOutWindowRiseTime) {

        BRTReaders::CSOFAReader sofaReader;
        Common::CGlobalParameters globalParameters;

        int sampleRateInSOFAFile = sofaReader.GetSampleRateFromSofa(_filePath);
        if (sampleRateInSOFAFile == -1) {
            BRT_Log (2, "No sample rate in BRIR SOFA file");
            return false;
        }
        if (globalParameters.GetSampleRate() != sampleRateInSOFAFile)
        {
            BRT_Log (2, "The sample rate in BRIR SOFA file doesn't match the configuration.");
            return false;
        }
        BRT_Log (0, "Loading BRIR SOFA File.....");
        bool result = sofaReader.ReadBRIRFromSofa(_filePath, brir, _fadeWindowThreshold, _fadeInWindowRiseTime, _fadeOutWindowThreshold, _fadeOutWindowRiseTime);
        if (result) {
            BRT_Log (0, "BRIR SOFA file loaded successfully.");
            return true;
        }
        else {
            BRT_Log (2, "Error loading BRIR SOFA file");
            return false;
        }
    }

    static bool LoadNearFieldSOSFilter(std::string _ildFilePath, std::shared_ptr<BRTServices::CSphericalSOSTable> _sosFilter) {
            
        BRTReaders::CSOFAReader sofaReader;
        Common::CGlobalParameters globalParameters;

        int sampleRateInSOFAFile = sofaReader.GetSampleRateFromSofa(_ildFilePath);
        if (sampleRateInSOFAFile == -1) {
            BRT_Log (2, "No sample rate in ILD SOFA file");
            return false;
        }
        if (globalParameters.GetSampleRate() != sampleRateInSOFAFile)
        {
            BRT_Log (2, "The sample rate in ILD SOFA file doesn't match the configuration");
            return false;
        }
        
        BRT_Log (0, "Loading ILD SOFA File.....");
        bool result = sofaReader.ReadSOSFiltersFromSofa(_ildFilePath, _sosFilter);
        if (result) {
            BRT_Log (0, "ILD SOFA file loaded successfully: ");
            return true;
        }
        else {
            BRT_Log (2, "Error loading ILD SOFA file");;
            return false;
        }
    }
    
    static bool LoadDirectivityTFSofaFile(std::string _directivityFilePath, std::shared_ptr<BRTServices::CSphericalInterpolatedFIRTable> _directivityTF) {
            
        BRTReaders::CSOFAReader sofaReader;
        Common::CGlobalParameters globalParameters;

        int sampleRateInSOFAFile = sofaReader.GetSampleRateFromSofa(_directivityFilePath);
        if (sampleRateInSOFAFile == -1) {
            BRT_Log (2, "No sample rate in DirectivityTF SOFA file");
            return false;
        }
        if (globalParameters.GetSampleRate() != sampleRateInSOFAFile)
        {
            BRT_Log (2, "The sample rate in DirectivityTF SOFA file doesn't match the configuration");
            return false;
        }
        
        BRT_Log (0, "Loading DirectivityTF SOFA File.....");
        bool result = sofaReader.ReadDirectivityFromSofa(_directivityFilePath, _directivityTF, HRTFRESAMPLINGSTEP, BRTServices::TEXTRAPOLATION_METHOD::nearest_point);
        if (result) {
            BRT_Log (0, "DirectivityTF SOFA file loaded successfully: ");
            return true;
        }
        else {
            BRT_Log (2, "Error loading DirectivityTF SOFA file");
            return false;
        }
    }
    
private:

};
#endif


template <class T>
void WriteLog (std::string logText, const T& value, std::string sourceID = "")
{
  #ifdef DEBUG_LOG_CATx
    std::ostringstream os;
    os << logtext << value;
    string fulltext = os.str();
    __android_log_print(ANDROID_LOG_DEBUG, "BRT", fulltext.c_str());
  #else
    std::cerr << logText << " " << value;
    std::cerr << " (source " << sourceID << ")";
    std::cerr << std::endl;
  #endif
}

inline Common::CTransform ComputeListenerTransformFromMatrix(float* listenerMatrix, float scale)
{
    // SET LISTENER POSITION

    // Inverted 4x4 listener matrix, as provided by Unity
    float L[16];
    for (int i = 0; i < 16; i++)
        L[i] = listenerMatrix[i];

    float listenerpos_x = -(L[0] * L[12] + L[1] * L[13] + L[2] * L[14]) * scale;    // From Unity documentation, if listener is rotated
    float listenerpos_y = -(L[4] * L[12] + L[5] * L[13] + L[6] * L[14]) * scale;    // From Unity documentation, if listener is rotated
    float listenerpos_z = -(L[8] * L[12] + L[9] * L[13] + L[10] * L[14]) * scale;    // From Unity documentation, if listener is rotated
    //float listenerpos_x = -L[12] * scale;    // If listener is not rotated
    //float listenerpos_y = -L[13] * scale;    // If listener is not rotated
    //float listenerpos_z = -L[14] * scale;    // If listener is not rotated
    Common::CTransform listenerTransform;
    listenerTransform.SetPosition(Common::CVector3(listenerpos_x, listenerpos_y, listenerpos_z));

    // SET LISTENER ORIENTATION

    //float w = 2 * sqrt(1.0f + L[0] + L[5] + L[10]);
    //float qw = w / 4.0f;
    //float qx = (L[6] - L[9]) / w;
    //float qy = (L[8] - L[2]) / w;
    //float qz = (L[1] - L[4]) / w;
    // http://forum.unity3d.com/threads/how-to-assign-matrix4x4-to-transform.121966/
    float tr = L[0] + L[5] + L[10];
    float w, qw, qx, qy, qz;
    if (tr > 0.0f)            // General case
    {
        w = sqrt(1.0f + tr) * 2.0f;
        qw = 0.25f * w;
        qx = (L[6] - L[9]) / w;
        qy = (L[8] - L[2]) / w;
        qz = (L[1] - L[4]) / w;
    }
    // Cases with w = 0
    else if ((L[0] > L[5]) && (L[0] > L[10]))
    {
        w = sqrt(1.0f + L[0] - L[5] - L[10]) * 2.0f;
        qw = (L[6] - L[9]) / w;
        qx = 0.25f * w;
        qy = -(L[1] + L[4]) / w;
        qz = -(L[8] + L[2]) / w;
    }
    else if (L[5] > L[10])
    {
        w = sqrt(1.0f + L[5] - L[0] - L[10]) * 2.0f;
        qw = (L[8] - L[2]) / w;
        qx = -(L[1] + L[4]) / w;
        qy = 0.25f * w;
        qz = -(L[6] + L[9]) / w;
    }
    else
    {
        w = sqrt(1.0f + L[10] - L[0] - L[5]) * 2.0f;
        qw = (L[1] - L[4]) / w;
        qx = -(L[8] + L[2]) / w;
        qy = -(L[6] + L[9]) / w;
        qz = 0.25f * w;
    }

    Common::CQuaternion unityQuaternion = Common::CQuaternion(qw, qx, qy, qz);
    listenerTransform.SetOrientation(unityQuaternion.Inverse());
    return listenerTransform;
}

inline Common::CTransform ComputeSourceTransformFromMatrix(float* M, float scale)
{
    Common::CTransform sourceTransform;

    // POSITION
    float x = M[12] * scale;
    float y = M[13] * scale;
    float z = M[14] * scale;

    sourceTransform.SetPosition(Common::CVector3(x, y, z));

    // ROTATION
    float tr = M[0] + M[5] + M[10];

    float qw, qx, qy, qz;

    if (tr > 0.0f)
    {
        float s = sqrtf(tr + 1.0f) * 2.0f;
        qw = 0.25f * s;
        qx = (M[6] - M[9]) / s;
        qy = (M[8] - M[2]) / s;
        qz = (M[1] - M[4]) / s;
    }
    else if (M[0] > M[5] && M[0] > M[10])
    {
        float s = sqrtf(1.0f + M[0] - M[5] - M[10]) * 2.0f;
        qw = (M[6] - M[9]) / s;
        qx = 0.25f * s;
        qy = (M[1] + M[4]) / s;
        qz = (M[2] + M[8]) / s;
    }
    else if (M[5] > M[10])
    {
        float s = sqrtf(1.0f + M[5] - M[0] - M[10]) * 2.0f;
        qw = (M[8] - M[2]) / s;
        qx = (M[1] + M[4]) / s;
        qy = 0.25f * s;
        qz = (M[6] + M[9]) / s;
    }
    else
    {
        float s = sqrtf(1.0f + M[10] - M[0] - M[5]) * 2.0f;
        qw = (M[1] - M[4]) / s;
        qx = (M[2] + M[8]) / s;
        qy = (M[6] + M[9]) / s;
        qz = 0.25f * s;
    }

    Common::CQuaternion q(qw, qx, qy, qz);
    sourceTransform.SetOrientation(q);

    return sourceTransform;
}
