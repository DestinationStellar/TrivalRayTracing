#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "object3d.hpp"
#include "material.hpp"
#include "texture.hpp"

class ConstantMedium : public Object3D {
public:
    ConstantMedium(Object3D* b, double d, Texture* m)
        : boundary(b), neg_inv_density(-1/d) {
            phase_function = new Isotropic(m);
            material = phase_function;
    }
    
    ConstantMedium(Object3D* b, double d, Vector3f c)
        : boundary(b), neg_inv_density(-1/d) {
            phase_function = new Isotropic(c);
            material = phase_function;
    }   

    virtual bool intersect(
        const Ray& r, Hit& rec, float t_min = 0.0, float t_max = infinity
    ) const override {
        // Print occasional samples when debugging. To enable, set enableDebug true.
        const bool enableDebug = false;
        const bool debugging = enableDebug && random_double() < 0.00001;

        Hit rec1, rec2;

        if (!boundary->intersect(r, rec1, -infinity, infinity))
            return false;

        if (!boundary->intersect(r, rec2, rec1.getT()+0.0001, infinity))
            return false;

        if (debugging) std::cerr << "\nt_min=" << rec1.getT() << ", t_max=" << rec2.t << '\n';

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

        if (debugging) {
            std::cerr << "hit_distance = " <<  hit_distance << '\n'
                    << "rec.t = " <<  rec.t << '\n'
                    << "rec.p = " <<  rec.getIntersectP() << '\n';
        }

        rec.normal = Vector3f(1,0,0);  // arbitrary
        rec.frontFace = true;     // also arbitrary
        rec.material = phase_function;

        return true;
    }

    virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
        return boundary->bounding_box(time0, time1, output_box);
    }
protected:
    Object3D* boundary;
    Material* phase_function;
    double neg_inv_density;
};

#endif // CONSTANT_MEDIUM_H