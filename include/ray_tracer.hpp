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
#include "hit_kdtree.hpp"

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
    RayTracer(SceneGenerator &generator, char* out) : outputfile(out) {
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

class SPPM {
public:
    int numRounds, numPhotons, ckpt_interval;
    char* output;
    int w, h;
    Camera* camera;
    vector<Hit*> hitPoints;
    HitKDTree* hitKDTree;
    Group* illuminants;
    Group* group;
    Vector3f background;

    SPPM(SceneGenerator& generator, char* o) : output(o) {
        camera = generator.getCamera();
        numRounds = generator.getNumRounds();
        numPhotons = generator.getNumPhotons();
        ckpt_interval = generator.getCKPT();
        group = generator.getGroup();
        illuminants = generator.getLight();
        background = generator.getBackGround();
        w = camera->getWidth();
        h = camera->getHeight();
        hitKDTree = nullptr;
        for (int u = 0; u < w; ++u)
            for (int v = 0; v < h; ++v) hitPoints.push_back(new Hit());
        std::cout << "Width: " << w << " Height: " << h << std::endl;
    }

    ~SPPM() {
        for (int u = 0; u < w; ++u)
            for (int v = 0; v < h; ++v) delete hitPoints[u * w + v];
        delete hitKDTree;
    }

    void forward(Ray ray, Hit* hit) {
        int depth = 0;
        Vector3f attenuation(1, 1, 1);
        while (true) {
            if (++depth > TRACE_DEPTH || near_zero(attenuation)) return;
            hit->t = infinity;
            if (!group->intersect(ray, *hit)) {
                hit->fluxLight += hit->attenuation*background;
                return;
            }
            ray.origin += ray.getDirection() * hit->t;
            shared_ptr<Material> material = hit->material;
            Vector3f N(hit->normal);
            float type = RND2;
            if (type <= material->type.x()) {  // Diffuse
                hit->attenuation = attenuation * hit->color;
                hit->fluxLight += hit->attenuation * material->emission;
                return;
            } else if (type <= material->type.x() + material->type.y()) {
                float cost = Vector3f::dot(ray.getDirection(), N);
                ray.direction = (ray.getDirection() - N * (cost * 2)).normalized();
            } else {
                float n = material->refr;
                float R0 = ((1.0 - n) * (1.0 - n)) / ((1.0 + n) * (1.0 + n));
                if (Vector3f::dot(N, ray.getDirection()) > 0) {  // inside the medium
                    N.negate();
                    n = 1 / n;
                }
                n = 1 / n;
                float cost1 =
                    -Vector3f::dot(N, ray.getDirection());  // cosine theta_1
                float cost2 =
                    1.0 - n * n * (1.0 - cost1 * cost1);  // cosine theta_2
                float Rprob =
                    R0 + (1.0 - R0) * pow(1.0 - cost1,
                                          5.0);   // Schlick-approximation
                if (cost2 > 0 && RND2 > Rprob) {  // refraction getDirection()
                    ray.getDirection() =
                        ((ray.getDirection() * n) + (N * (n * cost1 - sqrt(cost2))))
                            .normalized();
                } else {  // reflection getDirection()
                    ray.getDirection() =
                        (ray.getDirection() + N * (cost1 * 2)).normalized();
                }
            }


            attenuation = attenuation * hit->color;
        }
    }

    void backward(Ray ray, const Vector3f& color, long long int seed=-1) {
        int depth = 0;
        Vector3f attenuation = color * Vector3f(250, 250, 250);
        while (true) {
            if (++depth > TRACE_DEPTH || attenuation.max() < 1e-3) return;
            Hit hit;
            if (!group->intersect(ray, hit)) return;
            ray.getOrigin() += ray.getDirection() * hit.t;
            Material* material = hit.material;
            Vector3f N(hit.normal);
            float type = RND2;
            if (type <= material->type.x()) {  // Diffuse
                hitKDTree->update(hitKDTree->root, hit.p, attenuation,
                                  ray.getDirection());
                ray.getDirection() = diffDir(N, -1, seed);
            } else if (type <= material->type.x() + material->type.y()) {
                float cost = Vector3f::dot(ray.getDirection(), N);
                ray.getDirection() = (ray.getDirection() - N * (cost * 2)).normalized();
            } else {
                float n = material->refr;
                float R0 = ((1.0 - n) * (1.0 - n)) / ((1.0 + n) * (1.0 + n));
                if (Vector3f::dot(N, ray.getDirection()) > 0) {  // inside the medium
                    N.negate();
                    n = 1 / n;
                }
                n = 1 / n;
                float cost1 =
                    -Vector3f::dot(N, ray.getDirection());  // cosine theta_1
                float cost2 =
                    1.0 - n * n * (1.0 - cost1 * cost1);  // cosine theta_2
                float Rprob =
                    R0 + (1.0 - R0) * pow(1.0 - cost1,
                                          5.0);   // Schlick-approximation
                if (cost2 > 0 && RND2 > Rprob) {  // refraction getDirection()
                    ray.direction =
                        ((ray.getDirection() * n) + (N * (n * cost1 - sqrt(cost2))))
                            .normalized();
                } else {  // reflection getDirection()
                    ray.getDirection() =
                        (ray.getDirection() + N * (cost1 * 2)).normalized();
                }
            }
            attenuation = attenuation * hit.color;
        }
    }

    void render() {
        time_t start = time(NULL);
        Vector3f color = Vector3f::ZERO;
        for (int round = 0; round < numRounds; ++round) {
            float elapsed = (time(NULL) - start),
                  progress = (1. + round) / numRounds;
            fprintf(stderr,
                    "\rRendering (%d/%d Rounds) %5.2f%% Time: %.2f/%.2f sec\n",
                    round + 1, numRounds, progress * 100., elapsed,
                    elapsed / progress);
#pragma omp parallel for schedule(dynamic, 1)
            for (int x = 0; x < w; ++x) {
                for (int y = 0; y < h; ++y) {
                    Ray camRay =
                        camera->generateRay(Vector2f(x + RND, y + RND));
                    hitPoints[x * h + y]->reset(-camRay.getDirection());
                    forward(camRay, hitPoints[x * h + y]);
                }
            }
            setHitKDTree();
            int photonsPerLight = numPhotons / illuminants->getGroupSize();
// photon tracing pass
#pragma omp parallel for schedule(dynamic, 1)
            for (int i = 0; i < photonsPerLight; ++i) {
                for (int j = 0;j < illuminants.size(); ++j) {
                    // cout << i << " "<< j << " In" <<endl;
                    Ray ray = illuminants[j]->randomRay(-1, (long long)round * numPhotons + (round + 1) * w * h + i);
                    // cout << i << " "<< j << " Out" <<endl;
                    backward(ray, illuminants[j]->material->emission, (long long)round * numPhotons + i);
                } 
            }
            if ((round + 1) % ckpt_interval == 0) {
                char filename[100];
                sprintf(filename, "ckpt-%d.bmp", round + 1);
                save(filename, round + 1, numPhotons);
            }
        }
        save("result.bmp", numRounds, numPhotons);
    }

    void save(std::string filename, int numRounds, int numPhotons) {
        Image outImg(w, h);
        for (int u = 0; u < w; ++u)
            for (int v = 0; v < h; ++v) {
                Hit* hit = hitPoints[u * h + v];
                outImg.SetPixel(
                    u, v,
                    hit->flux / (M_PI * hit->r2 * numPhotons * numRounds) +
                        hit->fluxLight / numRounds);
            }
        outImg.SaveImage(output);
    }

    void setHitKDTree() {
        if (hitKDTree) delete hitKDTree;
        hitKDTree = new HitKDTree(&hitPoints);
    }
};

#endif // RAY_TRACER_H