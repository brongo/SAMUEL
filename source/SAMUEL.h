#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace HAYDEN
{
	class TGAHeader
	{
		public:
			int numMips = 0;
			int isCompressed = 0;
			uint32 compressedSize = 0;
			uint32 decompressedSize = 0;
	};

	class FileExportItem
	{
	public:
		std::string filename;
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

	class SAMUEL
	{
		private:
			int _hasFatalError = 0;
			int _errorCode = 0;
			std::string _basePath;
			std::vector<std::string> _streamDBFileList;

		public:
			std::vector<FileExportItem> fileExportList;
			std::vector<StreamDBFile> streamDBFiles;
			ResourceFile resourceFile;
			PackageMapSpec packageMapSpec;

		public:
			// Init Functions
			void SetBasePath(std::string basePath) { _basePath = basePath; }
			void LoadPackageMapSpec();

			// Helper Functions - LoadResource()
			void UpdateStreamDBFileList(std::string resourceFileName);
			void ReadStreamDBFiles();

			// Helper Functions - ReadEmbeddedTGAHeaders()
			byte* GetCompressedFileHeader(FILE& f, uint64 fileOffset, uint64 compressedSize);
			TGAHeader ReadTGAHeader(const char* tmpDecompressedHeader);

			// Helper Functions - SearchStreamDBFilesForIndex()
			int FindMatchingIndex(uint64 streamIndex, int streamDBNumber);

			// Helper Functions - ExportAll()
			void BuildFileExportList();
			std::vector<TGAHeader> ReadEmbeddedTGAHeaders(ResourceFile& resourceFile);
			uint64 CalculateStreamDBIndex(uint64 resourceId, int mipCount = -6);
			void SearchStreamDBFilesForIndex(FileExportItem& streamDBData);

			// Public API Functions
			void LoadResource(std::string fileName);
			void ExportAll();

			// Debug Functions
			void PrintMatchesToCSV();
			void PrintUnmatchedToCSV();
	};
}
