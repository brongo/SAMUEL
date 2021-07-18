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

	class StreamDBData
	{
		public:
			std::string filename;
			std::string streamDBFileName;
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
			std::vector<std::string> _globalStreamDBFileList;
			std::vector<std::string> _resourceStreamDBFileList;

		public:
			std::vector<StreamDBData> streamDBData;
			std::vector<StreamDBFile> streamDBFiles;
			ResourceFile resourceFile;
			PackageMapSpec packageMapSpec;

		public:
			// Init Functions
			void SetBasePath(std::string basePath) { _basePath = basePath; }
			void SetGlobalStreamDBFileList();
			void LoadPackageMapSpec(); 

			// Helper Functions
			void readStreamDBEntries(std::vector<std::string> fileList);

			// Public API Functions
			void LoadResource(std::string fileName);

			// Unsorted
			void getStreamDBDataIndexes(ResourceFile& resourceFile);
			int findMatchingIndex(uint64 streamIndex, int streamDBNumber);
			void searchStreamDBFilesForIndex(StreamDBData& streamDBData);
			byte* getCompressedFileData(FILE& f, uint64 fileOffset, uint64 compressedSize);
			TGAHeader readTGAHeader(const char* tmpDecompressedHeader);

			// Debug Functions
			void printMatchesToCSV();
			void printUnmatchedToCSV();
	};
}
