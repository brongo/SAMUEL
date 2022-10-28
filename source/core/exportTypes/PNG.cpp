#include "PNG.h"

namespace HAYDEN
{
    // Convert DDS file to PNG (using DirectXTex on Windows, else use Detex library)
    std::vector<uint8_t> PNGFile::ConvertDDStoPNG(std::vector<uint8_t> inputDDS, bool reconstructZ)
    {
        std::vector<uint8_t> outputPNG;

#ifdef _WIN32
        
        // Windows systems use the DirectXTex library to convert a DDS file to PNG format
        HRESULT initCOM = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(initCOM))
        {
            fprintf(stderr, "ERROR: Failed to initialize the COM library. \n");
            return outputPNG;
        }

        // Read DDS binary data into DirectX::ScratchImage
        DirectX::ScratchImage scratchImage;
        if (DirectX::LoadFromDDSMemory(inputDDS.data(), inputDDS.size(), DirectX::DDS_FLAGS_NONE, NULL, scratchImage) != 0)
        {
            fprintf(stderr, "ERROR: Failed to read DDS data from given file. \n");
            return outputPNG;
        }

        // Decompress DDS image and storeRead DDS binary data into DirectX::ScratchImage
        DirectX::ScratchImage scratchImageDecompressed;
        if (DirectX::Decompress(*scratchImage.GetImage(0, 0, 0), DXGI_FORMAT_UNKNOWN, scratchImageDecompressed) != 0)
        {
            // If decompression failed, just use the data as-is.
            // Most images in DOOM Eternal have block compression, but some do not.
            scratchImageDecompressed = std::move(scratchImage);
        }

        // Construct a temporary image object to check the format
        auto tmpImage = scratchImageDecompressed.GetImage(0, 0, 0);

        // BC5 normals need to be converted to an intermediate format before converting to PNG
        if (tmpImage->format == DXGI_FORMAT_R8G8_UNORM)
        {
            // Convert to RGBA8_UNORM
            DirectX::ScratchImage rgba8Image;
            DirectX::Convert(*scratchImageDecompressed.GetImage(0, 0, 0), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, rgba8Image);
            scratchImageDecompressed = std::move(rgba8Image);

            // Reconstruct the blue channel if requested
            if (reconstructZ)
            {
                HRESULT restoreZ;
                DirectX::ScratchImage scratchReconstructedZ;
                DirectX::TexMetadata imgMeta = scratchImageDecompressed.GetMetadata();

                restoreZ = DirectX::TransformImage(scratchImageDecompressed.GetImages(), scratchImageDecompressed.GetImageCount(), scratchImageDecompressed.GetMetadata(),
                    [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t w, size_t y)
                    {
                        static const DirectX::XMVECTORU32 s_selectz = { { {DirectX::XM_SELECT_0, DirectX::XM_SELECT_0, DirectX::XM_SELECT_1, DirectX::XM_SELECT_0 } } };
                        UNREFERENCED_PARAMETER(y);

                        for (size_t j = 0; j < w; ++j)
                        {
                            const DirectX::XMVECTOR value = inPixels[j];
                            DirectX::XMVECTOR z;

                            DirectX::XMVECTOR x2 = DirectX::XMVectorMultiplyAdd(value, DirectX::g_XMTwo, DirectX::g_XMNegativeOne);
                            x2 = DirectX::XMVectorSqrt(DirectX::XMVectorSubtract(DirectX::g_XMOne, DirectX::XMVector2Dot(x2, x2)));
                            z = DirectX::XMVectorMultiplyAdd(x2, DirectX::g_XMOneHalf, DirectX::g_XMOneHalf);

                            outPixels[j] = XMVectorSelect(value, z, s_selectz);
                        }
                    }, scratchReconstructedZ);

                if (FAILED(restoreZ))
                    return outputPNG;

                scratchImageDecompressed = std::move(scratchReconstructedZ);
            }
        }

        // Construct final raw image for converting to PNG
        auto rawImage = scratchImageDecompressed.GetImage(0, 0, 0);

        // Convert to PNG
        DirectX::Blob pngImage;
        if (DirectX::SaveToWICMemory(*rawImage, DirectX::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), pngImage, &GUID_WICPixelFormat32bppBGRA) != 0) {
            fprintf(stderr, "ERROR: Failed to write new PNG file. \n");
            return outputPNG;
        }

        // Copy DirectX::Blob data into byte vector
        auto* p = reinterpret_cast<unsigned char*>(pngImage.GetBufferPointer());
        auto  n = pngImage.GetBufferSize();

        outputPNG.reserve(n);
        std::copy(p, p + n, std::back_inserter(outputPNG));

#else
        // Non-Windows systems use the Detex library to convert a DDS file to PNG format
        bool failed = 0;
        fs::path fullPath = fs::temp_directory_path() / "samuel.tmp";

        FILE* outFile = fopen(fullPath.string().c_str(), "wb");
        fwrite(inputDDS.data(), 1, inputDDS.size(), outFile);
        fclose(outFile);

        // Load DDS file
        detexTexture pngTexture;
        pngTexture.format = DETEX_PIXEL_FORMAT_RGBA8;
        detexTexture* ddsTexture = NULL;
        std::unique_ptr<uint8_t> pngData;

        if (!detexLoadDDSFile(fullPath.c_str(), &ddsTexture))
        {
            // Try loading as raw (for rgba8 textures)
            pngTexture.width = *(int*)(inputDDS.data() + 12);
            pngTexture.height = *(int*)(inputDDS.data() + 16);
            pngTexture.width_in_blocks = pngTexture.width;
            pngTexture.height_in_blocks = pngTexture.height;
            pngTexture.data = inputDDS.data() + 128;
        }
        else
        {
            // Create output PNG 
            pngTexture.width = ddsTexture->width;
            pngTexture.height = ddsTexture->height;
            pngTexture.width_in_blocks = ddsTexture->width;
            pngTexture.height_in_blocks = ddsTexture->height;
            pngData.reset(new uint8_t[detexGetPixelSize(pngTexture.format) * pngTexture.width * pngTexture.height]);
            pngTexture.data = pngData.get();

            // Decompress DDS
            if (!detexDecompressTextureLinear(ddsTexture, pngTexture.data, DETEX_PIXEL_FORMAT_RGBA8))
            {
                fprintf(stderr, "ERROR: Failed to decompress DDS file. \n");
                failed = 1;
            }
        }

        // Save as PNG
        if (!failed)
        {
            if (!detexSavePNGFile(&pngTexture, fullPath.c_str()))
                fprintf(stderr, "ERROR: Failed to convert DDS file to PNG. \n");

            if (!readFile(fullPath, outputPNG))
                fprintf(stderr, "ERROR: Failed to read PNG file to memory. \n");
        }

        // Clean up memory and temp files
        if (ddsTexture)
            free(ddsTexture);
        std::error_code ec;
        fs::remove(fullPath, ec);

#endif
        return outputPNG;
    }
}