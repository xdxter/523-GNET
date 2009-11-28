#include "Base/Timer.h"
#include "Base/Turnkey.h"
#pragma once;

namespace GNET {
	class Peer;
	class Connection;
	struct Datagram;

	///A class which handles the connection establishment following a particular protocol.
	class ConnectProtocol {
	public:

		///Constructor
		ConnectProtocol(int ms_delay, Peer* peer, Connection* connection);

		///A function which does the handshaking and sets the appropriate connection state.
		///e.g.stste is WAITING_FOR_SYN
		void TryConnecting(int max_attempts, int ms_delay, Turnkey<bool> *key);

		///A function which receives a Connect, SYN or a ACK packet.
		bool HandlePacket(Datagram *data);

		///Updates the current Connection state and decides if disconnection or resending of some requests is necessary.
		void Update();

	private: 
		Peer* peer;
		Connection* connection;
	
		///A boolean which is true/false according to the connection phase.
		///Is set true when a client is in the Connection phase.
		bool is_instigator;
		
		///Keeps track of the number of Connection attempts.
		int attempts;

		///Keeps track of the state in the Connection phase.
		int state;
		Timer timer;

		///Timer Variables
		int ms_delay,
			max_attempts;

		Turnkey<bool> *key;

		///A function which performs the handshaking.
		void Handshake(char mode);
	};
}