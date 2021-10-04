#ifndef OBJECT3D_H
#define OBJECT3D_H

#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"
#include "aabb.hpp"
#include "utils.hpp"

// Base class for all 3d entities.
class Object3D {
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(shared_ptr<Material> material) {
        this->material = material;
    }

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const = 0;

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const = 0;

    virtual double pdf_value(const Vector3f& o, const Vector3f& v) const {
        return 0.0;
    }

    virtual Vector3f random(const Vector3f& o) const {
        return Vector3f(1,0,0);
    }
protected:

    shared_ptr<Material> material;
};

#endif

