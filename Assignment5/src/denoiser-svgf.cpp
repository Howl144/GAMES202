#include "denoiser.h"
#include <algorithm>
#define NORMAL_DISTANCE 0.1f
#define PLANE_DISTANCE 5.0f

bool Denoiser_SVGF::isReprjValid(const float depth,const float preDepth,const float fwidthZ,
                                 const Float3 normal, const Float3 preNormal,const float fwidthNormal,
                                 const int meshId,const int preMeshId) {

    // check if deviation of depths is acceptable
    if (std::fabs(depth - preDepth) / (fwidthZ + 1e-2) > 10.0f)
        return false;

    // check normals for compatibility
    if (Distance(normal, preNormal) / (fwidthNormal + 1e-2) > 16.0)
        return false;

    // Since the grayscale is the result of direct path-tracing, there is a mutual contribution that can occur without meshid.
    //To mitigate this, add meshid, which can cause other problems such as artifacts.
    if (meshId != preMeshId)
        return false;

    return true;
}

bool Denoiser_SVGF::loadPrevData(const int x,const int y,const int width,const int height, const FrameInfo &frameInfo,const int id,
                                 Float3& prevIllumination,Float3& prevMoments, float& historyLength,
                                 Matrix4x4 pre_World_To_Screen) {
    Matrix4x4 world_to_local = Inverse(frameInfo.m_matrix[id]);
    Matrix4x4 pre_local_to_world = m_preFrameInfo.m_matrix[id];
    const auto world_position = frameInfo.m_position(x, y);
    const auto local_position = world_to_local(world_position, Float3::EType::Point);
    const auto pre_world_position = pre_local_to_world(local_position, Float3::EType::Point);
    const auto pre_screen_position = pre_World_To_Screen(pre_world_position, Float3::EType::Point);

    prevIllumination = Float3(0, 0, 0);
    prevMoments = Float3(0, 0, 0);
    historyLength = 0.0;
    bool v[4] = {false};
    //由于没有Float2，这里暂时用Float3代替
    const Float3 offset[4] = {Float3(0, 0, 0), Float3(1, 0, 0),
                              Float3(0, 1, 0), Float3(1, 1, 0)};

    if (x + 1 >= width || y + 1 >= height || 
        pre_screen_position.x >= width ||
        pre_screen_position.y >= height || 
        pre_screen_position.x < 0 ||
        pre_screen_position.y < 0)
        return false;

    const Float3 normal = frameInfo.m_normal(x, y);
    const float ddxN = Distance(frameInfo.m_normal(x + 1, y), normal);
    const float ddyN = Distance(frameInfo.m_normal(x, y + 1), normal);
    const float fwidthNormal = ddxN + ddyN;
    const float depth = frameInfo.m_depth(x, y);
    const float ddxZ = std::fabs(frameInfo.m_depth(x + 1, y) - depth);
    const float ddyZ = std::fabs(frameInfo.m_depth(x, y + 1) - depth);
    const float fwidthZ = ddxZ + ddyZ;
    
    // check for all 4 taps of the bilinear filter for validity
    bool valid = false;
    for (int sampleIdx = 0; sampleIdx < 1; sampleIdx++) {
        const Float3 loc = pre_screen_position + offset[sampleIdx];

        if (loc.x < 0 || loc.x >= width || loc.y < 0 || loc.y >= height)
            continue;

        const Float3 preNormal = m_preFrameInfo.m_normal(loc.x, loc.y);
        const float preDepth = m_preFrameInfo.m_depth(loc.x, loc.y);
        const int preMeshId = m_preFrameInfo.m_id(loc.x, loc.y);
  
        v[sampleIdx] = isReprjValid(depth,preDepth,fwidthZ,
                                    normal, preNormal,fwidthNormal,
                                    id, preMeshId);
  
        valid = valid || v[sampleIdx];
    }
    if (valid) {
        //float sumw = 0;
        //const float x =  frac(pre_screen_position.x);
        //const float y =  frac(pre_screen_position.y);

        ////bilinear weights
        //const float w[4] = {(1 - x) * (1 - y), x * (1 - y),
        //                    (1 - x) *  y     , x *  y};

        //// perform the actual bilinear interpolation
        //for (int sampleIdx = 0; sampleIdx < 4; sampleIdx++) {
        //    const Float3 loc = pre_screen_position + offset[sampleIdx];
        //    if (loc.x < 0 || loc.x >= width || loc.y < 0 || loc.y >= height)
        //        continue;
        //    if (v[sampleIdx]) {
        //        prevIllumination += Float3(w[sampleIdx]) * m_preFrameInfo.m_beauty(loc.x, loc.y);
        //        prevMoments += Float3(w[sampleIdx]) * m_hisMoments(loc.x, loc.y);
        //        sumw += w[sampleIdx];
        //    }
        //}

        //// redistribute weights in case not all taps were used
        //valid = (sumw >= 0.01);
        //prevIllumination = valid ? prevIllumination / sumw : Float3(0, 0, 0);
        //prevMoments = valid ? prevMoments / sumw : Float3(0, 0, 0);

        prevIllumination = m_preFrameInfo.m_beauty(pre_screen_position.x, pre_screen_position.y);
        prevMoments = m_hisMoments(pre_screen_position.x, pre_screen_position.y);
    }
    
    if (!valid) // perform cross-bilateral filter in the hope to find some suitable
                // samples somewhere
    {
        float nValid = 0.0;
        const int radius = 1;
        for (int yy = -radius; yy <= radius; yy++) {
            for (int xx = -radius; xx <= radius; xx++) {
                const Float3 loc = pre_screen_position + Float3(xx, yy,0);
                if (loc.x < 0 || loc.x >= width || loc.y < 0 || loc.y >= height)
                    continue;
                const Float3 preWorldPos = m_preFrameInfo.m_position(loc.x, loc.y);
                const Float3 preNormal = m_preFrameInfo.m_normal(loc.x, loc.y);
                const float preDepth = m_preFrameInfo.m_depth(loc.x, loc.y);
                const int preMeshId = m_preFrameInfo.m_id(loc.x, loc.y);
                if (isReprjValid(depth, preDepth, fwidthZ, 
                                 normal,preNormal, fwidthNormal,
                                 id, preMeshId)) {
                    prevIllumination += m_preFrameInfo.m_beauty(loc.x, loc.y);
                    prevMoments += m_hisMoments(loc.x, loc.y);
                    nValid += 1.0;
                }
            }
        }
        if (nValid > 0) {
            valid = true;
            prevIllumination /= nValid;
            prevMoments /= nValid;
        }
    }

    if (valid) {
        historyLength = m_historyLength(pre_screen_position.x, pre_screen_position.y);
    }
    return valid;
}

void Denoiser_SVGF::neighborhoodStandardDeviation(const FrameInfo &frameInfo,const int x, const int y, Float3& mean, Float3& stdDev) {
    Float3 m1;
    Float3 m2;

    const int radius = 8;
    const float weight = (radius * 2.0f + 1.0) * (radius * 2.0f + 1.0);

    for (int dx = -radius; dx <= radius; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
            Float3 sampleCoord = Float3(x,y,0) + Float3(dx, dy,0);
            Float3 sampleColor = frameInfo.m_beauty(sampleCoord.x, sampleCoord.y);

            m1 += sampleColor;
            m2 += sampleColor * sampleColor;
        }
    }
    mean = m1 / weight;
    Float3 variance = (m2 / weight) - (mean * mean);
    variance.x = std::fmax(variance.x, 0.0);
    variance.y = std::fmax(variance.y, 0.0);
    variance.z = std::fmax(variance.z, 0.0);
    stdDev.x = std::sqrt(variance.x);
    stdDev.y = std::sqrt(variance.y);
    stdDev.z = std::sqrt(variance.z);
}

Float3 Denoiser_SVGF::clipAABB(const Float3 aabbMin, const Float3 aabbMax,Float3 &prevIllumination) {
    Float3 aabbCenter = Float3(0.5) * (aabbMax + aabbMin);
    Float3 extentClip = Float3(0.5) * (aabbMax - aabbMin) + 0.001f;
    Float3 colorVector = prevIllumination - aabbCenter;
    Float3 colorVectorClip = colorVector / extentClip;
    colorVectorClip = Abs(colorVectorClip);
    float maxAbsUnit = std::fmax(std::fmax(colorVectorClip.x, colorVectorClip.y), colorVectorClip.z);
    if (maxAbsUnit > 1.0)
        return aabbCenter + colorVector / maxAbsUnit;
    return prevIllumination;
}

void Denoiser_SVGF::Reprojection(const FrameInfo &frameInfo) {
    const int height = frameInfo.m_beauty.m_height;
    const int width = frameInfo.m_beauty.m_width;
    Matrix4x4 pre_World_To_Screen =
        m_preFrameInfo.m_matrix[m_preFrameInfo.m_matrix.size() - 1];
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            // TODO: Reproject
            const int id = frameInfo.m_id(x, y);
            if (id == -1) {
                continue;
            }

            Float3 illumination = frameInfo.m_beauty(x, y);
            float historyLength;
            Float3 prevIllumination;
            //由于没有Float2，这里暂时用Float3代替
            Float3 prevMoments;
            bool success = loadPrevData(x, y,width,height, frameInfo,id,
                                        prevIllumination, prevMoments, historyLength,
                                        pre_World_To_Screen);

            historyLength = std::fmin(32.0, success ? historyLength + 1.0f : 1.0f);

            if (success) {
                Float3 stdDev;
                Float3 mean;
                neighborhoodStandardDeviation(frameInfo, x, y, mean, stdDev);
                Float3 aabbMin = mean - stdDev;
                Float3 aabbMax = mean + stdDev;
                prevIllumination = clipAABB(aabbMin, aabbMax, prevIllumination);
            }

            // this adjusts the alpha for the case where insufficient history is available.
            // It boosts the temporal accumulation to give the samples equal weights in the beginning.
            const float alpha = success ? std::fmax(m_Alpha, 1.0 / historyLength) : 1.0;
            const float alphaMoments = success ? std::fmax(m_MomentsAlpha, 1.0 / historyLength) : 1.0;



            // compute first two moments of luminance
            Float3 moments;
            moments.x = Luminance(illumination);
            moments.y = moments.x * moments.x;

            // temporal integration of the moments
            moments = Lerp(prevMoments, moments, alphaMoments);

            float variance = std::fmax(0.f, moments.y - moments.x * moments.x);
            // temporal integration of illumination

            m_accColor(x, y) = Lerp(prevIllumination, illumination, alpha);
            m_curFrameVariance(x,y) = variance;
            m_moments(x, y) = moments;
            m_tmpHisLength(x,y) = historyLength;
        }
    }
    std::swap(m_tmpHisLength, m_historyLength);
}

void Denoiser_SVGF::VarianceEstimation(const FrameInfo &frameInfo) {
    const int height = frameInfo.m_beauty.m_height;
    const int width = frameInfo.m_beauty.m_width;
    const int radius = 3;
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            const Float3 ipos = Float3(x, y, 0);
            float h = m_historyLength(ipos.x, ipos.y);
            if (h < 4.0){// not enough temporal history available
                float sumWIllumination = 0.0;
                Float3 sumIllumination = Float3(0, 0, 0);
                Float3 sumMoments = Float3(0, 0, 0);

                const Float3 illuminationCenter = m_accColor(ipos.x, ipos.y);
                const float lIlluminationCenter = Luminance(illuminationCenter);

                const Float3 nCenter = frameInfo.m_normal(ipos.x, ipos.y);
                const float zCenter = frameInfo.m_depth(ipos.x, ipos.y);
                // depth-gradient estimation from screen-space derivatives
                float dgrad_x = std::fmax(1e-8, std::fabs(frameInfo.m_depth(ipos.x + 1, ipos.y) - zCenter)) * 3.0;
                float dgrad_y = std::fmax(1e-8, std::fabs(frameInfo.m_depth(ipos.x, ipos.y + 1) - zCenter)) * 3.0;
                float maxDgrad = std::fmax(dgrad_x, dgrad_y);

                // compute first and second moment spatially. This code also applies cross-bilateral
                // filtering on the input illumination.

                for (int yy = -radius; yy <= radius; yy++) {
                    for (int xx = -radius; xx <= radius; xx++) {
                        const Float3 p = ipos + Float3(xx, yy,0);
                        bool inside = false;
                        if (p.x >= 0 && p.y >= 0 && p.x < width && p.y < height)
                            inside = true;

                        if (inside) {
                            const Float3 illuminationP = m_accColor(p.x, p.y);
                            const Float3 momentsP = m_moments(p.x, p.y);
                            const float lIlluminationP = Luminance(illuminationP);
                            const float zP = frameInfo.m_depth(p.x, p.y);
                            const Float3 nP = frameInfo.m_normal(p.x,p.y);

                            // calculate the normal, depth and luminance weights
                            float nw = normalWeight(nCenter, nP);
                            float dw = depthWeight(zCenter, zP, maxDgrad,xx, yy);
                            float lw = luminanceWeight(lIlluminationCenter,
                                                       lIlluminationP, 1.0);
                            float w = nw * dw * lw;
                            sumWIllumination += w;
                            sumIllumination += illuminationP * w;
                            sumMoments += momentsP * w;
                        }
                    }
                }

                // Clamp sum to >0 to avoid NaNs.
                sumWIllumination = std::fmax(sumWIllumination, 1e-6f);
                sumIllumination /= sumWIllumination;
                sumMoments /= sumWIllumination;

                // compute variance using the first and second moments
                float variance = sumMoments.y - sumMoments.x * sumMoments.x;

                // give the variance a boost for the first frames
                if (h != 0)
                    variance *= 4.0 / h;

                m_tmpColor(ipos.x, ipos.y) = sumIllumination;
                m_curFrameVariance(ipos.x, ipos.y) = variance;
            } else {
                // do nothing
                m_tmpColor(ipos.x, ipos.y) = m_accColor(ipos.x, ipos.y);
            }
        }
    }
    std::swap(m_tmpColor, m_accColor);
}

float Denoiser_SVGF::normalWeight(const Float3 &center_normal, const Float3 &normal) {
    return std::pow(std::fmax(0.0, Dot(center_normal, normal)), m_phiNormal);
}

float Denoiser_SVGF::depthWeight(const float center_depth, const float depth,
                                 const float dgrad, 
                                 const float offset_x, const float offset_y) {
    const float eps = 1e-8;
    Float3 offset{offset_x, offset_y,0};
    return std::exp( (-std::fabs(center_depth - depth)) / (std::fabs( m_phiDepth * dgrad * Length(offset)+ eps)) );
}

float Denoiser_SVGF::luminanceWeight(const float center_lum, const float lum,
                                     const float variance) {
    const float eps = 1e-10;
    return std::exp( (-std::fabs(center_lum - lum)) /
               (m_phiColor * std::sqrt(std::fmax(0.0, variance) + eps)));
}

float Denoiser_SVGF::computeVarianceCenter(const Float3& ipos,const int width,const int height) {
    float sum = 0.f;

    const float kernel[2][2] = {{1.0 / 4.0, 1.0 / 8.0}, {1.0 / 8.0, 1.0 / 16.0}};

    const int radius = 1;
    for (int yy = -radius; yy <= radius; yy++) {
        for (int xx = -radius; xx <= radius; xx++) {
            Float3 p = ipos + Float3(xx, yy,0);
            p.x = std::fmin(std::fmax(p.x, 0.0), width - 1.0);
            p.y = std::fmin(std::fmax(p.y, 0.0), height - 1.0);
            const float k = kernel[std::abs(xx)][std::abs(yy)];
            sum += m_curFrameVariance(p.x,p.y) * k;
        }
    }

    return sum;
}

void Denoiser_SVGF::ATrousWaveletFilter(const FrameInfo &frameInfo,const int stepSize) {
    const int height = frameInfo.m_beauty.m_height;
    const int width = frameInfo.m_beauty.m_width;
    const float kernelWeights[3] = {1.0, 2.0 / 3.0, 1.0 / 6.0};
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: SVGF
            const Float3 ipos = Float3(x, y, 0);
            const Float3 illuminationCenter = m_accColor(ipos.x, ipos.y);
            const float lIlluminationCenter = Luminance(illuminationCenter);

            //variance,filtered using 3 * 3 gaussin blur
            const float var = computeVarianceCenter(ipos,width,height);

            const float zCenter = frameInfo.m_depth(ipos.x, ipos.y);
            const Float3 nCenter = frameInfo.m_normal(ipos.x, ipos.y);

            float dgrad_x = std::fmax(1e-8, std::fabs(frameInfo.m_depth(ipos.x + 1, ipos.y) - zCenter)) * stepSize;
            float dgrad_y = std::fmax(1e-8, std::fabs(frameInfo.m_depth(ipos.x, ipos.y + 1) - zCenter)) * stepSize;
            
            float maxDgrad = std::fmax(dgrad_x, dgrad_y);

            //explicitly store/accumulate center pixel with weight 1 to prevent issues
            //with the edge-stopping functions
            float sumWIllumination = 1.0;
            float sumVariance = m_curFrameVariance(ipos.x, ipos.y);
            Float3 sumIllumination = illuminationCenter;

            for (int yy = -2; yy <= 2; yy++) {
                for (int xx = -2; xx <= 2; xx++) {
                    const Float3 p = ipos + Float3(xx, yy,0) * stepSize;
                    bool inside = false;
                    if (p.x >= 0 && p.y >= 0 && p.x < width && p.y < height)
                        inside = true;

                    const float kernel = kernelWeights[std::abs(xx)] * kernelWeights[std::abs(yy)];

                    if (inside &&
                        (xx != 0 ||
                         yy != 0)) // skip center pixel, it is already accumulated
                    {
                        const Float3 illuminationP = m_accColor(p.x,p.y);
                        const float lIlluminationP = Luminance(illuminationP);
                        const float zP = frameInfo.m_depth(p.x,p.y);
                        const Float3 nP = frameInfo.m_normal(p.x,p.y);
                        const float varP = m_curFrameVariance(p.x,p.y);

                        // calculate the normal, depth and luminance weights
                        float nw = normalWeight(nCenter, nP);
                        float dw = depthWeight(zCenter, zP, maxDgrad, xx, yy);
                        float lw = luminanceWeight(lIlluminationCenter, lIlluminationP, var);

                        const float wIllumination = nw * dw * lw * kernel;

                        // alpha channel contains the variance, therefore the weights need
                        // to be squared, see paper for the formula(4.3 (1) (2))
                        sumWIllumination += wIllumination;
                        sumIllumination += Float3(wIllumination) * illuminationP;
                        sumVariance += wIllumination * wIllumination * varP;
                    }
                }
            }

            // renormalization is different for variance, check paper for the formula
            m_tmpColor(ipos.x, ipos.y) = sumIllumination / sumWIllumination;
            m_tmpCurFrameVar(ipos.x, ipos.y) = sumVariance / (sumWIllumination * sumWIllumination);
        }
    }
    std::swap(m_tmpColor,m_accColor);
    std::swap(m_tmpCurFrameVar,m_curFrameVariance);
}

void Denoiser_SVGF::Init(const FrameInfo &frameInfo) {
    const int height = frameInfo.m_beauty.m_height;
    const int width = frameInfo.m_beauty.m_width;
    m_accColor = CreateBuffer2D<Float3>(width, height);
    m_tmpColor = CreateBuffer2D<Float3>(width, height);
    m_curFrameVariance = CreateBuffer2D<float>(width, height);
    m_tmpCurFrameVar = CreateBuffer2D<float>(width, height);
    m_hisMoments = CreateBuffer2D<Float3>(width, height);
    m_moments = CreateBuffer2D<Float3>(width, height);
    m_historyLength = CreateBuffer2D<int>(width, height);
    m_tmpHisLength = CreateBuffer2D<int>(width, height);
}

void Denoiser_SVGF::Maintain(const FrameInfo &frameInfo) { 
    m_preFrameInfo = frameInfo; 
    m_preFrameInfo.m_beauty.Copy(m_accColor);
    m_hisMoments.Copy(m_moments);
}

Buffer2D<Float3> Denoiser_SVGF::ProcessFrame(const FrameInfo &frameInfo) {

    /*
    Reproject:
        - takes: frameInfo,m_preFrameInfo, m_hisMoments, m_historyLength
        returns: illumination,variance, moments, historyLength
    Variance/filter moments:
        - takes: frameInfo,m_accColor,m_moments, m_historyLength,
        - returns: filtered illumination + variance 
    a-trous:
        - takes: frameInfo,m_accColor,m_curFrameVariance
        - returns: final color
    */
     if (m_useTemportal) {
         Reprojection(frameInfo);
         VarianceEstimation(frameInfo);
     } else {
         Init(frameInfo);
         m_preFrameInfo = frameInfo;
         Reprojection(frameInfo);
         VarianceEstimation(frameInfo);
     }

    for (int i = 0; i < m_FilterIterations; i++) {
        int stepSize = 1 << i;
        ATrousWaveletFilter(frameInfo, stepSize);
    }

    // Maintain
    Maintain(frameInfo);
    if (!m_useTemportal) {
        m_useTemportal = true;
    }

    return m_accColor;
}
