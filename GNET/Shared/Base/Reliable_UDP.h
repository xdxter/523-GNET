#include "GNET_Types.h"
#include "Timer.h"

#pragma once

namespace GNET {

	struct RudpItem{
		Datagram * dat;
		Timer resend_timer;
	};

	class ReliableTracker {	
	public:
		ReliableTracker(){}
		~ReliableTracker(){}
		void Update()
		{
			printf("number of rudp pack in progress is %d\n", out.size());
			for (it = out.begin(); it != out.end(); it++) {
				if(it->second->resend_timer.Finished()) {
					
				}
			}
		}

		void AddOutgoingPack(DataPack * pack, SOCKADDR_IN * remote) 
		{
			Datagram * dat = new Datagram;
			INetPacket* net = static_cast<INetPacket*>(g_NetPackets[ pack->GetType() ].instantiate());
			memcpy(net,pack, g_NetPackets[pack->GetType()].size);
			dat->pack = net;
			dat->sock = new SOCKADDR_IN(*remote);

			RudpItem * item = new RudpItem;
			item->dat = dat;
			item->resend_timer.Reset(time_out);
			out[ReliableKey(ADDR(*remote), pack->seq_num)] = item;
			printf("ADDED!!!!!!!!!!!\n");
		}
	private:
		ReliableTable out;
		ReliableTableIter it;
		static const unsigned time_out = 10000;
		//	Connection(SOCKADDR_IN remote, Peer* peer);
		//	void Update();
		//	bool HandlePacket(Datagram *data);
		//	int Seq_Num();
	};
	
}