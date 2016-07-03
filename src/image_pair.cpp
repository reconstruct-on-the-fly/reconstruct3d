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

    Mat H1(4, 4, img1.type());
    Mat H2(4, 4, img2.type());
    stereoRectifyUncalibrated(masked_points1, masked_points2, F, img1.size(), H1, H2);

    // Create borders
    Mat img1_borders, img2_borders;

    copyMakeBorder(img1, img1_borders, img1.rows/2, img1.rows/2, img1.cols/2, img1.cols/2, BORDER_CONSTANT);
    copyMakeBorder(img2, img2_borders, img2.rows/2, img2.rows/2, img2.cols/2, img2.cols/2, BORDER_CONSTANT);

    // Rectify images
    Mat rectified1(img1_borders.size(), img1_borders.type());
    warpPerspective(img1_borders, rectified1, H1, img1_borders.size());
    imwrite(obj_name + "_left_rectified.jpg", rectified1);

    Mat rectified2(img2_borders.size(), img2_borders.type());
    warpPerspective(img2_borders, rectified2, H2, img2_borders.size());
    imwrite(obj_name + "_right_rectified.jpg", rectified2);

    drawEpipolarLines(masked_points1, masked_points2, F, obj_name);

    return ImagePair(rectified1, rectified2);
}

void
ImagePair::drawEpipolarLines(std::vector<cv::Point2f> points1,
    std::vector<cv::Point2f> points2, cv::Mat F, std::string obj_name)
{
    // Draw epilines in rectified images
    time_t t;
    srand((unsigned) time(&t));

    Mat  left_lines = img1.clone();
    Mat right_lines = img2.clone();

    vector<cv::Vec3f> epilines1, epilines2;

    computeCorrespondEpilines(points1, 1, F, epilines1);
    computeCorrespondEpilines(points2, 2, F, epilines2);

    for(auto line: epilines1)
    {
        cv::line(left_lines,
                 cv::Point(0,-line[2]/line[1]),
                 cv::Point(img1.cols,-(line[2]+
                                       line[0]*img1.cols)/line[1]),
                 cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
    }

    imwrite(obj_name + "_left_epipolar_lines.jpg", left_lines);

    for(auto line: epilines2)
    {
        cv::line(right_lines,
                 cv::Point(0,-line[2]/line[1]),
                 cv::Point(img2.cols,-(line[2]+
                                       line[0]*img2.cols)/line[1]),
                 cv::Scalar(rand() % 255, rand() % 255, rand() % 255));
    }

    imwrite(obj_name + "_right_epipolar_lines.jpg", right_lines);
}
