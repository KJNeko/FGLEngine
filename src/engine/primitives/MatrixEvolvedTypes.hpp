//
// Created by kj16609 on 2/22/24.
//

#pragma once

#include "CoordinateSpace.hpp"

namespace fgl::engine
{

	template < MatrixType MType >
	consteval CoordinateSpace EvolvedType();

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::ModelToWorld >()
	{
		return CoordinateSpace::World;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::WorldToCamera >()
	{
		return CoordinateSpace::Camera;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::CameraToScreen >()
	{
		return CoordinateSpace::Screen;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::WorldToScreen >()
	{
		return CoordinateSpace::Screen;
	}

} // namespace fgl::engine
