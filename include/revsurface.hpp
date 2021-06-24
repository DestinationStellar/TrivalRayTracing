#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include "utils.hpp"
#include <tuple>
#include <iostream>

const int resolution = 10;
const int NEWTON_STEPS = 100;
const float NEWTON_EPS = 1e-4;

class RevSurface : public Object3D {

    shared_ptr<Curve> pCurve;
    AABB bound_box;
    // to determine the axis
    Vector3f point;
    // for cylinder
    double radius;
    double maxy, miny;

public:
    RevSurface(shared_ptr<Curve> pCurve, shared_ptr<Material> material) : pCurve(pCurve), Object3D(material) {
        // Check flat.
        for (const auto &cp : pCurve->getControls()) {
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
        }
        bound_box = AABB(Vector3f(-pCurve->radius, pCurve->y_min - 3, -pCurve->radius),
                 Vector3f(pCurve->radius, pCurve->y_max + 3, pCurve->radius));
        point = Vector3f::ZERO;
        miny = pCurve->y_min;
        maxy = pCurve->y_max;
        radius = pCurve->radius;
    }

    ~RevSurface() override {}

    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const override {

        static const double eps = 0.01;
        Vector3f ro = r.getOrigin();
        Vector3f rd = r.getDirection();
        double dt, dl, d0;

        // intersect with the cylinder
        double l, a, b, c;
        a = (rd.x() * rd.x() + rd.z() * rd.z());
        b = (2 * rd.x() * (ro.x() - point.x()) + 2 * rd.z() * (ro.z() - point.z()));
        c = point.x() * point.x() + point.z() * point.z() + ro.x() * ro.x() + ro.z() * ro.z() - 2 * ro.x() * point.x() - 2 * ro.z() * point.z() - radius * radius;
        double det = b * b - 4 * a * c;

        if (det < 0) return false;
        else det = std::sqrt(det);
        if ((-b - det) / (2 * a) > tmin) l = (-b - det) / (2 * a);
        else if ((-b + det) / (2 * a) > tmin) l = (-b + det) / (2 * a);
        else return false;
        Vector3f near = ro + l * rd;

        double t = (near.y() - miny) / (maxy - miny);
        if (t < 0 || t > 1) return false;
        Vector3f np = near - point;
        float theta = std::acos(np.x() / radius);


        float s = t, tr = l;
        Vector3f p = r.pointAtParameter(tr);
        double eta = 1;
        for (int i = 0; i < NEWTON_STEPS; ++i) {
            
            if (theta < 0.0 || theta >= 2 * M_PI) theta = std::acos(np.x() / radius);
            if (s >= 1) s = 1 - FLT_EPSILON;
            if (s <= 0) s = 0 + FLT_EPSILON;
            // if (tr >= tmin && tr < tmax) tr = l;
            p = r.pointAtParameter(tr);
            CurvePoint fs = pCurve->caculate(s);
            Vector3f F(p.x() - fs.V.x() * cos(theta),
                       p.y() - fs.V.y(),
                       p.z() - fs.V.x() * sin(theta));
            if (F.length() < eps) {
                if (tr >= tmin && tr < tmax) {
                    Vector3f n(-fs.T.y() * cos(theta), fs.T.x(), -fs.T.y() * sin(theta));
                    n.normalize();
                    h.set(tr, material, n, r);
                    return true;
                }
                return false;
            }
            
            Matrix3f JF(r.getDirection().x(), -cos(theta) * fs.T.x(), sin(theta) * fs.V.x(),
                        r.getDirection().y(), -fs.T.y(),              0,
                        r.getDirection().z(), -sin(theta) * fs.T.x(), -cos(theta) * fs.V.x());
            Vector3f delta = JF.inverse() * F;
            tr -= eta * delta.x(); s -= eta * delta.y(); theta -= eta * delta.z();
            eta *= 0.99;
            if (std::isnan(tr) || std::isnan(s) || std::isnan(theta))
                return false;
        }
        return false;
    }

    bool newtonIntersect(const Ray &r, Hit &h, float tmin, float tmax) const {
        float theta, mu, t=tmin;
        getUV(r, t, theta, mu);
        Vector3f normal, hit_point;
        // cout << "begin!" << endl;
        if (!newton(r, t, theta, mu, normal, hit_point)) {
            // cout << "Not Intersect! t:" << t << " theta: " << theta / (2 *
            // M_PI)
            //      << " mu: " << mu << endl;
            return false;
        }
        if (!std::isnormal(mu) || !std::isnormal(theta) || !std::isnormal(t)) return false;
        if (t < 0 || mu < pCurve->range[0] || mu > pCurve->range[1] || t > tmax) {
            return false;
        }  
        h.set(t, material, normal.normalized(), r);
        // std::cout << "Intersect! t:" << t << " theta: " << theta / (2 * M_PI)
        //      << " mu: " << mu << std::endl;
        return true;
    }

    bool newton(const Ray &r, float &t, float &theta, float &mu,
                Vector3f &normal, Vector3f &hit_point) const {
        Vector3f dmu, dtheta;
        float range0 = pCurve->range[0],range1 = pCurve->range[1];
        for (int i = 0; i < NEWTON_STEPS; ++i) {
            if (theta < 0.0) theta += 2 * M_PI;
            if (theta >= 2 * M_PI) theta = fmod(theta, 2 * M_PI);
            if (mu >= pCurve->range[1]) mu = pCurve->range[1] - FLT_EPSILON;
            if (mu <= pCurve->range[0]) mu = pCurve->range[0] + FLT_EPSILON;
            hit_point = getPoint(theta, mu, dtheta, dmu);
            Vector3f f = r.getOrigin() + r.getDirection() * t - hit_point;
            float dist2 = f.squaredLength();
            normal = Vector3f::cross(dmu, dtheta);
            if (dist2 < NEWTON_EPS) {
                return true;
            }
            float D = Vector3f::dot(r.getDirection(), normal);
            t -= Vector3f::dot(dmu, Vector3f::cross(dtheta, f)) / D;
            mu -= Vector3f::dot(r.getDirection(), Vector3f::cross(dtheta, f)) / D;
            theta += Vector3f::dot(r.getDirection(), Vector3f::cross(dmu, f)) / D;
        }
        return false;
    }

    Vector3f getPoint(const float &theta, const float &mu, Vector3f &dtheta,
                      Vector3f &dmu) const {
        Vector3f pt;
        Quat4f rot;
        rot.setAxisAngle(theta, Vector3f::UP);
        Matrix3f rotMat = Matrix3f::rotation(rot);
        CurvePoint cp = pCurve->caculate(mu);
        pt = rotMat * cp.V;
        dmu = rotMat * cp.T;
        dtheta = Vector3f(-cp.V.x() * sin(theta), 0, -cp.V.x() * cos(theta));
        return pt;
    }

    void getUV(const Ray &r, const float &t, float &theta, float &mu) const {
        Vector3f pt(r.getOrigin() + r.getDirection() * t);
        theta = atan2(-pt.z(), pt.x()) + M_PI;
        mu = (pCurve->y_max - pt.y()) / (pCurve->y_max - pCurve->y_min);
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const {
        output_box = bound_box;
        return true;
    }

protected:

    inline float function(const Vector3f &dir, const Vector3f &ori, float x_t, float y_t) const {
        return (y_t-ori[1])*(y_t-ori[1])*(dir[0]*dir[0]+dir[2]*dir[2]) + 
                2*(y_t-ori[1])*(ori[0]*dir[0]*dir[1]+ori[2]*dir[2]*dir[1]) + 
                (ori[0]*ori[0] + ori[2]*ori[2]) * dir[1] * dir[1] - dir[1]*dir[1]*x_t*x_t;
    }

    inline float derive(const Vector3f &dir, const Vector3f &ori, float x_t, float y_t, float x_td, float y_td) const {
        return 2*(dir[0]*dir[0]+dir[2]*dir[2])*(y_t-ori[1])*y_td + 
                2*(ori[0]*dir[0]*dir[1]+ori[2]*dir[2]*dir[1])*y_td - 2*dir[1]*dir[1]*x_t*x_td;
    }

};

#endif //REVSURFACE_HPP