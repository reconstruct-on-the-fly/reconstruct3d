#include "mesh.h"

#include <fstream>

Mesh::Mesh(){}

Mesh::Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces)
    : m_vertices(vertices), m_faces(faces){}

static Mesh
generateMesh(DepthMap depthMap)
{
    std::vector<cv::Point3f> vertices;
    std::vector<cv::Point3i> faces;

    auto image = depthMap.getImage();

    const unsigned int rows = image.rows;
    const unsigned int cols = image.cols;

    for(unsigned int i = 0; i < rows; ++i)
    {
        for(unsigned int j = 0; j < cols; ++j)
        {
            int v1 = i*cols + j;           // current vertex
            int v2 = i*cols + j + 1;       // foward vertex
            int v3 = (i + 1)*cols + j;     // underneath vertex
            int v4 = (i + 1)*cols + j + 1; // diagonal vertex

            vertices.push_back(cv::Point3f(i, j, image.at<float>(j, i)));

            if(j >= cols - 2 || i >= rows - 2) continue;

            faces.push_back(cv::Point3i(v1, v2, v3));
            faces.push_back(cv::Point3i(v2, v3, v4));
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

