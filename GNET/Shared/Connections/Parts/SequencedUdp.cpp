#include "Connections/Parts/SequencedUdp.h"

using namespace GNET;

SequencedUdp::SequencedUdp() {
	seq_num = 0;
}

bool SequencedUdp::HandlePacket(Datagram *dat){
	DataPack* data = dynamic_cast<DataPack*>(dat->pack);
	if (data && dat->flags & SEQUENCED)
	{
		if ( data->seq_num >= this->seq_num) {
			this->seq_num = data->seq_num+1;
			DBG_PRINT("[SeqUDP] --> Accepting Packet with seq_num " << data->seq_num);
			return false;
		}
		else
		{
			DBG_PRINT("[SeqUDP] --> Ignoring Packet with seq_num " << data->seq_num);
			return true;
		}
	}
	else
		return false;
}