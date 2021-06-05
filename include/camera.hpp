#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include "utils.hpp"
#include <vecmath.h>
#include <float.h>
#include <cmath>
#include <vector>
#include <iostream>


class Camera {
public:
    Camera(const Vector3f &lookFrom, const Vector3f &lookAt, const Vector3f &vup, int imgW, int imgH) {
        this->origin = lookFrom;
        this->direction = (lookAt-lookFrom).normalized(); //w
        this->horizontal = Vector3f::cross(this->direction, vup); // u
		this->horizontal.normalize();
        this->up = Vector3f::cross(this->horizontal, this->direction); // v
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
    Vector3f origin;
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
    PerspectiveCamera(const Vector3f &lookFrom, const Vector3f &lookAt,
            const Vector3f &vup, int imgW, int imgH, float angle, float aperture, float focus_dis) : Camera(lookFrom, lookAt, vup, imgW, imgH) {
        // angle is in radian.
        auto half_height = tan(angle/2);
        auto half_width = (imgW/imgH) * half_height;
        focus_horizontal = focus_dis * half_width * horizontal * 2;
        focus_vertical = focus_dis * half_height * up * 2;
        focus_origin = origin + focus_dis*direction - focus_horizontal/2 -focus_vertical/2; 
        len_radius = aperture/2; 
    }

    Ray generateRay(const Vector2f &point) override {
        Vector3f rd = len_radius * random_in_unit_disk();
        Vector3f offset = horizontal * rd.x() + up * rd.y();
        Vector3f dir_ray = focus_origin + (point.x()/(width-1)) * focus_horizontal + (point.y()/(height-1)) * focus_vertical - origin - offset;
        dir_ray.normalize();
        return Ray(origin+offset,dir_ray);
    }
protected:
    float len_radius;
    Vector3f focus_origin; // 焦平面坐标系原点 位于焦平面左下角
    Vector3f focus_horizontal; // 焦平面水平轴
    Vector3f focus_vertical; // 焦平面垂直轴
};

#endif //CAMERA_H
