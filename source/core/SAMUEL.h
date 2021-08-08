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
			void Init(const std::string basePath);
			void LoadResource(const std::string fileName);
			void ExportAll(const std::string outputDirectory);

		private:
			std::string _basePath;
			std::vector<std::string> _StreamDBFileList;
			std::vector<StreamDBFile> _StreamDBFileData; 
			ResourceFile _ResourceFile;
			PackageMapSpec _PackageMapSpec;
			FileExporter _Exporter;

			// Called by Init()
			void SetBasePath(const std::string basePath) { _basePath = basePath; }
			void LoadPackageMapSpec();

			// Called by LoadResource()
			void UpdateStreamDBFileList(const std::string resourceFileName);
			void ReadStreamDBFiles();
	};
}
