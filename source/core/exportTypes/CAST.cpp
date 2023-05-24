#include "CAST.h"

using namespace HAYDEN;

#include <fstream>
#include <iostream>
#include <cmath>

#include "../cast/cast.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace Cast;

void CAST::ConvertFromLWO(LWO &lwo) {

}

int findIndex(const std::vector<uint8_t> &vec, uint8_t value) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == value) {
            return i;  // Return the index when a match is found
        }
    }

    return -1;  // Return -1 if the value is not found
}

void invertPositionAndFixQuaternion(Cast::Vector3 &position, Cast::Vector4 &quaternion) {
    // Invert position on the X-axis
    position.x *= -1.0f;

    // Fix up the quaternion rotation
    float angle = -M_PI;  // Negative rotation around the X-axis
    float sinHalfAngle = std::sin(angle * 0.5f);

    float qx = std::sin(angle * 0.5f) * 1.0f;
    float qw = std::cos(angle * 0.5f);

    float newQuatX = quaternion.w * qx + quaternion.x * qw;
    float newQuatY = quaternion.y * qw + quaternion.z * qx;
    float newQuatZ = quaternion.z * qw - quaternion.y * qx;
    float newQuatW = quaternion.w * qw - quaternion.x * qx;

    quaternion.x = newQuatX;
    quaternion.y = newQuatY;
    quaternion.z = newQuatZ;
    quaternion.w = newQuatW;
}

void CAST::ConvertFromMD6(const MD6Mesh &md6Mesh, const MD6Skl &md6Skl) {
    CastNode rootNode(CastId::Root);

    CastNode modelNode(CastId::Model);

    modelNode.addProperty(std::make_unique<StringCastProperty>("n", md6Mesh.header().m_sklFilename));

    CastNode skeletonNode(CastId::Skeleton);
    for (const auto &item: md6Skl.bones()) {
        CastNode boneNode(CastId::Bone);
        boneNode.addProperty(std::make_unique<StringCastProperty>("n", item.m_name));
        boneNode.addProperty(std::make_unique<Uint32CastProperty>("p", item.m_parentId));
        boneNode.addProperty(std::make_unique<Vector4CastProperty>("lr",item.m_quat));
        boneNode.addProperty(std::make_unique<Vector3CastProperty>("lp",item.m_pos));
        boneNode.addProperty(std::make_unique<Vector3CastProperty>("s", item.m_scale));

        skeletonNode.addChildren(std::move(boneNode));
    }

    modelNode.addChildren(std::move(skeletonNode));
    for (int i = 0; i < md6Mesh.header().m_meshInfo.size(); i++) {
        const MD6MeshInfo &meshInfo = md6Mesh.header().m_meshInfo[i];
        const Mesh &mesh = md6Mesh.meshGeometry()[i];

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
            const Vertex &vertex = mesh.m_vertices[j];
            const Normal &normal = mesh.m_normals[j];
            const UV &uv = mesh.m_uv[j];
            const BoneIds &boneId = mesh.m_boneIds[j];

            vertices->array()[j] = {vertex.X, -vertex.Z, vertex.Y};
            normals->array()[j] = {-normal.Xn, normal.Yn, -normal.Zn};
            uvs->array()[j] = {uv.U, 1 - uv.V};
            boneIds->array()[j * 4 + 0] = findIndex(md6Mesh.header().m_boneIds, boneId.a);
            boneIds->array()[j * 4 + 1] = findIndex(md6Mesh.header().m_boneIds, boneId.b);
            boneIds->array()[j * 4 + 2] = findIndex(md6Mesh.header().m_boneIds, boneId.c);
            boneIds->array()[j * 4 + 3] = findIndex(md6Mesh.header().m_boneIds, boneId.d);
        }

        std::memcpy(faces->data(), mesh.m_faces.data(), mesh.m_faces.size() * sizeof(mesh.m_faces.front()));
        std::memcpy(boneWeights->data(), mesh.m_boneWeights.data(),
                    mesh.m_boneWeights.size() * sizeof(mesh.m_boneWeights.front()));

        meshNode.addProperty(std::move(vertices));
        meshNode.addProperty(std::move(normals));
        meshNode.addProperty(std::move(boneIds));
        meshNode.addProperty(std::move(boneWeights));
        meshNode.addProperty(std::move(uvs));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("ul", 1));
        meshNode.addProperty(std::make_unique<Uint32CastProperty>("mi", 4));
        meshNode.addProperty(std::move(faces));

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

