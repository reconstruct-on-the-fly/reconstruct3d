#ifndef DISPARITY_MAP_H
#define DISPARITY_MAP_H

#include <opencv2/core/core.hpp>
#include "image_pair.h"

class DisparityMap{

public:
    DisparityMap();
    DisparityMap(cv::Mat image);

    static DisparityMap generateDisparityMap(cv::Mat left, cv::Mat right,
                                             bool no_filter=false);
    static DisparityMap generateDisparityMap(ImagePair imagePair,
                                             bool no_filter=false);
    static void preprocessImages(cv::Mat &left, cv::Mat &right);
    static int sadAt(int i, int j, cv::Mat left, cv::Mat right,
                     int window_size, int offset);
    static DisparityMap merge(std::vector<cv::Mat> maps);

    cv::Mat getImage();

private:
    cv::Mat m_image;
};
#endif
