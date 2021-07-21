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
			std::vector<FileExportItem> GetFileExportList() const { return fileExportList; }
			void Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles);

		private:

			// Data used by file export process
			std::vector<FileExportItem> fileExportList;

			// Helper Functions - ExportAll()
			void BuildFileExportList(const ResourceFile& resourceFile);
			std::vector<EmbeddedTGAHeader> ReadEmbeddedTGAHeaders(const ResourceFile& resourceFile);
			uint64 CalculateStreamDBIndex(const uint64 resourceId, const int mipCount = -6) const;
			int FindMatchingIndex(const uint64 streamIndex, const int streamDBNumber, const std::vector<StreamDBFile>& streamDBFiles) const;
			void SearchStreamDBFilesForIndex(FileExportItem& streamDBData, const std::vector<StreamDBFile>& streamDBFiles) const;
	};
}