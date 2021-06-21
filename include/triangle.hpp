#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		this->vertices[0]=a;
		this->vertices[1]=b;
		this->vertices[2]=c;
		this->e_1=vertices[0]-vertices[1];
		this->e_2=vertices[0]-vertices[2];
		this->normal=Vector3f::cross(e_1,e_2);
		normal.normalize();
	}

	bool intersect(const Ray& ray,  Hit& hit , float tmin = 0.0 , float tmax = infinity) const override {
		Vector3f s=vertices[0]-ray.getOrigin();
		Vector3f result=Vector3f(Matrix3f(s,e_1,e_2).determinant(),Matrix3f(ray.getDirection(),s,e_2).determinant(),Matrix3f(ray.getDirection(),e_1,s).determinant());
		result = result/Matrix3f(ray.getDirection(),e_1,e_2).determinant();
		if(result.x()>=tmin&&result.x()<tmax){
			if(result.y()>=0&&result.z()>=0&&result.y()+result.z()<=1){
				// if(Vector3f::dot(normal,ray.getOrigin())<0){
                // 	normal = -normal;
            	// }
				hit.set(result.x(),material,normal,ray);
				return true;
			}
		}
        return false;
	}

	bool bounding_box(double time0, double time1, AABB& output_box) const override {
		return true;
	}

	Vector3f normal;
	Vector3f vertices[3];
protected:
	Vector3f e_1;
	Vector3f e_2;
};

#endif //TRIANGLE_H
