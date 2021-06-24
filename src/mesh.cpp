#include "bvh.hpp"
#include "mesh.hpp"
#include "aabb.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "../deps/tiny_obj/tiny_obj_loader.h"


#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <sstream>

bool Mesh::intersect(const Ray &r, Hit &h, float tmin, float tmax) const {

    // Optional: Change this brute force method into a faster one.
    return triangle_bvh->intersect(r, h, tmin, tmax);
}

bool Mesh::bounding_box(double time0, double time1, AABB& output_box) const {

    triangle_bvh->bounding_box(time0, time1, output_box);
    std::cout << "bounding box of mesh: min "<< output_box.min() << " max "<< output_box.max() << std::endl;

	return true;
}

Mesh::Mesh(const char *filename, shared_ptr<Material> material) : Object3D(material) {

    // std::ifstream f;
    // f.open(filename);
    // if (!f.is_open()) {
    //     std::cout << "Cannot open " << filename << "\n";
    //     return;
    // }
    // std::string line;
    // std::string vTok("v");
    // std::string fTok("f");
    // std::string texTok("vt");
    // char bslash = '/', space = ' ';
    // std::string tok;
    // int texID;
    // while (true) {
    //     std::getline(f, line);
    //     if (f.eof()) {
    //         break;
    //     }
    //     if (line.size() < 3) {
    //         continue;
    //     }
    //     if (line.at(0) == '#') {
    //         continue;
    //     }
    //     std::stringstream ss(line);
    //     ss >> tok;
    //     if (tok == vTok) {
    //         Vector3f vec;
    //         ss >> vec[0] >> vec[1] >> vec[2];
    //         v.push_back(vec);
    //     } else if (tok == fTok) {
    //         if (line.find(bslash) != std::string::npos) {
    //             std::replace(line.begin(), line.end(), bslash, space);
    //             std::stringstream facess(line);
    //             TriangleIndex trig;
    //             facess >> tok;
    //             for (int ii = 0; ii < 3; ii++) {
    //                 facess >> trig[ii] >> texID;
    //                 trig[ii]--;
    //             }
    //             t.push_back(trig);
    //         } else {
    //             TriangleIndex trig;
    //             for (int ii = 0; ii < 3; ii++) {
    //                 ss >> trig[ii];
    //                 trig[ii]--;
    //             }
    //             t.push_back(trig);
    //         }
    //     } else if (tok == texTok) {
    //         Vector2f texcoord;
    //         ss >> texcoord[0];
    //         ss >> texcoord[1];
    //     }
    // }

    triangle.reserve((size_t)5e5);

    std::string inputfile = filename;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = ""; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
    if (!reader.Error().empty()) {
        std::cerr << "TinyObjReader: " << reader.Error();
    }
    exit(1);
    }

    if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    std::cout<< "obj loaded!" << std::endl;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            Vector3f tri_v[3];

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];

                tri_v[v] = Vector3f(float(vx), float(vy), float(vz));

            }

            shared_ptr<Object3D> tri = make_shared<Triangle>(tri_v[0], tri_v[1], tri_v[2], material);
            triangle.push_back(tri);

            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    // computeNormal();

    triangle_bvh = make_shared<BVHnode>(triangle, 0, triangle.size(), 0, 0);

    

    // f.close();
}

void Mesh::computeNormal() {
    n.resize(t.size());
    for (int triId = 0; triId < (int) t.size(); ++triId) {
        TriangleIndex& triIndex = t[triId];
        Vector3f a = v[triIndex[1]] - v[triIndex[0]];
        Vector3f b = v[triIndex[2]] - v[triIndex[0]];
        b = Vector3f::cross(a, b);
        n[triId] = b / b.length();
    }
}
