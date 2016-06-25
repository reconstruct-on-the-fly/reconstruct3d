#include "disparity_map.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>
#include <opencv2/ximgproc/edge_filter.hpp>
#include "opencv2/stitching.hpp"
#include <limits>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;

DisparityMap::DisparityMap(){}

DisparityMap::DisparityMap(Mat image): m_image(image){}

void
DisparityMap::preprocessImages(Mat &left, Mat &right)
{
    cvtColor(left, left, COLOR_BGR2GRAY);
    cvtColor(right, right, COLOR_BGR2GRAY);

    Ptr<CLAHE> clahe = createCLAHE(1, Size(2, 2));
    //clahe->apply(left, left);
    //clahe->apply(right, right);
    //GaussianBlur(left, left, Size(3, 3), 0);
    //GaussianBlur(right, right, Size(3, 3), 0);
    //imwrite("/vagrant/left_blur.jpg", left);
}

int
DisparityMap::sadAt(int i, int j, Mat left, Mat right, int window_size,
                    int offset)
{
    int half_window_size = (window_size - 1) / 2;
    int sad = 0;
    for(int ii = -half_window_size; ii < half_window_size; ++ii)
    {
        for(int jj = -half_window_size; jj < half_window_size; ++jj)
        {
            sad += abs(left.at<uchar>(i + ii, j + jj) -
                       right.at<uchar>(i + ii, j + jj + offset));
        }
    }

    return sad;
}

DisparityMap
DisparityMap::generateDisparityMap(
        cv::Mat left, cv::Mat right, std::string objname,
        int window_size, int min_disp, int max_disp,
        bool wls_filter, double wls_lambda, double wls_sigma,
        bool noise_reduction_filter, int noise_reduction_window_size,
        float noise_reduction_threshold
    )
{
    // Custom Params
    int half_window_size = (window_size - 1) / 2;

    DisparityMap::preprocessImages(left, right);

    /* Disparity Algorithm */
    const int rows = left.rows;
    const int cols = left.cols;

    Mat disparity = Mat(rows, cols, CV_32S);
    int best_disp = 0;

    for(int i = half_window_size; i < rows - half_window_size; ++i)
    {
        for(int j = half_window_size + max_disp; j < cols - half_window_size; ++j)
        {
            int min_sad = numeric_limits<int>::max();
            for(int disp = min_disp; (disp >= -max_disp) ; --disp)
            {
                int sad = DisparityMap::sadAt(i, j, left, right,
                                              window_size, disp);

                if(sad <= min_sad)
                {
                    min_sad = sad;
                    best_disp = abs(disp);
                }
            }
            disparity.at<int>(i, j) = best_disp;
        }
    }

    Rect roi(max_disp + half_window_size, half_window_size,
             cols - max_disp - half_window_size, rows - half_window_size);
    disparity = disparity(roi);
    disparity = DisparityMap::normalize_image(disparity, 30, 0.4);

    if(wls_filter)
    {
        double lambda = wls_lambda;
        double sigma  = wls_sigma;

        Ptr<DisparityWLSFilter> wls_filter;
        wls_filter = createDisparityWLSFilterGeneric(false);
        wls_filter->setLambda(lambda);
        wls_filter->setSigmaColor(sigma);

        Mat filtered_disp;
        disparity.convertTo(disparity, CV_16S);
        wls_filter->filter(disparity, left(roi), filtered_disp);
        normalize(filtered_disp, disparity, 0, 255, NORM_MINMAX, CV_8UC1);
    } else {
        normalize(disparity, disparity, 0, 255, NORM_MINMAX, CV_8UC1);
    }

    return DisparityMap(disparity);
}

Mat
DisparityMap::normalize_image(Mat image, int window_size, float threshold)
{
    int half_window_size = (window_size - 1) / 2;

    const int rows = image.rows;
    const int cols = image.cols;

    Mat normalized = Mat(rows, cols, CV_32S);

    for(int i = half_window_size; i < rows - half_window_size; ++i)
    {
        for(int j = half_window_size; j < cols - half_window_size; ++j)
        {
            int window_sum = 0;

            for(int ii = -half_window_size; ii < half_window_size; ++ii)
            {
                for(int jj = -half_window_size; jj < half_window_size; ++jj)
                {
                    window_sum += image.at<int>(i + ii, j + jj);
                }
            }

            float window_avg = float(window_sum)/float(window_size*window_size);

            auto pixel_value = image.at<int>(i, j);

            if (abs(1 - pixel_value/window_avg) >= threshold)
                normalized.at<int>(i, j) = (uchar) window_avg;
            else
                normalized.at<int>(i, j) = pixel_value;
        }
    }

    return normalized;
}

Mat
DisparityMap::getImage()
{
    return m_image;
}

DisparityMap
DisparityMap::merge(vector<Mat> maps)
{
    Mat merged_maps;

    Stitcher stitcher = Stitcher::createDefault();
    Stitcher::Status status = stitcher.stitch(maps, merged_maps);

    if (status != Stitcher::OK)
    {
        cout << "error found while stitching: " << status << endl;
    }

    return DisparityMap(merged_maps);
}
