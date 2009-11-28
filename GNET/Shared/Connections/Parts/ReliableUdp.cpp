#include "GNET_Peer.h"
//#include "Connection.h"

using namespace GNET;


void ReliableUdp::Update()
{
	for (it = out_list.begin(); it != out_list.end(); it++) {
		if(it->second->timer.Finished()) {
			DBG_PRINT("[RUDP] --> Resending Packet with seqID = " << it->first);
			peer->Send(it->second->dat);
			it->second->timer.Reset(RUDP_TIMEOUT);
		}
	}
}

bool ReliableUdp::SendingPacket(Datagram * dgram)
{
	// If this is a datapacket, which should be reliable...
	if (dgram->flags & RELIABLE && dgram->pack->GetType() != DATA_PACKET) {	
		// Store this Datagram
		RudpItem * item = new RudpItem;
		item->dat = new Datagram(*dgram);
		item->timer.Reset(RUDP_TIMEOUT);
		out_list.insert( 
			ReliableTablePair( 
				static_cast<DataPack*>(dgram->pack)->seq_num, 
				item  ));		
	}

	return true; // I'll allow it.
}

void ReliableUdp::SendAckPack(SOCKADDR_IN* sock, unsigned int seq_num) {
	Datagram *dgram = new Datagram;
	dgram->flags = RELIABLE;
	dgram->pack = new RudpAckPack;
	static_cast<RudpAckPack*>(dgram->pack)->seq_num = seq_num;
	dgram->sock = new SOCKADDR_IN( *sock );
	peer->Send(dgram);
}

bool ReliableUdp::HandlePacket(Datagram * dgram)
{
	if (dgram->flags & RELIABLE) {
		switch (dgram->pack->GetType()) {
			case DATA_PACKET:
				{
					DataPack *data = static_cast<DataPack*>(dgram->pack);					
					DBG_PRINT("[RUDP] " << SOCK_PRNT(*dgram->sock) << " --> Datapacket received with seqID = " << dynamic_cast<DataPack*>(dgram->pack)->seq_num);

					it = in_list.find( data->seq_num );
					
					if (it == in_list.end()) // Received a new Data Packet
					{
						RudpItem * item = new RudpItem;
						item->timer.Reset(RUDP_ID_RESERVATION_TIMEOUT); // This isn't really a great way to do this... but it works for now.

						SendAckPack( dgram->sock, data->seq_num );

						return false; // this packet has not been fully handled.

					} else // Already received this one.
					{
						// Resend Ack packet
						SendAckPack( dgram->sock, data->seq_num );

						DBG_PRINT("[RUDP] --> Received repetitive packet, decide to abandon.");

						return true; // this packet has been handled
					}

				}
				break;
			case RUDP_ACK_PACKET:
				{
					RudpAckPack * pack = static_cast<RudpAckPack*>(dgram->pack);
					it = out_list.find(pack->seq_num);
					if(it != out_list.end())	//if this ACK is recognized
					{						
						DBG_PRINT("[RUDP] " <<  SOCK_PRNT(*dgram->sock) << " --> Rudp Ack for packet "<< pack->seq_num <<", removing from the list");
						out_list.erase(it);
					}
					return true;

				}
				break;			
		}
	}

	return false; // packet has not been handled
}