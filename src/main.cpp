#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "camera.h"
#include "image_pair.h"
#include "disparity_map.h"
#include "depth_map.h"


using namespace cv;
using namespace std;

inline void print_usage()
{
    cout << "Usage:\n$reconstruct3d calibration_file image1 image2" << endl;
}

int main(int argc, char** argv)
{
    /* Argument Checking */
    if (argc != 4)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    string calibration_file = argv[1];

    if(calibration_file == "")
    {
        cout << "Empty calibration_file" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }

    Mat image1, image2;
    image1 = imread(argv[2], CV_LOAD_IMAGE_COLOR);
    image2 = imread(argv[3], CV_LOAD_IMAGE_COLOR);

    if (!image1.data || !image2.data)
    {
        cout << "Could not open or find the image" << endl;
        print_usage();
        exit(EXIT_FAILURE);
    }

    /* Camera Creation */
    Camera camera = Camera::createFromFile(calibration_file);

    /* Image Rectification */
    Mat R = Mat::eye(3, 3, CV_64F);
    Mat Q;
    Vec<double, 3> T;
    T[0] = 0; T[1] = 3; T[2] = 0;

    ImagePair imagePair(image1, image2, R, T);
    ImagePair newImagePair = imagePair.rectify(camera, Q);

    imwrite("/vagrant/rectified_image1.jpg", newImagePair.getImage1());
    imwrite("/vagrant/rectified_image2.jpg", newImagePair.getImage2());

    /* Disparity Map */
    DisparityMap disparityMap = DisparityMap::generateDisparityMap(newImagePair);

    /* Depth Map */
    DepthMap depthMap = DepthMap::generateDepthMap(disparityMap, Q);

    /* Save Results */
    imwrite("/vagrant/diparity_map.jpg", disparityMap.getImage());
    imwrite("/vagrant/depth_map.jpg", depthMap.getImage());

    return EXIT_SUCCESS;
}
