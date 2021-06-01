#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>
#include <iostream>
using namespace std;

class Triangle: public Object3D {

public:
	Triangle() = delete;

    // a b c are three vertex positions of the triangle
	Triangle( const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
		this->vertices[0]=a;
		this->vertices[1]=b;
		this->vertices[2]=c;
	}

	bool intersect( const Ray& ray,  Hit& hit , float tmin) override {
		Vector3f e_1=vertices[0]-vertices[1];
		Vector3f e_2=vertices[0]-vertices[2];
		Vector3f s=vertices[0]-ray.getOrigin();
		Vector3f result=Vector3f(Matrix3f(s,e_1,e_2).determinant(),Matrix3f(ray.getDirection(),s,e_2).determinant(),Matrix3f(ray.getDirection(),e_1,s).determinant());
		result = result/Matrix3f(ray.getDirection(),e_1,e_2).determinant();
		if(result.x()>=tmin&&result.x()<hit.getT()){
			if(result.y()>=0&&result.z()>=0&&result.y()+result.z()<=1){
				this->normal=Vector3f::cross(e_1,e_2);
				normal.normalize();
				if(Vector3f::dot(normal,ray.getOrigin())<0){
                	normal = -normal;
            	}
				hit.set(result.x(),material,normal);
				return true;
			}
		}
        return false;
	}
	Vector3f normal;
	Vector3f vertices[3];
protected:
};

#endif //TRIANGLE_H
