#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <vector>
#include <vecmath.h>
#include <iostream>
#include "group.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "scene_parser.hpp"
#include "scene_generator.hpp"
#include "pdf.hpp"
#include "image.hpp"

#define MIN_WEIGHT 1e-3f
#define TRACE_DEPTH 20

class RayTracer {
public:
    RayTracer(SceneParser &parser, char* out) : outputfile(out) {
        baseGroup = parser.getGroup();
        lights = parser.getLights();
        camera = parser.getCamera();
        image_height = camera->getHeight();
        image_width = camera->getWidth();
        backgroundColor = parser.getBackgroundColor();
        max_depth = parser.getMaxDepth();
        renderedImg = new Image(image_width,image_height);
    }
    RayTracer(SceneGenerator& generator, char* out) : outputfile(out) {
        baseGroup = generator.getGroup();
        lights = generator.getLight();
        camera = generator.getCamera();
        backgroundColor = generator.getBackGround();
        image_height = generator.getImageHeight();
        image_width = generator.getImageWidth();
        max_depth = generator.getMaxDepth();
        init_weight = generator.getInitWeight();
        renderedImg = new Image(image_width,image_height);
    }
    ~RayTracer()=default;

    void render() {
        for (int x = 0; x < image_width; ++x) {
            printf("\rrendering image pass %.3lf%%", x*100.f/image_width);
            #pragma omp parallel for schedule(dynamic, 60), num_threads(12)
            for (int y = 0; y < image_height; ++y) {
                Vector3f finalColor = Vector3f::ZERO;
                float actual_samples = sample_per_pixel; 
                for (int i=0; i<sample_per_pixel; i++) {
                    float bias_x = random_double(0,1);
                    float bias_y = random_double(0,1);
                    Ray camRay = camera->generateRay(Vector2f(x+bias_x, y+bias_y));
                    Vector3f color = traceRay(camRay, max_depth, init_weight);
                    for (int i = 0; i < 3; i++) {
                        if(color[i] != color[i]||color[i] < 0) {
                            color[i] = 0;
                            actual_samples -= 1;
                        }
                    }
                    finalColor += color;
                }
                if (actual_samples < 1.0) actual_samples = 1.0;
                finalColor = finalColor/actual_samples;
                renderedImg->SetPixel(x, y, finalColor);
            }
        }
        printf("\rrendering image pass 100.000%%\n");
        renderedImg->SaveImage(outputfile);
        printf("Successfully rendered image!\n");
    }

    Vector3f traceRay(Ray &camRay, int depth, float weight) {
        Hit record;
        if (depth<=0||weight<MIN_WEIGHT){
            return Vector3f::ZERO;
        }

        if (!baseGroup->intersect(camRay, record, 0.001, infinity)) {
            return backgroundColor;
        }
        Vector3f color = Vector3f::ZERO;
        ScatterRecord srec;
        Vector3f attenuation;
        Vector3f emitted = record.getMaterial()->emitted(record, record.u, record.v, record.getIntersectP()); 
        if(!record.getMaterial()->scatter(camRay,record,srec)) {
            if (depth == max_depth) {
                return record.getMaterial()->emitted(record, record.u, record.v, record.getIntersectP(),true);
            }
            else {
                return emitted;
            }
        }
        if(srec.is_specular) {
            return srec.attenuation * traceRay(srec.specular_ray, depth-1, weight);
        }
        shared_ptr<PDF> light_ptr = make_shared<HittablePDF>(lights, record.getIntersectP());
        MixturePDF p(light_ptr, srec.pdf_ptr);
        Ray scattered = Ray(record.getIntersectP(), p.generate(), camRay.getTime());
        auto pdf_val = p.value(scattered.getDirection());

        return emitted
             + srec.attenuation * record.getMaterial()->scatterPDF(camRay, record, scattered)
                                * traceRay(scattered, depth-1, weight) / pdf_val; // *attenuation.length() 暂时去除权重
    }

private:
    Camera* camera;
    Group* baseGroup;
    Group* lights;
    Vector3f backgroundColor;
    int image_width, image_height, sample_per_pixel;
    float max_depth, init_weight;
    Image* renderedImg;
    char* outputfile;
};

#endif // RAY_TRACER_H