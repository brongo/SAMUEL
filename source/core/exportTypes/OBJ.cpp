#include "OBJ.h"

namespace HAYDEN
{
    void OBJFile::ConvertFromLWO(LWO& lwo)
    {
        uint64_t vertexCount = 0;
        Objects.resize(lwo.Header.Metadata.NumMeshes);

        for (uint64_t i = 0; i < Objects.size(); i++)
        {
            Objects[i].ObjectName = ("o Object_" + std::to_string(i));

            // build Vert strings
            for (uint64_t j = 0; j < lwo.Geo[i].Vertices.size(); j++)
            {
                std::stringstream x;
                std::stringstream y;
                std::stringstream z;

                x << std::fixed << std::setprecision(8) << lwo.Geo[i].Vertices[j].X;
                y << std::fixed << std::setprecision(8) << lwo.Geo[i].Vertices[j].Y;
                z << std::fixed << std::setprecision(8) << lwo.Geo[i].Vertices[j].Z;

                std::string str = ("v " + x.str() + " " + y.str() + " " + z.str());
                Objects[i].Vertices.push_back(str);
            }

            // build UV strings
            for (uint64_t j = 0; j < lwo.Geo[i].UVs.size(); j++)
            {
                std::stringstream u;
                std::stringstream v;

                u << std::fixed << std::setprecision(8) << lwo.Geo[i].UVs[j].U;
                v << std::fixed << std::setprecision(8) << lwo.Geo[i].UVs[j].V;

                std::string str = ("vt " + u.str() + " " + v.str());
                Objects[i].UVs.push_back(str);
            }

            // build Normal strings
            for (uint64_t j = 0; j < lwo.Geo[i].Normals.size(); j++)
            {
                std::stringstream xn;
                std::stringstream yn;
                std::stringstream zn;

                xn << std::fixed << std::setprecision(8) << lwo.Geo[i].Normals[j].Xn;
                yn << std::fixed << std::setprecision(8) << lwo.Geo[i].Normals[j].Yn;
                zn << std::fixed << std::setprecision(8) << lwo.Geo[i].Normals[j].Zn;

                std::string str = ("vn " + xn.str() + " " + yn.str() + " " + zn.str());
                Objects[i].Normals.push_back(str);
            }

            // build Face strings
            uint64_t offset = 1;

            if (i > 0)
            {
                // offset the face index by the number of vertices that came before it
                vertexCount += lwo.Geo[i - 1].Vertices.size();
                offset = vertexCount + 1;
            }

            for (uint64_t j = 0; j < lwo.Geo[i].Faces.size(); j++)
            {
                std::string f1 = std::to_string(lwo.Geo[i].Faces[j].F1 + offset);
                std::string f2 = std::to_string(lwo.Geo[i].Faces[j].F2 + offset);
                std::string f3 = std::to_string(lwo.Geo[i].Faces[j].F3 + offset);

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
}