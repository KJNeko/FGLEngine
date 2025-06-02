//
// Created by kj16609 on 6/2/25.
//
#include "Instance.hpp"

#include "GLFW/glfw3.h"
#include "debug/logging.hpp"

namespace fgl::engine
{

	// Callback function for vulkan messaging.
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
		const vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
		[[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT message_type,
		const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data,
		[[maybe_unused]] void* p_user_data )
	{
		using namespace fgl::engine;

		switch ( message_severity )
		{
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
				// log::debug( p_callback_data->pMessage );
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
				// log::info( p_callback_data->pMessage );
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
				// log::warn( p_callback_data->pMessage );
				break;
			case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
				// log::error( p_callback_data->pMessage );
				break;
			default:
				{
					// log::critical( "Unknown severity from debug callback: {}", p_callback_data->pMessage );
					throw std::runtime_error { "Unknown severity from debug callback" };
				}
		}

		return VK_FALSE;
	}

	Instance::InfoObject::InfoObject() : m_extensions( required_extensions ), m_layers( required_layers )
	{
		m_app_info.pApplicationName = "FGL Engine";
		m_app_info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );

		m_app_info.pEngineName = "Titor Engine";
		m_app_info.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );

		m_app_info.apiVersion = VK_API_VERSION_1_4;

		m_debug_create_info.setMessageSeverity(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError );

		m_debug_create_info.setMessageType(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance );

		m_debug_create_info.setPfnUserCallback( debugCallback );

		m_create_info.setPApplicationInfo( &m_app_info );

		m_create_info.setEnabledLayerCount( m_layers.size() );
		m_create_info.setPpEnabledLayerNames( m_layers.data() );

		// add any extra extensions we need for the window (TODO: Take this directly from `Window`)
		std::uint32_t glfw_ext_count { 0 };
		// Don't free these, As GLFW manages them
		const char** glfw_exts { glfwGetRequiredInstanceExtensions( &glfw_ext_count ) };

		if ( glfw_ext_count == 0 )
		{
			const char* error = nullptr;
			glfwGetError( &error );
			throw std::runtime_error { std::string( "Failed to get required extensions for GLFW: " )
				                       + ( error ? error : "Unknown error" ) };
		}

		m_extensions.insert( m_extensions.end(), glfw_exts, glfw_exts + glfw_ext_count );

		log::verbose( "Extensions: {}", m_extensions );

		m_create_info.setEnabledExtensionCount( m_extensions.size() );
		m_create_info.setPpEnabledExtensionNames( m_extensions.data() );

		m_create_info.setPNext( &m_debug_create_info );
	}

	Instance::Instance( const vk::raii::Context& vk_ctx ) : m_vk_instance( vk_ctx, InfoObject().m_create_info )
	{}

	Instance::~Instance()
	{}
} // namespace fgl::engine