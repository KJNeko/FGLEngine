//
// Created by kj16609 on 10/12/24.
//

#include <cstdint>

#include "engine/descriptors/Descriptor.hpp"

namespace fgl::engine
{

	class UBOBuffer
	{
		// descriptors::Descriptor descriptor;

	  public:

		UBOBuffer( std::size_t reserve_size, std::size_t TSize );

		//! Updates a index with the given `data` pointer. `data + TSize` must be valid
		void updateData( std::size_t idx, void* data );
	};

} // namespace fgl::engine
