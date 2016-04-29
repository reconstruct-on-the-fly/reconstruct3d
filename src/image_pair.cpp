#include "image_pair.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;
using namespace std;

ImagePair::ImagePair(Mat img1, Mat img2, Mat R, Vec<double, 3> T)
    : m_img1(img1), m_img2(img2), m_R(R), m_T(T) {}

ImagePair::ImagePair(cv::Mat img1, cv::Mat img2)
    : ImagePair(img1, img2, Mat1d(3,3), Vec<double, 3>()) {}

ImagePair
ImagePair::rectify(Camera camera, Mat &Q)
{
    Mat R1, R2, P1, P2;

    stereoRectify(camera.getCameraMatrix(), camera.getDistortionCoefs(),
        camera.getCameraMatrix(), camera.getDistortionCoefs(), m_img1.size(),
        m_R, m_T, R1, R2, P1, P2, Q,
        CALIB_ZERO_DISPARITY, 1, m_img1.size());

    Mat rmap1, rmap2, rmap3, rmap4;

    initUndistortRectifyMap(camera.getCameraMatrix(),
        camera.getDistortionCoefs(), R1, P1, m_img1.size(), CV_16SC2, rmap1,
        rmap2);
    initUndistortRectifyMap(camera.getCameraMatrix(),
        camera.getDistortionCoefs(), R2, P2, m_img2.size(), CV_16SC2, rmap3,
        rmap4);

    Mat r_img1, r_img2;

    remap(m_img1, r_img1, rmap1, rmap2, INTER_LINEAR);
    remap(m_img2, r_img2, rmap3, rmap4, INTER_LINEAR);

    return ImagePair(r_img1, r_img2);
}

cv::Mat
ImagePair::getImage1()
{
    return m_img1;
}

cv::Mat
ImagePair::getImage2()
{
    return m_img2;
}
