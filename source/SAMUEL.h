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
			void Init(std::string basePath);
			void LoadResource(std::string fileName);
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
			void SetBasePath(std::string basePath) { _basePath = basePath; }
			void LoadPackageMapSpec();

			// Helper Functions - LoadResource()
			void UpdateStreamDBFileList(std::string resourceFileName);
			void ReadStreamDBFiles();

			// Debug Functions
			void PrintMatchesToCSV();
			void PrintUnmatchedToCSV();
	};
}
