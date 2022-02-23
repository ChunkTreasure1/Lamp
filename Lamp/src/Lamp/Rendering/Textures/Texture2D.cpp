#include "lppch.h"
#include "Texture2D.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include <stb/stb_image.h>

#define KHRONOS_STATIC
#include <ktx2/ktxvulkan.h>


namespace Lamp
{
	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffers)
	{
		return CreateRef<Texture2D>(format, width, height, saveBuffers);
	}

	Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path, bool generateMips)
	{
		return CreateRef<Texture2D>(path, generateMips);
	}

	Texture2D::Texture2D(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffer)
		: m_saveBuffer(saveBuffer)
	{
		ImageSpecification imageSpec{};
		imageSpec.format = format;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		m_image = std::reinterpret_pointer_cast<Image2D>(Image2D::Create(imageSpec));

		if (m_saveBuffer)
		{
			m_buffer.Allocate(width * height * 4);
		}
	}

	Texture2D::Texture2D(const std::filesystem::path& path, bool generateMips)
	{
		Load(path, generateMips);
	}

	Texture2D::~Texture2D()
	{
		m_buffer.Release();
	}

	void Texture2D::Bind(uint32_t slot) const
	{
	}

	void Texture2D::SetData(const void* data, uint32_t size)
	{
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		auto device = VulkanContext::GetCurrentDevice();

		VkDeviceSize deviceSize = size;

		stagingBufferMemory = Utility::CreateBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		VulkanAllocator allocator;

		void* newData = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(newData, data, size);
		allocator.UnmapMemory(stagingBufferMemory);

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), m_image->GetWidth(), m_image->GetHeight());
		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);

		if (m_saveBuffer)
		{
			m_buffer.Release();
			m_buffer.Allocate(size);
			m_buffer.Write(data, size);
		}
	}

	const uint32_t Texture2D::GetWidth() const
	{
		return m_image->GetSpecification().width;
	}

	const uint32_t Texture2D::GetHeight() const
	{
		return m_image->GetSpecification().height;
	}

	const uint32_t Texture2D::GetID() const
	{
		return uint32_t();
	}

	void Texture2D::Load(const std::filesystem::path& path, bool generateMips)
	{
		if (!std::filesystem::exists(path))
		{
			LP_CORE_ERROR("File not found {0}", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		if (path.extension() == ".ktx" || path.extension() == ".ktx2")
		{
			LoadKTX(path, generateMips);
		}
		else
		{
			LoadOther(path, generateMips);
		}
	}

	const VkDescriptorImageInfo& Texture2D::GetDescriptorInfo() const
	{
		if (IsValid())
		{
			return m_image->GetDescriptorInfo();
		}

		return Renderer::Get().GetDefaults().whiteTexture->GetDescriptorInfo();
	}

	void Texture2D::Setup(void* data, uint32_t size, uint32_t width, uint32_t height, bool generateMips, bool isHDR)
	{
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		auto device = VulkanContext::GetCurrentDevice();

		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		VulkanAllocator allocator;

		void* mappedData = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(mappedData, data, static_cast<size_t>(size));
		allocator.UnmapMemory(stagingBufferMemory);

		ImageSpecification imageSpec{};
		imageSpec.format = isHDR ? ImageFormat::RGBA32F : ImageFormat::RGBA;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;
		imageSpec.mips = generateMips ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

		m_image = std::reinterpret_pointer_cast<Image2D>(Image2D::Create(imageSpec));

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), width, height);

		if (generateMips)
		{
			Utility::GenerateMipMaps(m_image->GetHandle(), width, height, imageSpec.mips);
		}

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);

		if (m_saveBuffer)
		{
			m_buffer.Release();
			m_buffer.Allocate(size);
			m_buffer.Write(data, size);
		}
	}

	void Texture2D::LoadOther(const std::filesystem::path& path, bool generateMips)
	{
		VkDeviceSize size;
		void* imageData = nullptr;

		int width;
		int height;
		int channels;

		if (stbi_is_hdr(path.string().c_str()))
		{
			stbi_set_flip_vertically_on_load(0);
			imageData = stbi_loadf(path.string().c_str(), &width, &height, &channels, 4);
			size = width * height * 4 * sizeof(float);
		}
		else
		{
			stbi_set_flip_vertically_on_load(1);
			imageData = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
			size = width * height * 4;
		}

		Setup(imageData, size, width, height, generateMips, stbi_is_hdr(path.string().c_str()));

		stbi_image_free(imageData);
	}

	void Texture2D::LoadKTX(const std::filesystem::path& path, bool generateMips)
	{
		ktxTexture2* texture = nullptr;
		KTX_error_code result = ktxTexture_CreateFromNamedFile(path.string().c_str(), KTX_TEXTURE_CREATE_NO_FLAGS, (ktxTexture**)&texture);
		LP_CORE_ASSERT(result == KTX_SUCCESS, "Unable to load file!");

		if (ktxTexture2_NeedsTranscoding(texture))
		{
			auto device = VulkanContext::GetCurrentDevice();
			const auto& features = device->GetPhysicalDevice()->GetFeatures();

			ktx_texture_transcode_fmt_e tf;

			if (features.textureCompressionASTC_LDR)
			{
				tf = KTX_TTF_ASTC_4x4_RGBA;
			}
			else if (features.textureCompressionETC2)
			{
				tf = KTX_TTF_ETC2_RGBA;
			}
			else if (features.textureCompressionBC)
			{
				tf = KTX_TTF_BC7_RGBA;
			}

			result = ktxTexture2_TranscodeBasis(texture, tf, 0);
			LP_CORE_ASSERT(result == KTX_SUCCESS, "Unable to transcode texture!");
		}

		ktxVulkanTexture newTexture;
		ktxVulkanDeviceInfo deviceInfo;

		auto device = VulkanContext::GetCurrentDevice();

		ktxVulkanDeviceInfo_Construct(&deviceInfo, device->GetPhysicalDevice()->GetHandle(), device->GetHandle(), device->GetGraphicsQueue(), device->GetCommandPool(), nullptr);

		//ktxTexture_VkUploadEx(texture, &deviceInfo, &newTexture, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RGBA;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = newTexture.width;
		imageSpec.height = newTexture.height;
		imageSpec.mips = generateMips ? static_cast<uint32_t>(std::floor(std::log2(std::max(newTexture.width, newTexture.height)))) + 1 : 1;

		m_image = std::reinterpret_pointer_cast<Image2D>(Image2D::Create(imageSpec));

		//ktxTexture_Destroy(texture);
		ktxVulkanDeviceInfo_Destruct(&deviceInfo);
	}
}