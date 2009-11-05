#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;


void ReliableTracker::Update()
{
	for (it = out.begin(); it != out.end(); it++) {
		if(it->second->timer.Finished()) {
			if(it->second->sender == true)	//
			printf("resending....\n");
			peer->Send(it->second->dat);
			it->second->timer.Reset(RUDP_TIMEOUT);
		}
	}
}

void ReliableTracker::AddOutgoingPack(DataPack * pack, SOCKADDR_IN * remote)
{
	Datagram * dat = new Datagram;
	INetPacket* net = static_cast<INetPacket*>(g_NetPackets[ pack->GetType() ].instantiate());
	memcpy(net,pack, g_NetPackets[pack->GetType()].size);
	dat->pack = net;
	dat->sock = new SOCKADDR_IN(*remote);

	RudpItem * item = new RudpItem;
	item->dat = dat;
	item->sender = true;
	item->timer.Reset(RUDP_TIMEOUT);
	out[ReliableKey(ADDR(*remote), pack->seq_num)] = item;
	printf("ADDED!!!!!!!!!!!\n");
}

void ReliableTracker::HandlePacket(Datagram * dat)
{
	if (dynamic_cast<DataPack*>(dat->pack))
	{
		RudpItem * item = new RudpItem;
		item->sender = false;
		item->timer.Reset(RUDP_ID_RESERVATION_TIMEOUT);

		RUDPAckPack ackPack;
		ackPack.seq_num = dynamic_cast<DataPack*>(dat->pack)->seq_num;
		in[ReliableKey(ADDR(*dat->sock), ackPack.seq_num)] = item;
		printf("Sending Rudp Ack for received packet %d......\n", ackPack.seq_num);
		peer->Send(&ackPack, dat->sock);
	}
	else if(dynamic_cast<RUDPAckPack*>(dat->pack))
	{
		RUDPAckPack * pack = dynamic_cast<RUDPAckPack*>(dat->pack);
		printf("Rudp Ack for packet %d has been received, removing it...", pack->seq_num);
		it = out.find(ReliableKey(ADDR(*dat->sock), pack->seq_num));
		out.erase(it);
	}
}