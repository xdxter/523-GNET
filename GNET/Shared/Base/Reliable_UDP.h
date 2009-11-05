#include "GNET_Types.h"
#include "Timer.h"

#pragma once
namespace GNET {

	class Peer;

	class ReliableTracker {	
	public:

	private:
		ReliableTable in;
		ReliableTable out;
	};
	
}