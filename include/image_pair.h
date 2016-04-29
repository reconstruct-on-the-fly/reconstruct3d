#ifndef IMAGE_PAIR_H
#define IMAGE_PAIR_H

#include <opencv2/core/core.hpp>
#include "camera.h" 

class ImagePair{

public:
    ImagePair(cv::Mat img1, cv::Mat img2, cv::Mat R, cv::Vec<double, 3> T);
    ImagePair(cv::Mat img1, cv::Mat img2);
    ImagePair rectify(Camera camera, cv::Mat &Q);

private:
    cv::Mat m_img1, m_img2;
    cv::Mat m_R;
    cv::Vec<double, 3> m_T;
};

#endif
