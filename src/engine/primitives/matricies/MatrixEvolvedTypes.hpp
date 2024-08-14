//
// Created by kj16609 on 2/22/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl::engine
{

	enum class MatrixType
	{
		InvalidMatrix,
		ModelToWorld,

		WorldToCamera,
		CameraToScreen,

		//WorldToScreen is two combined matricies (WorldToView and ViewToScreen)
		WorldToScreen
	};

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
