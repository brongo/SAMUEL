#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include "idFileTypes/MD6.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "Oodle.h"
#include "ResourceFileReader.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    class MD6ExportTask
    {
        public:

            bool Export(const fs::path exportPath, const std::string resourcePath, const std::vector<StreamDBFile>& streamDBFiles);
            MD6ExportTask(const ResourceEntry resourceEntry);

        private:

            // Name of the file we are exporting, as it appears in a *.resources file
            std::string _FileName;

            // For locating the MD6 header, which is embedded in a *.resources file
            uint64_t _ResourceID = 0;
            uint64_t _ResourceDataOffset = 0;
            uint64_t _ResourceDataLength = 0;
            uint64_t _ResourceDataLengthDecompressed = 0;

            // For locating the MD6 model (+LODs), which is embedded in a *.streamdb file
            std::string _StreamDBFilePath;
            uint64_t _StreamedDataHash = 0;
            uint64_t _StreamedDataLength = 0;
            uint64_t _StreamedDataLengthDecompressed = 0;
            int32_t _StreamCompressionType = 0;
            int32_t _StreamDBNumber = -1;                            // index into std::vector<StreamDBFile>& streamDBFiles

            // Matching StreamDBEntry for the MD6 model
            StreamDBEntry _StreamDBEntry;

            // Serialized MD6 header extracted from *.resources file
            MD6 _MD6;
    };
}