#ifndef AABB_H
#define AABB_H

#include <cmath>

#include "utils.hpp"
#include "ray.hpp"

class AABB {
    public:
        AABB() {}
        AABB(const Vector3f& a, const Vector3f& b) { 
            minimum = a;
            maximum = b;  
        }

        Vector3f min() const {return minimum; }
        Vector3f max() const {return maximum; }

        bool intersect(const Ray& r, float &t_min, float t_max) const {
            float tmin = t_min, tmax = t_max;
            for (int a = 0; a < 3; a++) {
                auto t0 = fmin((minimum[a] - r.getOrigin()[a]) / r.getDirection()[a],
                               (maximum[a] - r.getOrigin()[a]) / r.getDirection()[a]);
                auto t1 = fmax((minimum[a] - r.getOrigin()[a]) / r.getDirection()[a],
                               (maximum[a] - r.getOrigin()[a]) / r.getDirection()[a]);
                tmin = fmax(t0, tmin);
                tmax = fmin(t1, tmax);
                if (tmax <= tmin)
                    return false;
            }
            t_min = tmin;
            return true;
        }

        double area() const {
            auto a = maximum.x() - minimum.x();
            auto b = maximum.y() - minimum.y();
            auto c = maximum.z() - minimum.z();
            return 2*(a*b + b*c + c*a);
        }

        int longest_axis() const {
            auto a = maximum.x() - minimum.x();
            auto b = maximum.y() - minimum.y();
            auto c = maximum.z() - minimum.z();
            if (a > b && a > c)
                return 0;
            else if (b > c)
                return 1;
            else
                return 2;
        }
        static AABB surrounding_box(AABB box0, AABB box1) {
            Vector3f small(fmin(box0.min().x(), box1.min().x()),
                    fmin(box0.min().y(), box1.min().y()),
                    fmin(box0.min().z(), box1.min().z()));

            Vector3f big  (fmax(box0.max().x(), box1.max().x()),
                    fmax(box0.max().y(), box1.max().y()),
                    fmax(box0.max().z(), box1.max().z()));

            return AABB(small,big);
        }

    public:
        Vector3f minimum;
        Vector3f maximum;
};



#endif // AABB_H