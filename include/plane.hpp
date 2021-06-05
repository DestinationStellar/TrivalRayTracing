#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

// function: ax+by+cz=d
// choose your representation , add more fields and fill in the functions

class Plane : public Object3D {
public:
    Plane() {

    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal=normal.normalized();
        this->d=d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        float t=(d-Vector3f::dot(normal,r.getOrigin()))/(Vector3f::dot(normal,r.getDirection()));
        if(t>=tmin&&t<h.getT()){
            // if(Vector3f::dot(normal,r.getOrigin())<0){
            //     normal = -normal;
            // }
            h.set(t,material,normal,r);
            return true;
        }
        return false;
    }

protected:
    Vector3f normal;
    float d;
};

#endif //PLANE_H
		

