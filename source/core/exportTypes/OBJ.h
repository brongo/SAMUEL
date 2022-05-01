#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

#include "../idFileTypes/LWO.h"

namespace HAYDEN
{
    struct OBJFile_Object
    {
	std::string ObjectName;
	std::vector<std::string> Vertices;
	std::vector<std::string> Normals;
	std::vector<std::string> UVs;
	std::string UseMaterialLine;
	std::vector<std::string> Faces;
    };

    class OBJFile
    {
	public:
            std::string SignatureLine = "# Exported by SAMUEL v2.0.9 \n# https://github.com/brongo/SAMUEL";
	    std::string MaterialLine;
	    std::vector<OBJFile_Object> Objects;
	    void ConvertFromLWO(LWO& lwo);
    };
}