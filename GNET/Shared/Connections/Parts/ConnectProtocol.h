#include "Base/Timer.h"
#include "Base/Turnkey.h"
#pragma once;

namespace GNET {
	class Peer;
	class Connection;
	struct Datagram;

	class ConnectProtocol {
	public:
		ConnectProtocol(int ms_delay, Peer* peer, Connection* connection);
		void TryConnecting(int max_attempts, int ms_delay, Turnkey<bool> *key);
		bool HandlePacket(Datagram *data);
		void Update();

	private: 
		Peer* peer;
		Connection* connection;
	
		bool is_instigator;
		int attempts;
		int state;
		Timer timer;

		int ms_delay,
			max_attempts;

		Turnkey<bool> *key;

		void Handshake(char mode);
	};
}