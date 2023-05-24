#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "exportTypes/DDSHeader.h"
#include "exportTypes/PNG.h"

#include "idFileTypes/BIM.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "ExportManager.h"
#include "Oodle.h"
#include "ResourceFileReader.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN {
    class BIMExportTask {
    public:

        // Getters
        BIM GetBIMData() { return _BIM; }

        std::vector<uint8_t> GetBIMRawImage() { return _BIM.RawImageData; }

        // Helper function for locating streamed file data in *.streamdb
        bool LocateFileInStreamDB(const std::vector<StreamDBFile> &streamDBFiles);

        // Main Export function
        bool Export(const fs::path exportPath, const std::string resourcePath, bool reconstructZ = false);

        // Constructor
        BIMExportTask(const ResourceEntry resourceEntry);

    private:

        // m_name of the file we are exporting, as it appears in a *.resources file
        std::string _FileName;

        // For locating the BIM header, which is embedded in a *.resources file
        uint64_t _ResourceID = 0;
        uint64_t _ResourceDataOffset = 0;
        uint64_t _ResourceDataLength = 0;
        uint64_t _ResourceDataLengthDecompressed = 0;

        // For locating the BIM images (+mips), which is embedded in a *.streamdb file
        fs::path _StreamDBFilePath;
        uint64_t _StreamedDataHash = 0;
        uint64_t _StreamedDataLength = 0;
        uint64_t _StreamedDataLengthDecompressed = 0;
        int32_t _StreamCompressionType = 0;
        int32_t _StreamDBNumber = -1;                            // index into std::vector<StreamDBFile>& streamDBFiles
        bool _IsStreamed = 1;

        // Matching StreamDBEntry for the BIM data
        const StreamDBEntry *_StreamDBEntry;

        // For converting the image from BIM format to DDS/PNG
        int32_t _ImgType = 0;
        int32_t _ImgMipCount = 0;
        int32_t _ImgPixelWidth = 0;
        int32_t _ImgPixelHeight = 0;

        // Serialized BIM header extracted *.resources file
        BIM _BIM;
    };
}