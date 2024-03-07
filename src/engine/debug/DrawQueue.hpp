//
// Created by kj16609 on 3/6/24.
//

#pragma once

namespace fgl::debug
{



	class DrawQueue
	{
		std::thread draw_consumer;

		//TODO: Replace with ring buffer
		std::mutex mtx;
		std::queue<  >



	};


}
