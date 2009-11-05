#include "GNET_Peer.h"
#include "Connection.h"

using namespace GNET;


void ReliableTracker::Update()
{
	for (it = out.begin(); it != out.end(); it++) {
		if(it->second->timer.Finished()) {
			if(it->second->sender == true)	//
			printf("RUDP------>Resending Packet...\n");
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
	printf("RUDP------>Added new outgoing packet....\n");
}

bool ReliableTracker::HandlePacket(Datagram * dat)
{
	if (dynamic_cast<DataPack*>(dat->pack))
	{
		printf("RUDP------>Received a reliable datapack with seq ID = %d from %d\n", dynamic_cast<DataPack*>(dat->pack)->seq_num, dat->sock->sin_port);
		it = in.find(ReliableKey(ADDR(*dat->sock), dynamic_cast<DataPack*>(dat->pack)->seq_num));

		//A repetitive datapack or it hasn't been 1 min since this ID is used
		if (it != in.end())	
		{
			RUDPAckPack ackPack;
			ackPack.seq_num = dynamic_cast<DataPack*>(dat->pack)->seq_num;
			peer->Send(&ackPack, dat->sock);	//resend rudp ack for this packet
			printf("RUDP------>Received repetitive packet, decide to abandon.....\n");
			return false;						//tell logc_thread to not proceed a repetitive packet
		}
		// new rudp packet
		else			
		{
			RudpItem * item = new RudpItem;
			item->sender = false;
			item->timer.Reset(RUDP_ID_RESERVATION_TIMEOUT);

			RUDPAckPack ackPack;
			ackPack.seq_num = dynamic_cast<DataPack*>(dat->pack)->seq_num;
			in[ReliableKey(ADDR(*dat->sock), ackPack.seq_num)] = item;
			printf("RUDP------>Sending Rudp Ack for received packet %d......\n", ackPack.seq_num);
			peer->Send(&ackPack, dat->sock);
			return true;
		}
	}
	else if(dynamic_cast<RUDPAckPack*>(dat->pack))
	{
		RUDPAckPack * pack = dynamic_cast<RUDPAckPack*>(dat->pack);
		it = out.find(ReliableKey(ADDR(*dat->sock), pack->seq_num));
		if(it != in.end())	//if this ACK is recognized
		{
			printf("Rudp Ack for packet %d has been received, removing it...\n", pack->seq_num);
			out.erase(it);
		}
	}
}