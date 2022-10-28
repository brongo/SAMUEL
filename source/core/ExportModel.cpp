#include "ExportModel.h"

namespace HAYDEN
{
    // Constructor
    ModelExportTask::ModelExportTask(const ResourceEntry resourceEntry)
    {
        _FileName = resourceEntry.Name;
        _ResourceDataOffset = resourceEntry.DataOffset;
        _ResourceDataLength = resourceEntry.DataSize;
        _ResourceDataLengthDecompressed = resourceEntry.DataSizeUncompressed;
        _ResourceID = resourceEntry.StreamResourceHash;
        return;
    }

    // Write the model data to OBJ file
    void ModelExportTask::WriteOBJFile(const int modelType)
    {
        fs::path exportFolder = ModelExportPath;
        fs::path materialFile = ModelExportPath / fs::path(_FileName).filename().replace_extension(".mtl");
        fs::path outputFile = ModelExportPath / fs::path(_FileName).filename().replace_extension(".obj");

        // Create output directories if needed
        if (!fs::exists(ModelExportPath))
        {
            if (!mkpath(ModelExportPath))
            {
                fprintf(stderr, "Error: Failed to create directories for file: %s \n", ModelExportPath.string().c_str());
                return;
            }
        }

        // Construct OBJ file from model data
        OBJFile objFile;

        if (modelType == 31)
            objFile.ConvertFromMD6(_MD6);

        if (modelType == 67)
            objFile.ConvertFromLWO(_LWO);

        // Open output stream
        std::ofstream file;
        file.open(outputFile.string(), std::ios::out);

        // print header
        file << objFile.SignatureLine + "\n\n";
        objFile.MaterialLibrary = "mtllib " + materialFile.filename().string() + "\n\n";
        file << objFile.MaterialLibrary;

        for (int i = 0; i < objFile.Objects.size(); i++)
        {
            if (modelType == 67)
            {
                // print object name
                file << objFile.Objects[i].ObjectName + "\n";
            }

            // print verts
            for (int j = 0; j < objFile.Objects[i].Vertices.size(); j++)
                file << objFile.Objects[i].Vertices[j] + "\n";

            // print uvs
            for (int j = 0; j < objFile.Objects[i].UVs.size(); j++)
                file << objFile.Objects[i].UVs[j] + "\n";

            // print normals
            for (int j = 0; j < objFile.Objects[i].Normals.size(); j++)
                file << objFile.Objects[i].Normals[j] + "\n";

            if (modelType == 67)
            {
                // print usemtls
                file << objFile.Objects[i].GroupLine + "\n";
                file << objFile.Objects[i].UseMaterialLine + "\n";

                // print faces
                for (int j = 0; j < objFile.Objects[i].Faces.size(); j++)
                    file << objFile.Objects[i].Faces[j] + "\n";
            }

        }

        if (modelType == 31)
        {
            for (uint64_t i = 0; i < objFile.FaceMaterialGroups.size(); i++)
            {
                // print usemtls ONLY if not a duplicate of previous (works because they are sorted alphabetically)
                if (i == 0)
                {
                    file << objFile.FaceMaterialGroups[i].GroupLine + "\n";
                    file << objFile.FaceMaterialGroups[i].UseMaterialLine + "\n";
                }

                if (i > 0)
                {
                    if (objFile.FaceMaterialGroups[i].GroupLine != objFile.FaceMaterialGroups[i - 1].GroupLine)
                    {
                        file << objFile.FaceMaterialGroups[i].GroupLine + "\n";
                        file << objFile.FaceMaterialGroups[i].UseMaterialLine + "\n";
                    }
                }
            
                // print faces
                for (uint64_t j = 0; j < objFile.FaceMaterialGroups[i].Faces.size(); j++)
                    file << objFile.FaceMaterialGroups[i].Faces[j] + "\n";
            }
        }

        file.close();
        return;
    }

    // Write the MD6 material data to MTL file. File is written to <ModelExportPath>
    void ModelExportTask::WriteMTLFile()
    {
        std::ofstream mtlfile;
        fs::path materialFile = ModelExportPath / fs::path(_FileName).filename().replace_extension(".mtl");
        mtlfile.open(materialFile.string(), std::ios::out);

        for (int mtlNum = 0; mtlNum < MaterialData.size(); mtlNum++)
        {
            fs::path mtlPath = MaterialData[mtlNum].DeclFileName;
            std::string mtlName = mtlPath.filename().replace_extension("").string();

            fs::path diffuseTexture;
            fs::path specularTexture;
            fs::path normalTexture;

            for (int j = 0; j < MaterialData[mtlNum].TextureTypes.size(); j++)
            {
                // diffuse types
                if (MaterialData[mtlNum].TextureTypes[j] == "albedo")
                    diffuseTexture = MaterialData[mtlNum].TextureNames[j];
                if (MaterialData[mtlNum].TextureTypes[j] == "eyealbedomap")
                    diffuseTexture = MaterialData[mtlNum].TextureNames[j];

                // specular types
                if (MaterialData[mtlNum].TextureTypes[j] == "specular")
                    specularTexture = MaterialData[mtlNum].TextureNames[j];

                // normal types
                if (MaterialData[mtlNum].TextureTypes[j] == "normal")
                    normalTexture = MaterialData[mtlNum].TextureNames[j];
                if (MaterialData[mtlNum].TextureTypes[j] == "eyeemissivemap")
                    normalTexture = MaterialData[mtlNum].TextureNames[j];
            }

            if (!diffuseTexture.empty())
                diffuseTexture = "images" / diffuseTexture.filename().replace_extension(".png");

            if (!specularTexture.empty())
                specularTexture = "images" / specularTexture.filename().replace_extension(".png");

            if (!normalTexture.empty())
                normalTexture = "images" / normalTexture.filename().replace_extension(".png");

            mtlfile << "newmtl " + mtlName + " \n";
            mtlfile << "illum 4 \n";                                        // DOOM Eternal uses Maya, which always exports as illum 4.
            mtlfile << "Kd 0.00 0.00 0.00 \n";
            mtlfile << "Ka 0.00 0.00 0.00 \n";
            mtlfile << "Ks 0.50 0.50 0.50 \n";

            if (!diffuseTexture.empty())
                mtlfile << "map_Kd " + diffuseTexture.string() + " \n";

            if (!normalTexture.empty())
                mtlfile << "map_bump " + normalTexture.string() + " \n";

            if (!specularTexture.empty())
                mtlfile << "map_Ks " + specularTexture.string() + " \n";
        }

        mtlfile.close();
        return;
    }

    // Export Material2 DECL files used by this model. Files are written to <ModelExportPath>/material2/
    void ModelExportTask::ExportMaterial2Decls(const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources)
    {
        for (uint64_t i = 0; i < MaterialData.size(); i++)
        {
            bool found = 0;
            std::string targetFileName = "generated/decls/material2/" + MaterialData[i].DeclFileName + ".decl";

            // Construct .decl export path
            fs::path targetFilePath(targetFileName);
            fs::path outputFile = ModelExportPath / "material2" / targetFilePath.filename();

            // Locate this file in current .resources
            for (uint64_t j = 0; j < resourceData.size(); j++)
            {
                if (resourceData[j].Version != 0)
                    continue;

                if (resourceData[j].Name == targetFileName)
                {
                    DECLExportTask declExportTask(resourceData[j]);
                    found = declExportTask.Export(outputFile, ResourcePath);
                    break;
                }
            }

            if (found)
                continue;

            // Unable to locate file in current .resources, search all global archives
            for (int j = 0; j < globalResources->Files.size(); j++)
            {
                for (int k = 0; k < globalResources->Files[j].Entries.size(); k++)
                {
                    if (globalResources->Files[j].Entries[k].Version != 0)
                        continue;

                    if (globalResources->Files[j].Entries[k].Name == targetFileName)
                    {
                        DECLExportTask declExportTask(globalResources->Files[j].Entries[k]);
                        found = declExportTask.Export(outputFile, globalResources->Files[j].ResourcePath.string());
                        break;
                    }
                }
                if (found)
                    break;
            }
        }
        return;
    }

    // Parse Material2 DECL files used by this MD6 model, to determine which BIM textures need to be exported.
    void ModelExportTask::ReadMaterial2Decls()
    {
        for (int i = 0; i < MaterialData.size(); i++)
        {
            // model path
            fs::path materialFolder = ModelExportPath / "material2";

            // decl path
            fs::path declFileName = MaterialData[i].DeclFileName + ".decl";
            fs::path materialFile = materialFolder / declFileName.filename();

            std::ifstream inputStream = std::ifstream(materialFile);
            if (inputStream.is_open())
            {
                DeclFile declFile;
                declFile.SetFileName(materialFile.string());
                std::string line;
                while (std::getline(inputStream, line))
                {
                    DeclSingleLine declSingleLine;
                    declSingleLine.ReadFromStream(inputStream, line);
                    declFile.SetLineData(declSingleLine);
                    declFile.LineCount++;
                }

                MaterialData[i].ParsedDeclFile = declFile;
                inputStream.close();
            }
        }

        // find textures referenced in this file and add to MaterialData
        for (int i = 0; i < MaterialData.size(); i++)
        {
            DeclFile& thisDeclFile = MaterialData[i].ParsedDeclFile;
            for (int lineNumber = 0; lineNumber < thisDeclFile.LineCount; lineNumber++)
            {
                DeclSingleLine lineData = thisDeclFile.GetLineData(lineNumber);
                if (lineData.GetLineVariable() != "filePath")
                    continue;

                DeclSingleLine prevLineData = thisDeclFile.GetLineData(lineNumber - 1);
                std::string textureType = prevLineData.GetLineVariable();
                std::string textureName = stripQuotes(lineData.GetLineValue());

                if (textureName != "_default") // these don't matter
                {
                    MaterialData[i].TextureNames.push_back(textureName);
                    MaterialData[i].TextureTypes.push_back(textureType);
                }
            }
        }
        return;
    }

    // Export BIM textures used by this MD6 model. Files are written to <ModelExportPath>/images/
    void ModelExportTask::ExportBIMTextures(const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources, const MaterialInfo& materialInfo, const std::vector<StreamDBFile>& streamDBFiles)
    {
        for (uint64_t i = 0; i < materialInfo.TextureNames.size(); i++)
        {
            bool found = 0;
            std::string targetFileName = materialInfo.TextureNames[i];

            // Construct .bim export path
            fs::path targetFilePath(targetFileName);
            fs::path outputFile = ModelExportPath / "images" / targetFilePath.filename().replace_extension(".png");

            // Locate this file in .resources
            for (uint64_t j = 0; j < resourceData.size(); j++)
            {
                // Skip non-image files
                if (resourceData[j].Version != 21)
                    continue;

                // Drop $ qualifiers from resourceData.Name
                std::string compName = resourceData[j].Name;
                size_t pos = compName.find("$");

                if (pos != -1)
                    compName = compName.substr(0, pos);

                // Found
                if (compName == targetFileName)
                {
                    // Check for smoothness texture - will have a slash somewhere in the truncated $ stuff
                    if (pos != -1)
                    {
                        std::string smoothnessCheck = resourceData[j].Name.substr(pos, resourceData[j].Name.length());
                        if ((smoothnessCheck.rfind("/") != -1) && (materialInfo.TextureTypes[i] != "smoothness"))
                            continue;
                    }

                    BIMExportTask bimExportTask(resourceData[j]);
                    found = bimExportTask.Export(outputFile, ResourcePath, streamDBFiles, true);
                    break;
                }
            }

            if (found)
                continue;

            // Unable to locate file in current .resources, search all global archives
            for (int j = 0; j < globalResources->Files.size(); j++)
            {
                // Search each global archive
                for (int k = 0; k < globalResources->Files[j].Entries.size(); k++)
                {
                    // Skip non-image files
                    if (globalResources->Files[j].Entries[k].Version != 21)
                        continue;

                    // Drop $ qualifiers from resourceData.Name
                    std::string compName = globalResources->Files[j].Entries[k].Name;
                    size_t pos = compName.find("$");

                    if (pos != -1)
                        compName = compName.substr(0, pos);

                    // Found
                    if (compName == targetFileName)
                    {
                        // Check for smoothness texture - will have a slash somewhere in the truncated $ stuff
                        if (pos != -1)
                        {
                            std::string smoothnessCheck = globalResources->Files[j].Entries[k].Name.substr(pos, globalResources->Files[j].Entries[k].Name.length());
                            if ((smoothnessCheck.rfind("/") != -1) && (materialInfo.TextureTypes[i] != "smoothness"))
                                continue;
                        }

                        BIMExportTask bimExportTask(globalResources->Files[j].Entries[k]);
                        found = bimExportTask.Export(outputFile, globalResources->Files[j].ResourcePath.string(), streamDBFiles, true);
                        break;
                    }
                }

                if (found)
                    break;
            }
        }
        return;
    }

    // Main export function for models.
    // Return 1 for success, 0 for failure.
    bool ModelExportTask::Export(const fs::path exportPath, const std::string resourcePath, const std::vector<StreamDBFile>& streamDBFiles, const std::vector<ResourceEntry>& resourceData, const GLOBAL_RESOURCES* globalResources, const int modelType)
    {
        ModelExportPath = exportPath;
        ResourcePath = resourcePath;
        std::vector<uint8_t> modelData;
        ResourceFileReader resourceFile(resourcePath);

        // Extract model header from .resources file and read it
        std::vector<uint8_t> binaryData = resourceFile.GetEmbeddedFileHeader(resourcePath, _ResourceDataOffset, _ResourceDataLength, _ResourceDataLengthDecompressed);

        if (binaryData.empty())
            return 0;

        // FOR DEBUG ONLY
        // writeToFilesystem(binaryData, exportPath);

        // Read binary data from our extracted model header 
        if (modelType == 31)
        {
            _MD6Header.ReadBinaryHeader(binaryData);
            _StreamedDataLengthDecompressed = _MD6Header.StreamDiskLayout[0].DecompressedSize;           // [0] = LOD_ZERO
            _StreamedDataLength = _MD6Header.StreamDiskLayout[0].CompressedSize;                         // [0] = LOD_ZERO
        }

        if (modelType == 67)
        {
            _LWOHeader.ReadBinaryHeader(binaryData);
            _StreamedDataLength = _LWOHeader.StreamDiskLayout[0].CompressedSize;                      // [0] = LOD_ZERO
            _StreamedDataLengthDecompressed = _LWOHeader.StreamDiskLayout[0].DecompressedSize;        // [0] = LOD_ZERO
        }

        // Convert resourceID to streamFileID
        _StreamedDataHash = resourceFile.CalculateStreamDBIndex(_ResourceID);

        // Locate and extract the model geometry from .streamdb file
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

        // Extract model geometry from .streamdb file.
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

        // FOR DEBUGGING ONLY - EXPORT RAW BINARIES AND ABORT
        // writeToFilesystem(binaryData, exportPath);
        // writeToFilesystem(modelData, exportPath);
        // return 0;

        // Serialize model data and get materials (MD6)
        if (modelType == 31)
        {
            _MD6.Serialize(_MD6Header, modelData);
            for (int i = 0; i < _MD6Header.MeshInfo.size(); i++)
            {
                MaterialInfo materialInfo;
                materialInfo.DeclFileName = _MD6Header.MeshInfo[i].MaterialDeclName;
                MaterialData.push_back(materialInfo);
            }
        }

        // Serialize model data and get materials (LWO)
        if (modelType == 67)
        {
            _LWO.Serialize(_LWOHeader, modelData);
            for (int i = 0; i < _LWOHeader.MeshInfo.size(); i++)
            {
                MaterialInfo materialInfo;
                materialInfo.DeclFileName = _LWOHeader.MeshInfo[i].MaterialDeclName;
                MaterialData.push_back(materialInfo);
            }
        }

        // Remove any duplicate materials
        std::sort(MaterialData.begin(), MaterialData.end(), [](const MaterialInfo& a, const MaterialInfo& b) {
            return (a.DeclFileName < b.DeclFileName);
        });

        auto it = std::unique(MaterialData.begin(), MaterialData.end(), [](const MaterialInfo& a, const MaterialInfo& b) {
            if (a.DeclFileName == b.DeclFileName)
                return 1;
            return 0;
        });

        size_t listSize = it - MaterialData.begin();
        MaterialData.resize(listSize);

        // Export the material2 .decls and parse them for textures used in this model
        ExportMaterial2Decls(resourceData, globalResources);
        ReadMaterial2Decls();

        // Find required textures and export them
        for (int i = 0; i < MaterialData.size(); i++)
            ExportBIMTextures(resourceData, globalResources, MaterialData[i], streamDBFiles);

        WriteOBJFile(modelType);
        WriteMTLFile();
        return 1;
    }
}
