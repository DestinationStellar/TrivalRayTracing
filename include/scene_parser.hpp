#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <cassert>
#include <vecmath.h>
#include <memory>

class Camera;
class Light;
class Material;
class Texture;
class Object3D;
class Group;
class Sphere;
class Plane;
class Rectangle;
class Triangle;
class Transform;
class Mesh;
class MovingSphere;
class Curve;
class RevSurface;

using std::shared_ptr;

#define MAX_PARSER_TOKEN_LENGTH 1024

class SceneParser {
public:

    SceneParser() = delete;
    SceneParser(const char *filename);

    ~SceneParser();

    int getSamplePerPixel() const {
        return sample_per_pixel;
    }

    int getMaxDepth() const {
        return max_depth;
    }

    int getInitWeight() const {
        return init_weight;
    }

    Camera *getCamera() const {
        return camera;
    }

    Vector3f getBackgroundColor() const {
        return background_color;
    }

    int getNumLights() const {
        return num_lights;
    }

    Light *getLight(int i) const {
        assert(i >= 0 && i < num_lights);
        return lights[i];
    }

    Light *getAmbientLight() const {
        return ambientLight;
    }

    int getNumMaterials() const {
        return num_materials;
    }

    shared_ptr<Material> getMaterial(int i) const {
        assert(i >= 0 && i < num_materials);
        return materials[i];
    }

    int getNumTextures() const {
        return num_textures;
    }

    shared_ptr<Texture> getTexture(int i) const {
        assert(i >= 0 && i < num_textures);
        return textures[i];
    }

    Group *getGroup() const {
        return group;
    }

private:

    void parseFile();
    void parseGlobal();
    void parsePerspectiveCamera();
    void parseBackground();
    void parseLights();
    Light *parsePointLight();
    Light *parseDirectionalLight();
    Light *parseAmbientLight();
    void parseMaterials();
    shared_ptr<Material> parseMaterial(char token[MAX_PARSER_TOKEN_LENGTH]);
    void parseTextures();
    shared_ptr<Texture> parseTexture(char token[MAX_PARSER_TOKEN_LENGTH]);
    shared_ptr<Object3D> parseObject(char token[MAX_PARSER_TOKEN_LENGTH]);
    Group* parseGroup();
    shared_ptr<Sphere> parseSphere();
    shared_ptr<Plane> parsePlane();
    shared_ptr<Rectangle> parseRectangle();
    shared_ptr<Triangle> parseTriangle();
    shared_ptr<Mesh> parseTriangleMesh();
    shared_ptr<Transform> parseTransform();
    shared_ptr<MovingSphere> parseMovingSphere();
    shared_ptr<Curve> parseBezierCurve();
    shared_ptr<Curve> parseBsplineCurve();
    shared_ptr<RevSurface> parseRevSurface();


    int getToken(char token[MAX_PARSER_TOKEN_LENGTH]);

    Vector3f readVector3f();

    float readFloat();
    int readInt();

    FILE *file;
    int sample_per_pixel;
    int max_depth;
    float init_weight;
    Camera *camera;
    Vector3f background_color;
    int num_lights;
    Light **lights;
    Light *ambientLight;
    int num_materials;
    shared_ptr<Material>* materials;
    shared_ptr<Material> current_material;
    int num_textures;
    shared_ptr<Texture> *textures;
    shared_ptr<Texture> current_texture;
    Group *group;
};

#endif // SCENE_PARSER_H
