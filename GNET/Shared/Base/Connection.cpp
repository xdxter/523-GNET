#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;

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
}
Connection::Connection(SOCKADDR_IN remote, Peer* peer) {
	this->remote = remote;
	this->peer = peer;
	seq_num_out = 0;

	is_instigator = false;
	connect_state = NOT_CONNECTED;
}

void Connection::TryConnecting(int max_attempts, int ms_delay) {
	Handshake(SEND_CON);
	connect_state = WAITING_FOR_SYN;	
	is_instigator = true;
	this->attempts = max_attempts;
	this->ms_delay = ms_delay;
}

void Connection::Handshake(int i) {		
	connect_timer.Stop();
	// Add Connect Packet to queue...
	peer->Send(
		(i == SEND_CON? (INetPacket*) new ConPack :
		(i == SEND_SYN? (INetPacket*) new SynPack :
							   new AckPack)),
		&remote,false);

	printf("Sending %s packet\n",
		(i == 1?"ConPack" :
		(i == 2? "SynPack" :
				 "AckPack")));
	// Set to wait for Syn
	connect_timer.Reset(ms_delay);
}

void Connection::Update() {
	// if timed out, send again!
	if (connect_state != CONNECTED && connect_state != NOT_CONNECTED && connect_timer.Finished()) {
		if (is_instigator && --attempts == 0) {
			peer->connecting.SetResult(false);
			peer->connecting.Pulse();
			connect_state = NOT_CONNECTED;
		} else {
			if (connect_state == WAITING_FOR_SYN)
				Handshake(SEND_CON); 
			if (connect_state == WAITING_FOR_ACK)
				Handshake(SEND_SYN); 


		}
	}

	// Run down RUDP timers, send ACK1's
}

bool Connection::HandlePacket(Datagram *data) {
	if (is_instigator) {
		if (dynamic_cast<SynPack*>(data->pack)) {
			printf("Received SynPack\n");
			if (connect_state == WAITING_FOR_SYN) {
				peer->connecting.SetResult(true);
				peer->connecting.Pulse();
				connect_state = CONNECTED;
				is_connecting = false;
			}
			Handshake(SEND_ACK);
			return true;
		}
	}
	if (!is_instigator) {
		if (dynamic_cast<ConPack*>(data->pack)) {
			printf("Received ConPack\n");
			Handshake(SEND_SYN);
			if (connect_state == NOT_CONNECTED)
				connect_state = WAITING_FOR_ACK;			
			return true;
		}
		if (dynamic_cast<AckPack*>(data->pack)) {
			printf("Received AckPack\n");
			if (connect_state == WAITING_FOR_ACK) {
				connect_state = CONNECTED;
				is_connecting = false;				
			}
			return true;
		}
	}

	// RUDP stuff
	if (data->reliable) {
		// if it's an ACK2, we finished something
		// otherwise, we need to send an ACK1...
	}	
}

int Connection::Seq_Num() {
	return seq_num_out++;
}