#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#include "scene_parser.hpp"
#include "utils.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "material.hpp"
#include "object3d.hpp"
#include "group.hpp"
#include "mesh.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "rectangle.hpp"
#include "triangle.hpp"
#include "transform.hpp"
#include "moving_sphere.hpp"
#include "curve.hpp"
#include "revsurface.hpp"

SceneParser::SceneParser(const char *filename) {

    // initialize some reasonable default values
    group = nullptr;
    camera = nullptr;
    background_color = Vector3f(0.5, 0.5, 0.5);
    num_lights = 0;
    lights = nullptr;
    ambientLight = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;
    num_textures = 0;
    textures = nullptr;
    current_texture = nullptr;
    sample_per_pixel = 100;
    max_depth = 10;
    init_weight = 2;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    if (num_lights == 0) {
        printf("WARNING:    No lights specified\n");
    }
}

SceneParser::~SceneParser() {

    // delete group;
    // delete camera;

    // int i;
    // for (i = 0; i < num_materials; i++) {
    //     delete materials[i];
    // }
    // delete[] materials;
    // for (i = 0; i < num_lights; i++) {
    //     delete lights[i];
    // }
    // delete[] lights;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "Global")){
            parseGlobal();
        } else if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        } else if (!strcmp(token, "Background")) {
            parseBackground();
        } else if (!strcmp(token, "Lights")) {
            parseLights();
        } else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Textures")) {
            parseTextures();
        } else if (!strcmp(token, "Group")) {
            group = parseGroup();
        } else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}
// ====================================================================
// ====================================================================

void SceneParser::parseGlobal() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    while(getToken(token)) {
        if (!strcmp(token, "sample")){
            sample_per_pixel = readInt();
        } else if (!strcmp(token, "depth")){
            max_depth = readInt();
        } else if (!strcmp(token, "weight")){
            init_weight = readFloat();
        } else if (!strcmp(token, "}")){
            break;
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "lookFrom"));
    Vector3f lookFrom = readVector3f();
    getToken(token);
    assert (!strcmp(token, "lookAt"));
    Vector3f lookAt = readVector3f();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();
    float aperture = 0, focus = (lookAt-lookFrom).length(), time0 = 0, time1 = 0;
    while (getToken(token)){
        if (!strcmp(token, "aperture")) {
            aperture = readFloat();
        } else if (!strcmp(token, "focus")) {
            focus = readFloat();
        } else if (!strcmp(token, "time0")) {
            time0 = readFloat();
            getToken(token);
            assert(!strcmp(token, "time1"));
            time1 = readFloat();
        } else if (!strcmp(token, "}")) {
            break;
        }
    }
    camera = new PerspectiveCamera(lookFrom, lookAt, up, width, height, angle_radians, aperture, focus, time0, time1);
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            background_color = readVector3f();
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light *[num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (strcmp(token, "DirectionalLight") == 0) {
            lights[count] = parseDirectionalLight();
        } else if (strcmp(token, "PointLight") == 0) {
            lights[count] = parsePointLight();
        } else {
            printf("Unknown token in parseLight: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "AmbientLight"));
    ambientLight = parseAmbientLight();
    getToken(token);
    assert (!strcmp(token, "}"));
}

Light *SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light *SceneParser::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new PointLight(position, color);
}

Light *SceneParser::parseAmbientLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new AmbientLight(color);
}

// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of materials
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new shared_ptr<Material> [num_materials];
    // read in the materials
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        if (!strcmp(token, "Material")||
            !strcmp(token, "Lambertian")||
            !strcmp(token, "Metal")||
            !strcmp(token, "Dielectric")||
            !strcmp(token, "Light")) {
            materials[count] = parseMaterial(token);
        } else {
            printf("Unknown token in parseMaterial: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}


shared_ptr<Material> SceneParser::parseMaterial(char type[MAX_PARSER_TOKEN_LENGTH]) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Vector3f ambientColor(0, 0, 0), diffuseColor(0, 0, 0), specularColor(0, 0, 0), albedo(0, 0, 0);
    float shininess = 0, fuzz = 0, ir=0, illumination=1;
    shared_ptr<Texture> m_texture = nullptr;
    Vector3f lightColor(1, 1, 1);
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "ambientColor") == 0) {
            ambientColor = readVector3f();
        } else if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();
        } else if (strcmp(token, "specularColor") == 0) {
            specularColor = readVector3f();
        } else if (strcmp(token, "shininess") == 0) {
            shininess = readFloat();
        } else if (strcmp(token, "albedo") == 0){
            albedo = readVector3f();
        } else if (strcmp(token, "fuzz") == 0){
            fuzz = readFloat();
        } else if (strcmp(token, "ir") == 0){
            ir = readFloat(); 
        } else if (strcmp(token, "texture") == 0) {
            m_texture = getTexture(readInt());
        } else if (strcmp(token, "lightColor") == 0) {
            lightColor = readVector3f();
        } else if (strcmp(token, "illumination")==0) {
            illumination = readFloat();
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    shared_ptr<Material> answer = nullptr;
    if(!strcmp(type, "Material")||!strcmp(type, "Lambertian")){
        if (!m_texture) m_texture = make_shared<SolidColor>(Vector3f::ZERO);
        answer = make_shared<Lambertian>(ambientColor, diffuseColor, specularColor, shininess, m_texture);
    } else if (!strcmp(type, "Metal")){
        answer = make_shared<Metal>(ambientColor, diffuseColor, specularColor, shininess, albedo, fuzz);
    } else if (!strcmp(type, "Dielectric")){
        answer = make_shared<Dielectric>(ambientColor, diffuseColor, specularColor, shininess, ir);
    } else if (!strcmp(type, "Light")){
        if (m_texture) {
            answer = make_shared<DiffuseLight>(ambientColor, diffuseColor, specularColor, shininess, m_texture, illumination);
        } else {
            answer = make_shared<DiffuseLight>(ambientColor, diffuseColor, specularColor, shininess, lightColor, illumination);
        } 
    }
    return answer;
}

// ====================================================================
// ====================================================================

void SceneParser::parseTextures() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of textures
    getToken(token);
    assert (!strcmp(token, "numTextures"));
    num_textures = readInt();
    textures = new shared_ptr<Texture> [num_textures];
    // read in the textures
    int count = 0;
    while (num_textures > count) {
        getToken(token);
        if (!strcmp(token, "Color")||
            !strcmp(token, "Checker")||
            !strcmp(token, "Noise")||
            !strcmp(token, "Image")) {
            textures[count] = parseTexture(token);
        } else {
            printf("Unknown token in parseTexture: '%s'\n", token);
            exit(0);
        }
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

shared_ptr<Texture> SceneParser::parseTexture(char type[MAX_PARSER_TOKEN_LENGTH]) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    shared_ptr<Texture> answer = nullptr;
    getToken(token);
    assert (!strcmp(token, "{"));
    if(!strcmp(type, "Color")){
        getToken(token);
        assert(!strcmp(token, "color"));
        Vector3f color = readVector3f();
        answer = make_shared<SolidColor>(color);
    } else if (!strcmp(type, "Checker")){
        getToken(token);
        assert(!strcmp(token, "scale"));
        float scale = readFloat();
        getToken(token);
        assert(!strcmp(token, "color1"));
        Vector3f color1 = readVector3f();
        getToken(token);
        assert(!strcmp(token, "color2"));
        Vector3f color2 = readVector3f();
        answer = make_shared<CheckerTexture>(scale, color1, color2);
    } else if (!strcmp(type, "Noise")){
        getToken(token);
        assert(!strcmp(token, "scale"));
        float scale = readFloat();
        answer = make_shared<NoiseTexture>(scale);
    } else if (!strcmp(type, "Image")){
        char filename[MAX_PARSER_TOKEN_LENGTH];
        filename[0] = 0;
        getToken(token);
        assert(!strcmp(token, "file"));
        getToken(filename);
        answer = make_shared<ImageTexture>(filename);
    }
    getToken(token);
    assert (!strcmp(token, "}"));
    return answer;
}

// ====================================================================
// ====================================================================

shared_ptr<Object3D> SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    shared_ptr<Object3D> answer = nullptr;
    if (!strcmp(token, "Group")) {
        answer = (shared_ptr<Object3D>) parseGroup();
    } else if (!strcmp(token, "Sphere")) {
        answer = (shared_ptr<Object3D>) parseSphere();
    } else if (!strcmp(token, "Plane")) {
        answer = (shared_ptr<Object3D>) parsePlane();
    } else if (!strcmp(token, "Rectangle")) {
        answer = (shared_ptr<Object3D>) parseRectangle();
    } else if (!strcmp(token, "Triangle")) {
        answer = (shared_ptr<Object3D>) parseTriangle();
    } else if (!strcmp(token, "TriangleMesh")) {
        answer = (shared_ptr<Object3D>) parseTriangleMesh();
    } else if (!strcmp(token, "Transform")) {
        answer = (shared_ptr<Object3D>) parseTransform();
    } else if (!strcmp(token, "MovingSphere")) {
        answer = (shared_ptr<Object3D>) parseMovingSphere();
    } else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group *SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            shared_ptr<Object3D>object = parseObject(token);
            assert (object != nullptr);
            answer->addObject(count, object);

            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

shared_ptr<Sphere> SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return make_shared<Sphere>(center, radius, current_material);
}


shared_ptr<Plane> SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert (!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return make_shared<Plane>(normal, offset, current_material);
}

shared_ptr<Rectangle> SceneParser::parseRectangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "dirLen"));
    Vector3f d_len = readVector3f();
    getToken(token);
    assert (!strcmp(token, "dirWid"));
    Vector3f d_wid = readVector3f();
    getToken(token);
    assert (!strcmp(token, "len"));
    float len = readFloat();
    getToken(token);
    assert (!strcmp(token, "wid"));
    float wid = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return make_shared<Rectangle>(center, d_len, d_wid, len, wid, current_material);
}


shared_ptr<Triangle> SceneParser::parseTriangle() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return make_shared<Triangle>(v0, v1, v2, current_material);
}

shared_ptr<Mesh> SceneParser::parseTriangleMesh() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char filename[MAX_PARSER_TOKEN_LENGTH];
    // get the filename
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "obj_file"));
    getToken(filename);
    getToken(token);
    assert (!strcmp(token, "}"));
    const char *ext = &filename[strlen(filename) - 4];
    assert(!strcmp(ext, ".obj"));
    return make_shared<Mesh>(filename, current_material) ;
}


shared_ptr<Transform> SceneParser::parseTransform() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    shared_ptr<Object3D>object = nullptr;
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        } else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        } else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        } else if (!strcmp(token, "XRotate")) {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "YRotate")) {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "ZRotate")) {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert (!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert (!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert (!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert (!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            // otherwise this must be an object,
            // and there are no more transformations
            object = parseObject(token);
            break;
        }
        getToken(token);
    }

    assert(object != nullptr);
    getToken(token);
    assert (!strcmp(token, "}"));
    return make_shared<Transform>(matrix, object);
}

shared_ptr<MovingSphere> SceneParser::parseMovingSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center0"));
    Vector3f center0 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "center1"));
    Vector3f center1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "time0"));
    float time0 = readFloat();
    getToken(token);
    assert (!strcmp(token, "time1"));
    float time1 = readFloat();
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return make_shared<MovingSphere>(center0, center1, time0, time1, radius, current_material);
}

shared_ptr<Curve> SceneParser::parseBezierCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert (!strcmp(token, "]"));
        } else if (!strcmp(token, "}")) {
            break;
        } else {
            printf("Incorrect format for BezierCurve!\n");
            exit(0);
        }
    }
    return make_shared<BezierCurve>(controls);
}


shared_ptr<Curve> SceneParser::parseBsplineCurve() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "controls"));
    std::vector<Vector3f> controls;
    while (true) {
        getToken(token);
        if (!strcmp(token, "[")) {
            controls.push_back(readVector3f());
            getToken(token);
            assert (!strcmp(token, "]"));
        } else if (!strcmp(token, "}")) {
            break;
        } else {
            printf("Incorrect format for BsplineCurve!\n");
            exit(0);
        }
    }
    return make_shared<BsplineCurve>(controls);
}

shared_ptr<RevSurface> SceneParser::parseRevSurface() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "profile"));
    shared_ptr<Curve> profile;
    getToken(token);
    if (!strcmp(token, "BezierCurve")) {
        profile = parseBezierCurve();
    } else if (!strcmp(token, "BsplineCurve")) {
        profile = parseBsplineCurve();
    } else {
        printf("Unknown profile type in parseRevSurface: '%s'\n", token);
        exit(0);
    }
    getToken(token);
    assert (!strcmp(token, "}"));
    return make_shared<RevSurface>(profile, current_material);
}

// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert (0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}
