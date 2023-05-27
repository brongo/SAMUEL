#include "BIM.h"
#include "source/core/Oodle.h"

namespace HAYDEN {
//    void BIM::Serialize(const std::vector<uint8_t> binaryData) {
//        m_header = *(BIMHeader *) (binaryData.data());
//
//        for (int i = 0; i < m_header.MipCount; i++)
//            m_mipMaps.push_back(*(BIMMipmap *) (binaryData.data() + sizeof(BIMHeader) + (i * sizeof(BIMMipmap))));
//
//        // special handling for small images: these aren't in .streamdb, even if isStreamed = 1.
//        if (m_mipMaps[0].MipPixelHeight <= 32 && m_mipMaps[0].MipPixelWidth <= 32)
//            m_header.IsStreamed = 0;
//
//        // special handling for fonts: overrides streamDBMipCount so we can calculate streamDBIndex correctly.
//        if (m_header.TextureMaterialKind == 19)
//            m_header.StreamDBMipCount = 11;
//
//        // special handling for non-streaming images: this BIM header contains the whole file
//        if (m_header.IsStreamed == 0 && m_mipMaps[0].BoolIsCompressed == 0) {
//            // calculate header size
//            size_t rawDataStart = sizeof(BIMHeader) + (m_header.MipCount * sizeof(BIMMipmap));
//
//            // read the raw image data into uint8_t vector
//            m_rawImageData.insert(m_rawImageData.begin(), binaryData.data() + rawDataStart,
//                                  binaryData.data() + binaryData.size());
//        }
//
//        // special handling for $minmip= images: streamDBMipCount is encoded.
//        if (m_header.StreamDBMipCount > 8) {
//            // divide by 16 to get largest mip used in game (original image isn't used)
//            int largestMipUsed = m_header.StreamDBMipCount / 16;
//            size_t mipDataStart = sizeof(BIMHeader) + (sizeof(BIMMipmap) * largestMipUsed);
//
//            // replace original size & dimensions with data for the largest mip used
//            m_header.PixelHeight = *(int *) (binaryData.data() + mipDataStart + 8);
//            m_header.PixelWidth = *(int *) (binaryData.data() + mipDataStart + 12);
//            m_mipMaps[0].DecompressedSize = *(int *) (binaryData.data() + mipDataStart + 20);
//            m_mipMaps[0].CompressedSize = *(int *) (binaryData.data() + mipDataStart + 28);
//
//            // decode streamDBMipCount used for calculating streamDBIndex.
//            m_header.StreamDBMipCount = m_header.StreamDBMipCount % 16 - largestMipUsed;
//        }
//
//    }

    BIM::BIM(const ResourceManager &resourceManager, const std::string &resourcePath) {
        m_loaded = false;
        auto optHeaderData = resourceManager.queryFileByName(resourcePath);
        if (!optHeaderData.has_value()) {
            return;
        }
        if (optHeaderData->size() < sizeof(BIMHeader)) {
            return;
        }

        std::vector<uint8_t> headerData = optHeaderData.value();

        m_header = *(BIMHeader *) (headerData.data());
        uint64_t offset = sizeof(BIMHeader);

        for (int i = 0; i < m_header.MipCount; i++) {
            m_mipMaps.push_back(*(BIMMipmap *) (headerData.data() + offset));
            offset += sizeof(BIMMipmap);
        }

        // special handling for small images: these aren't in .streamdb, even if isStreamed = 1.
        if (m_mipMaps[0].MipPixelHeight <= 32 && m_mipMaps[0].MipPixelWidth <= 32)
            m_header.IsStreamed = 0;

        // special handling for fonts: overrides streamDBMipCount, so we can calculate streamDBIndex correctly.
        if (m_header.TextureMaterialKind == 19)
            m_header.StreamDBMipCount = 11;

        // special handling for $minmip= images: streamDBMipCount is encoded.
        if (m_header.StreamDBMipCount > 8) {
            // divide by 16 to get largest mip used in game (original image isn't used)
            int largestMipUsed = m_header.StreamDBMipCount / 16;
            size_t mipDataStart = sizeof(BIMHeader) + (sizeof(BIMMipmap) * largestMipUsed);

            // replace original size & dimensions with data for the largest mip used
            m_header.PixelHeight = *(int *) (headerData.data() + mipDataStart + 8);
            m_header.PixelWidth = *(int *) (headerData.data() + mipDataStart + 12);
            m_mipMaps[0].DecompressedSize = *(int *) (headerData.data() + mipDataStart + 20);
            m_mipMaps[0].CompressedSize = *(int *) (headerData.data() + mipDataStart + 28);

            // decode streamDBMipCount used for calculating streamDBIndex.
            m_header.StreamDBMipCount = m_header.StreamDBMipCount % 16 - largestMipUsed;
        }
        if (m_header.IsStreamed == 0 && m_mipMaps[0].BoolIsCompressed == 0) {
            m_data.insert(m_data.begin(), headerData.begin() + offset, headerData.end());
        } else {
            auto optStreamData = resourceManager.queryStreamDataByName(resourcePath, m_mipMaps[0].CompressedSize,
                                                                       m_header.StreamDBMipCount);
            if (optStreamData.has_value()) {
                m_data.insert(m_data.begin(), optStreamData->begin(),
                              optStreamData->begin() + m_mipMaps[0].CompressedSize);
                oodleDecompressInplace(m_data, m_mipMaps[0].DecompressedSize);
            }
        }

        m_loaded = true;
    }
}