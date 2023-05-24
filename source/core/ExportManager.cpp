#include "ExportManager.h"

namespace HAYDEN {
    // Returns the name of the resource folder (e.g. "gameresources_patch1") where files will be exported to
    std::string ExportManager::getResourceFolder(const std::string &resourcePath) {
        std::string resourceFolder;
        size_t offset1 = resourcePath.rfind('/');
        size_t offset2 = resourcePath.find('.');

        // this shouldn't happen, but if it somehow does, we just won't use the resourceFolder in our export path.
        if (offset1 < 8 || offset2 < offset1)
            return resourceFolder;

        // get only the resource folder name, without path or file extension
        size_t length = offset2 - offset1;
        resourceFolder = resourcePath.substr(offset1 + 1, length - 1);

        // separates dlc hub from regular hub folder
        if (resourcePath.substr(offset1 - 8, 8) == "/dlc/hub")
            resourceFolder = "dlc_" + resourceFolder;

        return resourceFolder;
    }

    // Return the full path where a file will be exported to
    fs::path
    ExportManager::buildOutputPath(const std::string &filePath, const fs::path &outputDirectory, ExportType exportType,
                                   const std::string &resourceFolder) {
        fs::path outputPath;
        std::string outputFilePath;
        switch (exportType) {
            case ExportType::COMP:
            case ExportType::DECL:
                outputPath = outputDirectory / resourceFolder;
                break;
            case ExportType::BIM:
                outputFilePath = filePath + ".png";
                outputPath = outputDirectory / resourceFolder;
                break;
            case ExportType::LWO:
            case ExportType::MD6:
                outputFilePath = fs::path(filePath).filename().replace_extension("").string();
                outputPath.replace_filename("models");
                break;
        }
        outputPath = outputPath.append(outputFilePath);
        outputPath.make_preferred();
        return std::move(outputPath);
    }

    // Main file export function
    bool ExportManager::exportFiles(const HAYDEN::ResourceManager &resourceManager,
                                    const std::string &resourcePath,
                                    const fs::path &outputDirectory,
                                    const std::vector<std::vector<std::string>> &filesToExport) {
        // Abort if this function was called without any files selected for extraction.
        if (filesToExport.empty())
            return false;

        // Determine output directory
        std::string resourceFolder = getResourceFolder(resourcePath);

        // Create seperate export lists for each type of file
        // Some files have the same names but different versions (types), so we need to keep this separate
        for (int i = 0; i < filesToExport.size(); i++) {
            int fileType = std::stoi(filesToExport[i][2]);

            switch (fileType) {
//                case 0:
//                    _DECLFileNames.push_back(filesToExport[i][0]);
//                    break;
//                case 1:
//                    _COMPFileNames.push_back(filesToExport[i][0]);
//                    break;
//                case 21:
//                    _BIMFileNames.push_back(filesToExport[i][0]);
//                    break;
                case 31: {
                    fs::path exportPath = buildOutputPath(fs::path(filesToExport[i][0]).stem().string(), outputDirectory,
                                                          ExportType::MD6, resourceFolder);

                    ModelExportTask modelExportTask(resourceManager, filesToExport[i][0]);
                    modelExportTask.exportMD6Model(outputDirectory/exportPath);
                    break;

                }
//                case 67:
//                    _LWOFileNames.push_back(filesToExport[i][0]);
//                    break;
                default: {
                    auto resourceData = resourceManager.queryFileByName(filesToExport[i][0]);
                    if (resourceData.has_value())
                        writeToFilesystem(resourceData.value(), outputDirectory / "raw" / fs::path(filesToExport[i][0]));
                    break;

                }
            }
        }

//        // Iterate through currently loaded .resources data and construct _ExportJobQueue
//        for (uint64_t i = 0; i < resourceData.size(); i++) {
//            ExportTask task;
//            ResourceEntry thisEntry = resourceData[i];
//
//            // Skip this entry if it contains no data to extract.
//            // This can happen with certain files that have been removed from the game.
//            if (thisEntry.DataSize == 0)
//                continue;
//
//            // Check the .resources entries against our list of files to extract.
//            switch (thisEntry.Version) {
//                case 0:
//                    task.Type = ExportType::DECL;
//                    if (std::find(_DECLFileNames.begin(), _DECLFileNames.end(), thisEntry.Name) == _DECLFileNames.end())
//                        continue;
//                    break;
//                case 1:
//                    task.Type = ExportType::COMP;
//                    if (std::find(_COMPFileNames.begin(), _COMPFileNames.end(), thisEntry.Name) == _COMPFileNames.end())
//                        continue;
//                    break;
//                case 21:
//                    task.Type = ExportType::BIM;
//                    if (std::find(_BIMFileNames.begin(), _BIMFileNames.end(), thisEntry.Name) == _BIMFileNames.end())
//                        continue;
//                    break;
//                case 31:
//                    task.Type = ExportType::MD6;
//                    if (std::find(_MD6FileNames.begin(), _MD6FileNames.end(), thisEntry.Name) == _MD6FileNames.end())
//                        continue;
//                    break;
//                case 67:
//                    task.Type = ExportType::LWO;
//                    if (std::find(_LWOFileNames.begin(), _LWOFileNames.end(), thisEntry.Name) == _LWOFileNames.end())
//                        continue;
//                    break;
//                default:
//                    continue;
//            }
//
//            // If a match is found, add this entry to _ExportJobQueue
//            task.Entry = thisEntry;
//            task.ExportPath = BuildOutputPath(thisEntry.Name, outputDirectory, task.Type, resourceFolder);
//            _ExportJobQueue.push_back(task);
//        }
//
//        // Iterate through _ExportJobQueue and complete the file export
//        for (int i = 0; i < _ExportJobQueue.size(); i++) {
//            switch (_ExportJobQueue[i].Type) {
//                case ExportType::BIM: {
//                    BIMExportTask bimExportTask(_ExportJobQueue[i].Entry);
//                    _ExportJobQueue[i].Result = bimExportTask.Export(_ExportJobQueue[i].ExportPath, resourcePath,
//                                                                     streamDBFiles, true);
//                    break;
//                }
//                case ExportType::COMP: {
//                    COMPExportTask compExportTask(_ExportJobQueue[i].Entry);
//                    _ExportJobQueue[i].Result = compExportTask.Export(_ExportJobQueue[i].ExportPath, resourcePath);
//                    break;
//                }
//                case ExportType::DECL: {
//                    DECLExportTask declExportTask(_ExportJobQueue[i].Entry);
//                    _ExportJobQueue[i].Result = declExportTask.Export(_ExportJobQueue[i].ExportPath, resourcePath);
//                    break;
//                }
//                case ExportType::MD6: {
//                    ModelExportTask modelExportTask(_ExportJobQueue[i].Entry);
//                    modelExportTask.Export(_ExportJobQueue[i].ExportPath, resourcePath, streamDBFiles, resourceData,
//                                           globalResources, 31);
//                    break;
//                }
//                case ExportType::LWO: {
//                    ModelExportTask modelExportTask(_ExportJobQueue[i].Entry);
//                    modelExportTask.Export(_ExportJobQueue[i].ExportPath, resourcePath, streamDBFiles, resourceData,
//                                           globalResources, 67);
//                    break;
//                }
//            }
//        }

        return true;
    }
}