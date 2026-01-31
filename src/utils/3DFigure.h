#pragma once
#include <iostream>
#include <vector>
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/vec3.hpp"
#include "../glm/gtc/quaternion.hpp"
#include "../glm/gtx/quaternion.hpp"
#include <string>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;
using namespace glm;

struct Material{
    string name;
    float ns;
    vec3 ka;
    vec3 kd = {0.7f, 0.7f, 0.7f};
    vec3 ks;
    float ni, d;
    float illum;
    string textureMap;
};

struct FaceElement{
    int vertexIndices[3];
    int textureIndices[3];
    int normalIndices[3];
};

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

struct SubMesh{
    string groupName;
    Material material;
    vector<FaceElement> faces;

    int startVertex = 0;
    int vertexCount = 0;
    
    vec3 offset = vec3(0.0f); 
    BoundingBox bbox;
};

class C3DFigure {
    vector<vec3> vertices;
    vector<vec3> normals;
    vector<vec3> textures;
    vector<SubMesh> subMeshes;

    BoundingBox boundingBox;

public:
    C3DFigure();
    ~C3DFigure();

    bool loadObject(string path);
    bool loadMtl(string path, map<string, Material>& materialMap);
    void normalization();
    BoundingBox getBoundingBox();
    vector<float> flatten();
    const vector<SubMesh>& getSubMeshes();
    vector<SubMesh>& getSubMeshesModifiable();
    void deleteSubMesh(int index);
    void saveObject(string path, glm::vec3 pos, glm::quat rot, glm::vec3 scale);
};