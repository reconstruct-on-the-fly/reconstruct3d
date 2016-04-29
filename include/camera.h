#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/core/core.hpp>
#include <string>

class Camera {
public:
    Camera(cv::Mat K, cv::Vec<double, 5> distortion_coefs);

    static Camera createFromFile(std::string filepath);
    cv::Mat getCameraMatrix();
    cv::Vec<double, 5> getDistortionCoefs();

private:
   cv::Mat m_K;                           // Camera Matrix
   cv::Vec<double, 5> m_distortion_coefs; // Distotion Coeficients

};

#endif
