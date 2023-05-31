#include "CAST.h"
#include "../ExportModel.h"

using namespace HAYDEN;

#include <fstream>
#include <iostream>
#include <cmath>

#include "../cast/cast.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace Cast;

//void CAST::ConvertFromLWO(LWO &lwo, const std::vector<MaterialInfo> &vector) {
//
//}

size_t findIndex(const std::vector<uint8_t> &vec, uint8_t value) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == value) {
            return i;  // Return the index when a match is found
        }
    }

    return -1;  // Return -1 if the value is not found
}

template<typename K, typename V>
bool contains(const std::map<K, V> map, K key) {
    return map.find(key) != map.end();
}


void addTextureIfPresent(const std::string &name, const std::string &castSemantic, const MaterialInfo &matInfo,
                         CastNode &materialNode) {
    if (contains(matInfo.TextureMapping, std::string(name))) {
        std::hash<std::string> hasher;
        std::string textureName = matInfo.TextureMapping.at(name);
        CastNode textureNode(CastId::File);
        textureNode.setHash(hasher(textureName));
        textureNode.addProperty(std::make_unique<StringCastProperty>("p", (fs::path("textures") / fs::path(
                textureName).filename().replace_extension(".png")).string()));
        materialNode.addProperty(std::make_unique<Uint64CastProperty>(castSemantic, textureNode.hash()));
        materialNode.addChildren(std::move(textureNode));
    }
}


void CAST::ConvertFromMD6(const MD6Mesh &md6Mesh, const MD6Skl &md6Skl, const std::vector<MaterialInfo> &materials) {
    std::hash<std::string> hasher;
    CastNode rootNode(CastId::Root);

    CastNode modelNode(CastId::Model);

    modelNode.addProperty(std::make_unique<StringCastProperty>("n", md6Mesh.header().m_sklFilename));

    CastNode skeletonNode(CastId::Skeleton);
    for (const auto &item: md6Skl.bones()) {
        CastNode boneNode(CastId::Bone);
        boneNode.addProperty(std::make_unique<StringCastProperty>("n", item.m_name));
        boneNode.addProperty(std::make_unique<Uint32CastProperty>("p", item.m_parentId));
        boneNode.addProperty(std::make_unique<Vector4CastProperty>("lr", item.m_quat));
        boneNode.addProperty(std::make_unique<Vector3CastProperty>("lp", item.m_pos));
        boneNode.addProperty(std::make_unique<Vector3CastProperty>("s", item.m_scale));

        skeletonNode.addChildren(std::move(boneNode));
    }

    modelNode.addChildren(std::move(skeletonNode));
    for (const auto &material: materials) {
        CastNode materialNode(CastId::Material);
        const std::string &materialShortName = fs::path(material.DeclFileName).stem().string();
        materialNode.setHash(hasher(materialShortName));
        materialNode.addProperty(std::make_unique<StringCastProperty>("n", materialShortName));
        materialNode.addProperty(std::make_unique<StringCastProperty>("t", "default"));
        addTextureIfPresent("albedo", "albedo", material, materialNode);
        addTextureIfPresent("normal", "normal", material, materialNode);
        addTextureIfPresent("specular", "specular", material, materialNode);
        addTextureIfPresent("smoothness", "gloss", material, materialNode);
        addTextureIfPresent("bloommaskmap", "emissive", material, materialNode);
        addTextureIfPresent("sssmask", "extra0", material, materialNode);
        addTextureIfPresent("colormask", "extra1", material, materialNode);
        modelNode.addChildren(std::move(materialNode));
    }

    for (int i = 0; i < md6Mesh.header().m_meshInfo.size(); i++) {
        const MD6MeshInfo &meshInfo = md6Mesh.header().m_meshInfo[i];
        const Mesh &mesh = md6Mesh.meshGeometry()[i];
        static MaterialInfo blankMaterial = MaterialInfo{"Blank", {}};
        MaterialInfo &material = blankMaterial;
        for (const auto &item: materials) {
            if (item.DeclFileName == meshInfo.MaterialDeclName) {
                material = item;
            }
        }

        CastNode meshNode(CastId::Mesh);
        meshNode.addProperty(std::make_unique<StringCastProperty>("n", meshInfo.MeshName));
        std::unique_ptr<Vector3CastProperty> vertices = std::make_unique<Vector3CastProperty>("vp");
        vertices->resize(mesh.m_vertices.size());

        std::unique_ptr<Vector3CastProperty> normals = std::make_unique<Vector3CastProperty>("vn");
        normals->resize(mesh.m_vertices.size());

        std::unique_ptr<Vector2CastProperty> uvs = std::make_unique<Vector2CastProperty>("u0");
        uvs->resize(mesh.m_vertices.size());

        std::unique_ptr<Uint8CastProperty> boneIds = std::make_unique<Uint8CastProperty>("wb");
        boneIds->resize(mesh.m_boneIds.size() * 4);

        std::unique_ptr<FloatCastProperty> boneWeights = std::make_unique<FloatCastProperty>("wv");
        boneWeights->resize(mesh.m_boneWeights.size() * 4);

        std::unique_ptr<Uint16CastProperty> faces = std::make_unique<Uint16CastProperty>("f");
        faces->resize(mesh.m_faces.size() * 3);

        for (int j = 0; j < mesh.m_vertices.size(); j++) {
            const Cast::Vector3 &vertex = mesh.m_vertices[j];
            const Cast::Vector3 &normal = mesh.m_normals[j];
            const Cast::Vector2 &uv = mesh.m_uv[j];
            const BoneIds &boneId = mesh.m_boneIds[j];

            vertices->array()[j] = {vertex.x, -vertex.z, vertex.y};
            normals->array()[j] = {-normal.x, normal.y, -normal.z};
            uvs->array()[j] = {uv.x, 1 - uv.y};
            boneIds->array()[j * 4 + 0] = findIndex(md6Mesh.header().m_boneIds, boneId.a);
            boneIds->array()[j * 4 + 1] = findIndex(md6Mesh.header().m_boneIds, boneId.b);
            boneIds->array()[j * 4 + 2] = findIndex(md6Mesh.header().m_boneIds, boneId.c);
            boneIds->array()[j * 4 + 3] = findIndex(md6Mesh.header().m_boneIds, boneId.d);
        }

        std::memcpy(faces->data(), mesh.m_faces.data(), mesh.m_faces.size() * sizeof(Face));
        std::memcpy(boneWeights->data(), mesh.m_boneWeights.data(), mesh.m_boneWeights.size() * sizeof(BoneWeights));

        meshNode.addProperty(std::move(vertices));
        meshNode.addProperty(std::move(normals));
        meshNode.addProperty(std::move(boneIds));
        meshNode.addProperty(std::move(boneWeights));
        meshNode.addProperty(std::move(uvs));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("ul", 1));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("mi", 4));
        meshNode.addProperty(std::move(faces));
        meshNode.addProperty(
                std::make_unique<Uint64CastProperty>("m", hasher(fs::path(material.DeclFileName).stem().string())));

        modelNode.addChildren(std::move(meshNode));
    }

    rootNode.addChildren(std::move(modelNode));
    addRoot(std::move(rootNode));
}

void CAST::ConvertFromLWO(const LWO &lwo, const std::vector<MaterialInfo> &materials) {
    std::hash<std::string> hasher;
    CastNode rootNode(CastId::Root);

    CastNode modelNode(CastId::Model);
    for (const auto &material: materials) {
        CastNode materialNode(CastId::Material);
        const std::string &materialShortName = fs::path(material.DeclFileName).stem().string();
        materialNode.setHash(hasher(materialShortName));
        materialNode.addProperty(std::make_unique<StringCastProperty>("n", materialShortName));
        materialNode.addProperty(std::make_unique<StringCastProperty>("t", "default"));
        addTextureIfPresent("albedo", "albedo", material, materialNode);
        addTextureIfPresent("normal", "normal", material, materialNode);
        addTextureIfPresent("specular", "specular", material, materialNode);
        addTextureIfPresent("smoothness", "gloss", material, materialNode);
        addTextureIfPresent("bloommaskmap", "emissive", material, materialNode);
        addTextureIfPresent("sssmask", "extra0", material, materialNode);
        addTextureIfPresent("colormask", "extra1", material, materialNode);
        modelNode.addChildren(std::move(materialNode));
    }

    for (int i = 0; i < lwo.header().m_meshInfo.size(); i++) {
        const LWOMeshInfo &meshInfo = lwo.header().m_meshInfo[i];
        const Mesh &mesh = lwo.meshGeometry()[i];
        static MaterialInfo blankMaterial = MaterialInfo{"Blank", {}};
        MaterialInfo &material = blankMaterial;
        for (const auto &item: materials) {
            if (item.DeclFileName == meshInfo.MaterialDeclName) {
                material = item;
            }
        }

        CastNode meshNode(CastId::Mesh);
        meshNode.addProperty(std::make_unique<StringCastProperty>("n", std::string("Mesh") + "_" + std::to_string(i)));
        std::unique_ptr<Vector3CastProperty> vertices = std::make_unique<Vector3CastProperty>("vp");
        vertices->resize(mesh.m_vertices.size());

        std::unique_ptr<Vector3CastProperty> normals = std::make_unique<Vector3CastProperty>("vn");
        normals->resize(mesh.m_vertices.size());

        std::unique_ptr<Vector2CastProperty> uvs = std::make_unique<Vector2CastProperty>("u0");
        uvs->resize(mesh.m_vertices.size());

        std::unique_ptr<Uint16CastProperty> faces = std::make_unique<Uint16CastProperty>("f");
        faces->resize(mesh.m_faces.size() * 3);

        for (int j = 0; j < mesh.m_vertices.size(); j++) {
            const Cast::Vector3 &vertex = mesh.m_vertices[j];
            const Cast::Vector3 &normal = mesh.m_normals[j];
            const Cast::Vector2 &uv = mesh.m_uv[j];

            vertices->array()[j] = {vertex.x, -vertex.z, vertex.y};
            normals->array()[j] = {-normal.x, normal.y, -normal.z};
            uvs->array()[j] = {uv.x, 1 - uv.y};
        }

        std::memcpy(faces->data(), mesh.m_faces.data(), mesh.m_faces.size() * sizeof(Face));

        meshNode.addProperty(std::move(vertices));
        meshNode.addProperty(std::move(normals));
        meshNode.addProperty(std::move(uvs));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("ul", 1));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("mi", 0));
        meshNode.addProperty(std::move(faces));
        meshNode.addProperty(
                std::make_unique<Uint64CastProperty>("m", hasher(fs::path(material.DeclFileName).stem().string())));

        modelNode.addChildren(std::move(meshNode));
    }

    rootNode.addChildren(std::move(modelNode));
    addRoot(std::move(rootNode));
}


void CAST::toFile(std::ofstream &stream) {

    CastHeader header{0x74736163, 1, static_cast<uint32_t>(m_roots.size()), 0};

    std::cout << "Serializing" << std::endl;
    stream.write((char *) &header, sizeof(CastHeader));

    for (const auto &item: m_roots) {
        item.serialize(stream);
    }


}

