//
// Created by kj16609 on 10/29/24.
//

#include "FlameGraph.hpp"

#include <cassert>
#include <imgui.h>

#include "engine/FGL_DEFINES.hpp"
#include "engine/clock.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::debug
{

	struct Node
	{
		std::string_view name { "" };
		profiling_clock::time_point start {};
		profiling_clock::time_point end {};
		std::vector< Node > children {};
		Node* parent { nullptr };
		void drawImGui() const;

		using duration = profiling_clock::duration;

		duration getDuration() const { return end - start; }

		duration getTotalTime() const
		{
			if ( parent != nullptr )
				return parent->getTotalTime();
			else
				return getDuration();
		}

		Node() = default;

		FGL_DELETE_COPY( Node );

		Node( Node&& other ) :
		  name( std::move( other.name ) ),
		  start( other.start ),
		  end( other.end ),
		  children( std::move( other.children ) ),
		  parent( other.parent )
		{
			for ( auto& child : children ) child.parent = this;
		}

		Node& operator=( Node&& other ) noexcept
		{
			name = std::move( other.name );
			start = other.start;
			end = other.end;
			children = std::move( other.children );

			for ( auto& child : children ) child.parent = this;

			parent = other.parent;
			return *this;
		}
	};

	//! If true then the percentage will be of the total frame time instead of a percentage of the parent time
	inline static bool percent_as_total { true };

	void Node::drawImGui() const
	{
		const auto diff { end - start };

		FGL_ASSERT( end > start, "Node ended before it began!" );

		const auto time { getDuration() };

		// Total runtime of the frame

		double percent { 100.0f };

		if ( percent_as_total )
		{
			const auto total_time { getTotalTime() };
			percent = ( static_cast< double >( time.count() ) / static_cast< double >( total_time.count() ) ) * 100.0f;
		}
		else if ( parent )
		{
			const auto parent_time { this->parent->getDuration() };
			percent = ( static_cast< double >( time.count() ) / static_cast< double >( parent_time.count() ) ) * 100.0f;
		}

		const std::string str { std::format(
			"{} -- {:2.2f}ms, ({:2.2f}%)",
			name,
			( std::chrono::duration_cast< std::chrono::microseconds >( diff ).count() / 1000.0f ),
			percent ) };

		ImGuiTreeNodeFlags flags { ImGuiTreeNodeFlags_None };

		if ( children.empty() ) flags |= ImGuiTreeNodeFlags_Leaf;

		if ( ImGui::TreeNodeEx( name.data(), flags, str.c_str() ) )
		{
			for ( const auto& child : children ) child.drawImGui();

			ImGui::TreePop();
		}
	}

	inline static Node previous_root {};
	inline static Node root {};
	inline static Node* active { &root };

	//TODO: Noop most of this so it won't hurt performance later when using a specific define set.

	namespace timing
	{
		void reset()
		{
			previous_root = std::move( root );
			root.name = "Update Time";
			root.children.clear();
			root.start = profiling_clock::now();
			active = &root;
		}

		ScopedTimer push( const std::string_view name )
		{
			Node new_node {};
			new_node.name = name;
			new_node.parent = active;
			new_node.start = profiling_clock::now();
			assert( active );
			active->children.emplace_back( std::move( new_node ) );
			active = &active->children.back();
			return {};
		}

		namespace internal
		{
			void pop()
			{
				auto getDepth = [ & ]() -> std::size_t
				{
					const Node* current { active };
					std::size_t depth { 0 };
					while ( current != nullptr )
					{
						current = current->parent;
						++depth;
					}
					return depth;
				};

				std::string padding { "" };

				for ( std::size_t i = 0; i < getDepth(); i++ )
				{
					padding += "\t";
				}

				FGL_ASSERT( active, "Active node in framegraph was null!" );
				active->end = profiling_clock::now();

				const auto diff { active->end - active->start };

				FGL_ASSERT( diff >= decltype( diff ) { 0 }, "Popped node ended before it began!" );
				FGL_ASSERT( active->end > active->start, "Node ended before it began!" );

				active = active->parent;
			}
		} // namespace internal

		void render()
		{
			ImGui::Checkbox( "Percentage of frame time", &percent_as_total );
			ImGui::SameLine();
			ImGui::TextDisabled( "(?)" );
			if ( ImGui::BeginItemTooltip() )
			{
				ImGui::TextUnformatted(
					"Changes the percentage output to be of the total frame time instead of the parent time" );
				ImGui::EndTooltip();
			}

			previous_root.drawImGui();
		}
	} // namespace timing

} // namespace fgl::engine::debug