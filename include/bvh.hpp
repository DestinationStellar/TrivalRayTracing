#ifndef BVHH
#define BVHH


#include "object3d.hpp"
#include "group.hpp"

#include <algorithm>
#include <memory>

using std::shared_ptr;


class BVHnode : public Object3D  {
    public:
        BVHnode();

        BVHnode(const Group& list, double time0, double time1)
            : BVHnode(list.getObjects(), 0, list.getGroupSize(), time0, time1)
        {}

        BVHnode(
            const std::vector<shared_ptr<Object3D>>& src_objects,
            size_t start, size_t end, double time0, double time1);

        virtual bool intersect(
            const Ray& r, Hit& rec, float tmin = 0.0, float tmax = infinity) const override;

        virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

    public:
        shared_ptr<Object3D>  left;
        shared_ptr<Object3D>  right;
        AABB box;
};


inline bool box_compare(const shared_ptr<Object3D> a, const shared_ptr<Object3D> b, int axis) ;


bool box_x_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) ;

bool box_y_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) ;

bool box_z_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) ;


#endif
