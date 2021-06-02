#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include <vector>
#include <iostream>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH, int samp) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
		this->horizontal.normalize();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
        this->sample = samp;
    }

    // Generate rays for each screen-space coordinate
    virtual std::vector<Ray> generateRay(const Vector2f &point) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
    int sample;
};

// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float angle, int samp) : Camera(center, direction, up, imgW, imgH, samp) {
        // angle is in radian.
        f_x= f_y = 2*tanf(angle/2)/height;
        c_x= width/2.0f;
        c_y= height/2.0f;
    }

    std::vector<Ray> generateRay(const Vector2f &point) override {
        Matrix3f R=Matrix3f(horizontal,-up,direction);
        std::vector<Ray> ray_vec;
        Vector3f dir_ray = Vector3f::ZERO;
        for(int i=0;i<sample;i++){
            float bias_x = (rand()%sample)/(float)sample;
            float bias_y = (rand()%sample)/(float)sample;
            dir_ray=Vector3f((point.x()+bias_x-c_x)*f_x,(c_y-point.y()-bias_y)*f_y,1.0);
            dir_ray.normalize();
            dir_ray=R*dir_ray;
            ray_vec.push_back(Ray(center,dir_ray));
        }
        return ray_vec; 
    }
protected:
    float f_x;
    float f_y;
    int c_x;
    int c_y;
};

#endif //CAMERA_H
