#include "GNET_Types.h"
#include <time.h>

#pragma once
	
namespace GNET {
	class Peer;

	class Connection {	
	public:
		SOCKADDR_IN remote;

		Connection(SOCKADDR_IN remote, Peer* peer);
		void Update();
		void HandlePacket(Datagram *data);
		int Seq_Num();

		void TryConnecting(int max_attempts = 7, int ms_delay = 500);

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

		bool is_instigator;
		int max_attempts;
		int ms_delay;

		void Handshake(int i);
	};
	
}
