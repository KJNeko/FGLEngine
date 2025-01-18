//
// Created by kj16609 on 10/29/24.
//

#include "FlameGraph.hpp"

#include <cassert>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include <imgui.h>
#pragma GCC diagnostic pop

#include "engine/FGL_DEFINES.hpp"
#include "engine/clock.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::debug
{

	struct Node
	{
		std::string_view m_name { "" };
		ProfilingClock::time_point m_start {};
		ProfilingClock::time_point m_end {};
		std::vector< Node > m_children {};
		Node* m_parent { nullptr };
		void drawImGui() const;

		using duration = ProfilingClock::duration;

		duration getDuration() const { return m_end - m_start; }

		duration getTotalTime() const
		{
			if ( m_parent != nullptr )
				return m_parent->getTotalTime();
			else
				return getDuration();
		}

		Node() = default;

		FGL_DELETE_COPY( Node );

		Node( Node&& other ) noexcept :
		  m_name( std::move( other.m_name ) ),
		  m_start( other.m_start ),
		  m_end( other.m_end ),
		  m_children( std::move( other.m_children ) ),
		  m_parent( other.m_parent )
		{
			for ( auto& child : m_children ) child.m_parent = this;
		}

		Node& operator=( Node&& other ) noexcept
		{
			m_name = std::move( other.m_name );
			m_start = other.m_start;
			m_end = other.m_end;
			m_children = std::move( other.m_children );

			for ( auto& child : m_children ) child.m_parent = this;

			m_parent = other.m_parent;
			return *this;
		}
	};

	//! If true then the percentage will be of the total frame time instead of a percentage of the parent time
	inline static bool percent_as_total { true };

	void Node::drawImGui() const
	{
		const auto diff { m_end - m_start };

		FGL_ASSERT( m_end > m_start, "Node ended before it began!" );

		const auto time { getDuration() };

		// Total runtime of the frame

		double percent { 100.0f };

		if ( percent_as_total )
		{
			const auto total_time { getTotalTime() };
			percent = ( static_cast< double >( time.count() ) / static_cast< double >( total_time.count() ) ) * 100.0;
		}
		else if ( m_parent != nullptr )
		{
			const auto parent_time { this->m_parent->getDuration() };
			percent = ( static_cast< double >( time.count() ) / static_cast< double >( parent_time.count() ) ) * 100.0;
		}

		const std::string str { std::format(
			"{} -- {:2.2f}ms, ({:2.2f}%)",
			m_name,
			( static_cast< double >( std::chrono::duration_cast< std::chrono::microseconds >( diff ).count() )
			  / 1000.0 ),
			percent ) };

		ImGuiTreeNodeFlags flags { ImGuiTreeNodeFlags_None };

		if ( m_children.empty() ) flags |= ImGuiTreeNodeFlags_Leaf;

		if ( ImGui::TreeNodeEx( m_name.data(), flags, str.c_str() ) )
		{
			for ( const auto& child : m_children ) child.drawImGui();

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
			root.m_name = "Update Time";
			root.m_children.clear();
			root.m_start = ProfilingClock::now();
			active = &root;
		}

		ScopedTimer push( const std::string_view name )
		{
			Node new_node {};
			new_node.m_name = name;
			new_node.m_parent = active;
			new_node.m_start = ProfilingClock::now();
			assert( active );
			active->m_children.emplace_back( std::move( new_node ) );
			active = &active->m_children.back();
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
						current = current->m_parent;
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
				active->m_end = ProfilingClock::now();

				const auto diff { active->m_end - active->m_start };

				FGL_ASSERT( diff >= decltype( diff ) { 0 }, "Popped node ended before it began!" );
				FGL_ASSERT( active->m_end > active->m_start, "Node ended before it began!" );

				active = active->m_parent;
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