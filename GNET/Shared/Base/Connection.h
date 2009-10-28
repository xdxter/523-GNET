#include "GNET_Types.h"
#include <time.h>

#pragma once
	
namespace GNET {
	class Peer;

	class Connection {
	private:
		Peer* peer;
		int seq_num_out;
	
		enum {
			NOT_CONNECTED,
			WAITING_FOR_SYN,
			WAITING_FOR_ACK,
			CONNECTED
		};
		int connect_state;
		clock_t connect_timeout;

	public:
		SOCKADDR_IN remote;

		Connection(SOCKADDR_IN remote, Peer* peer);
		void TryConnect();
		void Update();
		void HandlePacket(Datagram *data);
		int Seq_Num();
	};
	
}
