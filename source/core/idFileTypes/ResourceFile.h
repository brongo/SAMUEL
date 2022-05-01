#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "ResourceFile.h"

#pragma pack(push)  // Not portable, sorry.
#pragma pack(1)     // Works on my machine (TM).

namespace fs = std::filesystem;

namespace HAYDEN
{
    /**
    *   Notes on .resources file format:
    *
    *   The .resources file is an archive/container format; multiple other files are embedded within.
    *     "Gameresources.resources" and "Warehouse.resources" are global - they are loaded at all times.
    *     Level-specific .resources files (e.g. "e1m1_intro.resources") are only loaded in that level.
    *
    *   The basic format of a .resources file is:
    *     (a) an index / table of contents, followed by
    *     (b) a series of embedded files (referenced via index section)
    */

    // Built-in struct in .resources binary file
    struct ResourceFileHeader // 0x7C bytes
    {
        /* 0x00 */ uint32_t Magic = 0;
        /* 0x04 */ uint32_t Version = 0;

        /* 0x08 */ uint32_t Unk08 = 0;
        /* 0x0C */ uint32_t Unk0C = 0;
        /* 0x10 */ uint64_t Unk10 = 0;
        /* 0x18 */ uint64_t Unk18 = 0;

        /* 0x20 */ uint32_t NumFileEntries = 0;
        /* 0x24 */ uint32_t NumDependencyEntries = 0;   // resource dependencies list
        /* 0x28 */ uint32_t NumDependencyIndexes = 0;   // index for resource dependencies
        /* 0x2C */ uint32_t NumPathStringIndexes = 0;   // index for file name/type strings

        /* 0x30 */ uint32_t Unk30 = 0;
        /* 0x34 */ uint32_t NumErrorLogs = 0;           // usually 0
        /* 0x38 */ uint32_t SizeStrings = 0;            // size of entire strings section, in bytes
        /* 0x3C */ uint32_t SizeErrorLog = 0;           // usually 0

        /* 0x40 */ uint64_t AddrPathStringOffsets = 0;
        /* 0x48 */ uint64_t AddrErrorLogs = 0;          // starting offset for error logs (usually none)
        /* 0x50 */ uint64_t AddrEntries = 0;            // starting offset for ResourceFileEntry structs
        /* 0x58 */ uint64_t AddrDependencyEntries = 0;  // starting offset for dependency entries

        /* 0x60 */ uint64_t AddrDependencyIndexes = 0;  // starting offset for dependency indexes
        /* 0x68 */ uint64_t AddrData = 0;               // starting offset for embedded file data
        /* 0x70 */ uint32_t AddrUnk70 = 0;
        /* 0x74 */ uint64_t AddrEndMarker = 0;          // "IDCL" marks end of resource file metadata, start of embedded files
    };

    // Built-in struct in .resources binary file
    struct ResourceFileEntry // 0x90 bytes
    {
        /* 0x00 */ uint64_t PathTuple_OffsetType = 0;   // add this to PathTuple_Index to get idx of the file type - always 0
        /* 0x08 */ uint64_t PathTuple_OffsetName = 0;   // add this to PathTuple_Index to get idx of the file name - always 1

        /* 0x10 */ uint64_t Unk10Dummy1 = 0;

        /* 0x18 */ uint64_t DependencyIndexNumber = 0;  // index for DependencyEntry
        /* 0x20 */ uint64_t PathTuple_Index = 0;        // divide by 2 to get the PathTuple array index (or multiply by 8 to get offset, as game does)

        /* 0x28 */ uint64_t Unk28Zero0 = 0;
        /* 0x30 */ uint64_t Unk30Zero1 = 0;

        /* 0x38 */ uint64_t DataOffset = 0;             // offset of embedded file, in bytes
        /* 0x40 */ uint64_t DataSize = 0;               // size of embedded file, in bytes
        /* 0x48 */ uint64_t DataSizeUncompressed = 0;   // size of embedded file when decompressed

        /* 0x50 */ uint64_t DataCheckSum = 0;           // murmurhash of decompressed data
        /* 0x58 */ uint64_t Timestamp = 0;              // divide by 1,000,000 for UNIX timestamp

        /* 0x60 */ uint64_t StreamResourceHash = 0;     // gets converted to .streamdb index, not shown here
                                                        // conversion is shown in FileExporter->FileExportList::CalculateStreamDBIndex 

        /* 0x68 */ uint32_t Version = 0;                // resource type enum,  0x43 = model,  0x21 = image,  0x00 = rs_streamfile
        /* 0x6C */ uint32_t HavokFlag1 = 0;             // 2 if HavokShape or RenderProg,  1 for certain SWF images (rare)

        /* 0x70 */ uint16_t CompressionMode = 0;        // 0 = uncompressed,  1 = zlib, 2 = kraken,  4 = kraken variant,  5 = leviathan (handled by sub_1408D8D00)
        /* 0x72 */ uint8_t  HavokFlag2 = 0;             // 0x41 for HavokShapes,  varies for RenderProg
        /* 0x73 */ uint8_t  HavokFlag3 = 0;             // 1 for RenderProg

        /* 0x74 */ uint32_t Unk74 = 0;
        /* 0x78 */ uint32_t Unk78Zero2 = 0;

        /* 0x7C */ uint32_t Flags = 0;                  // 8 = supports regen,  1 = required for regen?,  2 = ?? (always set?),  0x10000 = encrypted??
        /* 0x80 */ uint32_t DesiredCompressionMode = 0; // Desired compression mode,  only used while building?
        /* 0x84 */ uint16_t NumDependencies = 0;

        /* 0x86 */ uint16_t Unk86 = 0;                  // sub_1408E8DC0 reads this
        /* 0x88 */ uint64_t Unk88 = 0;
    };

    // Built-in struct in .resources binary file
    struct ResourceFileDependency // 0x20 bytes
    {
        /* 0x00 */ uint64_t AssetTypeStringIndex = 0;   // index into _stringEntries,  file "type" of the depedency
        /* 0x08 */ uint64_t FileNameStringIndex = 0;    // Index into _stringEntries,  file "name" of the dependency
        /* 0x10 */ uint32_t DependencyType = 0;         // 2 = RES_DEP_RESOURCE,  5 = RES_DEP_EMBEDDED_RESOURCE
        /* 0x14 */ uint32_t Unk14 = 0;                  // always 1 ??
        /* 0x18 */ uint32_t Unk18 = 0;                  // some kind of file ID ??
        /* 0x1C */ uint32_t Unk1C = 0;                  // file ID ??  but often 0x0000 for some file types, like .lwo
    };

    class ResourceFile
    {
        public:

            // External, passed into constructor
            std::string FilePath;

            // Getters
            uint32_t GetNumFileEntries() const { return _Header.NumFileEntries; }
            ResourceFileEntry& GetResourceFileEntry(int i) { return _FileEntries[i]; }
            std::string GetResourceStringEntry(int i) const { return _StringEntries[i]; }
            std::vector<uint64_t> GetAllPathStringIndexes() const { return _PathStringIndexes; }

            // Reads a binary .resources file from local filesystem
            ResourceFile(const fs::path& filePath);

        private:

            // Binary data within the .resources file
            ResourceFileHeader _Header;                             // first 0x7C bytes in file
            std::vector<ResourceFileEntry> _FileEntries;            // immediately after ResourceFileHeader,  repeating 0x90 byte sequence

            uint64_t _NumStrings = 0;                               // immediately after last ResourceFileEntry
            std::vector<uint64_t> _StringOffsets;                   // immediately after _numStrings
            std::vector<std::string> _StringEntries;                // immediately atter _stringOffsets

            std::vector<ResourceFileDependency> _FileDependencies;  // (not implemented)  immediately after _stringEntries,  repeating 0x20 byte sequence
            std::vector<uint32_t> _DependencyIndexes;               // (not implemented)  immediately after _DependencyEntries,  index into _DependencyEntries
            std::vector<uint64_t> _PathStringIndexes;               // immediately after _dependencyIndexes,  index into _stringEntries
    };
}

#pragma pack(pop)