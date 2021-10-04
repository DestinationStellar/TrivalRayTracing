#ifndef SCENE_GENERATOR_H
#define SCENE_GENERATOR_H

#include <iostream>
#include <vector>

#include "utils.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "rectangle.hpp"
#include "box.hpp"
#include "transform.hpp"
#include "mesh.hpp"
#include "constant_medium.hpp"
#include "bvh.hpp"
#include "curve.hpp"
#include "revsurface.hpp"

class SceneGenerator {
public:
    SceneGenerator() {}
    SceneGenerator(int index) {
        switch(index) {
            case 1:
                cornell_box();
                break;
            case 2:
                cornell_smoke();
                break;
            case 3:
                bunny();
                break;
            case 4:
                bezier();
                break;
            case 5:
                final_scene1();
                break;
            case 6:
                final_scene2();
                break;
            case 7:
                final_scene3();
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

    Group* getLight() {
        return lights;
    }

    int getImageWidth() {
        return imgW;
    }

    int getImageHeight() {
        return imgH;
    }

    int getSample() {
        return sample_per_pixel;
    }

    float getMaxDepth() {
        return max_depth;
    }

    float getInitWeight() {
        return init_weight;
    }

    Vector3f getBackGround() {
        return background;
    }

protected:
    Camera *camera;
    Group *group;
    Group *lights;
    int imgW, imgH, sample_per_pixel;
    float max_depth, init_weight;
    Vector3f background;


    void cornell_box() {
        std::cout<<"scene: cornell box"<<std::endl;
        imgW = 600;
        imgH = 600;
        sample_per_pixel = 200;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 278);
        Vector3f vup(0,1,0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = (lookfrom-lookat).length();

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(7);
        lights = new Group(1);

        shared_ptr<Material> red = make_shared<Lambertian>(Vector3f(.65, .05, .05));
        shared_ptr<Material> white = make_shared<Lambertian>(Vector3f(.73, .73, .73));
        shared_ptr<Material> green = make_shared<Lambertian>(Vector3f(.12, .45, .15));
        shared_ptr<Material> light = make_shared<DiffuseLight>(Vector3f(1, 1, 1), 7);
        shared_ptr<Material> wall = make_shared<Lambertian>(make_shared<ImageTexture>("resource/bricks.jpg"));
        shared_ptr<Material> e = make_shared<Metal>(Vector3f(0.9, 0.6, 0.6));

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        shared_ptr<Object3D> l = make_shared<Rectangle>(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, wall);

        shared_ptr<Object3D> s1 = make_shared<Sphere>(Vector3f(400, 50, 400), 50, e);
        shared_ptr<Object3D> s2 = make_shared<Sphere>(Vector3f(278, 50, 278), 50, e);
        shared_ptr<Object3D> s3 = make_shared<Sphere>(Vector3f(140, 50, 140), 50, e);

        Group s;
        s.addObject(s1);
        s.addObject(s2);
        s.addObject(s3);
        shared_ptr<Object3D> s_bvh = make_shared<BVHnode>(s, 0, 1);
        
        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(s_bvh);

        lights->addObject(l);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
    }

    void cornell_smoke() {
        std::cout<<"scene: cornell smoke"<<std::endl;
        imgW = 600;
        imgH = 600;
        sample_per_pixel = 200;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0,1,0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(6);
        lights = new Group(1);

        shared_ptr<Material> red = make_shared<Lambertian>(Vector3f(.65, .05, .05));
        shared_ptr<Material> white = make_shared<Lambertian>(Vector3f(.73, .73, .73));
        shared_ptr<Material> green = make_shared<Lambertian>(Vector3f(.12, .45, .15));
        shared_ptr<Material> light = make_shared<DiffuseLight>(Vector3f(1, 1, 1), 7);

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        shared_ptr<Object3D> l = make_shared<Rectangle>(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, white);

        shared_ptr<Object3D> box1 = make_shared<Box>(Vector3f(0, 0, 0), Vector3f(165, 330, 165), white);
        box1 = make_shared<Transform>(box1, Vector3f(1,1,1), Vector3f(265,0,295), 0, 15, 0);
        box1 = make_shared<ConstantMedium>(box1, 0.01, Vector3f(0, 0, 0));

        shared_ptr<Object3D> box2 = make_shared<Box>(Vector3f(0, 0, 0), Vector3f(165, 165, 165), white);
        box2 = make_shared<Transform>(box2, Vector3f(1,1,1), Vector3f(130,0,65), 0, -18, 0);
        box2 = make_shared<ConstantMedium>(box2, 0.01, Vector3f(1, 1, 1));

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(box1);
        group->addObject(box2);

        lights->addObject(l);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
    }

    void bunny() {
        std::cout<<"scene: bunny"<<std::endl;
        imgW = 600;
        imgH = 600;
        sample_per_pixel = 200;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0,1,0);
        float angle = 40.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(7);
        lights = new Group(1);

        shared_ptr<Material> red = make_shared<Lambertian>(Vector3f(.65, .05, .05));
        shared_ptr<Material> white = make_shared<Lambertian>(Vector3f(.73, .73, .73));
        shared_ptr<Material> green = make_shared<Lambertian>(Vector3f(.12, .45, .15));
        shared_ptr<Material> light = make_shared<DiffuseLight>(Vector3f(1, 1, 1), 7);

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        shared_ptr<Object3D> l = make_shared<Rectangle>(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, white);

        Group boxes;

        shared_ptr<Object3D> bunny = make_shared<Mesh>("mesh/bunny.fine.obj", make_shared<Lambertian>(Vector3f(0.79, 0.66, 0.44)));
        bunny = make_shared<Transform>(bunny, Vector3f(1000, 1000, 1000), Vector3f(278, 100, 278), 0, 180, 0);
        
        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);

        group->addObject(bunny);

        lights->addObject(l);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
    }

    void bezier() {
        std::cout<<"scene: bezier"<<std::endl;
        imgW = 400;
        imgH = 400;
        sample_per_pixel = 100;
        Vector3f lookfrom(278, 278, -800);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0,1,0);
        float angle = 30.0;
        float aperture = 0.0;
        float focus_dis = 10.0;

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(7);
        lights = new Group(1);

        shared_ptr<Material> red = make_shared<Lambertian>(Vector3f(.65, .05, .05));
        shared_ptr<Material> white = make_shared<Lambertian>(Vector3f(.73, .73, .73));
        shared_ptr<Material> green = make_shared<Lambertian>(Vector3f(.12, .45, .15));
        shared_ptr<Material> light = make_shared<DiffuseLight>(Vector3f(1, 1, 1), 7);

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, green);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,278), Vector3f(0,0,1), Vector3f(0,-1,0), 555, 555, red);
        shared_ptr<Object3D> l = make_shared<Rectangle>(Vector3f(278,554,278), Vector3f(-1,0,0), Vector3f(0,0,-1), 230, 205, light);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(278, 0, 278), Vector3f(-1,0,0),Vector3f(0,0,1), 555, 555, white);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(-1,0,0),Vector3f(0,0,-1), 555, 555, white);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(278, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, white);

        std::vector<Vector3f> points;
        points.push_back(Vector3f( -2, 2, 0 ));
        points.push_back(Vector3f( -4, 0, 0 ));
        points.push_back(Vector3f( 0, 0, 0 ));
        points.push_back(Vector3f( -2, -2, 0 ));
        shared_ptr<Curve> curve = make_shared<BezierCurve>(points);
        shared_ptr<Object3D> vase;

        shared_ptr<Material> vase_img = make_shared<Lambertian>(make_shared<ImageTexture>("resource/vase.png"));
        // vase = make_shared<RevSurface>(curve, vase_img);
        // vase = make_shared<Transform>(vase, Vector3f(30, 30, 30), Vector3f(278, 278, 278), 0, 0, 0);
        // group->addObject(vase);

        vase = make_shared<RevSurface>(curve, make_shared<Metal>(Vector3f(0.8,0.8,0.9)));
        vase = make_shared<Transform>(vase, Vector3f(30, 30, 30), Vector3f(278, 278, 278), 0, 0, 0);
        group->addObject(vase);

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(l);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);

        lights->addObject(l);

        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis);
    }

    void final_scene1() {
        std::cout<<"scene: final scene 1"<<std::endl;
        imgW = 1280;
        imgH = 720;
        sample_per_pixel = 2000;
        Vector3f lookfrom(478, 278, -600);
        Vector3f lookat(278, 278, 0);
        Vector3f vup(0, 1, 0);
        float angle = 40.0;
        float aperture = 1.0;
        float focus_dis = (lookat-lookfrom).length();

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(11);
        lights = new Group(2);

        Group boxes1;
        shared_ptr<Material> ground = make_shared<Lambertian>(Vector3f(0.48, 0.83, 0.53));

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

                shared_ptr<Object3D> box = make_shared<Box>(Vector3f(x0,y0,z0), Vector3f(x1,y1,z1), ground);
                boxes1.addObject(box);
            }
        }
        shared_ptr<Object3D> boxes1_bvh = make_shared<BVHnode>(boxes1, 0, 1);
        group->addObject(boxes1_bvh); // 1
        
        shared_ptr<Material> light = make_shared<DiffuseLight>(Vector3f(1, 1, 1), 7);
        shared_ptr<Object3D> globalLight = make_shared<Rectangle>(Vector3f(273, 554, 279.5), Vector3f(1,0,0), Vector3f(0,0,1), 300, 265, light);
        group->addObject(globalLight); // 2
        lights->addObject(globalLight);

        Vector3f center1 = Vector3f(400, 400, 200);
        Vector3f center2 = center1 + Vector3f(30,0,0);
        shared_ptr<Material> moving_sphere_material = make_shared<Lambertian>(Vector3f(0.7, 0.3, 0.1));
        shared_ptr<Object3D> moving_sphere = make_shared<MovingSphere>(center1, center2, 0, 1, 50, moving_sphere_material);
        group->addObject(moving_sphere); // 3

        shared_ptr<Material> d = make_shared<Dielectric>(1.5);
        shared_ptr<Object3D> d_sphere = make_shared<Sphere>(Vector3f(260, 150, 45), 50, d);
        group->addObject(d_sphere); // 4

        shared_ptr<Object3D> bunny = make_shared<Mesh>("mesh/bunny.fine.obj", make_shared<Lambertian>(Vector3f(0.79, 0.66, 0.44)));
        bunny = make_shared<Transform>(bunny, Vector3f(700, 700, 700), Vector3f(180, 90, -70), 0, 150, 0);
        group->addObject(bunny);

        std::vector<Vector3f> points;
        points.reserve(26);
        
        points.push_back(Vector3f( 0.000000, -0.459543, 0.0 ));
        points.push_back(Vector3f( 0.000000, -0.459544, 0.0 ));
        points.push_back(Vector3f( 0.000000, -0.459545, 0.0 ));
        points.push_back(Vector3f( -0.351882, -0.426747, 0.0 ));
        points.push_back(Vector3f( -0.848656, -0.278898, 0.0 ));
        points.push_back(Vector3f( -1.112097, 0.084005, 0.0 ));
        points.push_back(Vector3f( -1.164785, 1.105511, 0.0 ));
        points.push_back(Vector3f( -0.991667, 2.328629, 0.0 ));
        points.push_back(Vector3f( -1.029301, 2.503360, 0.0 ));
        points.push_back(Vector3f( -1.088800, 2.345600, 0.0 ));
        points.push_back(Vector3f( -1.278000, 1.162800, 0.0 ));
        points.push_back(Vector3f( -1.214800, 0.055200, 0.0 ));
        points.push_back(Vector3f( -0.915600, -0.381200, 0.0 ));
        points.push_back(Vector3f( -0.380400, -0.622000, 0.0 ));
        points.push_back(Vector3f( -0.144000, -0.968400, 0.0 ));
        points.push_back(Vector3f( -0.096800, -1.480000, 0.0 ));
        points.push_back(Vector3f( -0.128400, -2.112400, 0.0 ));
        points.push_back(Vector3f( -0.317200, -2.202800, 0.0 ));
        points.push_back(Vector3f( -0.994400, -2.262800, 0.0 ));
        points.push_back(Vector3f( -1.214800, -2.323200, 0.0 ));
        points.push_back(Vector3f( -1.199200, -2.398400, 0.0 ));
        points.push_back(Vector3f( -1.057600, -2.458800, 0.0 ));
        points.push_back(Vector3f( -0.711200, -2.458800, 0.0 ));
        points.push_back(Vector3f(  0.000000, -2.458800, 0.0 ));
        points.push_back(Vector3f(  0.000000, -2.458801, 0.0 ));
        points.push_back(Vector3f( 0.000000, -2.458802, 0.0 ));

        shared_ptr<Curve> curve = make_shared<BsplineCurve>(points);
        shared_ptr<Material> m = make_shared<Metal>(Vector3f(0.8,0.8,0.9),0.2);
        shared_ptr<Object3D> wineglass = make_shared<RevSurface>(curve, m, true);
        wineglass = make_shared<Transform>(wineglass, Vector3f(30, 30, 30), Vector3f(30, 200, 125), 0, 0, 0);
        group->addObject(wineglass);

        // shared_ptr<Material> m = make_shared<Metal>(Vector3f(0.8,0.8,0.9),1.0);
        // shared_ptr<Object3D> m_sphere = make_shared<Sphere>(Vector3f(0, 150, 145), 50, m);
        // group->addObject(m_sphere); // 5

        shared_ptr<Object3D> boundary = make_shared<Sphere>(Vector3f(360,150,145), 70, d);
        group->addObject(boundary); // 6
        boundary = make_shared<ConstantMedium>(boundary, 0.2, Vector3f(0.2, 0.4, 0.9));
        group->addObject(boundary); // 7
        boundary = make_shared<Sphere>(Vector3f(0,0,0), 5000, d);
        boundary = make_shared<ConstantMedium>(boundary, 0.0001, Vector3f(1, 1, 1));
        group->addObject(boundary); // 8

        shared_ptr<Texture> etext = make_shared<ImageTexture>("resource/sunmap.jpg");
        shared_ptr<Material> emat = make_shared<DiffuseLight>(etext, 5);
        shared_ptr<Object3D> esphere = make_shared<Sphere>(Vector3f(400,200,400), 100, emat);
        group->addObject(esphere); // 9
        shared_ptr<Texture> pertext = make_shared<NoiseTexture>(0.1);
        shared_ptr<Material> per_lam = make_shared<Lambertian>(pertext);
        shared_ptr<Object3D> per_sphere = make_shared<Sphere>(Vector3f(220,280,300),80,per_lam);
        group->addObject(per_sphere);
        
        Group boxes2;
        shared_ptr<Material> white = make_shared<Lambertian>(Vector3f(0.73, 0.73, 0.73));
        int ns = 1000;
        shared_ptr<Object3D> wsphere = nullptr;
        for (int j = 0; j < ns; j++) {
            wsphere = make_shared<Sphere>(random_vector3f(0,165), 10, white);
            boxes2.addObject(wsphere);
        }

        shared_ptr<Object3D> boxes2_bvh = make_shared<BVHnode>(boxes2, 0.0, 1.0);
        boxes2_bvh = make_shared<Transform>(boxes2_bvh, Vector3f(1,1,1), Vector3f(-100, 270, 395), 0.0, 15.0, 0.0);
        group->addObject(boxes2_bvh);
        
        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
                DegreesToRadians(angle), aperture, focus_dis, 0.0, 1.0);
    }

    void final_scene2() {
        std::cout<<"scene: final scene 2"<<std::endl;
        imgW = 1280;
        imgH = 720;
        sample_per_pixel = 1000;
        Vector3f lookfrom(278, 278, 1);
        Vector3f lookat(278, 278, 554);
        Vector3f vup(0,1,0);
        float angle = 80.0;
        float aperture = 0.0;
        float focus_dis = (lookat-lookfrom).length();

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(7);
        lights = new Group(5);

        shared_ptr<Material> head = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/1.jpg"), 1.0);
        shared_ptr<Material> behind = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/2.jpg"), 1.0);
        shared_ptr<Material> left = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/3.jpg"), 1.0);
        shared_ptr<Material> front = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/4.jpg"), 1.0);
        shared_ptr<Material> right = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/5.jpg"), 1.0);
        shared_ptr<Material> foot = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes3/6.jpg"), 1.0);

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,277.5), Vector3f(0,0,1), Vector3f(0,1,0), 555, 555, left);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,277.5), Vector3f(0,0,-1), Vector3f(0,1,0), 555, 555, right);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(277.5, 0.5, 278), Vector3f(0,0,1), Vector3f(1,0,0), 555, 555, foot);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(0,0,1), Vector3f(-1,0,0), 555, 555, head);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(277.5, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, front);
        shared_ptr<Object3D> r6 = make_shared<Rectangle>(Vector3f(277.5, 277.75, 0), Vector3f(1,0,0),Vector3f(0,1,0), 555, 555, behind);

        std::vector<Vector3f> points;
        points.push_back(Vector3f( 0, 6, 0 ));
        points.push_back(Vector3f( -0.8, 3.4, 0 ));
        points.push_back(Vector3f( -3, 0, 0 ));
        points.push_back(Vector3f( 0, -0.5, 0 ));
        shared_ptr<Curve> curve = make_shared<BezierCurve>(points);
        shared_ptr<Object3D> drop = make_shared<RevSurface>(curve, make_shared<Metal>(Vector3f(0.8,0.8,0.8),0.0),true);
        drop = make_shared<Transform>(drop, Vector3f(30, 30, 30), Vector3f(278, 220, 278), 0, 0, 0);
        group->addObject(drop);

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(r6);

        lights->addObject(r1);
        lights->addObject(r2);
        lights->addObject(r3);
        lights->addObject(r4);
        lights->addObject(r5);
        lights->addObject(r6);

        
        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
            DegreesToRadians(angle), aperture, focus_dis, 0.0, 1.0);
    }

    void final_scene3() {
        std::cout<<"scene: final scene 3"<<std::endl;
        imgW = 1280;
        imgH = 720;
        sample_per_pixel = 2000;
        Vector3f lookfrom(278, 278, 1);
        Vector3f lookat(278, 278, 554);
        Vector3f vup(0,1,0);
        float angle = 80.0;
        float aperture = 0.0;
        float focus_dis = (lookat-lookfrom).length();

        max_depth = 50;
        init_weight = 5;

        background = Vector3f(0,0,0);

        group = new Group(7);
        lights = new Group(5);

        shared_ptr<Material> head = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/1.png"), 1.0);
        shared_ptr<Material> behind = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/2.png"), 1.0);
        shared_ptr<Material> left = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/3.png"), 1.0);
        shared_ptr<Material> front = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/4.png"), 1.0);
        shared_ptr<Material> right = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/5.png"), 1.0);
        shared_ptr<Material> foot = make_shared<DiffuseLight>(make_shared<ImageTexture>("resource/skyboxes2/6.png"), 1.0);

        shared_ptr<Object3D> r1 = make_shared<Rectangle>(Vector3f(555,278,277.5), Vector3f(0,0,1), Vector3f(0,1,0), 555, 555, left);
        shared_ptr<Object3D> r2 = make_shared<Rectangle>(Vector3f(0,278,277.5), Vector3f(0,0,-1), Vector3f(0,1,0), 555, 555, right);
        shared_ptr<Object3D> r3 = make_shared<Rectangle>(Vector3f(277.5, 0.5, 278), Vector3f(0,0,1), Vector3f(1,0,0), 555, 555, foot);
        shared_ptr<Object3D> r4 = make_shared<Rectangle>(Vector3f(278, 555, 278), Vector3f(0,0,1), Vector3f(-1,0,0), 555, 555, head);
        shared_ptr<Object3D> r5 = make_shared<Rectangle>(Vector3f(277.5, 278, 555), Vector3f(-1,0,0),Vector3f(0,1,0), 555, 555, front);
        shared_ptr<Object3D> r6 = make_shared<Rectangle>(Vector3f(277.5, 277.75, 0), Vector3f(1,0,0),Vector3f(0,1,0), 555, 555, behind);

        shared_ptr<Object3D> dragon = make_shared<Mesh>("mesh/fixed.perfect.dragon.100K.0.07.obj", make_shared<Metal>(Vector3f(0, 0.545, 0.545), 0.5));
        dragon = make_shared<Transform>(dragon, Vector3f(150, 150, 150), Vector3f(278, 278, 278), 0, 180, 0);
        group->addObject(dragon);

        group->addObject(r1);
        group->addObject(r2);
        group->addObject(r3);
        group->addObject(r4);
        group->addObject(r5);
        group->addObject(r6);

        lights->addObject(r1);
        lights->addObject(r2);
        lights->addObject(r3);
        lights->addObject(r4);
        lights->addObject(r5);
        lights->addObject(r6);

        
        camera = new PerspectiveCamera(lookfrom, lookat, vup, imgW, imgH,
            DegreesToRadians(angle), aperture, focus_dis, 0.0, 1.0);
    }

};

#endif // SCENE_GENERATOR_h