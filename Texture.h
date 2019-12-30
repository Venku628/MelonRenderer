#pragma once

#include "Basics.h"

namespace MelonRenderer {
	class Texture
	{
		VkImageView m_textureImageView;
		VkImage m_textureImage;
		VkDeviceMemory m_textureMemory;

		friend class Renderer;
		friend class TextureManager;
	};
}
