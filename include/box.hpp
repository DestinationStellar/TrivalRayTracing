#ifndef BOX_H
#define BOX_H

#include <vecmath.h>
#include <vector>

#include "object3d.hpp"
#include "rectangle.hpp"

class Box : public Object3D {
public:
    Box() {}
    Box(const Vector3f &p0, const Vector3f &p1, Material* m):Object3D(m){
        box_min = p0;
        box_max = p1;

        sides.reserve(6);

        sides.push_back(Rectangle(Vector3f((p0[0]+p1[0])/2,(p0[1]+p1[1])/2,p0[2]), Vector3f(1,0,0), Vector3f(0,-1,0), p1[0]-p0[0], p1[1]-p0[1], m));
        sides.push_back(Rectangle(Vector3f((p0[0]+p1[0])/2,(p0[1]+p1[1])/2,p1[2]), Vector3f(1,0,0), Vector3f(0,1,0), p1[0]-p0[0], p1[1]-p0[1], m));

        sides.push_back(Rectangle(Vector3f(p0[0],(p0[1]+p1[1])/2,(p0[2]+p1[2])/2), Vector3f(0,0,1), Vector3f(0,1,0), p1[2]-p0[2], p1[1]-p0[1], m));
        sides.push_back(Rectangle(Vector3f(p1[0],(p0[1]+p1[1])/2,(p0[2]+p1[2])/2), Vector3f(0,0,-1), Vector3f(0,1,0), p1[2]-p0[2], p1[1]-p0[1], m));

        sides.push_back(Rectangle(Vector3f((p0[0]+p1[0])/2,p0[1],(p0[2]+p1[2])/2), Vector3f(1,0,0), Vector3f(0,0,1), p1[0]-p0[0], p1[2]-p0[2], m));
        sides.push_back(Rectangle(Vector3f((p0[0]+p1[0])/2,p1[1],(p0[2]+p1[2])/2), Vector3f(1,0,0), Vector3f(0,0,-1), p1[0]-p0[0], p1[2]-p0[2], m));
    }

    bool intersect(const Ray& r, Hit& rec, float t_min = 0.0, float t_max = infinity) const override {
        bool flag = false;
        float closest_t = t_max;
        for(auto it=sides.begin();it!=sides.end();it++){
            if((*it).intersect(r,rec,t_min,closest_t)){
                flag=true;
                closest_t=rec.getT();
            }
        }
        return flag;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        output_box = AABB(box_min, box_max);
        return true;
    }
protected:
    Vector3f box_min;
    Vector3f box_max;
    std::vector<Rectangle> sides;
};

#endif // BOX_H