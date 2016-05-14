#ifndef DISPARITY_MAP_H
#define DISPARITY_MAP_H

#include <opencv2/core/core.hpp>
#include "image_pair.h"

class DisparityMap{

public:
    DisparityMap();
    DisparityMap(cv::Mat image);

    static DisparityMap generateDisparityMap(cv::Mat left, cv::Mat right,
                                             bool gray_filter=true,
                                             bool no_filter=false);
    static DisparityMap generateDisparityMap(ImagePair imagePair,
                                             bool gray_filter=true,
                                             bool no_filter=false);

    cv::Mat getImage();

private:
    cv::Mat m_image;
};
#endif
