#include "ExportBIM.h"

#include "source/core/exportTypes/PNG.h"

namespace HAYDEN {
    bool BIMExportTask::exportBIMImage(const fs::path &exportPath, bool reconstructZ) {
        BIM bimTexture(m_resourceManager, m_resourcePath);
        if (!bimTexture.loaded())
            return false;


        ImageType bimType = static_cast<ImageType>(bimTexture.header().TextureFormat);
        BIMMipmap mip = bimTexture.mipMaps()[0];
        uint32_t width = mip.MipPixelWidth;
        uint32_t height = mip.MipPixelHeight;
        fs::path outputFile = exportPath / fs::path(m_fileName).replace_extension(".png");
        if (!fs::exists(outputFile.parent_path())) {
            if (!mkpath(outputFile.parent_path())) {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n",
                        outputFile.parent_path().string().c_str());
                return false;
            }
        }

        DDSHeaderBuilder ddsBuilder((int) width, (int) height, mip.DecompressedSize, bimType);
        std::vector<uint8_t> ddsFile = ddsBuilder.ConvertToByteVector();

        // Merge header and data into one byte vector
        ddsFile.insert(ddsFile.end(), bimTexture.data().begin(), bimTexture.data().end());

        // Convert DDS file to PNG format
        PNGFile pngFile;
        std::vector<uint8_t> pngData = pngFile.ConvertDDStoPNG(ddsFile, reconstructZ);

        if (pngData.empty()) {
            fprintf(stderr, "ERROR: Failed to read from given file. \n");
            return false;
        }

        // Write file to local filesystem
        return writeToFilesystem(pngData, outputFile);
    }

    BIMExportTask::BIMExportTask(const ResourceManager &resourceManager, const std::string &resourceName)
            : m_resourceManager(resourceManager) {
        m_resourcePath = resourceName;
        m_fileName = getFilename(resourceName);
    }
}
