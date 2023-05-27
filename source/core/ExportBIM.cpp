#include "ExportBIM.h"

#include "source/core/exportTypes/PNG.h"

namespace HAYDEN {
    // Constructor
//    BIMExportTask::BIMExportTask(const ResourceEntry resourceEntry) {
//        _FileName = resourceEntry.Name;
//        _ResourceDataOffset = resourceEntry.DataOffset;
//        _ResourceDataLength = resourceEntry.DataSize;
//        _ResourceDataLengthDecompressed = resourceEntry.DataSizeUncompressed;
//        _ResourceID = resourceEntry.StreamResourceHash;
//        return;
//    }
//
//    // Locate and extract embedded image data from .streamdb file
//    bool BIMExportTask::LocateFileInStreamDB(const std::vector<StreamDBFile> &streamDBFiles) {
//        for (int i = 0; i < streamDBFiles.size(); i++) {
//            const StreamDBEntry *entry = streamDBFiles[i].locateStreamDBEntry(_StreamedDataHash, _StreamedDataLength);
//            if (entry != nullptr) {
//                _StreamDBEntry = entry;
//                _StreamDBNumber = i;
//                _StreamDBFilePath = streamDBFiles[i].filePath();
//                return true;
//            }
//        }
//        return false;
//    }

    // Main export function for BIM files.
    // Convert BIM file to PNG format and write to local filesystem.
    // Return 1 for success, 0 for failure.
//    bool BIMExportTask::Export(const fs::path exportPath, const std::string resourcePath, bool reconstructZ) {
//        std::vector<uint8_t> rawImageData;
//        ResourceFileReader resourceFile(resourcePath);
//
//        // Extract BIM header from .resources file and read it
//        std::vector<uint8_t> binaryData = resourceFile.GetEmbeddedFileHeader(resourcePath, _ResourceDataOffset,
//                                                                             _ResourceDataLength,
//                                                                             _ResourceDataLengthDecompressed);
//
//        if (binaryData.empty())
//            return 0;
//
//        // Serialize binary data from our extracted BIM header
//        _BIM.Serialize(binaryData);
//        _ImgType = _BIM.Header.TextureFormat;
//        _ImgPixelWidth = _BIM.Header.PixelWidth;
//        _ImgPixelHeight = _BIM.Header.PixelHeight;
//        _ImgMipCount = _BIM.Header.StreamDBMipCount;
//
//        // Get StreamDB Index and Size from serialized BIM data
//        _StreamedDataLengthDecompressed = _BIM.MipMaps[0].DecompressedSize;
//        _StreamedDataLength = _BIM.MipMaps[0].CompressedSize;
//
//        // Sometimes a file is in streamdb even if IsStreamed == 0, so we have to check if the largest mip is compressed.
//        if (_BIM.Header.IsStreamed == 0 && _BIM.MipMaps[0].BoolIsCompressed == 0)
//            _IsStreamed = 0;
//
//        // Convert resourceID to streamFileID
//        _StreamedDataHash = resourceFile.CalculateStreamDBIndex(_ResourceID, _ImgMipCount);
//
//        // Locate and extract the BIM image from .streamdb file
//        if (_IsStreamed) {
//            // If no match is found, reduce _StreamedDataHash by 1 and search again.
//            // This is necessary for locating some BIM files (reason for this is unknown)
//            if (!LocateFileInStreamDB(streamDBFiles)) {
//                _StreamedDataHash--;
//                if (!LocateFileInStreamDB(streamDBFiles))
//                    return 0; // abort
//            }
//
//            // Extract streaming image data from .streamdb file,.
//            rawImageData = streamDBFiles[_StreamDBNumber].GetEmbeddedFile(_StreamDBFilePath, _StreamDBEntry);
//
//            // Decompress the streamed image data if needed (almost always).
//            if (_StreamDBEntry->m_compressedSize != _StreamedDataLengthDecompressed) {
//                rawImageData = oodleDecompress(rawImageData, _StreamedDataLengthDecompressed);
//                if (rawImageData.empty()) {
//                    fprintf(stderr, "Error: Failed to decompress: %s \n", _FileName.c_str());
//                    return 0;
//                }
//            }
//        } else {
//            // Non-streamed image, can grab image data directly from extracted .resources entry
//            rawImageData = GetBIMRawImage();
//        }
//
//        // Construct a DDS file header from serialized BIM data
//        DDSHeaderBuilder ddsBuilder(_ImgPixelWidth, _ImgPixelHeight, _StreamedDataLengthDecompressed,
//                                    static_cast<ImageType>(_ImgType));
//        std::vector<uint8_t> ddsFile = ddsBuilder.ConvertToByteVector();
//
//        // Merge header and data into one byte vector
//        ddsFile.insert(ddsFile.end(), rawImageData.begin(), rawImageData.end());
//
//        // Convert DDS file to PNG format
//        PNGFile pngFile;
//        std::vector<uint8_t> pngData = pngFile.ConvertDDStoPNG(ddsFile, reconstructZ);
//
//        if (pngData.empty()) {
//            fprintf(stderr, "ERROR: Failed to read from given file. \n");
//            return 0;
//        }
//
//        // Write file to local filesystem
//        return writeToFilesystem(pngData, exportPath);
//    }

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

        DDSHeaderBuilder ddsBuilder(width, height, mip.DecompressedSize, bimType);
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
