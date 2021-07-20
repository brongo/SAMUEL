#pragma once
#include <string>
#include <vector>

#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace HAYDEN
{
	class FileExportItem
	{
		public:
			std::string resourceFileName;
			std::string streamDBFileName;
			uint64 resourceFileHash = 0;
			uint64 resourceFileOffset = 0;
			uint64 resourceFileCompressedSize = 0;
			uint64 resourceFileDecompressedSize = 0;
			uint64 streamDBIndex = 0;
			uint64 streamDBFileOffset = 0;
			uint64 streamDBSizeCompressed = 0;
			uint64 streamDBSizeDecompressed = 0;
			int streamDBCompressionType = 0;
			int streamDBNumber = -1;
	};

	class FileExporter
	{
		public:

			FileExporter() {};
			std::vector<FileExportItem> GetFileExportList() { return fileExportList; }
			void Init(ResourceFile& resourceFile, std::vector<StreamDBFile>& streamDBFiles);

		private:

			// Data used by file export process
			std::vector<FileExportItem> fileExportList;

			// Helper Functions - ExportAll()
			void BuildFileExportList(ResourceFile& resourceFile);
			std::vector<EmbeddedTGAHeader> ReadEmbeddedTGAHeaders(ResourceFile& resourceFile);
			uint64 CalculateStreamDBIndex(uint64 resourceId, int mipCount = -6);
			int FindMatchingIndex(uint64 streamIndex, int streamDBNumber, std::vector<StreamDBFile>& streamDBFiles);
			void SearchStreamDBFilesForIndex(FileExportItem& streamDBData, std::vector<StreamDBFile>& streamDBFiles);
	};
}