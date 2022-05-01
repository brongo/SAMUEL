#include "StreamDBFile.h"

namespace HAYDEN
{
    // Attempts to locate a StreamDBEntry based on a known streamedFileID + streamedDataLength.
    StreamDBEntry StreamDBFile::LocateStreamDBEntry(const uint64_t streamedFileID, const uint64_t streamedDataLength) const
    {
        for (uint64_t i = 0; i < _StreamDBHeader.NumEntries; i++)
        {
            // Match
            if ((streamedFileID == _StreamDBEntries[i].FileID) && (streamedDataLength == _StreamDBEntries[i].CompressedSize))
                return _StreamDBEntries[i];

            // FileID matches, but compressed size doesn't. 
            if ((streamedFileID == _StreamDBEntries[i].FileID) && (streamedDataLength < _StreamDBEntries[i].CompressedSize))
            {
                // Sometimes it will match the next entry in sequence, so check this.
                if (streamedDataLength == _StreamDBEntries[i + 1].CompressedSize)
                    return _StreamDBEntries[i + 1];
            }
        }

        // No match found, return empty StreamDBEntry
        return StreamDBEntry();
    }

    // Extracts an embedded file in StreamDB, returning it as a byte vector
    std::vector<uint8_t> StreamDBFile::GetEmbeddedFile(const std::string streamDBFileName, const StreamDBEntry streamDBEntry) const
    {
        // Multiply streamDBEntry.Offset16 by 16 to get the real file offset
        uint64_t fileOffset = streamDBEntry.Offset16;
        fileOffset = fileOffset * 16;

        // Read from stream
        std::ifstream f;
        std::vector<uint8_t> fileData(streamDBEntry.CompressedSize);
        f.open(streamDBFileName, std::ios::in | std::ios::binary);
        f.seekg(fileOffset, std::ios_base::beg);
        f.read(reinterpret_cast<char*>(fileData.data()), streamDBEntry.CompressedSize);

        // Validate number of bytes read, 0 if failed
        fileData.resize(f.gcount());
        f.close();

        return fileData;
    }

    // Reads a binary StreamDB file from local filesystem
    StreamDBFile::StreamDBFile(const fs::path& filePath)
    {
        FilePath = filePath.string();

        FILE* f = fopen(FilePath.c_str(), "rb");
        if (f == NULL)
        {
            fprintf(stderr, "ERROR : StreamDBFile : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        if (f != NULL)
        {
            // Read .streamdb file header
            fseek(f, 0, SEEK_SET);
            fread(&_StreamDBHeader, sizeof(StreamDBHeader), 1, f);
            _StreamDBEntries.resize(_StreamDBHeader.NumEntries);

            // Read .streamdb file entries
            for (int i = 0; i < _StreamDBHeader.NumEntries; i++)
                fread(&_StreamDBEntries[i], sizeof(StreamDBEntry), 1, f);

            fclose(f);
        }
    }
}