#pragma once

#include "../cast/cast.h"
#include "../Utilities.h"
#include "../ResourceManager.h"

namespace HAYDEN {

    struct MD6SklHeader {
        uint16_t unkOffset;
        uint16_t unkCount;
        uint16_t boneNamesOffset;
        uint16_t boneCount;
        uint16_t offsets[24];

        uint32_t alignedBoneCount() { return (boneCount + 7) & 0x7ffffff8; }

        void fromData(const std::vector<uint8_t> &binaryData);
    };

    struct MD6SklBone {
        std::string m_name;
        int32_t m_parentId;
        Cast::Vector4 m_quat;
        Cast::Vector3 m_pos;
        Cast::Vector3 m_scale;
    };

    class MD6Skl {
    public:
        MD6Skl(const ResourceManager &resourceManager, const std::string &resourcePath);

        [[nodiscard]] bool loaded() const { return m_loaded; }

        [[nodiscard]] const MD6SklHeader &header() const { return m_header; }

        [[nodiscard]] const std::vector<MD6SklBone>& bones() const { return m_bones; };

    private:
        bool m_loaded;
        MD6SklHeader m_header;
        std::vector<MD6SklBone> m_bones;
    };
}