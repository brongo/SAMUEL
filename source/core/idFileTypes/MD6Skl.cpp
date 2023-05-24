#include "MD6Skl.h"
#include "../ResourceManager.h"

void HAYDEN::MD6SklHeader::fromData(const std::vector<uint8_t> &binaryData) {
    std::memcpy(this, binaryData.data(), sizeof(*this));
}

HAYDEN::MD6Skl::MD6Skl(const ResourceManager &resourceManager, const std::string &resourcePath) {
    m_loaded = false;
    auto headerData = resourceManager.queryFileByName(resourcePath);
    if (!headerData.has_value()) {
        return;
    }
    if (headerData->size() < sizeof(MD6SklHeader)) {
        return;
    }
    m_header.fromData(headerData.value());
    m_bones.resize(m_header.boneCount);

    uint8_t *data = headerData->data();
    uint64_t offset = m_header.offsets[2]+4;
    Cast::Vector4 *quats = (Cast::Vector4 *) (data + offset);
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        m_bones[boneId].m_quat = quats[boneId];
    }

    offset += sizeof(Cast::Vector4) * m_header.alignedBoneCount();
    Cast::Vector3 *scl = (Cast::Vector3 *) (data + offset);
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        m_bones[boneId].m_scale = scl[boneId];
    }
    offset += sizeof(Cast::Vector3) * m_header.alignedBoneCount();

    Cast::Vector3 *pos = (Cast::Vector3 *) (data + offset);
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        m_bones[boneId].m_pos = pos[boneId];
    }


    offset = m_header.offsets[4] + 4;
    std::vector<int16_t> parents(m_header.boneCount);
    std::vector<int16_t> remapTable(m_header.boneCount);

    int16_t *parentData = (int16_t *) (data + offset);
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        m_bones[boneId].m_parentId = parentData[boneId];
    }
    offset += sizeof(int16_t) * m_header.alignedBoneCount();

    int16_t *remapTableData = (int16_t *) (data + offset);
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        remapTable[boneId] = remapTableData[boneId];
    }

    offset = m_header.boneNamesOffset + 4;
    for (int boneId = 0; boneId < m_header.boneCount; ++boneId) {
        uint32_t strLen = *(uint32_t *) (data + offset);
        offset += 4;
        m_bones[boneId].m_name = std::string((char *) (data + offset), strLen);
        offset += strLen;
    }
    m_loaded = true;
}
