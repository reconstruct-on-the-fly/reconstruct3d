#include "image_pair.h"

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace cv;
using namespace std;

ImagePair::ImagePair() {}

ImagePair::ImagePair(Mat _img1, Mat _img2)
    : img1(_img1), img2(_img2) {}

cv::Rect findROI(cv::Mat image)
{
    const unsigned int rows = image.rows;
    const unsigned int cols = image.cols;

    size_t x0 = cols, y0 = rows;
    size_t x1 = 0, y1 = 0;

    for(size_t i = 0; i < rows; ++i)
    {
        for(size_t j = 0; j < cols; ++j)
        {
            int sum = 0;
            for (int ch = 0; ch < image.channels(); ch++)
                sum += image.data[image.step[0] * i + image.step[1] * j + ch];

            if(sum != 0)
            {
                x0 = min(x0, j);
                y0 = min(y0, i);
                x1 = max(x1, j);
                y1 = max(y1, i);
            }
        }
    }

    cout << "Valid ROI (" << x0 << ", " << y0 << ") to (" << x1 << ", " << y1 << ")\n";

    return cv::Rect(x0, y0, x1 - x0, y1 - y0);
}

// Code from: http://stackoverflow.com/questions/6087241/opencv-warpperspective/8229116#8229116

// Convert a vector of non-homogeneous 2D points to a vector of homogenehous 2D points.
void to_homogeneous(const std::vector< cv::Point2f >& non_homogeneous, std::vector< cv::Point3f >& homogeneous)
{
    homogeneous.resize(non_homogeneous.size());
    for (size_t i = 0; i < non_homogeneous.size(); i++) {
        homogeneous[i].x = non_homogeneous[i].x;
        homogeneous[i].y = non_homogeneous[i].y;
        homogeneous[i].z = 1.0;
    }
}

// Convert a vector of homogeneous 2D points to a vector of non-homogenehous 2D points.
void from_homogeneous(const std::vector< cv::Point3f >& homogeneous, std::vector< cv::Point2f >& non_homogeneous)
{
    non_homogeneous.resize(homogeneous.size());
    for (size_t i = 0; i < non_homogeneous.size(); i++) {
        non_homogeneous[i].x = homogeneous[i].x / homogeneous[i].z;
        non_homogeneous[i].y = homogeneous[i].y / homogeneous[i].z;
    }
}

// Transform a vector of 2D non-homogeneous points via an homography.
std::vector<cv::Point2f> transform_via_homography(const std::vector<cv::Point2f>& points, const cv::Matx33f& homography)
{
    std::vector<cv::Point3f> ph;
    to_homogeneous(points, ph);
    for (size_t i = 0; i < ph.size(); i++) {
        ph[i] = homography*ph[i];
    }
    std::vector<cv::Point2f> r;
    from_homogeneous(ph, r);
    return r;
}

// Find the bounding box of a vector of 2D non-homogeneous points.
cv::Rect_<float> bounding_box(const std::vector<cv::Point2f>& p)
{
    cv::Rect_<float> r;
    float x_min = std::min_element(p.begin(), p.end(), [](const cv::Point2f& lhs, const cv::Point2f& rhs) {return lhs.x < rhs.x; })->x;
    float x_max = std::max_element(p.begin(), p.end(), [](const cv::Point2f& lhs, const cv::Point2f& rhs) {return lhs.x < rhs.x; })->x;
    float y_min = std::min_element(p.begin(), p.end(), [](const cv::Point2f& lhs, const cv::Point2f& rhs) {return lhs.y < rhs.y; })->y;
    float y_max = std::max_element(p.begin(), p.end(), [](const cv::Point2f& lhs, const cv::Point2f& rhs) {return lhs.y < rhs.y; })->y;
    return cv::Rect_<float>(x_min, y_min, x_max - x_min, y_max - y_min);
}

// Warp the image src into the image dst through the homography H.
// The resulting dst image contains the entire warped image, this
// behaviour is the same of Octave's imperspectivewarp (in the 'image'
// package) behaviour when the argument bbox is equal to 'loose'.
// See http://octave.sourceforge.net/image/function/imperspectivewarp.html
void homography_warp(const cv::Mat& src, const cv::Mat& H, cv::Mat& dst)
{
    std::vector< cv::Point2f > corners;
    corners.push_back(cv::Point2f(0, 0));
    corners.push_back(cv::Point2f(src.cols, 0));
    corners.push_back(cv::Point2f(0, src.rows));
    corners.push_back(cv::Point2f(src.cols, src.rows));

    std::vector< cv::Point2f > projected = transform_via_homography(corners, H);
    cv::Rect_<float> bb = bounding_box(projected);

    cv::Mat_<double> translation = (cv::Mat_<double>(3, 3) << 1, 0, -bb.tl().x, 0, 1, -bb.tl().y, 0, 0, 1);

    cv::warpPerspective(src, dst, translation*H, bb.size());
}

ImagePair
ImagePair::rectify(std::string obj_name)
{
    cout << "Rectifying images..." << endl;

    Mat gray_img1(img1.size(), img1.type());
    Mat gray_img2(img2.size(), img2.type());

    // Grayscale images
    cvtColor(img1, gray_img1, COLOR_BGR2GRAY);
    cvtColor(img2, gray_img2, COLOR_BGR2GRAY);

    cout << "Matching Features..." << endl;
    Ptr<Feature2D> f2d = xfeatures2d::SURF::create();

    //-- Step 1: Detect the keypoints:
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    f2d->detect(gray_img1, keypoints_1);
    f2d->detect(gray_img2, keypoints_2);

    //-- Step 2: Calculate descriptors (feature vectors)
    Mat descriptors_1, descriptors_2;
    f2d->compute(gray_img1, keypoints_1, descriptors_1);
    f2d->compute(gray_img2, keypoints_2, descriptors_2);

    //-- Step 3: Matching descriptor vectors using BFMatcher :
    FlannBasedMatcher matcher;
    std::vector<DMatch> matches;
    matcher.match(descriptors_1, descriptors_2, matches);

    cout << matches.size() << " matches detected" << endl;

    double max_dist = 0, min_dist = 100, avg_dist = 0;
    int avg_count = 0;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_1.rows; i++ )
    {
        double dist = matches[i].distance;

        auto pt1 = keypoints_1[matches[i].queryIdx].pt;
        auto pt2 = keypoints_2[matches[i].trainIdx].pt;

        if (abs(pt1.y - pt2.y) < 20)
        {
            avg_dist += dist;
            ++avg_count;
        }

        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    avg_dist /= avg_count;

    // Selecting good matches
    std::vector<DMatch> good_matches;
    std::vector<Point2f> points1, points2;

    cout << "Selecting good matches..." << endl;
    for(int i = 0; i < descriptors_1.rows; i++)
    {
        if(abs(1 - matches[i].distance/avg_dist) <= 0.1)
        {
            auto pt1 = keypoints_1[matches[i].queryIdx].pt;
            auto pt2 = keypoints_2[matches[i].trainIdx].pt;

            if (abs(pt1.y - pt2.y) < 20)
            {
                good_matches.push_back(matches[i]);
                points1.push_back(pt1);
                points2.push_back(pt2);
            }
        }
    }

    //-- Draw only "good" matches
    Mat img_matches;
    drawMatches( img1, keypoints_1, img2, keypoints_2,
                 good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                 vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    imwrite(obj_name + "_feature_matching_points.jpg", img_matches);

    printf("# Features Matched: %lu ::  Max dist: %f :: Min dist: %f \n",
            good_matches.size(), max_dist, min_dist);

    cout << "Computing Fundamental Matrix..." << endl;
    std::vector<uchar> mask(points1.size(), 0);
    Mat F = findFundamentalMat(points1, points2, FM_LMEDS, 1.0, 0.99, mask);

    std::vector<Point2f> masked_points1, masked_points2;
    for (size_t i = 0; i < points1.size(); ++i)
    {
        if (mask[i] == 1)
        {
            masked_points1.push_back(points1[i]);
            masked_points2.push_back(points2[i]);
        }
    }

    cout << "Computing Rectification Matrix..." << endl;
    Mat lines_out1(img1.size(), img1.type());
    Mat lines_out2(img2.size(), img2.type());
    drawEpipolarLines(masked_points1, masked_points2, lines_out1, lines_out2,
                      F, obj_name);

    Mat H1_lines(4, 4, lines_out1.type());
    Mat H2_lines(4, 4, lines_out2.type());
    stereoRectifyUncalibrated(masked_points1, masked_points2, F, img1.size(),
                              H1_lines, H2_lines);

    warpPerspective(lines_out1, lines_out1, H1_lines, lines_out1.size());
    imwrite(obj_name + "_left_epipolar_lines_lines_out.jpg", lines_out1);

    warpPerspective(lines_out2, lines_out2, H2_lines, lines_out2.size());
    imwrite(obj_name + "_right_epipolar_lines_lines_out.jpg", lines_out2);



    Mat H1(4, 4, img1.type());
    Mat H2(4, 4, img2.type());
    stereoRectifyUncalibrated(masked_points1, masked_points2, F, img1.size(),
                              H1, H2);

    // Rectify images
    cout << "Applying Rectification Matrix..." << endl;
    Mat rectified1(img1.size(), img1.type());
    homography_warp(img1, H1, rectified1);

    Mat rectified2(img2.size(), img2.type());
    homography_warp(img2, H2, rectified2);

    copyMakeBorder(rectified1, rectified1, 0,
                   max(rectified1.rows, rectified2.rows) - rectified1.rows, 0,
                   max(rectified1.cols, rectified2.cols) - rectified1.cols,
                   BORDER_CONSTANT);
    copyMakeBorder(rectified2, rectified2, 0,
                   max(rectified1.rows, rectified2.rows) - rectified2.rows, 0,
                   max(rectified1.cols, rectified2.cols) - rectified2.cols,
                   BORDER_CONSTANT);

    imwrite(obj_name + "_left_rectified.jpg", rectified1);
    imwrite(obj_name + "_right_rectified.jpg", rectified2);

    return ImagePair(rectified1, rectified2);
}

void
ImagePair::drawEpipolarLines(std::vector<cv::Point2f> points1,
    std::vector<cv::Point2f> points2, cv::Mat &out1, cv::Mat &out2, cv::Mat F,
    std::string obj_name)
{
    cout << "Drawing Epipolar lines..." << endl;
    // Draw epilines in rectified images
    time_t t;
    srand((unsigned) time(&t));

    out1 = img1.clone();
    out2 = img2.clone();

    vector<cv::Vec3f> epilines1, epilines2;

    computeCorrespondEpilines(points1, 1, F, epilines1);
    computeCorrespondEpilines(points2, 2, F, epilines2);

    for(auto line: epilines1)
    {
        cv::line(out1,
                 cv::Point(0,-line[2]/line[1]),
                 cv::Point(img1.cols,-(line[2]+
                                       line[0]*img1.cols)/line[1]),
                 cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
    }

    imwrite(obj_name + "_left_epipolar_lines.jpg", out1);

    for(auto line: epilines2)
    {
        cv::line(out2,
                 cv::Point(0,-line[2]/line[1]),
                 cv::Point(img2.cols,-(line[2]+
                                       line[0]*img2.cols)/line[1]),
                 cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
    }

    imwrite(obj_name + "_right_epipolar_lines.jpg", out2);
}
