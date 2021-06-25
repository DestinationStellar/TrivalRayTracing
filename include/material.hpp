#ifndef MATERIAL_H
#define MATERIAL_H

#include <cassert>
#include <vecmath.h>

#include "ray.hpp"
#include "hit.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "pdf.hpp"

struct ScatterRecord {
    Ray specular_ray;
    bool is_specular;
    Vector3f attenuation;
    shared_ptr<PDF> pdf_ptr;
};
class Material {
public:

    Material(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0
        ) : ambientColor(a_color), diffuseColor(d_color), specularColor(s_color), shininess(s){

    }

    Material () {}

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

    virtual bool scatter(const Ray &r_in, const Hit &hit, ScatterRecord &srec) const {
        return false;
    }

    virtual double scatterPDF(const Ray &r_in, const Hit &hit, const Ray &scattered) const {
        return 0;
    }

    virtual Vector3f emitted(const Hit &hit, double u, double v, const Vector3f& p, bool isLight = false) {
        return Vector3f(0,0,0);
    }

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        Vector3f shaded = Vector3f::ZERO;
        float diffuse=Vector3f::dot(dirToLight,hit.getNormal());
        diffuse = clamp(diffuse, 0.0);
        Vector3f R=2*(Vector3f::dot(hit.getNormal(),dirToLight)*hit.getNormal())-dirToLight;
        R.normalize();
        float specular=Vector3f::dot(-ray.getDirection(),R);
        specular = clamp(specular, 0.0);
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
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0,
        shared_ptr<Texture> a = nullptr
    ): Material(a_color,d_color,s_color,s){
        albedo = a;
    }
    Lambertian(Vector3f c) {
        albedo = make_shared<SolidColor>(c);
    }
    Lambertian(shared_ptr<Texture> a) {
        albedo = a;
    }
    bool scatter(const Ray &r_in, const Hit &hit,  ScatterRecord &srec) const override {
        srec.is_specular = false;
        srec.attenuation = albedo->value(hit.u, hit.v, hit.getIntersectP());
        srec.pdf_ptr = make_shared<CosinePDF>(hit.getNormal());
        return true;
    }
    double scatterPDF(
        const Ray& r_in, const Hit& rec, const Ray& scattered
    ) const override {
        auto cosine = Vector3f::dot(rec.getNormal(), scattered.getDirection().normalized());
        return cosine < 0 ? 0 : cosine/M_PI;
    }
protected:
    shared_ptr<Texture> albedo;
};

class Metal : public Material {
public:
    Metal(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color , float s = 0,
        const Vector3f &a = Vector3f::ZERO, float f = 0 
    ): Material(a_color,d_color,s_color,s){
        albedo = a;
        fuzz = f;
    }
    Metal (const Vector3f &a, float f=0.0) : albedo(a), fuzz(f) {}
    bool scatter(const Ray &r_in, const Hit &hit, ScatterRecord &srec) const override {
        Vector3f reflected = reflect(r_in.getDirection().normalized(), hit.getNormal());
        srec.specular_ray =
            Ray(hit.getIntersectP(), reflected + fuzz*random_in_unit_sphere(), r_in.getTime());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        return true;
    }
protected:
    Vector3f albedo;
    float fuzz;
};

class Dielectric : public Material {
public:
    Dielectric(
        const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0,
        float i = 0, Vector3f a = Vector3f(1.0, 1.0, 1.0)
    ): Material(a_color,d_color,s_color,s){
        ir = i; attenuation = a;
    }

    Dielectric(float i = 0, Vector3f a = Vector3f(1.0, 1.0 ,1.0)): ir(i), attenuation(a) {}

    bool scatter(const Ray &r_in, const Hit &hit, ScatterRecord &srec) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = attenuation;
        double refraction_ratio = hit.getFrontFace() ? (1.0/ir) : ir;

        Vector3f unit_direction = r_in.getDirection().normalized();
        double cos_theta = fmin(Vector3f::dot(-unit_direction, hit.getNormal()), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        Vector3f direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_direction, hit.getNormal());
        else
            direction = refract(unit_direction, hit.getNormal(), refraction_ratio);

        srec.specular_ray = Ray(hit.getIntersectP(), direction, r_in.getTime());
        return true;
    }
protected:
    float ir;
    Vector3f attenuation;
    static double reflectance(double cosine, double ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }
};

class DiffuseLight : public Material {
    public:
        DiffuseLight(
            const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0,
            shared_ptr<Texture> a = nullptr, float i = 1.0
        ): Material(a_color,d_color,s_color,s), emit(a), illumination(i) {}

        DiffuseLight(
            const Vector3f &a_color, const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0,
            Vector3f c = Vector3f::ZERO, float i = 1.0
        ): Material(a_color,d_color,s_color,s), illumination(i) {
            emit = make_shared<SolidColor>(c);
        }

        DiffuseLight(Vector3f c, float i = 1.0): illumination(i) {
            emit = make_shared<SolidColor>(c);
        }

        DiffuseLight(shared_ptr<Texture> a, float i = 1.0): illumination(i) {
            emit = a;
        }

        Vector3f emitted(const Hit &hit, double u, double v, const Vector3f& p, bool isLight = false) override {
            if (!hit.getFrontFace()) {
                return Vector3f::ZERO;
            }
            if (isLight){
                return emit->value(u, v, p);
            }
            return emit->value(u, v, p)*illumination;
        }

    public:
        shared_ptr<Texture> emit;
        float illumination;
};

class Isotropic : public Material {
    public:
        Isotropic(Vector3f c) {
            albedo = make_shared<SolidColor>(c);
        }
        Isotropic(shared_ptr<Texture> a) : albedo(a) {}

        virtual bool scatter(
            const Ray& r_in, const Hit& rec, ScatterRecord &srec
        ) const override {
            srec.attenuation = albedo->value(rec.u, rec.v, rec.getIntersectP());
            srec.is_specular = false;
            srec.pdf_ptr = make_shared<SpherePDF>();
            return true;
        }

        double scatterPDF(const Ray& r_in, const Hit& rec, const Ray& scattered) const override {
            return 1 / (4 * M_PI);
        }

    public:
        shared_ptr<Texture> albedo;
};

#endif // MATERIAL_H
