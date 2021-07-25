#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include "PackageMapSpec.h"
#include "ResourceFile.h"
#include "StreamDBFile.h"
#include "Utilities.h"

namespace fs = std::filesystem;

namespace HAYDEN
{
	struct DDSHeader
	{
		byte magic[4]	= { 0x44, 0x44, 0x53, 0x20 };	// DDS 
		byte hsize[4]	= { 0x7C, 0x00, 0x00, 0x00 };	// 124
		byte flags[4]	= { 0x07, 0x10, 0x0A, 0x00 };	// DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE
		byte height[4]	= { 0x00, 0x01, 0x00, 0x00 };	// 256
		byte width[4]	= { 0x00, 0x01, 0x00, 0x00 };	// 256
		byte pitch[4]	= { 0x00, 0x80, 0x00, 0x00 };	// 32768
		byte depth[4]	= { 0x01, 0x00, 0x00, 0x00 };
		byte nmips[4]	= { 0x01, 0x00, 0x00, 0x00 };	
		byte unused[44] = { 0 };
		byte size[4]	= { 0x20, 0x00, 0x00, 0x00 };	// 32
		byte flags2[4]	= { 0x04, 0x00, 0x00, 0x00 };	// DDPF_FOURCC
		byte type[4]	= { 0x44, 0x58, 0x54, 0x31 };	// DXT1
		byte RGBbits[4] = { 0 };
		byte rBitMsk[4] = { 0 };
		byte gBitMsk[4] = { 0 };
		byte bBitMsk[4] = { 0 };
		byte aBitMsk[4] = { 0 };
		byte caps[4]	= { 0x00, 0x10, 0x00, 0x00 };	// DDSCAPS_TEXTURE
		byte caps2[4]	= { 0 };
		byte caps3[4]	= { 0 };
		byte caps4[4]	= { 0 };
		byte unused2[4] = { 0 };
	};

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
			void Init(const ResourceFile& resourceFile, const std::vector<StreamDBFile>& streamDBFiles, std::string outDirectory);
			void ExportTGAFiles(const std::vector<StreamDBFile>& streamDBFiles);

		private:
			std::string _OutDir;
			FileExportList _TGAExportList;

			// FileExporter Subroutines
			std::vector<byte> GetBinaryFileFromStreamDB(const FileExportItem& fileExportInfo, const StreamDBFile& streamDBFile);
			std::vector<byte> ConstructDDSFileHeader();
			fs::path BuildOutputPath(const std::string filepath);


			// Debug Functions
			void PrintMatchesToCSV(std::vector<FileExportItem>& fileExportList) const;
			void PrintUnmatchedToCSV(std::vector<FileExportItem>& fileExportList) const;
	};
}