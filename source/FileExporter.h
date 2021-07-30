#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include "DDSHeader.h"
#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace fs = std::filesystem;

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
			uint32 tgaPixelWidth = 0;
			uint32 tgaPixelHeight = 0;
			uint32 tgaImageType = 0;
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
			std::vector<EmbeddedMD6Header> _MD6HeaderData;
			std::vector<EmbeddedLWOHeader> _LWOHeaderData;

			// Subroutines
			uint64 CalculateStreamDBIndex(const uint64 resourceId, const int mipCount = -6) const;
			std::vector<byte> DecompressEmbeddedFileHeader(std::vector<byte> embeddedHeader, const uint64 decompressedSize);

			// Helper functions for FileExportList constructor
			void GetResourceEntries(const ResourceFile& resourceFile);
			void ParseEmbeddedFileHeaders(const ResourceFile& resourceFile);
			void GetStreamDBIndexAndSize();	
			void GetStreamDBFileOffsets(const std::vector<StreamDBFile>& streamDBFiles);		
			
	};

	class FileExporter
	{
		public:
			FileExporter() {};
			void Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, std::string outDirectory);
			void ExportTGAFiles(const std::vector<StreamDBFile>& streamDBFiles);
			void ExportMD6Files(const std::vector<StreamDBFile>& streamDBFiles);
			void ExportLWOFiles(const std::vector<StreamDBFile>& streamDBFiles);

		private:
			std::string _OutDir;
			FileExportList _TGAExportList;
			FileExportList _MD6ExportList;
			FileExportList _LWOExportList;

			// FileExporter Subroutines
			std::vector<byte> GetBinaryFileFromStreamDB(const FileExportItem& fileExportInfo, const StreamDBFile& streamDBFile);
			fs::path BuildOutputPath(const std::string filepath);

			// Debug Functions
			void PrintMatchesToCSV(std::vector<FileExportItem>& fileExportList) const;
			void PrintUnmatchedToCSV(std::vector<FileExportItem>& fileExportList) const;
	};
}