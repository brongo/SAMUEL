#include "ExportCOMP.h"

namespace HAYDEN
{
    COMPExportTask::COMPExportTask(const ResourceEntry resourceEntry)
    {
        _FileName = resourceEntry.Name;
        _ResourceDataOffset = resourceEntry.DataOffset;
        _ResourceDataLength = resourceEntry.DataSize;
        _ResourceDataLengthDecompressed = resourceEntry.DataSizeUncompressed;
        return;
    }

    bool COMPExportTask::Export(const fs::path exportPath, const std::string resourcePath)
    {
        ResourceFileReader resourceFile(resourcePath);
        std::vector<uint8_t> compFile = resourceFile.GetEmbeddedFileHeader(resourcePath, _ResourceDataOffset, _ResourceDataLength, _ResourceDataLengthDecompressed);

        if (compFile.empty())
            return 0;

        // Read compressed and decompressed size from comp file header
        int decompressedSize = *(int*)(compFile.data() + 0);
        int compressedSize = *(int*)(compFile.data() + 8);

        // Remove header and decompress remaining data
        compFile.erase(compFile.begin(), compFile.begin() + 16);
        compFile = oodleDecompress(compFile, decompressedSize);

        if (compFile.empty())
        {
            fprintf(stderr, "Error: Failed to decompress: %s \n", _FileName.c_str());
            return 0;
        }

        return writeToFilesystem(compFile, exportPath);
    }
}