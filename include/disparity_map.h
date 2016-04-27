#ifndef DISPARITY_MAP_H
#define DISPARITY_MAP_H

#include <opencv2/core/core.hpp>

class DisparityMap{

public:
    DisparityMap();
    DisparityMap(cv::Mat image);

    static DisparityMap generateDisparityMap(cv::Mat left, cv::Mat right);

    cv::Mat getImage();

private:
    cv::Mat m_image;
};
#endif
