#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include "../idFileTypes/LWO.h"
#include "../idFileTypes/MD6Mesh.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
    // Testing mesh merge based on material usage
    struct OBJFile_FaceMaterialGroup
    {
        std::string GroupLine;
        std::string UseMaterialLine;
        std::vector<std::string> Faces;
    };

    struct OBJFile_Object
    {
	std::string ObjectName;
	std::vector<std::string> Vertices;
	std::vector<std::string> Normals;
	std::vector<std::string> UVs;

    std::string GroupLine;
    std::string UseMaterialLine;
	std::vector<std::string> Faces;

    };

    class OBJFile
    {
	public:
            std::string SignatureLine = "# Exported with SAMUEL v2.1.2 by SamPT \n# https://github.com/brongo/SAMUEL";
            std::string MaterialLibrary;
	        std::vector<OBJFile_Object> Objects;
            std::vector<OBJFile_FaceMaterialGroup> FaceMaterialGroups;
            void ConvertFromLWO(LWO& lwo);
            void ConvertFromMD6(MD6Mesh& md6);
            void Serialize(const std::vector<Mesh> streamedGeometry, const std::vector<std::string> meshNames, const int modelType, const int numVertices, const int numFaces, const GeoFlags geoFlags, const GeoMetadata geoMeta);
    };
}
