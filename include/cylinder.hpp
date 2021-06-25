#ifndef CYLINDER_H
#define CYLINDER_H

#include <vecmath.h>
#include "object3d.hpp"

class Cylinder : public Object3D {
public:
    Cylinder(const Vector3f &center, float radius, float height, shared_ptr<Material> material) : Object3D(material) {
        this->center=center;
        this->radius=radius;
        this->miny = center.y() - height/2;
        this->maxy = center.y() + height/2;
    }
    Cylinder(const Vector3f &center, float radius, float ymin, float ymax, shared_ptr<Material> material) : Object3D(material) {
        this->center=center;
        this->radius=radius;
        this->miny = ymin;
        this->maxy = ymax;
        this->center.y()= (ymin + ymax)/2;
    }
    ~Cylinder(){}
    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity ) const override {
        Vector3f ro = r.getOrigin();
        Vector3f rd = r.getDirection();
        double dt, dl, d0;

        // intersect with the cylinder
        double l, a, b, c;
        a = (rd.x() * rd.x() + rd.z() * rd.z());
        b = (2 * rd.x() * (ro.x() - center.x()) + 2 * rd.z() * (ro.z() - center.z()));
        c = center.x() * center.x() + center.z() * center.z() + ro.x() * ro.x() + ro.z() * ro.z() - 2 * ro.x() * center.x() - 2 * ro.z() * center.z() - radius * radius;
        double det = b * b - 4 * a * c;

        if (det < 0) return false;
        else det = std::sqrt(det);
        if ((-b - det) / (2 * a) > tmin) l = (-b - det) / (2 * a);
        else if ((-b + det) / (2 * a) > tmin) l = (-b + det) / (2 * a);
        else return false;
        if (l > tmax) return false;
        Vector3f near = ro + l * rd;

        double t = (near.y() - miny) / (maxy - miny);
        if (t < 0 || t > 1) return false;
        Vector3f np = near - center;
        np.y() = 0;
        double theta = std::acos(np.x() / radius);
        h.set(l, material, np, r);
        h.u = theta / (2 * M_PI);
        h.v = t;
        return true;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const override {
        output_box = AABB(
            center - Vector3f(radius, (maxy-miny)/2, radius),
            center + Vector3f(radius, (maxy-miny)/2, radius));
        return true;
    }

    float getRadius() {
        return radius;
    }

protected:
    Vector3f center;
    float radius, miny, maxy;
};

#endif // CYLINDER_H