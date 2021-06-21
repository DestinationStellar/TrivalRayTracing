#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>

#include "scene_parser.hpp"
#include "scene_generator.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "ray_tracer.hpp"


#include <omp.h>
#include <pthread.h>

using namespace std;

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 4) {
        cout << "Usage: ./bin/FINAL <method> <input scene> <output bmp file>" << endl;
        return 1;
    }
    string method = argv[1];
    string input = argv[2];
    string outputFile = argv[3];  // only bmp is allowed.

    cout << "Hello! Computer Graphics!" << endl;
    srand((unsigned)time(nullptr));
    int sample_per_pixel = 100;
    int max_depth = 50;
    float init_weight = 100;
    int image_width = 400;
    int image_height = 400;
    Group* baseGroup = nullptr;
    Camera* camera = nullptr;
    RayTracer *rayTracer = nullptr;
    if(stoi(method) == 0) {
        int m = stoi(input);
        SceneGenerator sceneGenerator(m, image_width, image_height, sample_per_pixel, max_depth, init_weight);
        baseGroup = sceneGenerator.getGroup();
        camera = sceneGenerator.getCamera();
        rayTracer = sceneGenerator.getRayTracer();
    }
    else {
        SceneParser sceneParser(argv[2]);
        sample_per_pixel = sceneParser.getSamplePerPixel();
        max_depth = sceneParser.getMaxDepth();
        init_weight = sceneParser.getInitWeight();
        image_width = sceneParser.getCamera()->getWidth();
        image_height = sceneParser.getCamera()->getHeight();
        baseGroup = sceneParser.getGroup();
        camera = sceneParser.getCamera();
        rayTracer = new RayTracer(sceneParser);
    }

    Image renderedImg(image_width,image_height);

    // Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    for (int x = 0; x < image_width; ++x) {
         printf("\rrendering image pass %.3lf%%", x*100.f/image_width);
        #pragma omp parallel for schedule(dynamic, 60), num_threads(12)
        for (int y = 0; y < image_height; ++y) {
            Vector3f finalColor = Vector3f::ZERO;
            for (int i=0; i<sample_per_pixel; i++) {
                float bias_x = (rand()%sample_per_pixel)/(float)sample_per_pixel;
                float bias_y = (rand()%sample_per_pixel)/(float)sample_per_pixel;
                Ray camRay = camera->generateRay(Vector2f(x+bias_x, y+bias_y));
                finalColor += rayTracer->traceRay(camRay, max_depth, init_weight);
            }
            finalColor = finalColor/(float)sample_per_pixel;
            // gamma corrected 后续可能需要改到setpixel函数内部
            finalColor = Vector3f(sqrt(clamp(finalColor[0])), sqrt(clamp(finalColor[1])), sqrt(clamp(finalColor[2])));
            renderedImg.SetPixel(x, y, finalColor);
        }
    }
    printf("\rrendering image pass 100.000%%\n");
    renderedImg.SaveImage(argv[3]);
    cout << "Successfully rendered image!" << endl;
    // delete baseGroup;
    return 0;
}

