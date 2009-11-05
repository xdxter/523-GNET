#include "GNET_Types.h"
#include "Timer.h"

#pragma once

namespace GNET {

#define RUDP_TIMEOUT 10000
	struct RudpItem{
		Datagram * dat;
		Timer resend_timer;
	};

	class ReliableTracker {	
	public:
		ReliableTracker(Peer * _peer) {
			peer = _peer;
		}
		~ReliableTracker(){}
		void Update();
		void AddOutgoingPack(DataPack * pack, SOCKADDR_IN * remote);
	private:
		ReliableTable out;
		ReliableTableIter it;
		Peer * peer;
	};
	
}