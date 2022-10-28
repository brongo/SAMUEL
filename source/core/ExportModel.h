#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "exportTypes/DDSHeader.h"
#include "exportTypes/OBJ.h"
#include "exportTypes/PNG.h"

#include "idFileTypes/BIM.h"
#include "idFileTypes/DECL.h"
#include "idFileTypes/LWO.h"
#include "idFileTypes/MD6.h"
#include "idFileTypes/ResourceFile.h"
#include "idFileTypes/StreamDBFile.h"

#include "Common.h"
#include "ExportDECL.h"
#include "ExportBIM.h"
#include "Oodle.h"
#include "ResourceFileReader.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    // Holds material2 .decl data used by the model
    struct MaterialInfo
    {
        DeclFile ParsedDeclFile;
        std::string DeclFileName;
        std::vector<std::string> TextureNames;
        std::vector<std::string> TextureTypes;
    };

    class ModelExportTask
    {
        public:
            
            std::string ResourcePath;
            fs::path ModelExportPath;
            std::vector<MaterialInfo> MaterialData;

            // OBJ export functions. Consider moving to OBJ.h
            void WriteMTLFile();
            void WriteOBJFile(const int modelType);

            // Dependency export functions (material2 .decls and BIM textures)
            void ExportBIMTextures(const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources, const MaterialInfo& materialInfo, const std::vector<StreamDBFile>& streamDBFiles);
            void ExportMaterial2Decls(const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources);
            void ReadMaterial2Decls();

            bool Export(const fs::path exportPath, const std::string resourcePath, const std::vector<StreamDBFile>& streamDBFiles, const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources, const int modelType);
            ModelExportTask(const ResourceEntry resourceEntry);

        private:

            // Name of the file we are exporting, as it appears in a *.resources file
            std::string _FileName;

            // For locating the model header, which is embedded in a *.resources file
            uint64_t _ResourceID = 0;
            uint64_t _ResourceDataOffset = 0;
            uint64_t _ResourceDataLength = 0;
            uint64_t _ResourceDataLengthDecompressed = 0;

            // For locating the model geometry (+LODs), which is embedded in a *.streamdb file
            std::string _StreamDBFilePath;
            uint64_t _StreamedDataHash = 0;
            uint64_t _StreamedDataLength = 0;
            uint64_t _StreamedDataLengthDecompressed = 0;
            int32_t _StreamCompressionType = 0;
            int32_t _StreamDBNumber = -1;                            // index into std::vector<StreamDBFile>& streamDBFiles

            // Matching StreamDBEntry for the model
            StreamDBEntry _StreamDBEntry;

            // Serialized MD6 model
            MD6 _MD6;
            MD6_HEADER _MD6Header;

            // Serialized LWO model
            LWO _LWO;
            LWO_HEADER _LWOHeader;

            std::vector<Mesh> _StreamedGeometry;
    };
}