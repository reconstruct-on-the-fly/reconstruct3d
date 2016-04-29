#include <opencv2/core/core.hpp>
#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace cv;
using namespace std;

int read_from_file(Mat1d camera_matrix, Mat1d distortion_coefs)
{
    ifstream input_file("input.txt");
    double buffer;
    input_file >> buffer;

    for (int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++, input_file >> buffer)
        {
            camera_matrix.at<double>(i,j) = buffer;
        }
    }

    for (int i=0; i<5; i++, input_file >> buffer)
    {
        distortion_coefs.at<double>(0,i) = buffer;
    }
}

int main(int argc, char **argv)
{
    Mat1d camera_matrix(3,3);
    Mat1d distortion_coefs(1,5);

    read_from_file(camera_matrix, distortion_coefs);
    cout << camera_matrix << endl;
    cout << distortion_coefs << endl;
}
