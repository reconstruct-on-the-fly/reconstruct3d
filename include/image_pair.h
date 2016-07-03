#ifndef IMAGE_PAIR_H
#define IMAGE_PAIR_H

#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

class ImagePair{

public:
    ImagePair(cv::Mat _img1, cv::Mat _img2);

    ImagePair();

    ImagePair rectify(std::string obj_name);

    cv::Mat img1, img2;

private:
    void drawEpipolarLines(std::vector<cv::Point2f> points1,
                           std::vector<cv::Point2f> points2, cv::Mat F,
                           std::string obj_name);
};

#endif
