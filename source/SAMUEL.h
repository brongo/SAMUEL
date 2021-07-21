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
			
			// Public API Functions
			void Init(const std::string basePath);
			void LoadResource(const std::string fileName);
			void ExportAll();

		private:

			// Private Variables
			std::string _basePath;
			std::vector<std::string> _StreamDBFileList;
			std::vector<StreamDBFile> _StreamDBFileData; 
			ResourceFile _ResourceFile;
			PackageMapSpec _PackageMapSpec;
			FileExporter _Exporter;

			// Init Functions
			void SetBasePath(const std::string basePath) { _basePath = basePath; }
			void LoadPackageMapSpec();

			// Helper Functions - LoadResource()
			void UpdateStreamDBFileList(const std::string resourceFileName);
			void ReadStreamDBFiles();

			// Debug Functions
			void PrintMatchesToCSV() const;
			void PrintUnmatchedToCSV() const;
	};
}
