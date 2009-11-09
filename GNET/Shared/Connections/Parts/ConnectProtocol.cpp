#include "GNET_Peer.h"
#include "Connections/Parts/ConnectProtocol.h"

enum {
	NOT_CONNECTED,
	WAITING_FOR_SYN,
	WAITING_FOR_ACK,
	CONNECTED
};

enum {
	SEND_CON,
	SEND_SYN,
	SEND_ACK
};

namespace GNET {
	
	ConnectProtocol::ConnectProtocol(int ms_delay, Peer* peer, Connection* connection) {
		this->ms_delay = ms_delay;
		this->peer = peer;
		this->connection = connection;

		is_instigator = false;
		state = NOT_CONNECTED;
	}

	void ConnectProtocol::TryConnecting(int max_attempts, int ms_delay, Turnkey<bool> *key) {
		this->key = key;

		Handshake(SEND_CON);
		state = WAITING_FOR_SYN;	
		is_instigator = true;
		this->attempts = max_attempts;
		this->ms_delay = ms_delay;

		dd("Trying to Connect");
	}

	bool ConnectProtocol::HandlePacket(Datagram *data) {
		switch (data->pack->GetType()) {
			case CON_PACKET:
				{
					dd("Received Connect Packet");
					if (!is_instigator) {
						Handshake(SEND_SYN);
						if (state == NOT_CONNECTED)
							state = WAITING_FOR_ACK;
					}
					return true;
				}
			case SYN_PACKET:
				{
					dd("Received Syn Packet");
					if (is_instigator) {
						Handshake(SEND_ACK);
						if (state == WAITING_FOR_SYN) {
							key->SetResult(true);
							key->Pulse();
							key = 0;
							state = CONNECTED;
						}
					}
					return true;
				}
			case ACK_PACKET:
				{
					dd("Received Ack Packet");
					if (state == WAITING_FOR_ACK)
						state = CONNECTED;
					return true;
				}
			default:
				return false;
		}
	}

	void ConnectProtocol::Update() {
		// if timed out, send again!
		if (state != CONNECTED && state != NOT_CONNECTED && timer.Finished()) {
			if (is_instigator && --attempts == 0) {
				key->SetResult(false);
				key->Pulse();
				key = 0;
				//state = NOT_CONNECTED; // this line is unneccessary
				connection->Disconnect();
			} else {
				if (state == WAITING_FOR_SYN)
					Handshake(SEND_CON); 
				if (state == WAITING_FOR_ACK)
					Handshake(SEND_SYN); 
			}
		}
	}


	void ConnectProtocol::Handshake(char mode) {		
		// Ensure there is no race condition with the timer and a call to update...
		timer.Stop();
		// Add Connect Packet to queue...
		peer->Send(
			(mode == SEND_CON? (INetPacket*) new ConPack :
			(mode == SEND_SYN? (INetPacket*) new SynPack :
								   new AckPack)),
			&(connection->remote),false);
		// Set to wait for Syn
		timer.Reset(ms_delay);
	}

}