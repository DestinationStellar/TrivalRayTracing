#ifndef RAY_TRACER_H
#define RAY_TRACER_H

#include <vector>
#include <vecmath.h>
#include "group.hpp"
#include "light.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "scene_parser.hpp"

#define MIN_WEIGHT 1e-3f

class RayTracer {
public:
    RayTracer(SceneParser &parser){
        baseGroup = parser.getGroup();
        for (int li=0;li<parser.getNumLights();li++){
            lights.push_back(parser.getLight(li));
        }
        ambientLight = parser.getAmbientLight();
        backgroundColor = parser.getBackgroundColor();
    }
    ~RayTracer()=default;
    Vector3f traceRay(Ray &camRay, int depth, float weight){
        Hit record;
        if (depth<=0||weight<MIN_WEIGHT){
            return Vector3f::ZERO;
        }
        if (baseGroup->intersect(camRay,record,0.01)){
            Vector3f dirToLight, lightColor;
            Vector3f intersection = record.getIntersectP();
            ambientLight->getIllumination(intersection,dirToLight,lightColor);// the first two pramaters actually unused
            Vector3f color = record.getMaterial()->getAmbientColor()*lightColor;
            Hit hitBlock;
            for(auto light=lights.begin();light!=lights.end();light++){
                (*light)->getIllumination(intersection, dirToLight, lightColor);
                hitBlock.init();
                if(!baseGroup->intersect(Ray(intersection,dirToLight),hitBlock,0.01)){// TODO 可以进一步优化
                    color+=record.getMaterial()->Shade(camRay,record,dirToLight,lightColor);
                }
            }
            Ray scattered(camRay);
            Vector3f attenuation; 
            if(record.getMaterial()->Scatter(camRay,record,attenuation,scattered)){
                color += attenuation*traceRay(scattered, depth-1, weight*attenuation.length());
            }
            return color;
        }
        return backgroundColor;
    }

private:
    Group* baseGroup;
    std::vector<Light*> lights;
    Light *ambientLight;
    Vector3f backgroundColor;
};


#endif // RAY_TRACER_H