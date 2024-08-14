//
// Created by kj16609 on 2/22/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"

namespace fgl::engine
{

	enum class MatrixType
	{
		//! Can come from Transform with `Model` coordinate space.
		//! Simply used to allow it's template to be compiled without it eating shit.
		InvalidMatrix,
		ModelToWorld,

		WorldToCamera,
		CameraToScreen,

		//WorldToScreen is two combined matricies (WorldToView and ViewToScreen)
		WorldToScreen
	};

	//! Returns the 'Evolved' type for a given MType (ModelToWorld returns World)
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

	//! Returns the 'Devolved' type for a given MType (ModelToWorld returns Model)
	template < MatrixType MType >
	consteval CoordinateSpace DevolvedType();

	template <>
	consteval CoordinateSpace DevolvedType< MatrixType::ModelToWorld >()
	{
		return CoordinateSpace::Model;
	}

	template <>
	consteval CoordinateSpace DevolvedType< MatrixType::WorldToCamera >()
	{
		return CoordinateSpace::World;
	}

	template <>
	consteval CoordinateSpace DevolvedType< MatrixType::CameraToScreen >()
	{
		return CoordinateSpace::Camera;
	}

	template <>
	consteval CoordinateSpace DevolvedType< MatrixType::WorldToScreen >()
	{
		return CoordinateSpace::World;
	}

	//! Checks if CType can be evolved by MType. (The input type must match the input matrix type)
	template < CoordinateSpace CType, MatrixType MType >
	concept can_be_evolved = requires() {
		{
			DevolvedType< MType >() == CType
		};
	};

} // namespace fgl::engine
