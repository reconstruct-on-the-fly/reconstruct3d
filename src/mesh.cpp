#include "mesh.h"

#include <fstream>
#include <iostream>
#include <cstdio>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wpedantic"
#include "Simplify.h"
#pragma GCC diagnostic pop

Mesh::Mesh(){}

Mesh::Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces)
    : m_vertices(vertices), m_faces(faces){}

Mesh::Mesh(std::vector<cv::Point3f> vertices, std::vector<cv::Point3i> faces,
           std::vector< std::unordered_set<int> > _G)
    : m_vertices(vertices), m_faces(faces), G(_G) {}

template <typename T>
inline T interpolate(const T v, const T a0, const T b0, const T a1, const T b1)
{
    return (((v - a0) * (b1 - a1)) / (b0 - a0)) + a1;
}

inline int gcd(int a, int b)
{
    return (b == 0 ? a : gcd(b, a % b));
}

uchar readGrayPixel(cv::Mat image, int i, int j)
{
    auto k = image.at<cv::Vec3b>(i, j);
    // return 0.21 * k[2] + 0.72 * k[1] + 0.07 * k[0]; // Luminosity
    return (k[0] + k[1] + k[2])/3;
}

void
Mesh::generateMesh(DepthMap depthMap, float max_height)
{
    auto image = depthMap.getImage();

    const unsigned int rows = image.rows;
    const unsigned int cols = image.cols;

    auto div = gcd(rows, cols);
    float r = rows/div;
    float c = cols/div;

    if (max_height <= 0)
        max_height = std::max(c, r)/std::min(c, r);

    uchar max_pixel = readGrayPixel(image, 0, 0);
    uchar min_pixel = readGrayPixel(image, 0, 0);
    for(unsigned int i = 0; i < rows; ++i)
    {
        for(unsigned int j = 0; j < cols; ++j)
        {
            max_pixel = std::max(max_pixel, readGrayPixel(image, i, j));
            min_pixel = std::min(min_pixel, readGrayPixel(image, i, j));
        }
    }

    for(unsigned int i = 0; i < rows; ++i)
    {
        for(unsigned int j = 0; j < cols; ++j)
        {
            int v1 = i*cols + j;           // current vertex
            int v2 = i*cols + j + 1;       // forward vertex
            int v3 = (i + 1)*cols + j;     // underneath vertex
            int v4 = (i + 1)*cols + j + 1; // diagonal vertex

            const uchar k = readGrayPixel(image, i, j);
            m_vertices.push_back(cv::Point3f(
                    interpolate<float>(i, 0, rows, -r, r),
                    interpolate<float>(k, min_pixel, max_pixel,  0.0f, max_height),
                    interpolate<float>(j, 0, cols, -c, c)
                ));

            if(j >= cols - 2 || i >= rows - 2) continue;

            m_faces.push_back(cv::Point3i(v2 + 1, v4 + 1, v1 + 1));
            m_faces.push_back(cv::Point3i(v4 + 1, v3 + 1, v1 + 1));
        }
    }

    G = std::vector< std::unordered_set<int> >(m_vertices.size() + 1, std::unordered_set<int>());

    std::cout << "Generating Graph..." << std::endl;
    for(size_t i = 0; i < m_faces.size(); i++)
    {
        auto face = m_faces[i];

        G[face.x].insert(face.y); G[face.x].insert(face.z);
        G[face.y].insert(face.x); G[face.y].insert(face.z);
        G[face.z].insert(face.x); G[face.z].insert(face.y);
    }
    std::cout << "Graph Created..." << std::endl;
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

void
Mesh::generateMesh(cv::Mat image, std::string objname, float max_height,
                   float laplace_scale, int n_laplace_steps,
                   float simplification)
{
    auto depth_map = DepthMap(image);

    std::cout << "Generating Mesh..." << std::endl;
    generateMesh(depth_map, max_height);

    std::cout << "Writing " << objname + "-before-laplace.obj ..." << std::endl;
    createOBJ(objname + "-before-laplace.obj");

    std::cout << "Laplace Smoothing..." << std::endl;
    for (int i = 0; i < n_laplace_steps; ++i)
    {
        std::cout << "Smoothing " << i << "..." << std::endl;
        laplace_smooth(laplace_scale);
    }

    std::cout << "Writing " << objname + ".obj ..." << std::endl;

    auto outfile = objname + "-high-poly.obj";
    createOBJ(outfile);

    std::cout << "OBJ Created!" << std::endl;

    simplify(outfile, objname, simplification);
}

/*
 * laplace_smooth adapted from:
 *    http://www.gamedev.net/topic/642185-a-c-code-to-smooth-and-fix-cracks-in-meshes-generated-by-the-standard-original-marching-cubes-algorithm/
 */
void Mesh::laplace_smooth(float scale)
{
    std::vector<cv::Point3f> displacements(m_vertices.size() + 1,
                                           cv::Point3f(0, 0, 0));

    for(size_t i = 1; i <= m_vertices.size(); i++)
    {
        float weight = 1.0f / static_cast<float>(G[i].size());

        for(auto neighbour_j: G[i])
            displacements[i] += (m_vertices[neighbour_j] - m_vertices[i]) * weight;
    }

    for(size_t i = 1; i <= m_vertices.size(); i++)
        m_vertices[i] += displacements[i]*scale;
}

/*
 * simplify function adapted from:
 *    https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
 */
void
Mesh::simplify(std::string filename, std::string obj_name, float simplification)
{
    Simplify::load_obj(filename.c_str());

    if ((Simplify::triangles.size() < 3) || (Simplify::vertices.size() < 3))
    {
        std::cout << "WARN: Could not simplify obj " << filename << std::endl;
        return;
    }

    int target_count =  Simplify::triangles.size() >> 1;
    target_count = round((float)Simplify::triangles.size() * simplification);

    if (target_count < 4) {
        printf("WARN: Simplifcation ERROR: Object will not survive such extreme decimation\n");
        return;
    }

    double agressiveness = 7.0;

    printf("Starting simplification:\n");

    printf("Input: %zu vertices, %zu triangles (target %d)\n", Simplify::vertices.size(), Simplify::triangles.size(), target_count);

    size_t startSize = Simplify::triangles.size();
    Simplify::simplify_mesh(target_count, agressiveness, true);

    if (Simplify::triangles.size() >= startSize) {
        printf("WARN: simplification ERROR: Unable to reduce mesh.\n");
        return;
    }

    Simplify::write_obj((obj_name + "-simplified.obj").c_str());

    printf("Output: %zu vertices, %zu triangles (%f reduction)\n",
           Simplify::vertices.size(), Simplify::triangles.size(),
           (float)Simplify::triangles.size()/ (float) startSize);

    printf("Simplification Done!\n");
    return;
}
