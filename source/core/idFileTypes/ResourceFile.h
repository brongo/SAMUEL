#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

#include "ResourceFile.h"
#include "../Utilities.h"
#include "StreamDBFile.h"

namespace fs = std::filesystem;
namespace HAYDEN {
    class ResourceManager;

    // User-friendly struct for managing .resources data
    struct ResourceEntry {
        uint64_t DataOffset = 0;
        uint64_t DataSize = 0;
        uint64_t DataSizeUncompressed = 0;
        uint64_t StreamResourceHash = 0;
        uint32_t Version = 0;
        uint16_t CompressionMode = 0;
        std::string Name;
        std::string Type;
    };
/*  *
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
    HAYDEN_PACK(
    // Built-in struct in .resources binary file
            struct ResourceFileHeader // 0x7C bytes
            {
                /* 0x00 */ uint32_t m_magic;
                /* 0x04 */ uint32_t m_version;

                /* 0x08 */ uint32_t m_unk08;
                /* 0x0C */ uint32_t m_unk0C;
                /* 0x10 */ uint64_t m_unk10;
                /* 0x18 */ uint64_t m_unk18;

                /* 0x20 */ uint32_t m_numFileEntries;
                /* 0x24 */ uint32_t m_numDependencyEntries;   // resource dependencies list
                /* 0x28 */ uint32_t m_numDependencyIndexes;   // index for resource dependencies
                /* 0x2C */ uint32_t m_numPathStringIndexes;   // index for file name/type strings

                /* 0x30 */ uint32_t m_unk30;
                /* 0x34 */ uint32_t m_numErrorLogs;           // usually 0
                /* 0x38 */ uint32_t m_sizeStrings;            // size of entire strings section, in bytes
                /* 0x3C */ uint32_t m_sizeErrorLog;           // usually 0

                /* 0x40 */ uint64_t m_addrPathStringOffsets;
                /* 0x48 */ uint64_t m_addrErrorLogs;          // starting offset for error logs (usually none)
                /* 0x50 */ uint64_t m_addrEntries;            // starting offset for ResourceFileEntry structs
                /* 0x58 */ uint64_t m_addrDependencyEntries;  // starting offset for dependency entries

                /* 0x60 */ uint64_t m_addrDependencyIndexes;  // starting offset for dependency indexes
                /* 0x68 */ uint64_t m_addrData;               // starting offset for embedded file data
                /* 0x70 */ uint32_t m_addrUnk70;
                /* 0x74 */ uint64_t m_addrEndMarker;          // "IDCL" marks end of resource file metadata, start of embedded files
            })

    // Built-in struct in .resources binary file
    HAYDEN_PACK(
            struct ResourceFileEntry // 0x90 bytes
            {
                /* 0x00 */ uint64_t m_pathTuple_OffsetType;   // add this to PathTuple_Index to get idx of the file type - always 0
                /* 0x08 */ uint64_t m_pathTuple_OffsetName;   // add this to PathTuple_Index to get idx of the file name - always 1

                /* 0x10 */ uint64_t m_unk10Dummy1;

                /* 0x18 */ uint64_t m_dependencyIndexNumber;  // index for DependencyEntry
                /* 0x20 */ uint64_t m_pathTuple_Index;        // divide by 2 to get the PathTuple array index (or multiply by 8 to get offset, as game does)

                /* 0x28 */ uint64_t m_unk28Zero0;
                /* 0x30 */ uint64_t m_unk30Zero1;

                /* 0x38 */ uint64_t m_dataOffset;             // offset of embedded file, in bytes
                /* 0x40 */ uint64_t m_dataSize;               // size of embedded file, in bytes
                /* 0x48 */ uint64_t m_dataSizeUncompressed;   // size of embedded file when decompressed

                /* 0x50 */ uint64_t m_dataCheckSum;           // murmurhash of decompressed data
                /* 0x58 */ uint64_t m_timestamp;              // divide by 1,000,000 for UNIX timestamp

                /* 0x60 */ uint64_t m_streamResourceHash;     // gets converted to .streamdb index, not shown here
                // conversion is shown in FileExporter->FileExportList::calculateStreamDBIndex

                /* 0x68 */ uint32_t m_version;                // resource type enum,  0x43 = model,  0x21 = image,  0x00 = rs_streamfile
                /* 0x6C */ uint32_t m_havokFlag1;             // 2 if HavokShape or RenderProg,  1 for certain SWF images (rare)

                /* 0x70 */ uint16_t m_compressionMode;        // 0 = uncompressed,  1 = zlib, 2 = kraken,  4 = kraken variant,  5 = leviathan (handled by sub_1408D8D00)
                /* 0x72 */ uint8_t m_havokFlag2;             // 0x41 for HavokShapes,  varies for RenderProg
                /* 0x73 */ uint8_t m_havokFlag3;             // 1 for RenderProg

                /* 0x74 */ uint32_t m_unk74;
                /* 0x78 */ uint32_t m_unk78Zero2;

                /* 0x7C */ uint32_t m_flags;                  // 8 = supports regen,  1 = required for regen?,  2 = ?? (always set?),  0x10000 = encrypted??
                /* 0x80 */ uint32_t m_desiredCompressionMode; // Desired compression mode,  only used while building?
                /* 0x84 */ uint16_t m_numDependencies;

                /* 0x86 */ uint16_t m_unk86;                  // sub_1408E8DC0 reads this
                /* 0x88 */ uint64_t m_unk88;
            })

    // Built-in struct in .resources binary file
    HAYDEN_PACK(
            struct ResourceFileDependency // 0x20 bytes
            {
                /* 0x00 */ uint64_t m_assetTypeStringIndex;   // index into _stringEntries,  file "type" of the depedency
                /* 0x08 */ uint64_t m_fileNameStringIndex;    // Index into _stringEntries,  file "name" of the dependency
                /* 0x10 */ uint32_t m_dependencyType;         // 2 = RES_DEP_RESOURCE,  5 = RES_DEP_EMBEDDED_RESOURCE
                /* 0x14 */ uint32_t m_unk14;                  // always 1 ??
                /* 0x18 */ uint32_t m_unk18;                  // some kind of file ID ??
                /* 0x1C */ uint32_t m_unk1C;                  // file ID ??  but often 0x0000 for some file types, like .lwo
            }
    )

    class ResourceFile {
    public:


        // Reads a binary .resources file from local filesystem
        explicit ResourceFile(const fs::path &filePath);

        [[nodiscard]] std::optional<std::vector<uint8_t>> queryFileByName(const std::string &name) const;

        [[nodiscard]] std::optional<std::vector<uint8_t>>
        queryStreamDataByName(const std::string &name, uint64_t streamSize, int32_t mipCount = -6) const;


        // Getters
        [[nodiscard]] uint32_t fileEntryCount() const { return m_header.m_numFileEntries; }

        [[nodiscard]] const std::string &
        resourceStringEntryAt(size_t index) const { return m_stringEntries[index]; }

        [[nodiscard]] const std::vector<uint64_t> &allPathStringIndexes() const { return m_pathStringIndexes; }

        [[nodiscard]] const ResourceFileEntry &resourceFileEntryAt(size_t index) { return m_file_entries[index]; }

        [[nodiscard]] std::vector<std::pair<std::string, std::string>> filenameList() const;

        [[nodiscard]] std::vector<HAYDEN::ResourceEntry> fileList() const;

        [[nodiscard]] const fs::path &filepath() { return m_filePath; };

        [[nodiscard]] bool loaded() const { return m_loaded; };

    private:
        bool mountStreamDB(const fs::path &path);

        [[nodiscard]] std::vector<uint8_t> readHeader(const ResourceFileEntry &entry) const;

        [[nodiscard]] static uint64_t calculateStreamDBIndex(uint64_t resourceId, int mipCount = -6);

        fs::path m_filePath; // External, passed into constructor
        bool m_loaded;

        // Binary data within the .resources file
        ResourceFileHeader m_header{};                             // first 0x7C bytes in file
        std::vector<ResourceFileEntry> m_file_entries;            // immediately after ResourceFileHeader,  repeating 0x90 byte sequence

        std::vector<std::string> m_stringEntries;                // immediately after _stringOffsets

        std::vector<ResourceFileDependency> m_fileDependencies;  // (not implemented)  immediately after _stringEntries,  repeating 0x20 byte sequence
        std::vector<uint32_t> m_dependencyIndexes;               // (not implemented)  immediately after _DependencyEntries,  index into _DependencyEntries
        std::vector<uint64_t> m_pathStringIndexes;               // immediately after _dependencyIndexes,  index into _stringEntries

        std::vector<StreamDBFile> m_streamFiles;

        friend ResourceManager;

    };

}
