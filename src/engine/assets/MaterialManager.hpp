//
// Created by kj16609 on 1/23/25.
//
#pragma once

#include "engine/memory/buffers/BufferHandle.hpp"
#include "material/Material.hpp"
#include "memory/buffers/vector/DeviceVector.hpp"

namespace fgl::engine
{

	class MaterialManager
	{
		memory::Buffer m_material_data_pool;

		DeviceVector< DeviceMaterialData > m_material_data;

		friend class Material;

		void update( std::size_t idx, const DeviceMaterialData& material_data );

	  public:

		MaterialManager();
		~MaterialManager();

		[[nodiscard]] memory::Buffer& getBuffer();
		[[nodiscard]] memory::BufferSuballocation& getBufferSuballocation();
		[[nodiscard]] DeviceVector< DeviceMaterialData >& getMaterialData();
	};
} // namespace fgl::engine