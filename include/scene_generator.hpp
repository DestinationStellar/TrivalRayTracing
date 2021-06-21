#ifndef SCENE_GENERATOR_H
#define SCENE_GENERATOR_H

#include <iostream>

#include "utils.hpp"
#include "camera.hpp"
#include "ray_tracer.hpp"
#include "group.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "rectangle.hpp"
#include "box.hpp"
#include "transform.hpp"
#include "constant_medium.hpp"
#include "bvh.hpp"

class SceneGenerator {
public:
    SceneGenerator() {}
    SceneGenerator(int index, int &imgW, int &imgH, int &samp, int &dep, float &weigh) {
        switch(index) {
            case 1:
                cornell_box(imgW, imgH, samp, dep, weigh);
                break;
            case 2:
                cornell_smoke(imgW, imgH, samp, dep, weigh);
                break;
            case 3:
                next_week(imgW, imgH, samp, dep, weigh);
                break;
            default:
                std::cout<<"matched no scene!"<<std::endl;
                break;
        }
    }

    ~SceneGenerator() {}

    Camera* getCamera() {
        return camera;
    }

    Group* getGroup() {
        return group;
    }

    RayTracer* getRayTracer() {
        return rayTracer;
    }

protected:
    Camera *camera;
    Group *group;
    RayTracer *rayTracer;

    void cornell_box(int &imgW, int &imgH, int &samp, int &dep, float &weigh) {
        std::cout<<"scene: cornell box"<<std::endl;
        imgW = 600;
        imgH = 600;
        samp = 200;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0,1,0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        dep = 50;
        weigh = 5;

        Vector3f background(0,0,0);

        group = new Group(6);

        Material* red = new Lambertian(Vector3f(.65, .05, .05));
        Material* white = new Lambertian(Vector3f(.73, .73, .73));
        Material* green = new Lambertian(Vector3f(.12, .45, .15));
        Material* light = new DiffuseLight(Vector3f(1, 1, 1), 7);

        Object3D* r1 = new Rectangle(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        Object3D* r2 = new Rectangle(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        Object3D* l = new Rectangle(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        Object3D* r3 = new Rectangle(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        Object3D* r4 = new Rectangle(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        Object3D* r5 = new Rectangle(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, white);

        Object3D* box1 = new Box(Vector3f(0, 0, 0), Vector3f(165, 330, 165), white);
        box1 = new Transform(box1, Vector3f(1,1,1), Vector3f(265,0,295), 0, 15, 0);

        Object3D* box2 = new Box(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white);
        box2 = new Transform(box2, Vector3f(1,1,1), Vector3f(130,0,65), 0, -18, 0);

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(box1);
        group->addObject(box2);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
        rayTracer = new RayTracer(group, background, dep);
    }

    void cornell_smoke(int &imgW, int &imgH, int &samp, int &dep, float &weigh) {
        std::cout<<"scene: cornell smoke"<<std::endl;
        imgW = 600;
        imgH = 600;
        samp = 200;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0,1,0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        dep = 50;
        weigh = 5;

        Vector3f background(0,0,0);

        group = new Group(6);

        Material* red = new Lambertian(Vector3f(.65, .05, .05));
        Material* white = new Lambertian(Vector3f(.73, .73, .73));
        Material* green = new Lambertian(Vector3f(.12, .45, .15));
        Material* light = new DiffuseLight(Vector3f(1, 1, 1), 7);

        Object3D* r1 = new Rectangle(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        Object3D* r2 = new Rectangle(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        Object3D* l = new Rectangle(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        Object3D* r3 = new Rectangle(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        Object3D* r4 = new Rectangle(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        Object3D* r5 = new Rectangle(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, white);

        Object3D* box1 = new Box(Vector3f(0, 0, 0), Vector3f(165, 330, 165), white);
        box1 = new Transform(box1, Vector3f(1,1,1), Vector3f(265,0,295), 0, 15, 0);
        box1 = new ConstantMedium(box1, 0.01, Vector3f(0, 0, 0));

        Object3D* box2 = new Box(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white);
        box2 = new Transform(box2, Vector3f(1,1,1), Vector3f(130,0,65), 0, -18, 0);
        box2 = new ConstantMedium(box2, 0.01, Vector3f(1, 1, 1));

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(box1);
        group->addObject(box2);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
        rayTracer = new RayTracer(group, background, dep);
    }

    void next_week(int &imgW, int &imgH, int &samp, int &dep, float &weigh) {
        std::cout<<"scene: nextWeek"<<std::endl;
        imgW = 600;
        imgH = 600;
        samp = 200;
        Vector3f lookfrom(478, 278, -600);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0, 1, 0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        dep = 50;
        weigh = 5;

        Vector3f background(0,0,0);

        group = new Group(11);

        Group boxes1;
        Material* ground = new Lambertian(Vector3f(0.48, 0.83, 0.53));

        const int boxes_per_side = 20;
        for (int i = 0; i < boxes_per_side; i++) {
            for (int j = 0; j < boxes_per_side; j++) {
                auto w = 100.0;
                auto x0 = -1000.0 + i*w;
                auto z0 = -1000.0 + j*w;
                auto y0 = 0.0;
                auto x1 = x0 + w;
                auto y1 = random_double(1,101);
                auto z1 = z0 + w;

                Object3D* box = new Box(Vector3f(x0,y0,z0), Vector3f(x1,y1,z1), ground);
                boxes1.addObject(box);
            }
        }
        Object3D* boxes1_bvh = new BVHnode(boxes1, 0, 1);
        group->addObject(boxes1_bvh); // 1
        
        Material* light = new DiffuseLight(Vector3f(1, 1, 1), 7);
        Object3D* globalLight = new Rectangle(Vector3f(273, 554, 279.5), Vector3f(1,0,0), Vector3f(0,0,1), 300, 265, light);
        group->addObject(globalLight); // 2

        Vector3f center1 = Vector3f(400, 400, 200);
        Vector3f center2 = center1 + Vector3f(30,0,0);
        Material* moving_sphere_material = new Lambertian(Vector3f(0.7, 0.3, 0.1));
        Object3D* moving_sphere = new MovingSphere(center1, center2, 0, 1, 50, moving_sphere_material);
        group->addObject(moving_sphere); // 3

        Material* d = new Dielectric(1.5);
        Object3D* d_sphere = new Sphere(Vector3f(260, 150, 45), 50, d);
        group->addObject(d_sphere); // 4
        Material* m = new Metal(Vector3f(0.8,0.8,0.9),1.0);
        Object3D* m_sphere = new Sphere(Vector3f(0, 150, 145), 50, m);
        group->addObject(m_sphere); // 5

        Object3D* boundary = new Sphere(Vector3f(360,150,145), 70, d);
        group->addObject(boundary); // 6
        boundary = new ConstantMedium(boundary, 0.2, Vector3f(0.2, 0.4, 0.9));
        group->addObject(boundary); // 7
        boundary = new Sphere(Vector3f(0,0,0), 5000, d);
        boundary = new ConstantMedium(boundary, 0.0001, Vector3f(1, 1, 1));
        group->addObject(boundary); // 8

        char file_path[] = "E:\\TsinghuaUniversity\\2020-2021Spring\\CG\\project\\resource\\earthmap.jpg";
        Texture* etext = new ImageTexture(file_path);
        Material* emat = new Lambertian(etext);
        Object3D* esphere = new Sphere(Vector3f(400,200,400), 100, emat);
        group->addObject(esphere); // 9
        Texture* pertext = new NoiseTexture(4);
        Material* per_lam = new Lambertian(pertext);
        Object3D* per_sphere = new Sphere(Vector3f(220,280,300),80,per_lam);
        group->addObject(per_sphere);
        
        Group boxes2;
        Material* white = new Lambertian(Vector3f(0.73, 0.73, 0.73));
        int ns = 1000;
        Object3D* wsphere = nullptr;
        for (int j = 0; j < ns; j++) {
            wsphere = new Sphere(random_vector3f(0,165), 10, white);
            boxes2.addObject(wsphere);
        }

        Object3D* boxes2_bvh = new BVHnode(boxes2, 0.0, 1.0);
        boxes2_bvh = new Transform(boxes2_bvh, Vector3f(1,1,1), Vector3f(-100, 270, 395), 0.0, 15.0, 0.0);
        group->addObject(boxes2_bvh);
        
        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis, 0.0, 1.0);
        rayTracer = new RayTracer(group, background, dep);
    }

};

#endif // SCENE_GENERATOR_h