//
// Created by kj16609 on 6/20/24.
//

#include "Instance.hpp"

#include <GLFW/glfw3.h>

#include <unordered_set>

#include "engine/logging/logging.hpp"

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT { nullptr };
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT { nullptr };
PFN_vkSetDebugUtilsObjectNameEXT pfnVkSetDebugUtilsObjectNameEXT { nullptr };

// Callback function for vulkan messaging.
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	[[maybe_unused]] void* pUserData )
{
	using Bits = VkDebugUtilsMessageSeverityFlagBitsEXT;
	using namespace fgl::engine;

	if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
	{
		log::info( pCallbackData->pMessage );
	}
	else if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
	{
		log::warn( pCallbackData->pMessage );
	}
	else if ( pCallbackData->flags & Bits::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
	{
		log::error( pCallbackData->pMessage );
		std::abort();
	}
	else
	{
		//log::critical( "Unknown severity message: {}", pCallbackData->pMessage );
		//std::abort();
	}

	return VK_FALSE;
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* create_info,
	const VkAllocationCallbacks* allocator,
	VkDebugUtilsMessengerEXT* messenger )
{
	assert( pfnVkCreateDebugUtilsMessengerEXT );
	return pfnVkCreateDebugUtilsMessengerEXT( instance, create_info, allocator, messenger );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const * pAllocator )
{
	assert( pfnVkDestroyDebugUtilsMessengerEXT );
	return pfnVkDestroyDebugUtilsMessengerEXT( instance, messenger, pAllocator );
}

VKAPI_ATTR VkResult VKAPI_CALL
	vkSetDebugUtilsObjectNameEXT( VkDevice device, const VkDebugUtilsObjectNameInfoEXT* nameInfo )
{
	assert( pfnVkSetDebugUtilsObjectNameEXT );
	return pfnVkSetDebugUtilsObjectNameEXT( device, nameInfo );
}

namespace fgl::engine
{
	vk::Result CreateDebugUtilsMessengerEXT(
		vk::Instance instance,
		const vk::DebugUtilsMessengerCreateInfoEXT& pCreateInfo,
		[[maybe_unused]] const vk::AllocationCallbacks* pAllocator,
		vk::DebugUtilsMessengerEXT* pDebugMessenger )
	{
		return instance.createDebugUtilsMessengerEXT( &pCreateInfo, pAllocator, pDebugMessenger );
	}

	void DestroyDebugUtilsMessengerEXT(
		vk::Instance instance,
		vk::DebugUtilsMessengerEXT debugMessenger,
		[[maybe_unused]] const vk::AllocationCallbacks* pAllocator )
	{
		instance.destroyDebugUtilsMessengerEXT( debugMessenger );
	}

	vk::ApplicationInfo Instance::appInfo()
	{
		vk::ApplicationInfo info {};
		info.pApplicationName = "Mecha Game";
		info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );

		info.pEngineName = "titor";
		info.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );

		info.apiVersion = VK_API_VERSION_1_3;

		return info;
	}

	vk::InstanceCreateInfo Instance::
		createInfo( vk::ApplicationInfo* app_info, vk::DebugUtilsMessengerCreateInfoEXT* msger_info )
	{
		vk::InstanceCreateInfo info {};
		info.pApplicationInfo = app_info;

		assert( m_required_extensions.size() >= 1 );
		info.enabledExtensionCount = static_cast< uint32_t >( m_required_extensions.size() );
		info.ppEnabledExtensionNames = m_required_extensions.data();

		verifyGlfwExtensions();

		const static std::vector< const char* > validationLayers { "VK_LAYER_KHRONOS_validation" };

		if ( ENABLE_VALIDATION_LAYERS )
		{
			info.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
			info.ppEnabledLayerNames = validationLayers.data();

			info.pNext = msger_info;
		}
		else
		{
			info.enabledLayerCount = 0;
			info.pNext = nullptr;
		}

		return info;
	}

	std::vector< const char* > Instance::getRequiredInstanceExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions { glfwGetRequiredInstanceExtensions( &glfwExtensionCount ) };

		if ( glfwExtensions == nullptr ) throw std::runtime_error( "Failed to get required extensions from glfw" );

		std::vector< const char* > extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

		// "VK_KHR_surface" is guaranteed to be in this list
		assert( extensions.size() >= 1 );

		if ( ENABLE_VALIDATION_LAYERS )
		{
			extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
		}

		return extensions;
	}

	vk::DebugUtilsMessengerCreateInfoEXT Instance::createDebugMessengerInfo()
	{
		vk::DebugUtilsMessengerCreateInfoEXT create_info {};
		create_info.messageSeverity =
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		create_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		                        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
		                        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
		create_info.pfnUserCallback = debugCallback;
		create_info.pUserData = nullptr; // Optional

		return create_info;
	}

	void Instance::verifyGlfwExtensions()
	{
		std::vector< vk::ExtensionProperties > instance_extensions { vk::enumerateInstanceExtensionProperties() };

		std::cout << "available instance instance_extensions:" << std::endl;
		std::unordered_set< std::string > available {};
		//Get list of all available instance extensions
		for ( const auto& extension : instance_extensions )
		{
			std::cout << "\t" << extension.extensionName << std::endl;
			available.insert( extension.extensionName );
		}

		std::cout << "required instance instance_extensions:" << std::endl;
		//Get the list of the required extensions

		auto requiredExtensions { getRequiredInstanceExtensions() };
		for ( const char* required : requiredExtensions )
		{
			if ( std::find_if(
					 instance_extensions.begin(),
					 instance_extensions.end(),
					 [ required ]( const vk::ExtensionProperties& prop )
					 { return std::strcmp( prop.extensionName, required ); } )
			     == instance_extensions.end() )
			{
				log::critical( "Failed to find required extention: {}", std::string_view( required ) );
				throw std::runtime_error( "Failed to find required extention: " );
			}
			else
				std::cout << required << std::endl;
		}
	}

	void Instance::setupDebugMessenger()
	{
		if ( !ENABLE_VALIDATION_LAYERS )
		{
			log::info( "Validation disabled. Skipping debug messenger" );
			return;
		}

		log::info( "Setting up debug messenger" );

		pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<
			PFN_vkCreateDebugUtilsMessengerEXT >( m_instance.getProcAddr( "vkCreateDebugUtilsMessengerEXT" ) );
		pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<
			PFN_vkDestroyDebugUtilsMessengerEXT >( m_instance.getProcAddr( "vkDestroyDebugUtilsMessengerEXT" ) );
		pfnVkSetDebugUtilsObjectNameEXT =
			reinterpret_cast< PFN_vkSetDebugUtilsObjectNameEXT >( m_instance
		                                                              .getProcAddr( "vkSetDebugUtilsObjectNameEXT" ) );

		if ( !pfnVkCreateDebugUtilsMessengerEXT || !pfnVkDestroyDebugUtilsMessengerEXT )
		{
			log::critical(
				"Failed to load create/destroy messenger functions pfnVkCreateDebugUtilsMessengerEXT/pfnVkDestroyDebugUtilsMessengerEXT" );
			throw std::runtime_error( "failed to load debug messenger functions!" );
		}

		if ( !pfnVkSetDebugUtilsObjectNameEXT )
		{
			log::critical( "Failed to load debug object naming function: pfnVkSetDebugUtilsObjectNameEXT" );
			throw std::runtime_error( "failed to load debug object name function!" );
		}

		if ( CreateDebugUtilsMessengerEXT( m_instance, m_debug_info, nullptr, &m_debug_messenger )
		     != vk::Result::eSuccess )
		{
			throw std::runtime_error( "failed to set up debug messenger!" );
		}

		log::info( "Debug callback setup" );
	}

	Instance::Instance( vk::raii::Context& ctx ) :
	  m_app_info( appInfo() ),
	  m_debug_info( createDebugMessengerInfo() ),
	  m_required_extensions( getRequiredInstanceExtensions() ),
	  m_instance_info( createInfo( &m_app_info, &m_debug_info ) ),
	  m_instance( ctx, m_instance_info )
	{
		//Post setup.
		setupDebugMessenger();
	}

	Instance::~Instance()
	{
		if ( ENABLE_VALIDATION_LAYERS )
		{
			DestroyDebugUtilsMessengerEXT( m_instance, m_debug_messenger, nullptr );
		}
	}

} // namespace fgl::engine
