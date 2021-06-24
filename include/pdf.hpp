#ifndef PDF_H
#define PDF_H

#include <vecmath.h>
#include <cmath>

#include "ONB.hpp"
#include "utils.hpp"

class Group;

class Object3D;

class PDF  {
    public:
        virtual ~PDF() {}

        virtual double value(const Vector3f& direction) const = 0;
        virtual Vector3f generate() const = 0;
};


class CosinePDF : public PDF {
    public:
        CosinePDF(const Vector3f& w) { uvw.build_from_w(w); }

        virtual double value(const Vector3f& direction) const override {
            auto cosine = Vector3f::dot(direction.normalized(), uvw.w());
            return (cosine <= 0) ? 0 : cosine/M_PI;
        }

        virtual Vector3f generate() const override {
            return uvw.local(random_cosine_direction());
        }

    public:
        ONB uvw;
};

class SpherePDF : public PDF {
  public:
    SpherePDF() { }

    double value(const Vector3f& direction) const override {
        return 1/ (4 * M_PI);
    }

    Vector3f generate() const override {
        return random_unit_vector();
    }
};

class HittablePDF : public PDF {
    public:
        HittablePDF(const Group* l, const Vector3f& origin) : list(l), o(origin) {}

        virtual double value(const Vector3f& direction) const override ;

        virtual Vector3f generate() const override ;

    public:
        Vector3f o;
        const Group* list;
};


class MixturePDF : public PDF {
    public:
        MixturePDF(shared_ptr<PDF> p0, shared_ptr<PDF> p1) {
            p[0] = p0;
            p[1] = p1;
        }

        virtual double value(const Vector3f& direction) const override {
            return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
        }

        virtual Vector3f generate() const override {
            if (random_double() < 0.5)
                return p[0]->generate();
            else
                return p[1]->generate();
        }

    public:
        shared_ptr<PDF> p[2];
};

#endif // PDF_H