#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <vecmath.h>

#include "object3d.hpp"

class Rectangle : public Object3D {
public:
    Rectangle () {}
    Rectangle (const Vector3f &_center, const Vector3f &d_len, const Vector3f &d_wid, float len, float wid, Material *m): Object3D(m) {
        center = _center;
        dir_len = d_len.normalized();
        dir_wid = d_wid.normalized();
        halfL = len/2;
        halfW = wid/2;
        normal = Vector3f::cross(d_len, d_wid);
        d = Vector3f::dot(normal, center);
    }
    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const override {
        float t=(d-Vector3f::dot(normal,r.getOrigin()))/(Vector3f::dot(normal,r.getDirection()));
        if(t>=tmin&&t<tmax){
            Vector3f point = r.pointAtParameter(t);
            float x = Vector3f::dot(point-center,dir_len);
            float y = Vector3f::dot(point-center,dir_wid);
            if(abs(x) < halfL && abs(y) < halfW){
                h.u = x/(halfL*2) + 0.5;
                h.v = y/(halfW*2) + 0.5;
                h.set(t,material,normal,r);
                return true;
            }
        }
        return false;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        Vector3f point2 = center + dir_len * halfL + dir_wid * halfW;
        Vector3f point[4];
        point[0] = center - dir_len * halfL - dir_wid * halfW;
        point[1] = center - dir_len * halfL + dir_wid * halfW;
        point[2] = center + dir_len * halfL - dir_wid * halfW;
        point[3] = center + dir_len * halfL + dir_wid * halfW;
        for (int i = 0; i < 3; i++) {
            float tmin = point[0][i];
            float tmax = point[0][i];
            for (int j=1; j<4; j++) {
                tmin = fmin(tmin, point[j][i]);
                tmax = fmax(tmax, point[j][i]);
            }
            point[0][i] = tmin - 0.001;
            point[3][i] = tmax + 0.001;
        }
        output_box = AABB (
            point[0],
            point[3]
        );
        return true;
    }
protected:
    Vector3f normal;
    Vector3f center;
    Vector3f dir_len,dir_wid;
    float halfL,halfW;
    float d;
};

#endif // RECTANGLE_H