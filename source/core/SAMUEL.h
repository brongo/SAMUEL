#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "FileExporter.h"
#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "ResourceFileReader.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace HAYDEN
{
	class SAMUEL
	{
        public:
			// Startup and resource loader functions
			bool Init(const std::string resourcePath);
			bool LoadResource(const std::string fileName);

			// File export functions
			bool HasEnoughDiskSpace(); 
			bool ExportAll(const std::string outputDirectory);
			bool ExportSelected(const std::string outputDirectory, const std::vector<std::vector<std::string>> userSelectedFileList);

			bool HasDiskSpaceError() { return _HasDiskSpaceError; }
			bool HasResourceLoadError() { return _HasResourceLoadError; }
			std::string GetLastErrorMessage() { return _LastErrorMessage; }
			std::string GetLastErrorDetail() { return _LastErrorDetail; }
			std::vector<ResourceEntry> GetResourceData() { return _ResourceData; }

		private:
			bool _HasFatalError = 0;
			bool _HasDiskSpaceError = 0;
			bool _HasResourceLoadError = 0;
			std::string _LastErrorMessage;
			std::string _LastErrorDetail;
			std::string _BasePath;
			std::string _ResourcePath;
			std::vector<std::string> _StreamDBFileList;
			std::vector<StreamDBFile> _StreamDBFileData; 
			std::vector<ResourceEntry> _ResourceData;
			PackageMapSpec _PackageMapSpec;
			FileExporter _Exporter;

			// Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
			void ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail = "");

			// Called on startup
			bool FindBasePath(const std::string resourcePath);
			void LoadPackageMapSpec();

			// Reads streamdb data associated with this resource file (per packageMapSpec).
            void UpdateStreamDBFileList(const std::string resourceFileName);
			void ReadStreamDBFiles();
	};
}
