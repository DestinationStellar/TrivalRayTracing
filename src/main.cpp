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
#include "render.hpp"


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
    RayTracer *rayTracer = nullptr;
    if(stoi(method) == 0) {
        int m = stoi(input);
        SceneGenerator sceneGenerator(m);
        rayTracer = new RayTracer(sceneGenerator, argv[3]);
    }
    else {
        SceneParser sceneParser(argv[2]);
        rayTracer = new RayTracer(sceneParser, argv[3]);
    }

    cout << "scene loaded!" << endl;

    rayTracer->render();    

    return 0;
}

