#include "camera.h"
#include <fstream>
#include <cassert>


Camera::Camera(cv::Mat K, cv::Vec<double, 5> distortion_coefs) 
   : m_K(K), m_distortion_coefs(distortion_coefs) {}

Camera
Camera::createFromFile(std::string filepath)
{
    std::ifstream input_file(filepath);
    assert(input_file.good());

    cv::Mat1d K(3, 3);
    cv::Vec<double, 5> distortion_coefs;

    for (int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            double element;
            input_file >> element;
            K.at<double>(i, j) = element;
        }
    }

    for (int i = 0; i < 5; i++)
    {
        double element;
        input_file >> element;
        distortion_coefs[i] = element;
    }

    return Camera(K, distortion_coefs);
}

cv::Mat Camera::getCameraMatrix()
{
    return m_K;
}

cv::Vec<double, 5> Camera::getDistortionCoefs()
{
    return m_distortion_coefs;
}
