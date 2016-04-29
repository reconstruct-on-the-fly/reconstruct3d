#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "camera.h"
#include "image_pair.h"


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
    Mat R = Mat_<double>(3,3,0.0);
    Mat Q;
    Vec<double, 3> T;
    T[0] = 0; T[1] = 0; T[2] = 0;

    ImagePair imagePair(image1, image2, R, T);
    ImagePair newImagePair = imagePair.rectify(camera, Q);

    /* Open Window */
    string window_title = "Display Window";

    namedWindow(window_title, WINDOW_NORMAL);
    imshow(window_title, newImagePair.getImage1());
    imshow(window_title + "1", newImagePair.getImage2());
    waitKey(0);

    return EXIT_SUCCESS;
}
