#include "GNET_Types.h"
#include "Timer.h"

#pragma once

namespace GNET {

#define RUDP_TIMEOUT 1000
#define RUDP_ID_RESERVATION_TIMEOUT 60000	//id can't be reused in one minute
	struct RudpItem{
		bool sender;
		Datagram * dat;
		Timer timer;//used for both resend and ID reservation
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
		ReliableTable in;
		ReliableTableIter it;
		Peer * peer;
	};
	
}