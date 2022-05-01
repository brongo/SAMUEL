#include "ExportMD6.h"

namespace HAYDEN
{
    // Constructor
    MD6ExportTask::MD6ExportTask(const ResourceEntry resourceEntry)
    {
        _FileName = resourceEntry.Name;
        _ResourceDataOffset = resourceEntry.DataOffset;
        _ResourceDataLength = resourceEntry.DataSize;
        _ResourceDataLengthDecompressed = resourceEntry.DataSizeUncompressed;
        _ResourceID = resourceEntry.StreamResourceHash;
        return;
    }

    // Main export function for MD6 files.
    // Return 1 for success, 0 for failure.
    bool MD6ExportTask::Export(const fs::path exportPath, const std::string resourcePath, const std::vector<StreamDBFile>& streamDBFiles)
    {
        std::vector<uint8_t> modelData;
        ResourceFileReader resourceFile(resourcePath);

        // Extract MD6 header from .resources file and read it
        std::vector<uint8_t> binaryData = resourceFile.GetEmbeddedFileHeader(resourcePath, _ResourceDataOffset, _ResourceDataLength, _ResourceDataLengthDecompressed);

        if (binaryData.empty())
            return 0;

        // Serialize binary data from our extracted MD6 header
        _MD6.Serialize(binaryData);
        _StreamedDataLengthDecompressed = _MD6.StreamDBData[0].DecompressedSize;
        _StreamedDataLength = _MD6.StreamDBData[0].CompressedSize;

        // Convert resourceID to streamFileID
        _StreamedDataHash = resourceFile.CalculateStreamDBIndex(_ResourceID);

        // Locate and extract the .MD6 geometry from .streamdb file
        for (int i = 0; i < streamDBFiles.size(); i++)
        {
            _StreamDBEntry = streamDBFiles[i].LocateStreamDBEntry(_StreamedDataHash, _StreamedDataLength);

            if (_StreamDBEntry.Offset16 > 0)
            {
                // Match found in this file
                _StreamDBNumber = i;
                _StreamDBFilePath = streamDBFiles[i].FilePath;
                break;
            }
        }

        // Unable to locate geometry in .streamdb. Abort.
        if (_StreamDBEntry.Offset16 <= 0)
            return 0;

        // Extract .MD6 geometry from .streamdb file.
        modelData = streamDBFiles[_StreamDBNumber].GetEmbeddedFile(_StreamDBFilePath, _StreamDBEntry);

        // Decompress the streamed model geometry if needed (almost always).
        if (_StreamDBEntry.CompressedSize != _StreamedDataLengthDecompressed)
        {
            modelData = oodleDecompress(modelData, _StreamedDataLengthDecompressed);
            if (modelData.empty())
            {
                fprintf(stderr, "Error: Failed to decompress: %s \n", _FileName.c_str());
                return 0;
            }
        }

        // Merge raw md6 header and data into one byte vector
        modelData.insert(modelData.begin(), _MD6.RawMD6Header.begin(), _MD6.RawMD6Header.end());

        // Write file to local filesystem
        return writeToFilesystem(modelData, exportPath);
    }
}