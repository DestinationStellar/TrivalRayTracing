#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "object3d.hpp"
#include "material.hpp"
#include "texture.hpp"

class ConstantMedium : public Object3D {
public:
    ConstantMedium(shared_ptr<Object3D> b, double d, shared_ptr<Texture> m)
        : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<Isotropic>(m)) {
            material = phase_function;
    }
    
    ConstantMedium(shared_ptr<Object3D> b, double d, Vector3f c)
        : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<Isotropic>(c)) {
            material = phase_function;
    }   

    virtual bool intersect(
        const Ray& r, Hit& rec, float t_min = 0.0, float t_max = infinity
    ) const override {

        Hit rec1, rec2;

        if (!boundary->intersect(r, rec1, -infinity, infinity))
            return false;

        if (!boundary->intersect(r, rec2, rec1.getT()+0.0001, infinity))
            return false;

        if (rec1.getT() < t_min) rec1.t = t_min;
        if (rec2.getT() > t_max) rec2.t = t_max;

        if (rec1.t >= rec2.t)
            return false;

        if (rec1.t < 0)
            rec1.t = 0;

        const auto ray_length = r.getDirection().length();
        const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
        const auto hit_distance = neg_inv_density * log(random_double());

        if (hit_distance > distance_inside_boundary)
            return false;

        rec.t = rec1.t + hit_distance / ray_length;
        rec.intersectP = r.pointAtParameter(rec.t);

        rec.normal = Vector3f(1,0,0);  // arbitrary
        rec.frontFace = true;     // also arbitrary
        rec.material = phase_function;

        return true;
    }

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
        return boundary->bounding_box(time0, time1, output_box);
    }
protected:
    shared_ptr<Object3D> boundary;
    shared_ptr<Material> phase_function;
    double neg_inv_density;
};

#endif // CONSTANT_MEDIUM_H