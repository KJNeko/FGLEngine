//
// Created by kj16609 on 6/20/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{
	class Instance
	{
		static constexpr bool ENABLE_VALIDATION_LAYERS { true };

		vk::ApplicationInfo m_app_info;
		vk::DebugUtilsMessengerCreateInfoEXT m_debug_info;
		std::vector< const char* > m_required_extensions;
		vk::InstanceCreateInfo m_instance_info;

		vk::raii::Instance m_instance;

		vk::DebugUtilsMessengerEXT m_debug_messenger { VK_NULL_HANDLE };

		vk::ApplicationInfo appInfo();
		vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo();
		vk::InstanceCreateInfo
			createInfo( vk::ApplicationInfo* info, vk::DebugUtilsMessengerCreateInfoEXT* msger_info );

		std::vector< const char* > getRequiredInstanceExtensions();

		void verifyGlfwExtensions();

		void setupDebugMessenger();

	  public:

		FGL_DELETE_DEFAULT_CTOR( Instance )
		FGL_DELETE_COPY( Instance )
		FGL_DELETE_MOVE( Instance )

		Instance( vk::raii::Context& ctx );

		~Instance();

		inline operator vk::Instance() { return m_instance; }

		inline operator VkInstance() { return *m_instance; }
	};
} // namespace fgl::engine