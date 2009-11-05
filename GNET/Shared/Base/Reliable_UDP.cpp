
#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;


void ReliableTracker::Update()
{
	printf("number of rudp pack in progress is %d\n", out.size());
	for (it = out.begin(); it != out.end(); it++) {
		if(it->second->resend_timer.Finished()) {
			printf("resending....\n");
			peer->Send(it->second->dat);
			it->second->resend_timer.Reset(RUDP_TIMEOUT);
		}
		else
		{
			printf("not resending....\n");
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
	item->resend_timer.Stop();
	item->resend_timer.Reset(RUDP_TIMEOUT);
	out[ReliableKey(ADDR(*remote), pack->seq_num)] = item;
	printf("ADDED!!!!!!!!!!!\n");
}