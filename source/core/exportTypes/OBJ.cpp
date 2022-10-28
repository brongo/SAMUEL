#include "OBJ.h"

namespace HAYDEN
{
    void OBJFile::ConvertFromLWO(LWO& lwo)
    {
        uint64_t vertexCount = 0;
        Objects.resize(lwo.Header.Metadata.NumMeshes);

        for (uint64_t i = 0; i < Objects.size(); i++)
        {
            // construct material name
            fs::path mtlPath = lwo.Header.MeshInfo[i].MaterialDeclName;
            std::string mtlName = mtlPath.filename().replace_extension("").string();

            // set object name, group name, and usemtl (based on material name)
            Objects[i].ObjectName = "o " + mtlName;
            Objects[i].GroupLine = "g " + mtlName;
            Objects[i].UseMaterialLine = "usemtl " + mtlName;

            // build vert strings
            for (uint64_t j = 0; j < lwo.MeshGeometry[i].Vertices.size(); j++)
            {
                std::stringstream x;
                std::stringstream y;
                std::stringstream z;

                x << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Vertices[j].X;
                y << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Vertices[j].Y;
                z << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Vertices[j].Z;

                std::string str = ("v " + x.str() + " " + y.str() + " " + z.str());
                Objects[i].Vertices.push_back(str);
            }

            // build UV strings
            for (uint64_t j = 0; j < lwo.MeshGeometry[i].UVs.size(); j++)
            {
                std::stringstream u;
                std::stringstream v;

                u << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].UVs[j].U;
                v << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].UVs[j].V;

                std::string str = ("vt " + u.str() + " " + v.str());
                Objects[i].UVs.push_back(str);
            }

            // build Normal strings
            for (uint64_t j = 0; j < lwo.MeshGeometry[i].Normals.size(); j++)
            {
                std::stringstream xn;
                std::stringstream yn;
                std::stringstream zn;

                xn << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Normals[j].Xn;
                yn << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Normals[j].Yn;
                zn << std::fixed << std::setprecision(8) << lwo.MeshGeometry[i].Normals[j].Zn;

                std::string str = ("vn " + xn.str() + " " + yn.str() + " " + zn.str());
                Objects[i].Normals.push_back(str);
            }

            // build Face strings
            uint64_t offset = 1;

            if (i > 0)
            {
                // offset the face index by the number of vertices that came before it
                vertexCount += lwo.MeshGeometry[i - 1].Vertices.size();
                offset = vertexCount + 1;
            }

            for (uint64_t j = 0; j < lwo.MeshGeometry[i].Faces.size(); j++)
            {
                std::string f1 = std::to_string(lwo.MeshGeometry[i].Faces[j].F1 + offset);
                std::string f2 = std::to_string(lwo.MeshGeometry[i].Faces[j].F2 + offset);
                std::string f3 = std::to_string(lwo.MeshGeometry[i].Faces[j].F3 + offset);

                f1 = "f " + f1 + "/" + f1 + "/" + f1;
                f2 =  " " + f2 + "/" + f2 + "/" + f2;
                f3 =  " " + f3 + "/" + f3 + "/" + f3;

                // f3 comes before f2, otherwise the faces are inverted in Blender
                std::string str = f1 + f3 + f2;
                Objects[i].Faces.push_back(str);
            }
        }

        return;
    }

    void OBJFile::ConvertFromMD6(MD6& md6)
    {
        uint64_t vertexCount = 0;
        Objects.resize(md6.Header.NumMeshes);
        FaceMaterialGroups.resize(md6.Header.NumMeshes);

        for (uint64_t i = 0; i < Objects.size(); i++)
        {
            // For alternate config where original mesh names are retained
            // Objects[i].ObjectName = "o " + md6.Header.MeshInfo[i].MeshName;

            // construct material name
            fs::path mtlPath = md6.Header.MeshInfo[i].MaterialDeclName;
            std::string mtlName = mtlPath.filename().replace_extension("").string();

            // set object name, group name, and usemtl (based on material name)
            Objects[i].ObjectName = "o " + mtlName;
            Objects[i].GroupLine = "g " + mtlName;
            Objects[i].UseMaterialLine = "usemtl " + mtlName;

            // build Vert strings
            for (uint64_t j = 0; j < md6.MeshGeometry[i].Vertices.size(); j++)
            {
                std::stringstream x;
                std::stringstream y;
                std::stringstream z;

                x << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Vertices[j].X;
                y << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Vertices[j].Y;
                z << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Vertices[j].Z;

                std::string str = ("v " + x.str() + " " + y.str() + " " + z.str());
                Objects[i].Vertices.push_back(str);
            }

            // build UV strings
            for (uint64_t j = 0; j < md6.MeshGeometry[i].UVs.size(); j++)
            {
                std::stringstream u;
                std::stringstream v;

                u << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].UVs[j].U;
                v << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].UVs[j].V;

                std::string str = ("vt " + u.str() + " " + v.str());
                Objects[i].UVs.push_back(str);
            }

            // build Normal strings
            for (uint64_t j = 0; j < md6.MeshGeometry[i].Normals.size(); j++)
            {
                std::stringstream xn;
                std::stringstream yn;
                std::stringstream zn;

                xn << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Normals[j].Xn;
                yn << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Normals[j].Yn;
                zn << std::fixed << std::setprecision(8) << md6.MeshGeometry[i].Normals[j].Zn;

                std::string str = ("vn " + xn.str() + " " + yn.str() + " " + zn.str());
                Objects[i].Normals.push_back(str);
            }

            // build Face strings
            uint64_t offset = 1;

            if (i > 0)
            {
                // offset the face index by the number of vertices that came before it
                vertexCount += md6.MeshGeometry[i - 1].Vertices.size();
                offset = vertexCount + 1;
            }

            for (uint64_t j = 0; j < md6.MeshGeometry[i].Faces.size(); j++)
            {
                std::string f1 = std::to_string(md6.MeshGeometry[i].Faces[j].F1 + offset);
                std::string f2 = std::to_string(md6.MeshGeometry[i].Faces[j].F2 + offset);
                std::string f3 = std::to_string(md6.MeshGeometry[i].Faces[j].F3 + offset);

                f1 = "f " + f1 + "/" + f1 + "/" + f1;
                f2 = " " + f2 + "/" + f2 + "/" + f2;
                f3 = " " + f3 + "/" + f3 + "/" + f3;

                // f3 comes before f2, otherwise the faces are inverted in Blender
                std::string str = f1 + f3 + f2;
                Objects[i].Faces.push_back(str);
            }

            // Build face material groups
            FaceMaterialGroups[i].GroupLine = Objects[i].GroupLine;
            FaceMaterialGroups[i].UseMaterialLine = Objects[i].UseMaterialLine;
            FaceMaterialGroups[i].Faces = Objects[i].Faces;
        }

        // Sort face material groups alphabetically by material name
        std::sort(FaceMaterialGroups.begin(), FaceMaterialGroups.end(), [](const OBJFile_FaceMaterialGroup& a, const OBJFile_FaceMaterialGroup& b) {
            return (a.GroupLine < b.GroupLine);
        });

        return;
    }

    void OBJFile::Serialize(const std::vector<Mesh> streamedGeometry, const std::vector<std::string> meshNames, const int modelType, const int numVertices, const int numFaces, const GEO_FLAGS geoFlags, const GEO_METADATA geoMeta)
    {
        uint64_t vertexCount = 0;
        Objects.resize(streamedGeometry.size());
        FaceMaterialGroups.resize(streamedGeometry.size());

        for (uint64_t i = 0; i < Objects.size(); i++)
        {
            // For alternate config where original mesh names are retained
            // Objects[i].ObjectName = "o " + md6.Header.MeshInfo[i].MeshName;

            // construct material name
            fs::path mtlPath = meshNames[i];
            std::string mtlName = mtlPath.filename().replace_extension("").string();

            // set object name, group name, and usemtl (based on material name)
            Objects[i].ObjectName = "o " + mtlName;
            Objects[i].GroupLine = "g " + mtlName;
            Objects[i].UseMaterialLine = "usemtl " + mtlName;

            // build Vert strings
            for (uint64_t j = 0; j < streamedGeometry[i].Vertices.size(); j++)
            {
                std::stringstream x;
                std::stringstream y;
                std::stringstream z;

                x << std::fixed << std::setprecision(8) << streamedGeometry[i].Vertices[j].X;
                y << std::fixed << std::setprecision(8) << streamedGeometry[i].Vertices[j].Y;
                z << std::fixed << std::setprecision(8) << streamedGeometry[i].Vertices[j].Z;

                std::string str = ("v " + x.str() + " " + y.str() + " " + z.str());
                Objects[i].Vertices.push_back(str);
            }

            // build UV strings
            for (uint64_t j = 0; j < streamedGeometry[i].UVs.size(); j++)
            {
                std::stringstream u;
                std::stringstream v;

                u << std::fixed << std::setprecision(8) << streamedGeometry[i].UVs[j].U;
                v << std::fixed << std::setprecision(8) << streamedGeometry[i].UVs[j].V;

                std::string str = ("vt " + u.str() + " " + v.str());
                Objects[i].UVs.push_back(str);
            }

            // build Normal strings
            for (uint64_t j = 0; j < streamedGeometry[i].Normals.size(); j++)
            {
                std::stringstream xn;
                std::stringstream yn;
                std::stringstream zn;

                xn << std::fixed << std::setprecision(8) << streamedGeometry[i].Normals[j].Xn;
                yn << std::fixed << std::setprecision(8) << streamedGeometry[i].Normals[j].Yn;
                zn << std::fixed << std::setprecision(8) << streamedGeometry[i].Normals[j].Zn;

                std::string str = ("vn " + xn.str() + " " + yn.str() + " " + zn.str());
                Objects[i].Normals.push_back(str);
            }

            // build Face strings
            uint64_t offset = 1;

            if (i > 0)
            {
                // offset the face index by the number of vertices that came before it
                vertexCount += streamedGeometry[i - 1].Vertices.size();
                offset = vertexCount + 1;
            }

            for (uint64_t j = 0; j < streamedGeometry[i].Faces.size(); j++)
            {
                std::string f1 = std::to_string(streamedGeometry[i].Faces[j].F1 + offset);
                std::string f2 = std::to_string(streamedGeometry[i].Faces[j].F2 + offset);
                std::string f3 = std::to_string(streamedGeometry[i].Faces[j].F3 + offset);

                f1 = "f " + f1 + "/" + f1 + "/" + f1;
                f2 = " " + f2 + "/" + f2 + "/" + f2;
                f3 = " " + f3 + "/" + f3 + "/" + f3;

                // f3 comes before f2, otherwise the faces are inverted in Blender
                std::string str = f1 + f3 + f2;
                Objects[i].Faces.push_back(str);
            }

            // Build face material groups
            FaceMaterialGroups[i].GroupLine = Objects[i].GroupLine;
            FaceMaterialGroups[i].UseMaterialLine = Objects[i].UseMaterialLine;
            FaceMaterialGroups[i].Faces = Objects[i].Faces;
        }

        // Sort face material groups alphabetically by material name
        std::sort(FaceMaterialGroups.begin(), FaceMaterialGroups.end(), [](const OBJFile_FaceMaterialGroup& a, const OBJFile_FaceMaterialGroup& b) {
            return (a.GroupLine < b.GroupLine);
            });

        return;
    }
}