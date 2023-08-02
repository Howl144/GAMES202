#pragma once

#define NOMINMAX
#include <string>

#include "filesystem/path.h"

#include "util/image.h"
#include "util/mathutil.h"

struct FrameInfo {
  public:
    Buffer2D<Float3> m_beauty;
    Buffer2D<float> m_depth;
    Buffer2D<Float3> m_normal;
    Buffer2D<Float3> m_position;
    Buffer2D<float> m_id;
    std::vector<Matrix4x4> m_matrix;
};

class Denoiser_JBF {
 public:
    Denoiser_JBF(){};
    void Init(const FrameInfo &frameInfo, const Buffer2D<Float3> &filteredColor);
    void Maintain(const FrameInfo &frameInfo);
    void Reprojection(const FrameInfo &frameInfo);
    void TemporalAccumulation(const Buffer2D<Float3> &curFilteredColor);
    Buffer2D<Float3> Filter(const FrameInfo &frameInfo);
    Buffer2D<Float3> ATrousWaveletFilter(const FrameInfo &frameInfo);
    Buffer2D<Float3> ProcessFrame(const FrameInfo &frameInfo);
 public:
    float m_alpha = 0.2f;
    float m_sigmaPlane = 0.1f;
    //box
    float m_sigmaColor = 0.6f;
    //pink room
    //float m_sigmaColor = 10.0f;
    float m_sigmaNormal = 0.1f;
    float m_sigmaCoord = 32.0f;
    float m_colorBoxK = 1.0f;

 public:
    FrameInfo m_preFrameInfo;
    // 存储结果的buffer
    Buffer2D<Float3> m_accColor;
    // 临时存储结果的buffer
    Buffer2D<Float3> m_misc;
    // back projection 是否有效的buffer
    Buffer2D<bool> m_valid;
    bool m_useTemportal = false;
};

class Denoiser_SVGF {
public:
    Denoiser_SVGF(){};
    void Maintain(const FrameInfo &frameInfo);
    void Reprojection(const FrameInfo &frameInfo);
    void TemporalAccumulation(const FrameInfo &frameInfo);
    Buffer2D<Float3> ProcessFrame(const FrameInfo &frameInfo);
    void Init(const FrameInfo &frameInfo);
    bool isReprjValid(const float depth,const float preDepth,const float fwidthZ,
                      const Float3 normal, const Float3 preNormal,const float fwidthNormal,
                      const int meshId,const int preMeshId);
    void neighborhoodStandardDeviation(const FrameInfo &frameInfo,const int x,const int y,Float3& mean,Float3& stdDev);
    Float3 clipAABB(const Float3 radianceMin,const Float3 radianceMax,Float3& prevIllumination);
    bool loadPrevData(const int x,const int y,const int width,const int height, const FrameInfo &frameInfo,const int id,
                      Float3& prevIllumination, Float3& prevMoments,float& historyLength,
                      Matrix4x4 pre_World_To_Screen);
    float computeVarianceCenter(const Float3 &ipos, const int width, const int height);
    void ATrousWaveletFilter(const FrameInfo &frameInfo,const int stepSize);
    float normalWeight(const Float3 &center_normal, const Float3 &normal);
    float depthWeight(const float center_depth,const float depth,
                    const float dgrad,
                    const float offset_x,const float offset_y);
    float luminanceWeight(const float const center_lum,const float lum,const float variance);
public:
    float m_phiNormal = 128.0;
    float m_phiDepth = 1.0;
    float m_phiColor = 10.0;
    float m_Alpha = 0.05;
    float m_MomentsAlpha = 0.2;

    int m_FilterIterations = 4;
    bool m_useTemportal = false;
  public:
    FrameInfo m_preFrameInfo;

    Buffer2D<float> m_curFrameVariance;
    Buffer2D<float> m_tmpCurFrameVar;
    Buffer2D<float> m_hisFrameVariance;

    Buffer2D<int> m_historyLength;
    Buffer2D<int> m_tmpHisLength;
    
    Buffer2D<Float3> m_moments;
    // 存储结果的buffer
    Buffer2D<Float3> m_accColor;
    // 临时存储结果的buffer
    Buffer2D<Float3> m_tmpColor;
};