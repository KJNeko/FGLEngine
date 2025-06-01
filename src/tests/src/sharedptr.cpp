//
// Created by kj16609 on 5/8/25.
//

#include "../../engine/memory/swappable/enable_swappable_from_this.hpp"
#include "catch2/catch_test_macros.hpp"
#include "memory/swappable/FrozenSharedPtr.hpp"
#include "memory/swappable/SwappableSharedPtr.hpp"

using namespace fgl::engine;

struct ExampleObject : public enable_swappable_from_this< ExampleObject >
{
	inline static int uid_c { 0 };
	int uid { ++uid_c };

	ExampleObject() = default;
	~ExampleObject() = default;
};

TEST_CASE( "SharedPtr", "[shared_ptr]" )
{
	SECTION( "Throws on accessing invalidated pointer" )
	{
		ExampleObject object {};

		try
		{
			volatile auto ptr { object.swappable_from_this() };
			FAIL( "No exception" );
		}
		catch ( ... )
		{
			SUCCEED( "" );
		}
	}

	SECTION( "Swappable pointers can be shared via ctor" )
	{
		SwappableSharedPtr< ExampleObject > object1 {};
		SwappableSharedPtr< ExampleObject > object2 { object1 };

		REQUIRE( object1.get() == object2.get() );
	}

	SECTION( "Swappable pointers constructed seperately are not the same" )
	{
		SwappableSharedPtr< ExampleObject > object1 {};
		SwappableSharedPtr< ExampleObject > object2 {};

		REQUIRE( object1.get() != object2.get() );
	}

	SECTION( "Frozen pointers point to swapped objects" )
	{
		SwappableSharedPtr< ExampleObject > object1 {};
		SwappableSharedPtr< ExampleObject > object2 { object1 };

		WHEN( "Locked" )
		{
			auto locked { object1.freeze() };

			AND_THEN( "The swappable pointer is not swapped" )
			{
				THEN( "Locked should point to the same object" )
				{
					REQUIRE( locked.get() == object1.get() );
				}
			}

			AND_THEN( "The swappable pointer is swapped" )
			{
				auto old_object { object1.swapNew() };

				THEN( "Locked should not point to the new object" )
				{
					REQUIRE( locked.get() != object1.get() );
				}

				THEN( "Locked should point to the old object" )
				{
					REQUIRE( locked.get() == old_object.get() );
				}
			}
		}
	}

	SECTION( "A frozen pointer is created from a swappable pointer" )
	{
		SwappableSharedPtr< ExampleObject > object {};

		WHEN( "Freezing the swappable pointer" )
		{
			FrozenSharedPtr< ExampleObject > frozen_object { object.freeze() };

			AND_WHEN( "The swappable pointer is NOT swapped" )
			{
				THEN( "Frozen pointers should point to the same object" )
				{
					REQUIRE( frozen_object.get() == object.get() );
				}

				AND_THEN( "When we defrost the frozen pointer" )
				{
					SwappableSharedPtr< ExampleObject > defrosted_object { frozen_object.defrost() };

					THEN( "Defrosted pointers should match the original swappable if defrosted" )
					{
						REQUIRE( object.get() == defrosted_object.get() );
					}
				}
			}
		}
	}

	SECTION( "Swappable pointer deleted after locking" )
	{
		GIVEN( "A swappable pointer" )
		{
			SwappableSharedPtr< ExampleObject >* object { new SwappableSharedPtr< ExampleObject > {} };

			AND_WHEN( "A frozen pointer is created from the swappable pointer" )
			{
				auto frozen_object { object->freeze() };

				AND_WHEN( "The swappable pointer is destroyed" )
				{
					delete object;

					THEN( "The frozen pointer should still point to the object" )
					{
						REQUIRE( frozen_object.get() != nullptr );
					}

					THEN( "Defrosting should result in a new container set" )
					{
						auto defrosted_object { frozen_object.defrost() };

						REQUIRE( defrosted_object.get() != nullptr );
						REQUIRE( defrosted_object.use_count() == 1 );

						AND_THEN( "If defrosted again, The second container should match the first" )
						{
							auto defrosted_object_again { frozen_object.defrost() };

							REQUIRE( defrosted_object.use_count() == 2 );
							REQUIRE( defrosted_object_again.use_count() == 2 );
						}
					}
				}
			}
		}
	}
}
