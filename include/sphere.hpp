#ifndef SPHERE_H
#define SPHERE_H

#include <vecmath.h>
#include <cmath>
#include "object3d.hpp"
#include "utils.hpp"

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
        this->center=Vector3f(0,0,0);
        this->radius=1;
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center=center;
        this->radius=radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        Vector3f origin=r.getOrigin();
        Vector3f l=center-origin;
        if(l.squaredLength()>=radius*radius){
            float t_p=Vector3f::dot(l,r.getDirection().normalized());
            if(t_p>=0){
                float d=l.squaredLength()-t_p*t_p;
                d = clampFloatNegative(d);
                d = sqrt(d);
                if(radius*radius-d*d>0){
                    float t_tmp=sqrt(radius*radius-d*d);
                    float t=t_p-t_tmp;
                    if (t>=tmin && t<h.getT()){
                        Vector3f intersec_point=r.pointAtParameter(t);
                        Vector3f n=(intersec_point-center).normalized();
                        if(radius<0)n=-n;
                        h.set(t,material,n,r);
                        return true;
                    }
                }
            } 
        }
        return false;
    }

protected:
    Vector3f center;
    float radius;
};


#endif
