#include <queue>
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

bool Connection::SendingPacket(Datagram * dgram) { 
	bool send_handled = false;

	send_handled &= rudpTracker.SendingPacket(dgram);
	// ...
	// This is where we can append other stuff prior to sending.
	// if we follow the same format of 
	//    send_handled &= thing.SendingPacket(dgram);
	// ...

	return send_handled;
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
	if (!handled) handled = sequenceMonitor.HandlePacket(data);

	// this will return true if the packet has been handled
	return handled; 
}

void Connection::Connect() {
	peer->connection_events.Lock(); 
	peer->connection_events->push(remote); 
	peer->connection_events.Pulse();
	peer->connection_events.Unlock();
}

void Connection::Disconnect() {
	should_disconnect = true;

	peer->disconnect_events.Lock();
	peer->disconnect_events->push(remote); 
	peer->disconnect_events.Pulse();
	peer->disconnect_events.Unlock();
	
	DBG_PRINT("Disconnecting connection with " << SOCK_PRNT(remote));
}

void Connection::TryConnecting(int max_attempts, int ms_delay, Turnkey<bool>* key) {
	connectprotocol.TryConnecting(max_attempts, ms_delay, key);
}

int Connection::Seq_Num() {
	return seq_num_out++;
}
