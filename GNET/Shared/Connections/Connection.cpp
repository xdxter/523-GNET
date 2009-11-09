#include "GNET_Peer.h"
#include "Connections/Connection.h"
#include "Connections/Parts/ConnectProtocol.h"
#include "Connections/Parts/Heartbeat.h"

using namespace GNET;

#pragma warning(disable:4355) // we know what we're doing with 'this'
Connection::Connection(SOCKADDR_IN remote, Peer* peer) : 
	heartbeat(5000, 500, 4, peer, this),
	connectprotocol(peer->connect_timeout, peer, this),
	rudpTracker(peer)
{
	this->remote = remote;
	this->peer = peer;
	should_disconnect = false;

	seq_num_out = 0;
}

bool Connection::Update() {
	if (should_disconnect)
		return false;

	if (!should_disconnect)	connectprotocol.Update();
	if (!should_disconnect)	heartbeat.Update();
	if (!should_disconnect) rudpTracker.Update();

	// return true if connection is still alive
	return !should_disconnect;
}

bool Connection::HandlePacket(Datagram *data) {
	if (should_disconnect)
		return false;

	// Received packet, so connection is alive
	heartbeat.Reset();

	bool handled = false;
	if (!handled) handled = connectprotocol.HandlePacket(data);
	if (!handled) handled = heartbeat.HandlePacket(data);
	if (!handled) handled = rudpTracker.HandlePacket(data);
	
	// this will return true if the packet has been handled
	return handled; 
}

void Connection::Disconnect() {
	should_disconnect = true;
	dd("Disconnecting connection with " << remote.sin_addr.S_un.S_addr << ":" << remote.sin_port);
}

void Connection::TryConnecting(int max_attempts, int ms_delay, Turnkey<bool>* key) {
	connectprotocol.TryConnecting(max_attempts, ms_delay, key);
}

int Connection::Seq_Num() {
	return seq_num_out++;
}