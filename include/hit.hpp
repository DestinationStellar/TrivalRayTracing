#ifndef HIT_H
#define HIT_H

#include <vecmath.h>
#include "ray.hpp"
#include "utils.hpp"

class Material;

class Hit {
public:
    float u,v;
    // constructors
    Hit() {
        t = 1e38;
        time = 0;
        u = 0;
        v = 0;
        frontFace = true;
        material = nullptr;
        normal = Vector3f::ZERO;
        intersectP = Vector3f::ZERO;
        fluxLight = Vector3f::ZERO;
        attenuation = Vector3f(1);
    }

    Hit(float _t, float _time, bool _f, shared_ptr<Material> m, const Vector3f &n, const Vector3f &p) {
        t = _t;
        time = _time;
        frontFace = _f;
        material = m;
        normal = n;
        intersectP = p;
        fluxLight = Vector3f::ZERO;
        attenuation = Vector3f(1);
    }

    Hit(const Hit &h) {
        t = h.t;
        time = h.time;
        frontFace = h.frontFace;
        material = h.material;
        normal = h.normal;
        intersectP = h.intersectP;
    }

    // destructor
    ~Hit() = default;

    float getT() const {
        return t;
    }

    bool getFrontFace() const {
        return frontFace;
    }

    shared_ptr<Material> getMaterial() const {
        return material;
    }


    const Vector3f &getNormal() const {
        return normal;
    }

    const Vector3f &getIntersectP() const {
        return intersectP;
    }

    void set(float _t, shared_ptr<Material> m, const Vector3f &outside_n, const Ray &r) {
        t = _t;
        material = m;
        frontFace = Vector3f::dot(r.getDirection(), outside_n) < 0;
        normal = frontFace ? outside_n :-outside_n;
        intersectP = r.pointAtParameter(t);
    }

    
public:
    float t;
    float time; 
    bool frontFace;
    shared_ptr<Material> material;
    Vector3f normal;
    Vector3f intersectP;
    Vector3f fluxLight, attenuation;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
