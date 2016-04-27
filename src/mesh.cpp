#include "mesh.h"

#include <fstream>

Mesh::Mesh(){}

Mesh::Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces)
    : m_vertices(vertices), m_faces(faces){}

template <typename T>
inline T interpolate(const T v, const T a0, const T b0, const T a1, const T b1)
{
    return (((v - a0) * (b1 - a1)) / (b0 - a0)) + a1;
}

Mesh
Mesh::generateMesh(DepthMap depthMap, float max_height)
{
    std::vector<cv::Point3f> vertices;
    std::vector<cv::Point3i> faces;

    auto image = depthMap.getImage();

    const unsigned int rows = image.rows;
    const unsigned int cols = image.cols*image.channels();

    for(unsigned int i = 0; i < rows; ++i)
    {
        for(unsigned int j = 0; j < cols; ++j)
        {
            int v1 = i*cols + j;           // current vertex
            int v2 = i*cols + j + 1;       // foward vertex
            int v3 = (i + 1)*cols + j;     // underneath vertex
            int v4 = (i + 1)*cols + j + 1; // diagonal vertex

            const uchar k = image.at<uchar>(i, j);
            vertices.push_back(cv::Point3f(
                interpolate<float>(i, 0, rows, -1.0f, 1.0f),
                interpolate<float>(k, 0, 255,  -1.0f, max_height),
                interpolate<float>(j, 0, cols, -1.0f, 1.0f)
                ));

            if(j >= cols - 2 || i >= rows - 2) continue;

            faces.push_back(cv::Point3i(v2 + 1, v4 + 1, v1 + 1));
            faces.push_back(cv::Point3i(v4 + 1, v3 + 1, v1 + 1));
        }
    }

    return Mesh(vertices, faces);
}

void
Mesh::createOBJ(std::string filepath)
{
    const std::string header = "# 3D Terrain generated with Reconstruct 3D";

    std::ofstream objFile;
    objFile.open(filepath);

    objFile << header << "\n";

    for(auto vertex: m_vertices)
        objFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";

    objFile << "\n";

    for(auto face: m_faces)
        objFile << "f " << face.x << " " << face.y << " " << face.z << "\n";

    objFile.close();
}

