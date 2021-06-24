#ifndef UTILS_H
#define UTILS_H

#include <vecmath.h>
#include <limits>
#include <memory>

using std::shared_ptr;
using std::make_shared;

const double infinity = std::numeric_limits<double>::infinity();

#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

inline float clamp(float in, float _min = 0.0, float _max = infinity) {
    if (in < _min) return _min;
    else if (in > _max) return _max;
    return in;
}

inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    // Returns a random integer in [min,max].
    return static_cast<int>(random_double(min, max+1));
}

inline static Vector3f random_vector3f() {
    return Vector3f(random_double(), random_double(), random_double());
}

inline static Vector3f random_vector3f(double min, double max) {
    return Vector3f(random_double(min,max), random_double(min,max), random_double(min,max));
}

inline Vector3f random_in_unit_sphere() {
    while (true) {
        Vector3f p = random_vector3f(-1,1);
        if (p.squaredLength() >= 1) continue;
        return p;
    }
}

inline Vector3f random_unit_vector() {
    // two ways to produce

    // auto a = random_double(0, 2*pi);
    // auto z = random_double(-1, 1);
    // auto r = sqrt(1 - z*z);
    // return Vector3f(r*cos(a), r*sin(a), z);

    return random_in_unit_sphere().normalized();
}

inline Vector3f random_in_unit_disk() {
    while (true) {
        auto p = Vector3f(random_double(-1,1), random_double(-1,1), 0);
        if (p.squaredLength() >= 1) continue;
        return p;
    }
}

inline const bool near_zero(Vector3f &e) {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
}

inline Vector3f reflect(const Vector3f& v, const Vector3f& n) {
    return v - 2*Vector3f::dot(v,n)*n;
}

inline Vector3f refract(const Vector3f& uv, const Vector3f& n, double etai_over_etat) {
    auto cos_theta = fmin(Vector3f::dot(-uv, n), 1.0);
    Vector3f r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    Vector3f r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.squaredLength())) * n;
    return r_out_perp + r_out_parallel;
}

inline Vector3f random_cosine_direction() {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1-r2);

    auto phi = 2*M_PI*r1;
    auto x = cos(phi)*sqrt(r2);
    auto y = sin(phi)*sqrt(r2);

    return Vector3f(x, y, z);
}

// transforms a 3D point using a matrix, returning a 3D point
static Vector3f transformPoint(const Matrix4f &mat, const Vector3f &point) {
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D directino using a matrix, returning a direction
static Vector3f transformDirection(const Matrix4f &mat, const Vector3f &dir) {
    return (mat * Vector4f(dir, 0)).xyz();
}

#endif // UTILS_H