#ifndef SPHERE_H
#define SPHERE_H

#include <vecmath.h>
#include <cmath>
#include "object3d.hpp"
#include "utils.hpp"
#include "aabb.hpp"
#include "onb.hpp"

class Sphere : public Object3D {
public:
    Sphere() {
        // unit ball at the center
        this->center=Vector3f(0,0,0);
        this->radius=1;
    }

    Sphere(const Vector3f &center, float radius, shared_ptr<Material> material) : Object3D(material) {
        this->center=center;
        this->radius=radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity ) const override {
        Vector3f oc = r.getOrigin() - center;
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
        Vector3f n=(intersec_point-center)/radius;
        h.set(root, material, n, r);
        get_sphere_uv(n, h.u, h.v);
        return true;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        output_box = AABB(
            center - Vector3f(radius, radius, radius),
            center + Vector3f(radius, radius, radius));
        return true;
    }

    double pdf_value(const Vector3f& o, const Vector3f& v) const {
        Hit rec;
        if (!this->intersect(Ray(o, v), rec, 0.001, infinity))
            return 0;

        auto cos_theta_max = sqrt(1 - radius*radius/(center-o).squaredLength());
        auto solid_angle = 2*M_PI*(1-cos_theta_max);

        return  1 / solid_angle;
    }

    Vector3f random(const Vector3f& o) const {
        Vector3f direction = center - o;
        auto distance_squared = direction.squaredLength();
        ONB uvw;
        uvw.build_from_w(direction);
        return uvw.local(random_to_sphere(radius, distance_squared));
    }

protected:
    Vector3f center;
    float radius;
    static void get_sphere_uv(const Vector3f& p, float& u, float& v)  {
        auto phi = atan2(p.z(), p.x());
        auto theta = asin(p.y());
        u = 1-(phi + M_PI) / (2*M_PI);
        v = (theta + M_PI/2) / M_PI;
    }
    static Vector3f random_to_sphere(double radius, double distance_squared) {
        auto r1 = random_double();
        auto r2 = random_double();
        auto z = 1 + r2*(sqrt(1-radius*radius/distance_squared) - 1);

        auto phi = 2*M_PI*r1;
        auto x = cos(phi)*sqrt(1-z*z);
        auto y = sin(phi)*sqrt(1-z*z);

        return Vector3f(x, y, z);
    }
};


#endif
