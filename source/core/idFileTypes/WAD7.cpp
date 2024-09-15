#include "WAD7.h"
#include <memory>
#include <cstring>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

namespace HAYDEN
{
    // Reads binary .WAD7 file from local filesystem
    WAD7::WAD7(const fs::path filePath)
    {
        // Map file to memory
        size_t size = size = fs::file_size(filePath);

#ifdef _WIN32
        HANDLE fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if ((GetLastError() != ERROR_SUCCESS && GetLastError() != 183) || fileHandle == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        // Map the file to memory
        HANDLE fileMapping = CreateFileMappingW(fileHandle, nullptr, PAGE_READONLY, *((DWORD*)&size + 1), *(DWORD*)&size, nullptr);

        if (GetLastError() != ERROR_SUCCESS || !fileMapping) {
            CloseHandle(fileHandle);
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        // Get file's memory view
        unsigned char* memp = (unsigned char*)MapViewOfFile(fileMapping, FILE_MAP_READ, 0, 0, 0);

        if (GetLastError() != ERROR_SUCCESS || !memp) {
            CloseHandle(fileHandle);
            CloseHandle(fileMapping);
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }
#else
        int fileDescriptor = open(filePath.c_str(), O_RDONLY);

        if (fileDescriptor == -1) {
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        // Map the file to memory
        unsigned char* memp = (unsigned char*)(mmap(0, size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0));

        if (!memp) {
            close(fileDescriptor);
            fprintf(stderr, "ERROR : WAD7 : Failed to open %s for reading.\n", FilePath.c_str());
            return;
        }

        madvise(memp, size, MADV_WILLNEED);
#endif

        // Read .WAD7 file header
        memcpy(&Header, memp, sizeof(WAD7_HEADER));

        endianSwap64(Header.IndexStart);
        endianSwap64(Header.IndexSize);

        // Read entry count
        memcpy(&EntryCount, memp + Header.IndexStart, sizeof(uint32_t));
        endianSwap32(EntryCount);

        FileEntries.resize(EntryCount);
        EntryNames.resize(EntryCount);
        EntryTypes.resize(EntryCount);
        EntryVersions.resize(EntryCount);
        EmbeddedTypes.resize(EntryCount);

        size_t position = Header.IndexStart + sizeof(uint32_t);

        // Read .WAD7 file entries
        for (uint64_t i = 0; i < FileEntries.size(); i++)
        {
            uint32_t stringLength = 0;
            memcpy(&stringLength, memp + position, sizeof(uint32_t));
            position += sizeof(uint32_t);

            EntryNames[i] = std::string((char*)(memp + position), stringLength);
            position +=  stringLength;

            memcpy(&FileEntries[i], memp + position, sizeof(WAD7_ENTRY));
            position += sizeof(WAD7_ENTRY);

            endianSwap64(FileEntries[i].DataStartOffset);
            endianSwap32(FileEntries[i].UncompressedSize);
            endianSwap32(FileEntries[i].CompressedSize);
            endianSwap32(FileEntries[i].CompressionMode);
        }

        // Read first 4 bytes of each entry to identify entry type (IDCL or plaintext)
        for (uint64_t i = 0; i < FileEntries.size(); i++)
        {
            uint32_t buffer;
            memcpy(&buffer, memp + FileEntries[i].DataStartOffset, sizeof(uint32_t));

            // For IDCL files, we want to identify the contents to display in GUI
            if (buffer == 1279476809)
            {
                EntryTypes[i] = EntryType::TYPE_IDCL;

                // Read IDCL header
                ResourceFileHeader idclHeader;
                memcpy(&idclHeader, memp + FileEntries[i].DataStartOffset, sizeof(ResourceFileHeader));

                // Some WAD7 entries are empty, reason unknown
                if (idclHeader.NumFileEntries != 0)
                {
                    // Get version - for this type of IDCL only the first entry matters
                    ResourceFileEntry idclEntry;
                    memcpy(&idclEntry, memp + FileEntries[i].DataStartOffset + idclHeader.AddrEntries, sizeof(ResourceFileEntry));
                    EntryVersions[i] = idclEntry.Version;

                    // Get type string
                    uint64_t numStrings;
                    uint64_t addrStringCount = idclHeader.AddrEntries + (sizeof(ResourceFileEntry) * idclHeader.NumFileEntries);
                    memcpy(&numStrings, memp + FileEntries[i].DataStartOffset + addrStringCount, sizeof(uint64_t));

                    position = FileEntries[i].DataStartOffset + addrStringCount + sizeof(uint64_t);

                    std::vector<uint64_t> stringOffsets;
                    stringOffsets.resize(numStrings + 1);
                    for (uint64_t j = 0; j < numStrings; j++) {
                        memcpy(&stringOffsets[j], memp + position, sizeof(uint64_t));
                        position += sizeof(uint64_t);
                    }

                    stringOffsets[numStrings] = idclHeader.AddrDependencyEntries;

                    // Read strings into vector
                    std::vector<std::string> stringEntries;
                    stringEntries.resize(numStrings);
                    for (uint64_t i = 0; i < numStrings; i++)
                    {
                        int stringLength = stringOffsets[i + 1] - stringOffsets[i];
                        stringEntries[i] = std::string((char*)(memp + position), stringLength);
                        position += stringLength;
                    }
                    EmbeddedTypes[i] = stringEntries[idclEntry.PathTuple_OffsetType];
                }
                else
                {
                    EntryVersions[i] = 9999;
                    EmbeddedTypes[i] = "Empty File - DO NOT EXPORT";
                }

            }
            else
            {
                EntryTypes[i] = EntryType::TYPE_PLAINTEXT;
                EntryVersions[i] = 0;
            }
        }

#ifdef _WIN32
        UnmapViewOfFile(memp);
        CloseHandle(fileMapping);
        CloseHandle(fileHandle);
#else
        munmap(memp, size);
        close(fileDescriptor);
#endif
    }
}