#include "GNET_Types.h"

#include "Base/Timer.h"
#include "Base/Turnkey.h"

#include "Connections/Parts/Heartbeat.h"
#include "Connections/Parts/ConnectProtocol.h"
#include "Connections/Parts/ReliableUdp.h"
#include "Connections/Parts/SequencedUdp.h"

#pragma once
	
namespace GNET {
	class Peer;

	/// A class which handles everything related to a connection; 
	/// it also Handles the data packets for Heartbeat, Reliable 
	/// UDP, Sequenced UDP, and the connection protocol
	class Connection {	
	public:
		///The remote SOCKADDR_IN
		SOCKADDR_IN remote;

		///Constructor
		Connection(SOCKADDR_IN remote, Peer* peer);

		///This function updates the connection protocol, reliable udp and heartbeat.
		bool Update();

		///This function adds a sequence number to the (Relaible UDP) data packet.
		bool SendingPacket(Datagram * dgram);

		///Handles the data packets for Heartbeat,Relaible UDP,Sequencing of packets and the Connection protocol,
		///if the connection is alive.
		bool HandlePacket(Datagram *data);

		///This function tries connecting according to the maximum number of attempts, delay and the semaphore implementation.
		void TryConnecting(int max_attempts, int ms_delay, Turnkey<bool> *key);

		///This function increments the sequence number.
		int Seq_Num();

		
	protected:
		///Friend classe to the connection class
		friend Heartbeat;

		///Friend classe to the connection class
		friend ConnectProtocol;

		///Friend classe to the connection class
		friend ReliableUdp;

		///Function which establishes a connection
		void Connect();

		///Function which disconnects a client from an existing connection with the server
		void Disconnect();

	private:
		///Object of Peer class 
		Peer* peer;	

		///Object of Heartbeat class 
		Heartbeat heartbeat;

		///Object of ConnectProtocol class
		ConnectProtocol connectprotocol;

		///Object of class ReliableTracker
		ReliableUdp rudpTracker;

		///Object of class SequenceMonitor
		SequencedUdp sequenceMonitor;

		///The sequence number associated with a data packet.
		unsigned int seq_num_out;

		/// Boolean for triggering disconnection and removal of
		/// this Connection from the Peer's ConnectionTable
		bool should_disconnect;
	};
}
