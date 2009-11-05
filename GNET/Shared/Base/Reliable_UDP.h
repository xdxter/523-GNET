#include "GNET_Types.h"
#include "Timer.h"

#pragma once

namespace GNET {

#define RUDP_TIMEOUT 1000
	struct RudpItem{
		bool sender;
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
		void HandlePacket(Datagram * dat);
	private:
		ReliableTable out;
		ReliableTableIter it;
		Peer * peer;
	};
	
}