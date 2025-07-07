//
// Created by kj16609 on 8/4/24.
//

#pragma once

namespace fgl::engine
{

	inline bool operator==( const NormalVector& left, const NormalVector& right )
	{
		const glm::vec3 high_range { left.vec() + glm::vec3( std::numeric_limits< float >::epsilon() ) };
		const glm::vec3 low_range { left.vec() - glm::vec3( std::numeric_limits< float >::epsilon() ) };

		const auto within_high { glm::lessThanEqual( right.vec(), high_range ) };
		const auto within_low { glm::greaterThanEqual( right.vec(), low_range ) };

		return glm::all( within_high ) && glm::all( within_low );
	}

	inline bool operator==( const Vector& left, const Vector& right )
	{
		const glm::vec3 high_range { left.vec() + glm::vec3( std::numeric_limits< float >::epsilon() ) };
		const glm::vec3 low_range { left.vec() - glm::vec3( std::numeric_limits< float >::epsilon() ) };

		const auto within_high { glm::lessThanEqual( right.vec(), high_range ) };
		const auto within_low { glm::greaterThanEqual( right.vec(), low_range ) };

		return glm::all( within_high ) && glm::all( within_low );
	}

} // namespace fgl::engine
