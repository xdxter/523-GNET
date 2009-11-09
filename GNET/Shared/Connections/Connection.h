#include "GNET_Types.h"

#include "Base/Timer.h"
#include "Base/Turnkey.h"

#include "Connections/Parts/Heartbeat.h"
#include "Connections/Parts/ConnectProtocol.h"


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

	protected:
		friend Heartbeat;
		friend ConnectProtocol;

		void Disconnect();

	private:
		Peer* peer;	

		Heartbeat heartbeat;
		ConnectProtocol connectprotocol;

		int seq_num_out;
		bool should_disconnect;
	};
	
}
