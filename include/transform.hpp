#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vecmath.h>
#include "object3d.hpp"
#include "utils.hpp"

class Transform : public Object3D {
public:
    Transform() {}

    Transform(
        shared_ptr<Object3D> obj, const Vector3f &scale, const Vector3f &translate, float x, float y, float z
    ) : o(obj) {
        transform = Matrix4f::identity();
        transform = transform * Matrix4f::translation(translate);
        transform = transform * Matrix4f::rotateX(DegreesToRadians(x));
        transform = transform * Matrix4f::rotateY(DegreesToRadians(y));
        transform = transform * Matrix4f::rotateZ(DegreesToRadians(z));
        transform = transform * Matrix4f::scaling(scale[0], scale[1], scale[2]);
        transform_ray = transform.inverse();
    }

    Transform(const Matrix4f &m, shared_ptr<Object3D> obj) : o(obj) {
        transform = m;
        transform_ray = m.inverse();
    }

    ~Transform() {}

    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const override {
        Vector3f trSource = transformPoint(transform_ray, r.getOrigin());
        Vector3f trDirection = transformDirection(transform_ray, r.getDirection());
        Ray tr(trSource, trDirection);
        bool inter = o->intersect(tr, h, tmin, tmax);
        if (inter) {
            h.set(h.getT(), h.getMaterial(), transformDirection(transform_ray.transposed(), h.getNormal()).normalized(), r);
        }
        return inter;
    }
    
    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        o->bounding_box(time0, time1, output_box);
        Vector3f v[8];
        v[0] = output_box.min();
        v[7] = output_box.max();
        for (int i = 1; i < 4; i++) {
            v[i] = v[0]; v[i+3] = v[7];
            v[i][i-1] = v[7][i-1];
            v[i+3][i-1] = v[0][i-1];
        }
        for (int i = 0; i < 8; i++) {
            v[i] = transformPoint(transform, v[i]);
        }
        for (int i = 0; i < 3; i++) {
            float tmin = v[0][i];
            float tmax = v[0][i];
            for (int j=1; j<8; j++) {
                tmin = fmin(tmin, v[j][i]);
                tmax = fmax(tmin, v[j][i]);
            }
            v[0][i] = tmin;
            v[7][i] = tmax;
        }
        output_box.minimum = v[0];
        output_box.maximum = v[7];
		return true;
	}

protected:
    shared_ptr<Object3D> o; //un-transformed object
    Matrix4f transform;
    Matrix4f transform_ray;
};

#endif //TRANSFORM_H
