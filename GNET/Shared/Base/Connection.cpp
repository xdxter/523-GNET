#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;

Connection::Connection(SOCKADDR_IN remote, Peer* peer, bool i_am_the_instigator) {
	this->remote = remote;
	this->peer = peer;
	seq_num_out = 0;

	if (i_am_the_instigator) {
		connect_state = WAITING_FOR_SYN;
		Handshake(1);
	} else {		
		connect_state = WAITING_FOR_ACK;
		Handshake(2);
	}

}

void Connection::Handshake(int i) {			
	// Add Connect Packet to queue...
	peer->Send(
		(i == 1? (INetPacket*) new ConPack :
		(i == 2? (INetPacket*) new SynPack :
							   new AckPack)),
		&remote,false);

	// Set to wait for Syn
	connect_timeout = clock() + CLOCKS_PER_SEC * 5;
}

void Connection::Update() {
	// if timed out, send again!
	if (connect_state != CONNECTED && connect_state != NOT_CONNECTED && clock() > connect_timeout)  
		Handshake(connect_state); 

	// Run down RUDP timers, send ACK1's
}

void Connection::HandlePacket(Datagram *data) {
	// Connection Protocol Stuff
	if (connect_state == WAITING_FOR_ACK) {
		if (dynamic_cast<ConPack*>(data->pack))
			Handshake(2);
		if (dynamic_cast<AckPack*>(data->pack)) 
			connect_state = CONNECTED;
	}
	
	if (dynamic_cast<SynPack*>(data->pack)) {
		Handshake(3);
		connect_state = CONNECTED;
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