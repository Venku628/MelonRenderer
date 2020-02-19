#include "Pipeline.h"

namespace MelonRenderer
{
	bool Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {
				VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				nullptr,
				0,
				code.size(),
				reinterpret_cast<const uint32_t*>(code.data())
		};
		VkResult result = vkCreateShaderModule(Device::Get().m_device, &shaderModuleCreateInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			Logger::Log("Could not create shader module.");
			return false;
		}

		return true;
	}
}