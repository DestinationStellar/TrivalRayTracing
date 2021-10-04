#ifndef TEXTURE_H
#define TEXTURE_H

#include "perlin.hpp"
#include "utils.hpp"



#include <vecmath.h>
#include <iostream>


class Texture  {
    public:
        virtual Vector3f value(double u, double v, const Vector3f& p) const = 0;
};


class SolidColor : public Texture {
    public:
        SolidColor() {}
        SolidColor(Vector3f c) : color_value(c) {}

        SolidColor(double red, double green, double blue)
          : SolidColor(Vector3f(red,green,blue)) {}

        virtual Vector3f value(double u, double v, const Vector3f& p) const override {
            return color_value;
        }

    private:
        Vector3f color_value;
};


class CheckerTexture : public Texture {
    public:
        CheckerTexture() {}

        CheckerTexture(double scale, shared_ptr<Texture> _even, shared_ptr<Texture> _odd)
            :inv_scale(1.0/scale), even(_even), odd(_odd) {}

        CheckerTexture(double scale, Vector3f c1, Vector3f c2)
            : inv_scale(1.0/scale), even(make_shared<SolidColor>(c1)), odd(make_shared<SolidColor>(c2)) {}

        virtual Vector3f value(double u, double v, const Vector3f& p) const override {
            auto xInteger = static_cast<int>(std::floor(inv_scale * p.x()));
            auto yInteger = static_cast<int>(std::floor(inv_scale * p.y()));
            auto zInteger = static_cast<int>(std::floor(inv_scale * p.z()));

            bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

            return isEven ? even->value(u, v, p) : odd->value(u, v, p);
        }

    public:
        shared_ptr<Texture> odd;
        shared_ptr<Texture> even;
        double inv_scale;
};


class NoiseTexture : public Texture {
    public:
        NoiseTexture() {}
        NoiseTexture(double sc) : scale(sc) {}

        virtual Vector3f value(double u, double v, const Vector3f& p) const override {
            // return Vector3f(1,1,1)*0.5*(1 + noise.turb(scale * p));
            // return Vector3f(1,1,1)*noise.turb(scale * p);
            Vector3f s = scale * p;
            return Vector3f(1,1,1)*0.5*(1 + sin(s.z() + 10*noise.turb(s)));
        }

    public:
        perlin noise;
        double scale;
};


class ImageTexture : public Texture {
    public:
        const static int bytes_per_pixel = 3;

        ImageTexture()
          : data(nullptr), width(0), height(0), bytes_per_scanline(0) {}

        ImageTexture(const char* filename);

        ~ImageTexture();

        virtual Vector3f value(double u, double v, const Vector3f& p) const override;

    private:
        unsigned char *data;
        int width, height;
        int bytes_per_scanline;
};


#endif // TEXTURE_H
