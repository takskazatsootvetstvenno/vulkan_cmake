#include "App.h"
#include "Logger.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <climits>
#include <cstdint>
#include <fstream>
#include <memory>
using namespace std;

static_assert(CHAR_BIT == 8 && sizeof(int) == 4, "char must be 8 bits, int must be 4 bytes!");
static_assert(sizeof(float) == 4, "float must be 4 bytes");

constexpr glm::mat4 convertMatrix(const aiMatrix4x4& aiMat) noexcept {
    return {aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1, aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
            aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3, aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4};
}

void processMesh(aiMesh* aiMesh, const aiScene* scene, std::string_view basePath, std::vector<sge::Mesh>& meshes,
                 glm::mat4 posMat) {
    sge::Mesh mesh;
    mesh.m_pos.resize(aiMesh->mNumVertices);

    auto ai_material = scene->mMaterials[aiMesh->mMaterialIndex];

    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
        glm::vec3 pos = {aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z};
        mesh.m_pos[i].m_position = pos;
        mesh.m_pos[i].m_normal = {aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z};
        if (aiMesh->mTextureCoords[0] != nullptr) {
            mesh.m_pos[i].m_UV = {aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y};
        }
    }
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i) {
        aiFace face = aiMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) mesh.m_ind.push_back(face.mIndices[j]);
    }

    ai_material->Get(AI_MATKEY_METALLIC_FACTOR, mesh.m_material.m_metallicFactor);
    ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, mesh.m_material.m_roughnessFactor);
    ai_material->Get(AI_MATKEY_EMISSIVE_INTENSITY, mesh.m_material.m_emissiveFactor);

    aiColor3D baseColor(1.f, 1.f, 1.f);
    ai_material->Get(AI_MATKEY_BASE_COLOR, baseColor);
    if (baseColor.IsBlack()) ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor);
    mesh.m_material.m_baseColor = {baseColor[0], baseColor[1], baseColor[2], 1.f};

    aiColor3D emissiveFactor;
    ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor);
    mesh.m_material.m_emissiveFactor = glm::vec3{emissiveFactor[0], emissiveFactor[1], emissiveFactor[2]};

    mesh.m_material.m_hasColorMap = ai_material->GetTextureCount(aiTextureType_BASE_COLOR) ||
                                    ai_material->GetTextureCount(aiTextureType_DIFFUSE);
    mesh.m_material.m_hasOcclusionMap = ai_material->GetTextureCount(aiTextureType_LIGHTMAP);
    mesh.m_material.m_hasMetallicRoughnessMap = ai_material->GetTextureCount(aiTextureType_UNKNOWN);
    mesh.m_material.m_hasEmissiveMap = ai_material->GetTextureCount(aiTextureType_EMISSIVE);
    mesh.m_material.m_hasNormalMap = ai_material->GetTextureCount(aiTextureType_NORMALS);

    aiString str1;
    ai_material->GetTexture(aiTextureType_LIGHTMAP, 0, &str1);
    static size_t meshID = 0;
    mesh.setName(std::to_string(meshID) + " | " + std::string(basePath));
    ++meshID;
    if (auto basePosPathToFile = basePath.find_last_of('/'); basePosPathToFile != basePath.npos)
        basePath.remove_suffix(basePath.size() - basePosPathToFile - 1);
    else
        basePath = "";
    if (mesh.m_material.m_hasColorMap) {
        aiString str;
        ai_material->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
        if (str.length == 0) { ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &str); }
        mesh.m_material.m_baseColorPath = std::string(basePath) + str.C_Str();
    }
    if (mesh.m_material.m_hasMetallicRoughnessMap) {
        aiString str;
        // ai_material->GetTexture(aiTextureType_LIGHTMAP, 0, &str);
        ai_material->GetTexture(aiTextureType_UNKNOWN, 0, &str);
        mesh.m_material.m_MetallicRoughnessPath = std::string(basePath) + str.C_Str();
    }
    if (mesh.m_material.m_hasEmissiveMap) {
        aiString str;
        ai_material->GetTexture(aiTextureType_EMISSIVE, 0, &str);
        mesh.m_material.m_EmissivePath = std::string(basePath) + str.C_Str();
    }
    if (mesh.m_material.m_hasNormalMap) {
        aiString str;
        ai_material->GetTexture(aiTextureType_NORMALS, 0, &str);
        mesh.m_material.m_NormalPath = std::string(basePath) + str.C_Str();
    }

    aiShadingMode mode;
    ai_material->Get(AI_MATKEY_SHADING_MODEL, mode);
    switch (mode) {
        case aiShadingMode_Phong: mesh.m_materialType = sge::Mesh::MaterialType::Phong; break;
        case aiShadingMode_PBR_BRDF: mesh.m_materialType = sge::Mesh::MaterialType::PBR; break;
        default:
            mesh.m_materialType = sge::Mesh::MaterialType::Phong;
            LOG_ERROR("Model loading: Unsupported shading model!");
            break;
    }

    mesh.m_boundingBox.min = {aiMesh->mAABB.mMin.x, aiMesh->mAABB.mMin.y, aiMesh->mAABB.mMin.z};
    mesh.m_boundingBox.max = {aiMesh->mAABB.mMax.x, aiMesh->mAABB.mMax.y, aiMesh->mAABB.mMax.z};

    glm::vec3 result_min = posMat * glm::vec4(mesh.m_boundingBox.min, 1.f);
    glm::vec3 result_max = posMat * glm::vec4(mesh.m_boundingBox.max, 1.f);
    glm::vec3 v = glm::abs(result_max - result_min);
    const auto max = std::max(std::max(v.x, v.y), v.z);
    const float d = 1.f / (max / 10.f);
    posMat = glm::scale(posMat, glm::vec3(d));
    mesh.setModelMatrix(std::move(posMat));
    meshes.push_back(std::move(mesh));
}

void processNode(aiNode* node, const aiScene* scene, const std::string_view basePath, std::vector<sge::Mesh>& meshes,
                 glm::mat4 const& parentMat = glm::mat4(1.0f)) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* aiMesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(aiMesh, scene, basePath, meshes, parentMat * convertMatrix(node->mTransformation));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        processNode(node->mChildren[i], scene, basePath, meshes, parentMat * convertMatrix(node->mTransformation));
}

void load_model(sge::App& app, const std::string_view path, const glm::mat4 rootMatrix = glm::mat4{1.f}) {
    Assimp::Importer importer;
    unsigned int flags = aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_GenBoundingBoxes |
                         aiProcess_GenUVCoords | aiProcess_GenSmoothNormals;

    auto scene = importer.ReadFile(path.data(), flags);
    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
        LOG_ERROR("CLIENT: Can't open file:" << path);
        app.loadModels(std::vector<sge::Mesh>());
        return;
    }

    std::vector<sge::Mesh> meshes;
    auto& m = rootMatrix;

    processNode(scene->mRootNode, scene, path, meshes, m);
    app.loadModels(std::move(meshes));
}

int main() {
    {
#ifdef _DEBUG
#    ifdef _MSC_VER
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#    endif
#endif  // _DEBUG

        sge::App my_app({1280, 720}, "Vulkan engine");

        load_model(my_app, "Models/WaterBottle/WaterBottle.gltf");
        my_app.run();
    }
    return 0;
}
