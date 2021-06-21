#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H

#include <vecmath.h>
#include <cmath>
#include "object3d.hpp"
#include "utils.hpp"
#include "aabb.hpp"

class MovingSphere : public Object3D {
public:
    MovingSphere() {}
    MovingSphere(
        const Vector3f &cen0, const Vector3f &cen1, float t0, float t1, float r, Material* m
    ): Object3D(m), center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r)
    {};

    bool intersect(const Ray& r, Hit& h, float tmin, float tmax) const override {
        Vector3f oc = r.getOrigin() - center(r.getTime());
        auto a = r.getDirection().squaredLength();
        auto half_b = Vector3f::dot(oc, r.getDirection());
        auto c = oc.squaredLength() - radius*radius;

        auto discriminant = half_b*half_b - a*c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (root < tmin || tmax < root) {
            root = (-half_b + sqrtd) / a;
            if (root < tmin || tmax < root)
                return false;
        }
        Vector3f intersec_point=r.pointAtParameter(root);
        Vector3f n=(intersec_point-center(r.getTime()))/radius;
        h.set(root, material, n, r);
        get_sphere_uv(n, h.u, h.v);
        return true;
    }

    Vector3f center(float time) const {
        return center0 + ((time - time0) / (time1 - time0))*(center1 - center0);
    }

    bool bounding_box(double _time0, double _time1, AABB& output_box) const override {
        AABB box0(
            center(_time0) - Vector3f(radius, radius, radius),
            center(_time0) + Vector3f(radius, radius, radius));
        AABB box1(
            center(_time1) - Vector3f(radius, radius, radius),
            center(_time1) + Vector3f(radius, radius, radius));
        output_box = AABB::surrounding_box(box0, box1);
        return true;
    }

protected:
    Vector3f center0, center1;
    double time0, time1;
    double radius;
    static void get_sphere_uv(const Vector3f& p, float& u, float& v)  {
        auto phi = atan2(p.z(), p.x());
        auto theta = asin(p.y());
        u = 1-(phi + M_PI) / (2*M_PI);
        v = (theta + M_PI/2) / M_PI;
    }
};

#endif // MOVING_SPHERE_H