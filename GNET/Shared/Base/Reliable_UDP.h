#include "GNET_Types.h"
#include "Timer.h"

#pragma once

namespace GNET {

	struct RudpItem{
		Datagram * dat;
		int attempts;
	};

	class ReliableTracker {	
	public:
		ReliableTracker(){}
		~ReliableTracker(){}
		void Update(){printf("number of rudp pack in progress is %d\n", out.size());}

		void AddOutgoingPack(DataPack * pack, SOCKADDR_IN * remote)
		{
			Datagram * dat = new Datagram;
			INetPacket* net = static_cast<INetPacket*>(g_NetPackets[ pack->GetType() ].instantiate());
			memcpy(net,pack, g_NetPackets[pack->GetType()].size);
			dat->pack = net;
			dat->sock = new SOCKADDR_IN(*remote);

			RudpItem * item = new RudpItem;
			item->dat = dat;
			item->attempts = 0;
			out[ReliableKey(ADDR(*remote), pack->seq_num)] = item;
			printf("ADDED!!!!!!!!!!!\n");
		}
	private:
		ReliableTable out;
		//	Connection(SOCKADDR_IN remote, Peer* peer);
		//	void Update();
		//	bool HandlePacket(Datagram *data);
		//	int Seq_Num();
		//	void TryConnecting(int max_attempts = 7, int ms_delay = 500);
		//private:
		//	Peer* peer;
		//	int seq_num_out;
		//
		//	int connect_state;
		//	Timer connect_timer;

		//	bool is_instigator;
		//	int attempts;
		//	int ms_delay;
		//	void Handshake(int i);
	};
	
}