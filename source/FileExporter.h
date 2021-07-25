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

	class FileExportList
	{
		public:
			FileExportList() {};
			FileExportList(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, int fileType);
			std::vector<FileExportItem> GetFileExportItems() { return _ExportItems; }

		private:
			int _FileType = 0;
			std::string _ResourceFileName;
			std::vector<FileExportItem> _ExportItems;
			std::vector<EmbeddedTGAHeader> _TGAHeaderData;

			// Helper functions for FileExportList constructor
			void GetResourceEntries(const ResourceFile& resourceFile);
			void ParseEmbeddedTGAHeaders(const ResourceFile& resourceFile); 
			void GetStreamDBIndexAndSize();	
			void GetStreamDBFileOffsets(const std::vector<StreamDBFile>& streamDBFiles);
			uint64 CalculateStreamDBIndex(const uint64 resourceId, const int mipCount = -6) const;			
	};

	class FileExporter
	{
		public:
			FileExporter() {};
			void Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles);
			void ExportTGAFiles(const std::vector<StreamDBFile>& streamDBFiles);

		private:
			FileExportList _TGAExportList;

			// Debug Functions
			void PrintMatchesToCSV(std::vector<FileExportItem>& fileExportList) const;
			void PrintUnmatchedToCSV(std::vector<FileExportItem>& fileExportList) const;
	};
}