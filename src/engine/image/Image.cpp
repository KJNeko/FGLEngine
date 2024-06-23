//
// Created by kj16609 on 12/4/23.
//

#include "Image.hpp"

#include "ImageView.hpp"

namespace fgl::engine
{

	std::shared_ptr< ImageView > Image::getView()
	{
		if ( !view.expired() )
			return view.lock();
		else
		{
			auto ptr { std::make_shared< ImageView >( m_handle ) };
			view = ptr;
			return ptr;
		}
	}

	Image& Image::setName( const std::string str )
	{
		m_handle->setName( str );
		return *this;
	}

} // namespace fgl::engine