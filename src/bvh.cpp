#include "bvh.hpp"

inline bool box_compare(const shared_ptr<Object3D> a, const shared_ptr<Object3D> b, int axis) {
    AABB box_a;
    AABB box_b;

    if (!a->bounding_box(0,0, box_a) || !b->bounding_box(0,0, box_b))
        std::cerr << "No bounding box in BVHnode constructor.\n";

    return box_a.min()[axis] < box_b.min()[axis];
}


bool box_x_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) {
    return box_compare(a, b, 0);
}

bool box_y_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) {
    return box_compare(a, b, 1);
}

bool box_z_compare (const shared_ptr<Object3D> a, const shared_ptr<Object3D> b) {
    return box_compare(a, b, 2);
}


BVHnode::BVHnode(
    const std::vector<shared_ptr<Object3D>>& src_objects,
    size_t start, size_t end, double time0, double time1
) {
    auto objects = src_objects; // Create a modifiable array of the source scene objects

    int axis = random_int(0,2);
    auto comparator = (axis == 0) ? box_x_compare
                    : (axis == 1) ? box_y_compare
                                  : box_z_compare;

    size_t object_span = end - start;

    if (object_span == 1) {
        left = right = objects[start];
    } else if (object_span == 2) {
        if (comparator(objects[start], objects[start+1])) {
            left = objects[start];
            right = objects[start+1];
        } else {
            left = objects[start+1];
            right = objects[start];
        }
    } else {
        std::sort(objects.begin() + start, objects.begin() + end, comparator);

        auto mid = start + object_span/2;
        left = make_shared<BVHnode>(objects, start, mid, time0, time1);
        right = make_shared<BVHnode>(objects, mid, end, time0, time1);
    }

    AABB box_left, box_right;

    if (  !left->bounding_box (time0, time1, box_left)
       || !right->bounding_box(time0, time1, box_right)
    )
        std::cerr << "No bounding box in BVHnode constructor.\n";

    box = AABB::surrounding_box(box_left, box_right);
}


bool BVHnode::intersect(const Ray& r, Hit& rec, float t_min, float t_max) const {
    if (!box.intersect(r, t_min, t_max))
        return false;

    bool hit_left = left->intersect(r, rec, t_min, t_max);
    bool hit_right = right->intersect(r, rec, t_min, hit_left ? rec.t : t_max);

    return hit_left || hit_right;
}


bool BVHnode::bounding_box(double time0, double time1, AABB& output_box) const {
    output_box = box;
    return true;
}
