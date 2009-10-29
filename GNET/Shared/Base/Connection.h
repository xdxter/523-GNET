#include "GNET_Types.h"
#include "Timer.h"

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
	
		int connect_state;
		Timer connect_timer;

		bool is_instigator;
		int attempts;
		int ms_delay;

		void Handshake(int i);
	};
	
}
