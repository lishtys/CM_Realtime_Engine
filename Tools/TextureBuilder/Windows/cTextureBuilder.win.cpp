// Includes
//=========

#include "../cTextureBuilder.h"

#include <algorithm>
#include <Engine/Graphics/cSamplerState.h>
#include <Engine/Graphics/TextureFormats.h>
#include <Engine/Math/Functions.h>
#include <Engine/ScopeCleanUp/cScopeCleanUp.h>
#include <Engine/Windows/Functions.h>
#include <External/DirectXTex/Includes.h>
#include <fstream>
#include <locale>
#include <string>
#include <Tools/AssetBuildLibrary/Functions.h>
#include <utility>

// Helper Function Declarations
//=============================

namespace
{
	eae6320::cResult BuildTexture(const char *const i_path, const bool i_shouldTextureBeCompressed, const uint8_t i_desiredSamplerState,
		DirectX::ScratchImage &io_sourceImageThatMayNotBeValidAfterThisCall, DirectX::ScratchImage &o_texture);
	constexpr eae6320::Graphics::TextureFormats::eType GetFormat(const DXGI_FORMAT i_dxgiFormat);
	eae6320::cResult LoadSourceImage(const char *const i_path, DirectX::ScratchImage &o_image);
	eae6320::cResult WriteTextureToFile(const char* const i_path_target, const DirectX::ScratchImage &i_texture, const uint8_t i_desiredSamplerState);
}

// Inherited Implementation
//=========================

// Build
//------

eae6320::cResult eae6320::Assets::cTextureBuilder::Build(const std::vector<std::string>& i_arguments)
{
	auto result = eae6320::Results::Success;

	DirectX::ScratchImage sourceImage;
	DirectX::ScratchImage builtTexture;
	auto shouldComBeUninitialized = false;

	// Initialize COM
	{
		void* const thisMustBeNull = nullptr;
		if (SUCCEEDED(CoInitialize(thisMustBeNull)))
		{
			shouldComBeUninitialized = true;
		}
		else
		{
			result = Results::Failure;
			Assets::OutputErrorMessageWithFileInfo(m_path_source,
				"DirectXTex couldn't be used because COM couldn't be initiazed");
			if (shouldComBeUninitialized)
			{
				CoUninitialize();
			}
		}
	}
	// Load the source image
	if (!(result = LoadSourceImage(m_path_source, sourceImage)))
	{
		if (shouldComBeUninitialized)
		{
			CoUninitialize();
		}
	}
	// Build the texture
	i_arguments;	// One way to customize how the texture is built would be to pass in and use command arguments
	// The code I am providing always compresses the texture in a pre-defined way;
	// you will have to change the code to do anything more sophisticated
	constexpr auto compressTexture = true;
	// The code I am providing always assumes a pre-defined sampler state that your game will use;
	// you will have to change the code to do anything more sophisticated
	constexpr auto desiredSamplerState = []
	{
		auto samplerStateBits = uint8_t(0);
		Graphics::SamplerStates::EnableFiltering(samplerStateBits);
		Graphics::SamplerStates::SetEdgeBehaviorU(Graphics::SamplerStates::Clamp, samplerStateBits);
		Graphics::SamplerStates::SetEdgeBehaviorV(Graphics::SamplerStates::Clamp, samplerStateBits);
		return samplerStateBits;
	}();
	if (!(result = BuildTexture(m_path_source, compressTexture, desiredSamplerState,
		sourceImage, builtTexture)))
	{
		goto OnExit;
	}
	// Write the texture to a file
	if (!(result = WriteTextureToFile(m_path_target, builtTexture, desiredSamplerState)))
	{
		goto OnExit;
	}

OnExit:

	if (shouldComBeUninitialized)
	{
		CoUninitialize();
	}

	return result;
}

// Helper Function Definitions
//============================

namespace
{
	eae6320::cResult BuildTexture(const char *const i_path, const bool i_shouldTextureBeCompressed, const uint8_t i_desiredSamplerState,
		DirectX::ScratchImage &io_sourceImageThatMayNotBeValidAfterThisCall, DirectX::ScratchImage &o_texture)
	{
		const DWORD filterOptions = DirectX::TEX_FILTER_DEFAULT
			| ((eae6320::Graphics::SamplerStates::GetEdgeBehaviorU(i_desiredSamplerState) == eae6320::Graphics::SamplerStates::Tile)
				? DirectX::TEX_FILTER_WRAP_U : 0)
			| ((eae6320::Graphics::SamplerStates::GetEdgeBehaviorV(i_desiredSamplerState) == eae6320::Graphics::SamplerStates::Tile)
				? DirectX::TEX_FILTER_WRAP_V : 0)
			;

		// DirectX can only do image processing on uncompressed images
		DirectX::ScratchImage uncompressedImage;
		if (DirectX::IsCompressed(io_sourceImageThatMayNotBeValidAfterThisCall.GetMetadata().format))
		{
			// The uncompressed format is chosen naively and assumes "standard" textures
			// (it will lose precision on any source images that use more than 8 bits per channel
			// and lose information on any that aren't normalized [0,1])
			constexpr auto formatToDecompressTo = DXGI_FORMAT_R8G8B8A8_UNORM;
			const auto result = DirectX::Decompress(io_sourceImageThatMayNotBeValidAfterThisCall.GetImages(), io_sourceImageThatMayNotBeValidAfterThisCall.GetImageCount(),
				io_sourceImageThatMayNotBeValidAfterThisCall.GetMetadata(), formatToDecompressTo, uncompressedImage);
			if (FAILED(result))
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex failed to uncompress source image");
				return eae6320::Results::Failure;
			}
		}
		else
		{
			uncompressedImage = std::move(io_sourceImageThatMayNotBeValidAfterThisCall);
		}
		// Different platforms have different conventions for how memory in textures is laid out
		DirectX::ScratchImage flippedImage;
		{
			constexpr auto shouldTextureBeStoredTopToBottom =
#if defined ( EAE6320_PLATFORM_D3D )
				true;
#elif defined ( EAE6320_PLATFORM_GL )
				false;
#else
#error "No implementation exists for this platform"
#endif
				if (shouldTextureBeStoredTopToBottom)
				{
					// DirectXTex uses top-to-bottom because it matches Direct3D and so nothing needs to be done
					flippedImage = std::move(uncompressedImage);
				}
				else
				{
					constexpr auto flipVertically = static_cast<DWORD>(DirectX::TEX_FR_FLIP_VERTICAL);
					const auto result = DirectX::FlipRotate(uncompressedImage.GetImages(), uncompressedImage.GetImageCount(),
						uncompressedImage.GetMetadata(), flipVertically, flippedImage);
					if (FAILED(result))
					{
						eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex failed to flip the source image vertically");
						return eae6320::Results::Failure;
					}
				}
		}
		// Textures used by the GPU have size restrictions that standard images don't
		DirectX::ScratchImage resizedImage;
		{
			const auto flippedMetadata = flippedImage.GetMetadata();
			auto targetWidth = flippedMetadata.width;
			auto targetHeight = flippedMetadata.height;
			{
				// Direct3D will only load BC compressed textures whose dimensions are multiples of 4
				// ("BC" stands for "block compression", and each block is 4x4)
				if (i_shouldTextureBeCompressed)
				{
					// Round up to the nearest multiple of 4
					constexpr size_t blockSize = 4;
					targetWidth = eae6320::Math::RoundUpToMultiple_powerOf2(targetWidth, blockSize);
					targetHeight = eae6320::Math::RoundUpToMultiple_powerOf2(targetHeight, blockSize);
				}
				// Direct3D can't support textures over a certain size
				{
					if (!resizedImage.GetMetadata().IsVolumemap())
					{
						if (!resizedImage.GetMetadata().IsCubemap())
						{
							targetWidth = std::min<size_t>(targetWidth, D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
							targetHeight = std::min<size_t>(targetHeight, D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION);
						}
						else
						{
							targetWidth = std::min<size_t>(targetWidth, D3D11_REQ_TEXTURECUBE_DIMENSION);
							targetHeight = std::min<size_t>(targetHeight, D3D11_REQ_TEXTURECUBE_DIMENSION);
						}
					}
					else
					{
						targetWidth = std::min<size_t>(targetWidth, D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION);
						targetHeight = std::min<size_t>(targetHeight, D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION);
					}
				}
			}
			if ((targetWidth != flippedMetadata.width) || (targetHeight != flippedMetadata.height))
			{
				const auto result = DirectX::Resize(flippedImage.GetImages(), flippedImage.GetImageCount(),
					flippedMetadata, targetWidth, targetHeight, filterOptions, resizedImage);
			}
			else
			{
				resizedImage = std::move(flippedImage);
			}
		}
		// Generate MIP maps
		// (Note that this will overwrite any existing MIP maps)
		DirectX::ScratchImage imageWithMipMaps;
		{
			constexpr size_t generateAllPossibleLevels = 0;
			HRESULT result;
			if (!resizedImage.GetMetadata().IsVolumemap())
			{
				result = DirectX::GenerateMipMaps(resizedImage.GetImages(), resizedImage.GetImageCount(),
					resizedImage.GetMetadata(), filterOptions, generateAllPossibleLevels, imageWithMipMaps);
			}
			else
			{
				result = DirectX::GenerateMipMaps3D(resizedImage.GetImages(), resizedImage.GetImageCount(),
					resizedImage.GetMetadata(), filterOptions, generateAllPossibleLevels, imageWithMipMaps);
			}
			if (FAILED(result))
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex failed to generate MIP maps");
				return eae6320::Results::Failure;
			}
		}
		// Compress the texture
		if (i_shouldTextureBeCompressed)
		{
			// The texture builder code that I'm providing supports two kinds of compressed formats:
			//	* BC1 (compressed with no alpha, used to be known as "DXT1")
			//	* BC3 (compressed with alpha, used to be known as "DXT5")
			const auto formatToCompressTo = resizedImage.IsAlphaAllOpaque() ? DXGI_FORMAT_BC1_UNORM : DXGI_FORMAT_BC3_UNORM;
			constexpr DWORD useDefaultCompressionOptions = DirectX::TEX_COMPRESS_DEFAULT;
			constexpr float useDefaultThreshold = DirectX::TEX_THRESHOLD_DEFAULT;
			if (FAILED(DirectX::Compress(imageWithMipMaps.GetImages(), imageWithMipMaps.GetImageCount(),
				imageWithMipMaps.GetMetadata(), formatToCompressTo, useDefaultCompressionOptions, useDefaultThreshold, o_texture)))
			{
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex failed to compress the texture");
				return eae6320::Results::Failure;
			}
		}
		else
		{
			constexpr auto uncompressedFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			if (imageWithMipMaps.GetMetadata().format == uncompressedFormat)
			{
				// The texture is already in the final format
				o_texture = std::move(imageWithMipMaps);
			}
			else
			{
				// The texture is uncompressed but needs to be converted to the expected format
				constexpr float useDefaultThreshold = DirectX::TEX_THRESHOLD_DEFAULT;
				if (FAILED(DirectX::Convert(imageWithMipMaps.GetImages(), imageWithMipMaps.GetImageCount(),
					imageWithMipMaps.GetMetadata(), uncompressedFormat, filterOptions, useDefaultThreshold, o_texture)))
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex failed to convert the texture");
					return eae6320::Results::Failure;
				}
			}
		}

		return eae6320::Results::Success;
	}

	constexpr eae6320::Graphics::TextureFormats::eType GetFormat(const DXGI_FORMAT i_dxgiFormat)
	{
		switch (i_dxgiFormat)
		{
		case DXGI_FORMAT_BC1_UNORM: return eae6320::Graphics::TextureFormats::BC1;
		case DXGI_FORMAT_BC1_UNORM_SRGB: return eae6320::Graphics::TextureFormats::BC1_sRGB;
		case DXGI_FORMAT_BC2_UNORM: return eae6320::Graphics::TextureFormats::BC2;
		case DXGI_FORMAT_BC2_UNORM_SRGB: return eae6320::Graphics::TextureFormats::BC2_sRGB;
		case DXGI_FORMAT_BC3_UNORM: return eae6320::Graphics::TextureFormats::BC3;
		case DXGI_FORMAT_BC3_UNORM_SRGB: return eae6320::Graphics::TextureFormats::BC3_sRGB;
		case DXGI_FORMAT_BC4_UNORM: return eae6320::Graphics::TextureFormats::BC4;
		case DXGI_FORMAT_BC5_UNORM: return eae6320::Graphics::TextureFormats::BC5;
		case DXGI_FORMAT_BC7_UNORM: return eae6320::Graphics::TextureFormats::BC7;
		case DXGI_FORMAT_BC7_UNORM_SRGB: return eae6320::Graphics::TextureFormats::BC7_sRGB;

		case DXGI_FORMAT_R8G8B8A8_UNORM: return eae6320::Graphics::TextureFormats::Uncompressed;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return eae6320::Graphics::TextureFormats::Uncompressed_sRGB;
		}

		return eae6320::Graphics::TextureFormats::Unknown;
	}

	eae6320::cResult LoadSourceImage(const char *const i_path, DirectX::ScratchImage &o_image)
	{
		// DirectXTex uses wide strings
		const auto path = eae6320::Windows::ConvertUtf8ToUtf16(i_path);

		// Open the image based on its file extension
		// (An image's format can also often be deduced by looking at its actual bits
		// because the first of a file will be some kind of recognizable header,
		// but our TextureBuilder keeps things simple)
		HRESULT result = E_UNEXPECTED;
		{
			const std::wstring extension = path.substr(path.find_last_of(L'.') + 1);
			DirectX::TexMetadata* const dontReturnMetadata = nullptr;
			if (extension == L"dds")
			{
				constexpr DWORD useDefaultBehavior = DirectX::DDS_FLAGS_NONE
					// Just in case you happen to use any old-style DDS files with luminance
					// this will expand the single luminance channel to all RGB channels
					// (which keeps it greyscale rather than using a red-only channel format)
					| DirectX::DDS_FLAGS_EXPAND_LUMINANCE
					;
				if (FAILED(result = DirectX::LoadFromDDSFile(path.c_str(), useDefaultBehavior, dontReturnMetadata, o_image)))
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex couldn't load the DDS file");
				}
			}
			else if (extension == L"tga")
			{
				if (FAILED(result = DirectX::LoadFromTGAFile(path.c_str(), dontReturnMetadata, o_image)))
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "DirectXTex couldn't load the TGA file");
				}
			}
			else
			{
				// Try to Windows Imaging Component and hope it supports the image type
				constexpr DWORD useDefaultBehavior = DirectX::WIC_FLAGS_NONE
					// If an image has an embedded sRGB profile ignore it
					// since our renderer isn't gamma-correct
					// (we want all textures in the shaders to have the same values they do as source images)
					| DirectX::WIC_FLAGS_IGNORE_SRGB
					;
				if (FAILED(result = DirectX::LoadFromWICFile(path.c_str(), useDefaultBehavior, dontReturnMetadata, o_image)))
				{
					eae6320::Assets::OutputErrorMessageWithFileInfo(i_path, "WIC couldn't load the source image");
				}
			}
		}

		return SUCCEEDED(result) ? eae6320::Results::Success : eae6320::Results::Failure;
	}

	eae6320::cResult WriteTextureToFile(const char* const i_path_target, const DirectX::ScratchImage &i_texture, const uint8_t i_desiredSamplerState)
	{
		auto result = eae6320::Results::Success;

		// Open the file
		std::ofstream fout(i_path_target, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
		if (!fout.is_open())
		{
			result = eae6320::Results::Failure;
			eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target, "Target texture file couldn't be opened for writing");
			return result;
		}
		const eae6320::cScopeCleanUp scopeCleanUp([i_path_target, &fout]
		{
			if (fout.is_open())
			{
				fout.close();
				if (fout.is_open())
				{
					eae6320::Assets::OutputWarningMessageWithFileInfo(i_path_target,
						"Failed to close the target texture file after writing");
				}
			}
		});

		// Write the texture information
		eae6320::Graphics::TextureFormats::sTextureInfo textureInfo;
		{
			auto &metadata = i_texture.GetMetadata();
			if (metadata.width < (1u << (sizeof(textureInfo.width) * 8)))
			{
				textureInfo.width = static_cast<uint16_t>(metadata.width);
			}
			else
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"The width (%u) is too big for a sTextureInfo", metadata.width);
				return result;
			}
			if (metadata.height < (1u << (sizeof(textureInfo.height) * 8)))
			{
				textureInfo.height = static_cast<uint16_t>(metadata.height);
			}
			else
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"The height (%u) is too big for a sTextureInfo", metadata.height);
				return result;
			}
			if (metadata.mipLevels < (1u << (sizeof(textureInfo.mipMapCount) * 8)))
			{
				textureInfo.mipMapCount = static_cast<uint8_t>(metadata.mipLevels);
			}
			else
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"There are too many MIP levels (%u) for a sTextureInfo", metadata.mipLevels);
				return result;
			}
			textureInfo.format = GetFormat(metadata.format);
			if (textureInfo.format == eae6320::Graphics::TextureFormats::Unknown)
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"The DXGI_Format (%i) isn't valid for a sTextureInfo", metadata.format);
				return result;
			}
		}
		{
			const auto byteCountToWrite = sizeof(textureInfo);
			fout.write(reinterpret_cast<const char*>(&textureInfo), byteCountToWrite);
			if (!fout.good())
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"Failed to write %u bytes for the texture information", byteCountToWrite);
				return result;
			}
		}
		// Write the data for each MIP map
		{
			auto currentWidth = static_cast<uint_fast16_t>(textureInfo.width);
			auto currentHeight = static_cast<uint_fast16_t>(textureInfo.height);
			const auto blockSize = eae6320::Graphics::TextureFormats::GetSizeOfBlock(textureInfo.format);
			const auto mipMapCount = static_cast<uint_fast8_t>(textureInfo.mipMapCount);
			if (mipMapCount == i_texture.GetImageCount())
			{
				const auto* const mipMaps = i_texture.GetImages();
				for (uint_fast8_t i = 0; i < mipMapCount; ++i)
				{
					const auto& currentMipMap = mipMaps[i];
					// Calculate how much memory this MIP level uses
					size_t singleRowSize, currentMipLevelSize;
					{
						if (blockSize > 0)
						{
							// A non-zero block size means that the texture is using block compression
							const auto blockCount_singleRow = (currentWidth + 3) / 4;
							const auto byteCount_singleRow = blockCount_singleRow * blockSize;
							singleRowSize = byteCount_singleRow;
							const auto rowCount = (currentHeight + 3) / 4;
							const auto byteCount_currentMipLevel = byteCount_singleRow * rowCount;
							currentMipLevelSize = byteCount_currentMipLevel;
						}
						else
						{
							// A block of zero size means that the texture is uncompressed
							constexpr auto channelCount = 4;
							constexpr auto byteCount_singleTexel = channelCount * sizeof(uint8_t);
							const auto byteCount_singleRow = currentWidth * byteCount_singleTexel;
							singleRowSize = byteCount_singleRow;
							const auto byteCount_currentMipLevel = byteCount_singleRow * currentHeight;
							currentMipLevelSize = byteCount_currentMipLevel;
						}
					}
					if (singleRowSize != currentMipMap.rowPitch)
					{
						result = eae6320::Results::Failure;
						eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
							"Unexpected mismatch between calculated byte count for a single row of MIP map #%u (%u) and DirectXTex row pitch (%u)",
							i, singleRowSize, currentMipMap.rowPitch);
						return result;
					}
					if (currentMipLevelSize != currentMipMap.slicePitch)
					{
						result = eae6320::Results::Failure;
						eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
							"Unexpected mismatch between calculated byte count for MIP map #%u (%u) and DirectXTex slice pitch (%u)",
							i, currentMipLevelSize, currentMipMap.slicePitch);
						return result;
					}
					// Write this MIP map
					{
						fout.write(reinterpret_cast<const char*>(currentMipMap.pixels), currentMipLevelSize);
						if (!fout.good())
						{
							result = eae6320::Results::Failure;
							eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
								"Failed to write %u bytes for MIP map #%u", currentMipLevelSize, i);
							return result;
						}
					}
					// Update dimensions for the next iteration
					{
						currentWidth = std::max(currentWidth / 2, 1u);
						currentHeight = std::max(currentHeight / 2, 1u);
					}
				}
			}
			else
			{
				result = eae6320::Results::Failure;
				eae6320::Assets::OutputErrorMessageWithFileInfo(i_path_target,
					"Unexpected mismatch between MIP map count (%u) and DirectXTex image count (%u)",
					mipMapCount, i_texture.GetImageCount());
				return result;
			}
		}

		return result;
	}
}
