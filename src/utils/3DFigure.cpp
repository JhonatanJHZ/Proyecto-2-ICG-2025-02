#include "3DFigure.h"
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>
#include "../glm/geometric.hpp" 
#include "../glm/glm.hpp"

using namespace std;

C3DFigure::C3DFigure() {}
C3DFigure::~C3DFigure() {}

bool C3DFigure::loadMtl(string path, map<string, Material>& materialMap) {
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Aviso: No se encontró el archivo MTL. Usando color gris por defecto." << endl;
        return false;
    }

    string line;
    Material* currentMaterial = nullptr;
    while (getline(file, line)) {
        stringstream ss(line);
        string type;
        ss >> type;

        if (type == "newmtl") {
            string name;
            ss >> name;
            materialMap[name] = Material();
            currentMaterial = &materialMap[name];
            currentMaterial->name = name;
        }
        else if (currentMaterial) {
            if (type == "Ns") {
                ss >> currentMaterial->ns;
            }
            else if (type == "Ka") {
                ss >> currentMaterial->ka[0] >> currentMaterial->ka[1] >> currentMaterial->ka[2];
            }
            else if (type == "Kd") {
                ss >> currentMaterial->kd[0] >> currentMaterial->kd[1] >> currentMaterial->kd[2];
            }
            else if (type == "Ks") {
                ss >> currentMaterial->ks[0] >> currentMaterial->ks[1] >> currentMaterial->ks[2];
            }
            else if (type == "d") {
                ss >> currentMaterial->d;
            }
            else if (type == "illum") {
                int illumValue;
                ss >> illumValue;
                currentMaterial->illum = static_cast<float>(illumValue);
            }
            else if (type == "map_Kd") {
                ss >> currentMaterial->textureMap;
            }
        }
    }
    file.close();
    return true;
}

bool C3DFigure::loadObject(string path) {
    ifstream entrada(path);
    if (!entrada.is_open()) {
        cout << "Error abriendo el archivo" << endl;
        return false;
    }

    string linea;
    map<string, Material> materialMap;
    SubMesh* currentSubMesh = nullptr;

    while (getline(entrada, linea)) {
        stringstream ss(linea);
        string tipo;
        if (!(ss >> tipo)) continue;

        if (tipo == "v") {
            vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (tipo == "vn") {
            vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            normals.push_back(vn);
        }
        else if (tipo == "vt") {
            vec3 vt;
            ss >> vt.x >> vt.y >> vt.z;
            textures.push_back(vt);
        }
        else if (tipo == "mtllib") {
            string mtlFile;
            ss >> mtlFile;
            string dir = "";
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != string::npos) dir = path.substr(0, lastSlash + 1);
            loadMtl(dir + mtlFile, materialMap);
        }
        else if (tipo == "g") {
            string groupName;
            ss >> groupName;
            subMeshes.push_back(SubMesh());
            currentSubMesh = &subMeshes.back();
            currentSubMesh->groupName = groupName;
        }
        else if (tipo == "f") {
            if (currentSubMesh == nullptr) {
                subMeshes.push_back(SubMesh());
                currentSubMesh = &subMeshes.back();
            }
            string vData;
            vector<vec3> polygonVertices;
            while (ss >> vData) {
                vec3 vertexData(0.0f, 0.0f, 0.0f);
                size_t firstSlash = vData.find('/');
                size_t secondSlash = vData.find('/', firstSlash + 1);

                vertexData.x = static_cast<float>(stoi(vData.substr(0, firstSlash)) - 1);

                if (firstSlash != string::npos && firstSlash + 1 != secondSlash) {
                    string part = vData.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                    if (!part.empty()) vertexData.y = static_cast<float>(stoi(part) - 1);
                }

                if (secondSlash != string::npos) {
                    string part = vData.substr(secondSlash + 1);
                    if (!part.empty()) vertexData.z = static_cast<float>(stoi(part) - 1);
                }
                polygonVertices.push_back(vertexData);
            }

            if (polygonVertices.size() >= 3) {
                for (size_t i = 1; i < polygonVertices.size() - 1; ++i) {
                    FaceElement face;
                    face.vertexIndices[0] = static_cast<int>(polygonVertices[0].x);
                    face.textureIndices[0] = static_cast<int>(polygonVertices[0].y);
                    face.normalIndices[0] = static_cast<int>(polygonVertices[0].z);

                    face.vertexIndices[1] = static_cast<int>(polygonVertices[i].x);
                    face.textureIndices[1] = static_cast<int>(polygonVertices[i].y);
                    face.normalIndices[1] = static_cast<int>(polygonVertices[i].z);

                    face.vertexIndices[2] = static_cast<int>(polygonVertices[i + 1].x);
                    face.textureIndices[2] = static_cast<int>(polygonVertices[i + 1].y);
                    face.normalIndices[2] = static_cast<int>(polygonVertices[i + 1].z);

                    currentSubMesh->faces.push_back(face);
                }
            }
        }
        else if (tipo == "usemtl") {
            string mtlName;
            ss >> mtlName;
            subMeshes.push_back(SubMesh());
            currentSubMesh = &subMeshes.back();
            currentSubMesh->groupName = mtlName;

            if (materialMap.count(mtlName)) {
                currentSubMesh->material = materialMap[mtlName];
            }
            else {
                currentSubMesh->material.kd[0] = 0.7f;
                currentSubMesh->material.kd[1] = 0.7f;
                currentSubMesh->material.kd[2] = 0.7f;
            }
        }
    }

    if(normals.empty()){
        normals.assign(vertices.size(), vec3(0.0f));
        for (auto& subMesh : subMeshes) {
            for (auto& face : subMesh.faces) {
                int i0 = face.vertexIndices[0];
                int i1 = face.vertexIndices[1];
                int i2 = face.vertexIndices[2];

                vec3 v0 = vertices[i0];
                vec3 v1 = vertices[i1];
                vec3 v2 = vertices[i2];

                vec3 edge1 = v1 - v0;
                vec3 edge2 = v2 - v0;
                vec3 faceNormal = cross(edge1, edge2);

                normals[i0] += faceNormal;
                normals[i1] += faceNormal;
                normals[i2] += faceNormal;
                
                face.normalIndices[0] = face.vertexIndices[0];
                face.normalIndices[1] = face.vertexIndices[1];
                face.normalIndices[2] = face.vertexIndices[2];
            }
        }

        for (int i = 0; i < normals.size(); i++) {
            if (length(normals[i]) > 0.0f) {
                normals[i] = normalize(normals[i]);
            }
        }
    }
    entrada.close();
    return true;
}

void C3DFigure::normalization() {
    if (vertices.empty()) return;

    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
        minZ = std::min(minZ, v.z); maxZ = std::max(maxZ, v.z);
    }

    vec3 center = vec3((minX + maxX) / 2.0f, (minY + maxY) / 2.0f, (minZ + maxZ) / 2.0f);

    float dx = maxX - minX;
    float dy = maxY - minY;
    float dz = maxZ - minZ;
    float maxDim = max({dx, dy, dz});
    float scaleFactor = (maxDim == 0) ? 1.0f : 1.0f / maxDim;

    for (auto& v : vertices) {
        v = (v - center) * scaleFactor;
    }

    boundingBox.min = (vec3(minX, minY, minZ) - center) * scaleFactor;
    boundingBox.max = (vec3(maxX, maxY, maxZ) - center) * scaleFactor;

    for (auto& mesh : subMeshes) {
        if (mesh.faces.empty()) continue;
        
        vec3 subMin = vertices[mesh.faces[0].vertexIndices[0]];
        vec3 subMax = subMin;

        for (const auto& face : mesh.faces) {
            for (int i = 0; i < 3; ++i) {
                int idx = face.vertexIndices[i];
                if (idx < vertices.size()) {
                    vec3 v = vertices[idx];
                    subMin = glm::min(subMin, v);
                    subMax = glm::max(subMax, v);
                }
            }
        }
        mesh.bbox.min = subMin;
        mesh.bbox.max = subMax;
    }
}

vector<float> C3DFigure::flatten() {
    vector<float> data;
    int currentVertexOffset = 0; 
    for (auto& mesh : subMeshes) {
        mesh.startVertex = currentVertexOffset;
        int count = 0;

        for (const auto& face : mesh.faces) {
            for (int i = 0; i < 3; ++i) {
                int vIdx = face.vertexIndices[i];
                if (vIdx >= 0 && vIdx < vertices.size()) {
                    data.push_back(vertices[vIdx].x);
                    data.push_back(vertices[vIdx].y);
                    data.push_back(vertices[vIdx].z);
                    data.push_back(mesh.material.kd[0]);
                    data.push_back(mesh.material.kd[1]);
                    data.push_back(mesh.material.kd[2]);
                    count++;
                }
            }
        }
        mesh.vertexCount = count;    
        currentVertexOffset += count; 
    }
    return data;
}

BoundingBox C3DFigure::getBoundingBox(){
    return boundingBox;
}

const vector<SubMesh>& C3DFigure::getSubMeshes() { 
    return subMeshes; 
}

vector<SubMesh>& C3DFigure::getSubMeshesModifiable() {
    return subMeshes;
}

void C3DFigure::deleteSubMesh(int index) {
    if (index >= 0 && index < subMeshes.size()) {
        subMeshes.erase(subMeshes.begin() + index);
    }
}

void C3DFigure::saveObject(string filename, vec3 globalPos, quat globalRot, vec3 scale) {
    if (filename.empty()) return;
    
    string objName = filename;
    if (objName.length() < 4 || objName.substr(objName.length() - 4) != ".obj") {
        objName += ".obj";
    }
    
    string mtlName = objName.substr(0, objName.length() - 4) + ".mtl";
    string mtlSimpleName = mtlName;
    size_t lastSlash = mtlName.find_last_of("/\\");
    if (lastSlash != string::npos) {
        mtlSimpleName = mtlName.substr(lastSlash + 1);
    }

    ofstream objFile(objName);
    ofstream mtlFile(mtlName);
    
    if (!objFile.is_open() || !mtlFile.is_open()) {
        cout << "Error salvando archivo: " << objName << endl;
        return;
    }

    mtlFile << "# Generated by 3DViewer" << endl;
    
    objFile << "# Generated by 3DViewer" << endl;
    objFile << "mtllib " << mtlSimpleName << endl;

    int vertexOffset = 1; 
    
    for (const auto& mesh : subMeshes) {
        mtlFile << "newmtl " << mesh.groupName << endl;
        mtlFile << "Ns " << mesh.material.ns << endl;
        mtlFile << "Ka " << mesh.material.ka[0] << " " << mesh.material.ka[1] << " " << mesh.material.ka[2] << endl;
        mtlFile << "Kd " << mesh.material.kd[0] << " " << mesh.material.kd[1] << " " << mesh.material.kd[2] << endl;
        mtlFile << "Ks " << mesh.material.ks[0] << " " << mesh.material.ks[1] << " " << mesh.material.ks[2] << endl;
        mtlFile << "Ni " << mesh.material.ni << endl;
        mtlFile << "d " << mesh.material.d << endl;
        mtlFile << "illum 2" << endl; 
        if (!mesh.material.textureMap.empty()) {
            mtlFile << "map_Kd " << mesh.material.textureMap << endl;
        }
        mtlFile << endl;

        objFile << "usemtl " << mesh.groupName << endl;
        
        map<int, int> indexMap; 

        for (const auto& face : mesh.faces) {
            bool validFace = true;
            for (int k = 0; k < 3; ++k) {
                if (face.vertexIndices[k] < 0 || face.vertexIndices[k] >= vertices.size()) {
                    validFace = false;
                    break;
                }
            }
            if (!validFace) continue;

            for (int i = 0; i < 3; ++i) {
                int oldIdx = face.vertexIndices[i];
                
                if (indexMap.find(oldIdx) == indexMap.end()) {
                    vec3 v = vertices[oldIdx];
                    vec3 bakedV = v + mesh.offset; 
                    bakedV = bakedV * scale; 
                    bakedV = globalRot * bakedV; 
                    bakedV += globalPos; 
                    
                    objFile << "v " << bakedV.x << " " << bakedV.y << " " << bakedV.z << endl;
                    
                    indexMap[oldIdx] = vertexOffset++;
                }
            }

            objFile << "f";
            for (int i = 0; i < 3; ++i) {
                int oldIdx = face.vertexIndices[i];
                int finalIdx = indexMap[oldIdx];
                objFile << " " << finalIdx;
            }
            objFile << endl;
        }
    }
    
    objFile.close();
    mtlFile.close();
    cout << "Guardado exitoso: " << objName << endl;
}