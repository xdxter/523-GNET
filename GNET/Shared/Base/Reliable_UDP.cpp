#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;


void ReliableTracker::Update()
{
	for (it = out.begin(); it != out.end(); it++) {
		if(it->second->resend_timer.Finished()) {
			if(it->second->sender == true)	//
			printf("resending....\n");
			peer->Send(it->second->dat);
			it->second->resend_timer.Reset(RUDP_TIMEOUT);
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
	item->resend_timer.Reset(RUDP_TIMEOUT);
	item->sender = true;
	out[ReliableKey(ADDR(*remote), pack->seq_num)] = item;
	printf("ADDED!!!!!!!!!!!\n");
}

void ReliableTracker::HandlePacket(Datagram * dat)
{
	if (dynamic_cast<DataPack*>(dat->pack))
	{
		printf("1 1 1\n");
	}
	else if(dynamic_cast<RUDPAckPack*>(dat->pack))
	{
		printf("2 2 2 \n");
	}
}