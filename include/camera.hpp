#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>


class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up);
		this->horizontal.normalize();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(const Vector2f &point) = 0;
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
};

// You can add new functions or variables whenever needed.
class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float angle) : Camera(center, direction, up, imgW, imgH) {
        // angle is in radian.
        f_x= 2*tanf(angle/2)/width;
        f_y= 2*tanf(angle/2)/height;
        c_x= width/2.0f;
        c_y= height/2.0f;
    }

    Ray generateRay(const Vector2f &point) override {
        Vector3f dir_ray=Vector3f((point.x()-c_x)*f_x,(c_y-point.y())*f_y,1.0);
        dir_ray.normalize();
        Matrix3f R=Matrix3f(horizontal,-up,direction);
        dir_ray=R*dir_ray;
        return Ray(center,dir_ray); 
    }
protected:
    float f_x;
    float f_y;
    int c_x;
    int c_y;
};

#endif //CAMERA_H
