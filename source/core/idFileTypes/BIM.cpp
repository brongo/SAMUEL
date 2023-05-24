#include "BIM.h"

namespace HAYDEN
{
    void BIM::Serialize(const std::vector<uint8_t> binaryData)
    {
        Header = *(BIM_HEADER*)(binaryData.data());

        for (int i = 0; i < Header.MipCount; i++)
            MipMaps.push_back(*(BIMMipmap*)(binaryData.data() + sizeof(BIM_HEADER) + (i * sizeof(BIMMipmap))));

        // special handling for small images: these aren't in .streamdb, even if isStreamed = 1.
        if (MipMaps[0].MipPixelHeight <= 32 && MipMaps[0].MipPixelWidth <= 32)
            Header.BoolIsStreamed = 0;

        // special handling for fonts: overrides streamDBMipCount so we can calculate streamDBIndex correctly.
        if (Header.TextureMaterialKind == 19)
            Header.StreamDBMipCount = 11;

        // special handling for non-streaming images: this BIM header contains the whole file
        if (Header.BoolIsStreamed == 0 && MipMaps[0].BoolIsCompressed == 0)
        {
            // calculate header size
            size_t rawDataStart = sizeof(BIM_HEADER) + (Header.MipCount * sizeof(BIMMipmap));
            size_t rawDataSize = binaryData.size() - rawDataStart;

            // read the raw image data into uint8_t vector
            RawImageData.insert(RawImageData.begin(), binaryData.data() + rawDataStart, binaryData.data() + binaryData.size());
        }

        // special handling for $minmip= images: streamDBMipCount is encoded.
        if (Header.StreamDBMipCount > 8)
        {
            // divide by 16 to get largest mip used in game (original image isn't used)
            int largestMipUsed = Header.StreamDBMipCount / 16;
            size_t mipDataStart = sizeof(BIM_HEADER) + (sizeof(BIMMipmap) * largestMipUsed);

            // replace original size & dimensions with data for the largest mip used
            Header.PixelHeight = *(int*)(binaryData.data() + mipDataStart + 8);
            Header.PixelWidth = *(int*)(binaryData.data() + mipDataStart + 12);
            MipMaps[0].DecompressedSize = *(int*)(binaryData.data() + mipDataStart + 20);
            MipMaps[0].CompressedSize = *(int*)(binaryData.data() + mipDataStart + 28);

            // decode streamDBMipCount used for calculating streamDBIndex.
            Header.StreamDBMipCount = Header.StreamDBMipCount % 16 - largestMipUsed;
        }

        return;
    }
}