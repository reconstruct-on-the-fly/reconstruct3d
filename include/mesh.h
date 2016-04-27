#ifndef MESH_H
#define MESH_H

#include <vector>
#include <opencv2/core/core.hpp>
#include "depth_map.h"

class Mesh {

public:
    Mesh();
    Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces);

    static Mesh generateMesh(DepthMap depthMap, float max_height);

    void createOBJ(std::string filepath);

private:
    std::vector<cv::Point3f> m_vertices;
    std::vector<cv::Point3i> m_faces;

};

#endif
