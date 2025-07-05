//
// Created by kj16609 on 7/5/25.
//
#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#pragma GCC diagnostic pop

#include "rendering/RenderingFormats.hpp"

namespace fgl::engine
{

	template < typename T >
	consteval vk::Format format()
	{
		if constexpr ( std::same_as< T, glm::vec2 > ) // || std::same_as< T, glm::mat2 > )
		{
			return vk::Format::eR32G32Sfloat;
		}
		else if constexpr ( std::same_as< T, glm::vec3 > || std::same_as< T, glm::mat3 > )
		{
			return vk::Format::eR32G32B32Sfloat;
		}
		else if constexpr ( std::same_as< T, glm::vec4 > || std ::same_as< T, glm::mat4 > )
		{
			return vk::Format::eR32G32B32A32Sfloat;
		}
		else if constexpr ( std::same_as< T, std::uint32_t > )
		{
			return vk::Format::eR32Uint;
		}
		else
		{
			static_assert( false, "Format for type T not given" );
			return vk::Format::eUndefined;
		}
	}

	class AttributeBuilder
	{
		std::vector< vk::VertexInputAttributeDescription > m_attributes {};

		std::size_t m_location_counter { 0 };

	  public:

		AttributeBuilder() = default;

		AttributeBuilder( std::vector< vk::VertexInputAttributeDescription >&& attributes ) : m_attributes( attributes )
		{
			m_location_counter = attributes.size();
		}

		template < typename T, std::size_t offset >
		void add( std::size_t binding )
		{
			if constexpr ( std::same_as< glm::mat4, T > )
			{
				for ( std::size_t i = 0; i < 4; ++i )
					m_attributes.emplace_back(
						m_location_counter++,
						binding,
						format< T >(),
						offset + ( i * sizeof( glm::vec4 ) ) ); // size vec4, Since each attribute is one row
			}
			else if constexpr ( std::same_as< T, glm::mat3 > )
			{
				for ( std::size_t i = 0; i < 3; ++i )
					m_attributes.emplace_back(
						m_location_counter++,
						binding,
						format< T >(),
						offset + ( i * sizeof( glm::vec3 ) ) ); // size vec3, since each attribtue is one row
			}
			else
			{
				m_attributes.emplace_back( m_location_counter++, binding, format< T >(), offset );
			}
		}

		[[nodiscard]] auto get() const { return m_attributes; }
	};

} // namespace fgl::engine
