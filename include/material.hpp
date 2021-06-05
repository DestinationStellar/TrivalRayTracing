#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "utils.hpp"

class Material {
public:

    explicit Material(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0
        ) : ambientColor(a_color), diffuseColor(d_color), specularColor(s_color), shininess(s){

    }

    virtual ~Material() = default;

    Vector3f getDiffuseColor() const {
        return diffuseColor;
    }

    Vector3f getAmbientColor() const {
        return ambientColor;
    }

    Vector3f getSpecularColor() const {
        return specularColor;
    }

    virtual bool Scatter(const Ray &r_in, const Hit &hit, Vector3f &attenuation, Ray &scattered) = 0;

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        Vector3f shaded = Vector3f::ZERO;
        float diffuse=Vector3f::dot(dirToLight,hit.getNormal());
        diffuse = clampFloatNegative(diffuse);
        Vector3f R=2*(Vector3f::dot(hit.getNormal(),dirToLight)*hit.getNormal())-dirToLight;
        R.normalize();
        float specular=Vector3f::dot(-ray.getDirection(),R);
        specular = clampFloatNegative(specular);
        if(specular!=0){
            specular=pow(specular,shininess);
        }
        shaded = lightColor*(diffuseColor*diffuse+specularColor*specular);
        return shaded;
    }
    

protected:
    Vector3f ambientColor;
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};

class Lambertian : public Material {
public:
    Lambertian(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO,
        float s = 0, const Vector3f &a = Vector3f::ZERO
    ): Material(a_color,d_color,s_color,s){
        albedo = a;
    }
    bool Scatter(const Ray &r_in, const Hit &hit, Vector3f &attenuation, Ray &scattered) override {
        Vector3f scatter_direction = hit.getNormal() + random_unit_vector();
        // Catch degenerate scatter direction
        if (near_zero(scatter_direction)){
            scatter_direction = hit.getNormal();
        }
        scattered = Ray(hit.getIntersectP(),scatter_direction);
        attenuation = albedo;
        return true;
    }
protected:
    Vector3f albedo;
};

class Metal : public Material {
public:
    Metal(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO,
        float s = 0, const Vector3f &a = Vector3f::ZERO, float f = 0 
    ): Material(a_color,d_color,s_color,s){
        albedo = a;
        fuzz = f;
    }
    bool Scatter(const Ray &r_in, const Hit &hit, Vector3f &attenuation, Ray &scattered) override {
        Vector3f reflected = reflect(r_in.getDirection(), hit.getNormal());
        scattered = Ray(hit.getIntersectP(),reflected+fuzz*random_in_unit_sphere());
        attenuation = albedo;
        return (Vector3f::dot(scattered.getDirection(),hit.getNormal())>0);
    }
protected:
    Vector3f albedo;
    float fuzz;
};

class Dielectric : public Material {
public:
    Dielectric(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO,
        float s = 0, float i = 0
    ): Material(a_color,d_color,s_color,s){
        ir = i;
    }
    bool Scatter(const Ray &r_in, const Hit &hit, Vector3f &attenuation, Ray &scattered) override {
        attenuation = Vector3f(1.0, 1.0, 1.0);
        double refraction_ratio = hit.getFrontFace() ? (1.0/ir) : ir;

        Vector3f unit_direction = r_in.getDirection(); // unit vector required
        double cos_theta = fmin(Vector3f::dot(-unit_direction, hit.getNormal()), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        Vector3f direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, hit.getNormal());
        else
            direction = refract(unit_direction, hit.getNormal(), refraction_ratio);

        scattered = Ray(hit.getIntersectP(), direction);
        return true;
    }
protected:
    float ir;
    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }
};

class DiffuseLight : public Material {

};

#endif // MATERIAL_H
