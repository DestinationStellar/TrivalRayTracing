#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"
#include "ray_tracer.hpp"

#include <string>
#include <ctime>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/FINAL <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.

    // Main RayCasting Logic
    // First, parse the scene using SceneParser.
    // Then loop over each pixel in the image, shooting a ray
    // through that pixel and finding its intersection with
    // the scene.  Write the color at the intersection to that
    // pixel in your output image.
    cout << "Hello! Computer Graphics!" << endl;
    srand((unsigned)time(nullptr));
    SceneParser sceneParser(argv[1]);
    Image renderedImg(sceneParser.getCamera()->getWidth(),sceneParser.getCamera()->getHeight());
    Group* baseGroup = sceneParser.getGroup();
    RayTracer rayTracer(sceneParser);
    int sample_per_pixel = sceneParser.getSamplePerPixel();
    int max_depth = sceneParser.getMaxDepth();
    float init_weight = sceneParser.getInitWeight();
    // 循环屏幕空间的像素
    int total = sceneParser.getCamera()->getWidth()*sceneParser.getCamera()->getHeight();
    for (int x = 0; x < sceneParser.getCamera()->getWidth(); ++x) {
        for (int y = 0; y < sceneParser.getCamera()->getHeight(); ++y) {
            int current = x*sceneParser.getCamera()->getHeight()+y;
            printf("\rrendering image pass %.3lf%%", current*100.f/total);
            // 计算当前像素(x,y)处相机出射光线camRay
            Vector3f finalColor = Vector3f::ZERO;
            for (int i=0; i<sample_per_pixel; i++) {
                float bias_x = (rand()%sample_per_pixel)/(float)sample_per_pixel;
                float bias_y = (rand()%sample_per_pixel)/(float)sample_per_pixel;
                Ray camRay = sceneParser.getCamera()->generateRay(Vector2f(x+bias_x, y+bias_y));
                finalColor += rayTracer.traceRay(camRay, max_depth, init_weight);
            }
            finalColor = finalColor/(float)sample_per_pixel;
            renderedImg.SetPixel(x, y, finalColor);
        }
    }
    printf("\rrendering image pass 100.000%%\n");
    renderedImg.SaveImage(argv[2]);
    cout << "Successfully rendered image!" << endl;
    // delete baseGroup;
    return 0;
}

