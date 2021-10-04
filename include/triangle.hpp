#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, shared_ptr<Material> m) : 
		Object3D(m),an(Vector3f::ZERO),bn(Vector3f::ZERO),cn(Vector3f::ZERO)
	{
		this->vertices[0]=a;
		this->vertices[1]=b;
		this->vertices[2]=c;
		normal = Vector3f::cross((b - a), (c - a)).normalized();
        d = Vector3f::dot(normal, a);
        cen = (a + b + c) / 3;
        nSet = false;
        tSet = false;
	}

	bool intersect(const Ray& r,  Hit& h , float tmin = 0.0 , float tmax = infinity) const override {
		// Vector3f s=vertices[0]-ray.getOrigin();
		// Vector3f result=Vector3f(Matrix3f(s,e_1,e_2).determinant(),Matrix3f(ray.getDirection(),s,e_2).determinant(),Matrix3f(ray.getDirection(),e_1,s).determinant());
		// result = result/Matrix3f(ray.getDirection(),e_1,e_2).determinant();
		// if(result.x()>=tmin&&result.x()<tmax){
		// 	if(result.y()>=0&&result.z()>=0&&result.y()+result.z()<=1){
		// 		hit.set(result.x(),material,normal,ray);
		// 		return true;
		// 	}
		// }
        // return false;

		Vector3f o(r.getOrigin()), dir(r.getDirection());
        Vector3f v0v1 = vertices[1] - vertices[0];
        Vector3f v0v2 = vertices[2] - vertices[0];
        Vector3f pvec = Vector3f::cross(dir, v0v2);
        float det = Vector3f::dot(v0v1, pvec);
        // IF CULLING
        // if (det < FLT_EPSILON) return false;
        // ray and triangle are parallel if det is close to 0
        if (fabs(det) < 1e-10) return false;
        float invDet = 1 / det;
        Vector3f tvec = o - vertices[0];
        float u = Vector3f::dot(tvec, pvec) * invDet;
        if (u < 0 || u > 1) return false;
        Vector3f qvec = Vector3f::cross(tvec, v0v1);
        float v = Vector3f::dot(dir, qvec) * invDet;
        if (v < 0 || u + v > 1) return false;
        float t = Vector3f::dot(v0v2, qvec) * invDet;
        if (t <= 0 || t > h.getT()) return false;
        Vector3f p(o + dir * t);
        getUV(p, u, v);
		h.u = u;
		h.v = v;
        h.set(t, material, getNorm(p), r);
        return true;
	}

	bool bounding_box(double time0, double time1, AABB& output_box) const override {

		Vector3f p_min = vertices[0];
		Vector3f p_max = vertices[0];
		for (int i=1; i<3; i++) {
			for (int j=0; j<3; j++) {
				p_min[j] = fmin(p_min[j], vertices[i][j]);
				p_max[j] = fmax(p_max[j], vertices[i][j]);
			}
		}
		p_min = p_min - Vector3f(0.001,0.001,0.001);
		p_max = p_max + Vector3f(0.001,0.001,0.001);
    	output_box = AABB (p_min, p_max);
		return true;
	}

	void setVNorm(const Vector3f& anorm, const Vector3f& bnorm,
                  const Vector3f& cnorm) {
        an = anorm;
        bn = bnorm;
        cn = cnorm;
        nSet = true;
    }

    void setVT(const Vector2f& _at, const Vector2f& _bt, const Vector2f& _ct) {
        at = _at;
        bt = _bt;
        ct = _ct;
        tSet = true;
    }

	Vector3f normal;
	Vector3f vertices[3];
	bool nSet = false;
    bool tSet = false;
protected:
    Vector3f cen;
    Vector2f at, bt, ct;
    Vector3f an, bn, cn;
    Vector3f bound[2];
    float d;
    
	Vector3f getNorm(const Vector3f& p) const {
        if (!nSet) return normal;
        Vector3f va = (vertices[0] - p), vb = (vertices[1] - p), vc = (vertices[2] - p);
        float ra = Vector3f::cross(vb, vc).length(),
              rb = Vector3f::cross(vc, va).length(),
              rc = Vector3f::cross(va, vb).length();
        return (ra * an + rb * bn + rc * cn).normalized();
    }

    void getUV(const Vector3f& p, float& u, float& v) const {
        if (!tSet) return;
        Vector3f va = (vertices[0] - p), vb = (vertices[1] - p), vc = (vertices[2] - p);
        float ra = Vector3f::cross(vb, vc).length(),
              rb = Vector3f::cross(vc, va).length(),
              rc = Vector3f::cross(va, vb).length();
        Vector2f uv = (ra * at + rb * bt + rc * ct) / (ra + rb + rc);
        u = uv.x();
        v = uv.y();
    }
};

#endif //TRIANGLE_H
