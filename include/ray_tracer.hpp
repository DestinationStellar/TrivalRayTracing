#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <vector>
#include <vecmath.h>
#include "group.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "scene_parser.hpp"
#include "pdf.hpp"

#define MIN_WEIGHT 1e-3f

class RayTracer {
public:
    RayTracer(SceneParser &parser){
        baseGroup = parser.getGroup();
        // for (int li=0;li<parser.getNumLights();li++){
        //     lights.push_back(parser.getLight(li));
        // }
        // ambientLight = parser.getAmbientLight();
        backgroundColor = parser.getBackgroundColor();
        maxDepth = parser.getMaxDepth();
    }
    RayTracer(Group* g, Group* l, const Vector3f &b, float m): baseGroup(g), lights(l), backgroundColor(b), maxDepth(m) {}
    ~RayTracer()=default;
    Vector3f traceRay(Ray &camRay, int depth, float weight) {
        Hit record;
        if (depth<=0||weight<MIN_WEIGHT){
            return Vector3f::ZERO;
        }

        if (!baseGroup->intersect(camRay, record, 0.001, infinity)) {
            return backgroundColor;
        }

        // Vector3f dirToLight, lightColor;
        // Vector3f intersection = record.getIntersectP();
        // ambientLight->getIllumination(intersection,dirToLight,lightColor);// the first two pramaters actually unused
        // Vector3f color = record.getMaterial()->getAmbientColor()*lightColor;
        // Hit hitBlock;
        // for(auto light=lights.begin();light!=lights.end();light++){
        //     (*light)->getIllumination(intersection, dirToLight, lightColor);
        //     hitBlock.init();
        //     if(!baseGroup->intersect(Ray(intersection,dirToLight),hitBlock,0.01)){// TODO 可以进一步优化
        //         color+=record.getMaterial()->Shade(camRay,record,dirToLight,lightColor);
        //     }
        // }
        Vector3f color = Vector3f::ZERO;
        ScatterRecord srec;
        Vector3f attenuation;
        Vector3f emitted = record.getMaterial()->emitted(record, record.u, record.v, record.getIntersectP()); 
        if(!record.getMaterial()->scatter(camRay,record,srec)) {
            if (depth == maxDepth) {
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
    Group* baseGroup;
    Group* lights;
    // std::vector<Light*> lights;
    // Light *ambientLight;
    Vector3f backgroundColor;
    float maxDepth;
};


#endif // RAY_TRACER_H