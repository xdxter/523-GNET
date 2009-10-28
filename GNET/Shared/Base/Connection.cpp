#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;

Connection::Connection(SOCKADDR_IN remote, Peer* peer) {
	this->remote = remote;
	this->peer = peer;
	seq_num_out = 0;
	connect_state = NOT_CONNECTED;
}

void Connection::TryConnect() {		

	// Add Connect Packet to queue...
	peer->Send((INetPacket*) new ConPack,&remote,false);

	// Set to wait for Syn
	connect_state = WAITING_FOR_SYN;
	connect_timeout = clock() + CLOCKS_PER_SEC * 5;
}

void Connection::Update() {
	// if timed out, send again!
	if (clock() > connect_timeout) 
		TryConnect();
	

	// do RUDP stuff.
}

void Connection::HandlePacket(Datagram *data) {

	// write connection protocol handling code here
}

int Connection::Seq_Num() {
	return seq_num_out++;
}