#include "GNET_Types.h"

#include "Base/Timer.h"
#include "Base/Turnkey.h"

#include "Connections/Parts/Heartbeat.h"
#include "Connections/Parts/ConnectProtocol.h"
#include "Connections/Parts/ReliableUdp.h"


#pragma once
	
namespace GNET {
	class Peer;

	class Connection {	
	public:
		SOCKADDR_IN remote;

		Connection(SOCKADDR_IN remote, Peer* peer);
		bool Update();
		bool HandlePacket(Datagram *data);

		void TryConnecting(int max_attempts, int ms_delay, Turnkey<bool> *key);
		int Seq_Num();

		//temporary design
		inline void TrackRudpPacket(DataPack * pack, SOCKADDR_IN * sock){ rudpTracker.AddOutgoingPack(pack, sock);}

	protected:
		friend Heartbeat;
		friend ConnectProtocol;
		friend ReliableTracker;

		void Disconnect();

	private:
		Peer* peer;	

		Heartbeat heartbeat;
		ConnectProtocol connectprotocol;
		ReliableTracker rudpTracker;

		int seq_num_out;
		bool should_disconnect;
	};
	
}
