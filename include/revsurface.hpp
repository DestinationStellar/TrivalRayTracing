#ifndef REVSURFACE_HPP
#define REVSURFACE_HPP

#include "object3d.hpp"
#include "curve.hpp"
#include "utils.hpp"
#include "cylinder.hpp"
#include "bvh.hpp"
#include "mesh.hpp"
#include <tuple>
#include <iostream>

const int NEWTON_STEPS = 100;
const float NEWTON_EPS = 1e-4;

class RevSurface : public Object3D {
    // Definition for drawable surface.
    typedef std::tuple<unsigned, unsigned, unsigned> Tup3u;
    shared_ptr<Mesh> tri_mesh;
    // Surface is just a struct that contains vertices, normals, and
    // faces.  VV[i] is the position of vertex i, and VN[i] is the normal
    // of vertex i.  A face is a triple i,j,k corresponding to a triangle
    // with (vertex i, normal i), (vertex j, normal j), ...
    // Currently this struct is computed every time when canvas refreshes.
    // You can store this as member function to accelerate rendering.
public:
    RevSurface(shared_ptr<Curve> pCurve, shared_ptr<Material> material) : pCurve(pCurve), Object3D(material) {
        // Check flat.
        for (const auto &cp : pCurve->getControls()) {
            if (cp.z() != 0.0) {
                printf("Profile of revSurface must be flat on xy plane.\n");
                exit(0);
            }
        }
        resolution = 100; resolution_mesh = 30;
        miny = pCurve->y_min;
        maxy = pCurve->y_max;
        radius = pCurve->radius;
        meshInit();
        // pCurve->discretize(resolution, curvePoints);
        // std::vector<shared_ptr<Object3D>> cylinders;
        // cylinders.reserve(curvePoints.size()-1);
        // for (int i=0; i<curvePoints.size()-1; i++) {
        //     float r = fmax(fabs(curvePoints[i].V.x()), fabs(curvePoints[i+1].V.x()));
        //     cylinders.push_back(make_shared<Cylinder>(Vector3f(0,0,0), r, fmin(curvePoints[i].V.y(), curvePoints[i+1].V.y()),
        //                     fmax(curvePoints[i].V.y(), curvePoints[i+1].V.y()), material));
        // }
        // cylinder_bvh = make_shared<BVHnode>(cylinders, 0, cylinders.size(), 0, 0);
    }

    ~RevSurface() override {}

    bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const override {

        // Vector3f ro = r.getOrigin();
        // Vector3f rd = r.getDirection();
        // double dt, dl, d0;

        // // intersect with the cylinder
        // double l, a, b, c;
        // a = (rd.x() * rd.x() + rd.z() * rd.z());
        // b = (2 * rd.x() * ro.x() + 2 * rd.z() * ro.z());
        // c = ro.x() * ro.x() + ro.z() * ro.z() - radius * radius;
        // double det = b * b - 4 * a * c;

        // if (det < 0) return false;
        // else det = std::sqrt(det);
        // if ((-b - det) / (2 * a) > tmin) l = (-b - det) / (2 * a);
        // else if ((-b + det) / (2 * a) > tmin) l = (-b + det) / (2 * a);
        // else return false;
        // if(l > tmax) return false;
        // Vector3f np = ro + l * rd;

        // double t = (np.y() - miny) / (maxy - miny);
        // if (t < 0 || t > 1) return false;
        // double theta0 = std::acos(np.x() / radius);

        // Hit rec;
        // std::vector<float> t_vec;
        // if (cylinder_bvh->intersect(r, rec, tmin, tmax)) { 
        //     l = rec.getT();
        //     theta0 = rec.u * 2 * M_PI;
        //     np = ro + l * rd;
        //     t = (np.y() - miny) / (maxy - miny);
        //     for (int i = 0; i < curvePoints.size()-1; i++) {
        //         if (np.y() >= fmin(curvePoints[i].V.y(), curvePoints[i+1].V.y()) && np.y() < fmax(curvePoints[i].V.y(), curvePoints[i+1].V.y())) {
        //             t_vec.push_back((i + (np.y() - curvePoints[i].V.y())/(curvePoints[i+1].V.y() - curvePoints[i].V.y()))/(float)resolution);
        //         }
        //     }

        // }
        // bool flag = false;float cloest_t = tmax;
        // if (Levenberg_Marquardt(r, h, tmin, cloest_t, l, t, theta0)){
        //     flag = true;
        //     cloest_t = h.getT();
        // }
        // for (auto tt : t_vec) {
        //     if (Levenberg_Marquardt(r, h, tmin, cloest_t, l, tt, theta0)) {
        //         cloest_t = h.getT();
        //         flag = true;
        //     }
        // }

        // return flag;

        // float interval = 0.01;
        // for (int j = 0; j < 10; j ++){
        //     double s1 = t + interval;
        //     double s2 = t - interval;
        //     if (s1 <= 1)
        //         if (Levenberg_Marquardt(r, h, tmin, tmax, l, s1, theta0)) return true;
        //     if (s2 >= 0)
        //         if (Levenberg_Marquardt(r, h, tmin, tmax, l, s2, theta0)) return true;
        //     interval *= 1.45;
        // }
        
        // float interval = 100;
        // for (int j = 0; j <= interval; j ++){
        //     double s = pCurve->range[0] + (pCurve->range[1] - pCurve->range[0]) * (float)j/interval;
        //     if (Levenberg_Marquardt(r, h, tmin, tmax, l, s, theta0)) return true;

        // }

        return tri_mesh->intersect(r, h, tmin, tmax);
    }

    bool newton_iteration(const Ray &r, Hit &h, float tmin, float tmax, double tr, double s, double theta) const {
        double eps = 1, eta = 1;
        Vector3f F0 = Vector3f(0, 0, 0);
        for (int i = 0; i < NEWTON_STEPS; ++i) {
            if (s >= 1) s = 1.0 - FLT_EPSILON;
            if (s <= 0) s = FLT_EPSILON;
            Vector3f p = r.pointAtParameter(tr);
            CurvePoint fs = pCurve->caculate(s);
            Vector3f F(p.x() - fs.V.x() * cos(theta),
                    p.y() - fs.V.y(),
                    p.z() - fs.V.x() * sin(theta));
            if (F.length() < eps) {
                if(eps < 0.01){
                    if (tr >= tmin && tr < tmax) {
                        Vector3f n(-fs.T.y() * cos(theta), fs.T.x(), -fs.T.y() * sin(theta));
                        n.normalize();
                        h.set(tr, material, n, r);
                        h.u = theta/(2*M_PI);
                        h.v = 1 - s;   
                        return true;
                    }
                    return false;
                }
                eps *= 0.8, eta *= 0.8;
            }
            
            Matrix3f JF(r.getDirection().x(), -cos(theta) * fs.T.x(), sin(theta) * fs.V.x(),
                        r.getDirection().y(), -fs.T.y(),              0,
                        r.getDirection().z(), -sin(theta) * fs.T.x(), -cos(theta) * fs.V.x());
            Vector3f delta = JF.inverse() * F;
            tr -= eta * delta.x(); s -= eta * delta.y(); theta -= eta * delta.z();
        
            if (std::isnan(tr) || std::isnan(s) || std::isnan(theta))
                return false;
        }
        return false;
    }

    bool Levenberg_Marquardt(const Ray &r, Hit &h, float tmin, float tmax, double tr, double s, double theta) const {
        float epsilon1 = 1e-10, epsilon2 = 1e-10, epsilon = 0.01;
        int imax = 100; float nu=2.0;
        Vector3f X(tr, s, theta);
        CurvePoint fs = pCurve->caculate(s);
        Vector3f p = r.pointAtParameter(tr);
        Matrix3f JF(r.getDirection().x(), -cos(theta) * fs.T.x(), sin(theta) * fs.V.x(),
                    r.getDirection().y(), -fs.T.y(),              0,
                    r.getDirection().z(), -sin(theta) * fs.T.x(), -cos(theta) * fs.V.x());
        Vector3f F(p.x() - fs.V.x() * cos(theta),
                    p.y() - fs.V.y(),
                    p.z() - fs.V.x() * sin(theta));
        Matrix3f H = JF.transposed()*JF;
        Vector3f g = JF.transposed()*F;
        bool found = g.length() <= epsilon1 || F.length() < epsilon;
        float mu = - infinity;
        for (int i = 0; i<3; i++) {
            mu = fmax(mu, H(i,i));
        }
        mu *= 0.001;
        for (int i = 0; i<=imax; i++) {
            if (found) {
                if (X.x() >= tmin && X.x() < tmax && X.y() <= pCurve->range[1] && X.y() >= pCurve->range[0] && F.length() < epsilon) {
                    Vector3f n(-fs.T.y() * cos(X.z()), fs.T.x(), -fs.T.y() * sin(X.z()));
                    n.normalize();
                    h.set(X.x(), material, n, r);
                    h.u = X.z()/(2*M_PI);
                    h.v = 1 - X.y();
                    return true;
                } 
                return false;
            }
            H(0, 0) += mu;H(1, 1) += mu;H(2, 2) += mu;
            Vector3f delta =  H.inverse() * (- g);
            
            if (delta.length() < epsilon2*(X.length() * epsilon2)) { 
                found = true;
            } else {
                Vector3f X_new = X + delta;
                CurvePoint fs_new = pCurve->caculate(X_new.y());
                Vector3f p_new = r.pointAtParameter(X_new.x());
                Vector3f F_new(p_new.x() - fs_new.V.x() * cos(X_new.z()),
                            p_new.y() - fs_new.V.y(),
                            p_new.z() - fs_new.V.x() * sin(X_new.z()));
                float rho = (F.squaredLength()-F_new.squaredLength())/(Vector3f::dot(delta, mu*delta - g));
                if (rho > 0) {
                    X = X_new;
                    fs = fs_new;
                    p = p_new;
                    F = F_new;
                    JF = Matrix3f(r.getDirection().x(), -cos(X.z()) * fs.T.x(), sin(X.z()) * fs.V.x(),
                                r.getDirection().y(), -fs.T.y(),              0,
                                r.getDirection().z(), -sin(X.z()) * fs.T.x(), -cos(X.z()) * fs.V.x());
                    H = JF.transposed()*JF;
                    g = JF.transposed()*F;
                    found = g.length() <= epsilon1 || F.length() < epsilon;
                    mu = mu * fmax(1.0/3.0, 1-pow(2*rho-1, 3));
                    nu = 2.0;
                } else {
                    mu = mu * nu;
                    nu = 2 * nu;
                }
            }
            if (std::isnan(X.x()) || std::isnan(X.y()) || std::isnan(X.z()))
                return false;
            if (X.y() >= 1) X.y() = 1.0 - FLT_EPSILON;
            if (X.y() <= 0) X.y() = FLT_EPSILON;
            if (X.x() < tmin) X.x() = tmin + FLT_EPSILON;
            if (X.x() > tmax) X.x() = tmax - FLT_EPSILON;
        }
        return false;
    }

    bool bounding_box(double time0, double time1, AABB& output_box) const {
        output_box = AABB(Vector3f(-pCurve->radius, pCurve->y_min - 0.01, -pCurve->radius),
                 Vector3f(pCurve->radius, pCurve->y_max + 0.01, pCurve->radius));
        return true;
    }

    void meshInit() {
        std::vector<Vector3f> VV;
        std::vector<Vector3f> VN;
        std::vector<Tup3u> VF;
        std::vector<CurvePoint> curve_points;
        std::vector<shared_ptr<Object3D>> triangles;
        pCurve->discretize_mesh(resolution_mesh, curve_points);
        const int steps = 40;
        for (unsigned int ci = 0; ci < curve_points.size(); ++ci) {
            const CurvePoint &cp = curve_points[ci];
            for (unsigned int i = 0; i < steps; ++i) {
                float t = (float)i / steps;
                Quat4f rot;
                // 生成参数曲面
                rot.setAxisAngle(t * 2 * 3.14159, Vector3f::UP);
                Vector3f pnew = Matrix3f::rotation(rot) * cp.V;
                Vector3f pNormal = Vector3f::cross(cp.T, -Vector3f::FORWARD);
                Vector3f nnew = Matrix3f::rotation(rot) * pNormal;
                VV.push_back(pnew);
                VN.push_back(nnew);
                int i1 = (i + 1 == steps) ? 0 : i + 1;
                if (ci != curve_points.size() - 1) {
                    // 把四边形剖分成两个三角形
                    VF.emplace_back((ci + 1) * steps + i, ci * steps + i1,
                                    ci * steps + i);
                    VF.emplace_back((ci + 1) * steps + i, (ci + 1) * steps +
                    i1,
                                    ci * steps + i1);
                }
            }
        }
        for (int i = 0; i < VF.size(); ++i) {
            shared_ptr<Triangle> t = make_shared<Triangle>(VV[std::get<0>(VF[i])], VV[std::get<1>(VF[i])],
                       VV[std::get<2>(VF[i])], material);
            t->setVNorm(VN[std::get<0>(VF[i])], VN[std::get<1>(VF[i])],
                       VN[std::get<2>(VF[i])]);
            
            triangles.push_back(t);
        }
        tri_mesh = make_shared<Mesh>(triangles, material);

    }

protected:
    std::vector<CurvePoint> curvePoints;
    shared_ptr<BVHnode> cylinder_bvh;
    shared_ptr<Curve> pCurve;
    double radius;
    double maxy, miny;
    int resolution, resolution_mesh;
};

#endif //REVSURFACE_HPP