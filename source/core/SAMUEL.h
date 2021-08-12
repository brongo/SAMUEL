#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "FileExporter.h"
#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace HAYDEN
{
	class SAMUEL
	{
		public:
            bool CheckDependencies();
            bool Init(const std::string resourcePath);
			void LoadResource(const std::string fileName);
			void ExportAll(const std::string outputDirectory);
            void ExportSelected(const std::string outputDirectory, const std::vector<std::vector<std::string>> userSelectedFileList);
            std::string GetLastErrorMessage() { return _LastErrorMessage; }
            std::string GetLastErrorDetail() { return _LastErrorDetail; }
            ResourceFile GetResourceFile() { return _ResourceFile; }

		private:
            bool _HasFatalError = 0;
            std::string _LastErrorMessage;
            std::string _LastErrorDetail;
            std::string _BasePath;
			std::vector<std::string> _StreamDBFileList;
			std::vector<StreamDBFile> _StreamDBFileData; 
			ResourceFile _ResourceFile;
			PackageMapSpec _PackageMapSpec;
			FileExporter _Exporter;

            // Outputs to stderr, but also stores error message for passing to another application (Qt, etc).
            void ThrowError(bool isFatal, std::string errorMessage, std::string errorDetail = "");

			// Called by Init()
            bool FindOodleDLL();
            bool FindBasePath(const std::string resourcePath);
			void LoadPackageMapSpec();

			// Called by LoadResource()
			void UpdateStreamDBFileList(const std::string resourceFileName);
			void ReadStreamDBFiles();
	};
}
