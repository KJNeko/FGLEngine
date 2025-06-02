//
// Created by kj16609 on 6/2/25.
//
#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace fgl::engine
{

	class Instance
	{
		struct InfoObject
		{
			vk::ApplicationInfo m_app_info;
			vk::InstanceCreateInfo m_create_info;
			vk::DebugUtilsMessengerCreateInfoEXT m_debug_create_info;
			vk::DebugUtilsMessengerEXT m_debug_messenger;

			std::vector< const char* > m_extensions;
			std::vector< const char* > m_layers;

			InfoObject();
		};

		vk::raii::Instance m_vk_instance;

		inline static std::vector< const char* > required_extensions {};
		inline static std::vector< const char* > required_layers {};

	  public:

		operator vk::Instance() { return m_vk_instance; }

		operator VkInstance() const { return *m_vk_instance; }

		vk::raii::Instance& vkHandle() { return m_vk_instance; }

		explicit Instance( const vk::raii::Context& vk_ctx );
		~Instance();
	};

} // namespace fgl::engine