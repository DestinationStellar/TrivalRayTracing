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
    // 循环屏幕空间的像素
    for (int x = 0; x < sceneParser.getCamera()->getWidth(); ++x) {
        for (int y = 0; y < sceneParser.getCamera()->getHeight(); ++y) {
            // 计算当前像素(x,y)处相机出射光线camRay
            vector<Ray> camRay = sceneParser.getCamera()->generateRay(Vector2f(x, y));
            Hit hit;
            Vector3f finalColor = Vector3f::ZERO;
            // 判断camRay是否和场景有交点， 并返回最近交点的数据， 存储在hit中
            for(auto iter=camRay.begin();iter!=camRay.end();iter++){
                hit.set(1e38,nullptr,Vector3f::ZERO);
                bool isIntersect = baseGroup->intersect((*iter), hit, 0);
                if (isIntersect) {
                    // 找到交点之后， 累加来自所有光源的光强影响
                    for (int li = 0; li < sceneParser.getNumLights(); ++li) {
                        Light* light = sceneParser.getLight(li);
                        Vector3f L, lightColor;
                        // 获得光照强度
                        light->getIllumination((*iter).pointAtParameter(hit.getT()), L, lightColor);
                        // 计算局部光强
                        finalColor += hit.getMaterial()->Shade((*iter), hit, L, lightColor);
                    }
                } else {
                    // 不存在交点， 返回背景色
                    finalColor += sceneParser.getBackgroundColor();
                }
            }
            finalColor = finalColor/(float)camRay.size();
            renderedImg.SetPixel(x, y, finalColor);
        }
    }
    renderedImg.SaveImage(argv[2]);
    cout << "Successfully rendered image!" << endl;
    // delete baseGroup;
    return 0;
}

