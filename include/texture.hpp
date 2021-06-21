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

        CheckerTexture(Texture *_even, Texture *_odd)
            : even(_even), odd(_odd) {}

        CheckerTexture(Vector3f c1, Vector3f c2) {
            even = new SolidColor(c1);
            odd = new SolidColor(c2);
        }

        virtual Vector3f value(double u, double v, const Vector3f& p) const override {
            auto sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0)
                return odd->value(u, v, p);
            else
                return even->value(u, v, p);
        }

    public:
        Texture *odd;
        Texture *even;
};


class NoiseTexture : public Texture {
    public:
        NoiseTexture() {}
        NoiseTexture(double sc) : scale(sc) {}

        virtual Vector3f value(double u, double v, const Vector3f& p) const override {
            // return Vector3f(1,1,1)*0.5*(1 + noise.turb(scale * p));
            // return Vector3f(1,1,1)*noise.turb(scale * p);
            return Vector3f(1,1,1)*0.5*(1 + sin(scale*p.z() + 10*noise.turb(p)));
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
