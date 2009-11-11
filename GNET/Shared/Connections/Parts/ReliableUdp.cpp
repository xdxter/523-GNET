#include "GNET_Peer.h"
//#include "Connection.h"

using namespace GNET;


void ReliableTracker::Update()
{
	for (it = out.begin(); it != out.end(); it++) {
		if(it->second->timer.Finished()) {
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
	item->timer.Reset(RUDP_TIMEOUT);
	out[pack->seq_num] = item;
}

bool ReliableTracker::HandlePacket(Datagram * dat)
{
	DataPack* data = dynamic_cast<DataPack*>(dat->pack);
	if (data && data->reliable)
	{
		printf("RUDP------>Received a reliable datapack with seq ID = %d from port %d\n", dynamic_cast<DataPack*>(dat->pack)->seq_num,ntohs(dat->sock->sin_port));
		it = in.find(dynamic_cast<DataPack*>(dat->pack)->seq_num);

		//A repetitive datapack or it hasn't been 1 min since this ID is used
		if (it != in.end())	
		{
			//resend Ack packet
			RudpAckPack ackPack;
			ackPack.seq_num = dynamic_cast<DataPack*>(dat->pack)->seq_num;
			peer->Send(&ackPack, dat->sock);
			printf("RUDP------>Received repetitive packet, decide to abandon.....\n");
			return true;	//return says this packet has been handled
		}
		// new rudp packet
		else			
		{
			RudpItem * item = new RudpItem;
			item->timer.Reset(RUDP_ID_RESERVATION_TIMEOUT);

			RudpAckPack ackPack;
			ackPack.seq_num = dynamic_cast<DataPack*>(dat->pack)->seq_num;
			in[ackPack.seq_num] = item;
			printf("RUDP------>Sending Rudp Ack for received packet %d......\n", ackPack.seq_num);
			peer->Send(&ackPack, dat->sock);
			return false;
		}
	}
	else if(dynamic_cast<RudpAckPack*>(dat->pack))
	{
		RudpAckPack * pack = dynamic_cast<RudpAckPack*>(dat->pack);
		it = out.find(pack->seq_num);
		if(it != out.end())	//if this ACK is recognized
		{
			printf("RUDP--->Ack for packet %d has been received, removing it...\n", pack->seq_num);
			out.erase(it);
		}
		return true;
	}
	else
		return false;
}