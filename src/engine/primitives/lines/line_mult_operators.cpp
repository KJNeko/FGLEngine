//
// Created by kj16609 on 2/28/24.
//

#include "InfiniteLine.hpp"
#include "LineSegment.hpp"

namespace fgl::engine
{

	template <>
	LineSegment< EvolvedType< MatrixType::ModelToWorld >() >
		operator*( const Matrix< MatrixType::ModelToWorld > mat, const LineSegment< CoordinateSpace::Model > line )
	{
		return LineSegment<
			EvolvedType< MatrixType::ModelToWorld >() >( mat * line.getPosition(), mat * line.getEnd() );
	}

	template class LineSegment< CoordinateSpace::Model >;
	template class LineSegment< CoordinateSpace::World >;

	template class InfiniteLine< CoordinateSpace::Model >;
	template class InfiniteLine< CoordinateSpace::World >;
} // namespace fgl::engine
