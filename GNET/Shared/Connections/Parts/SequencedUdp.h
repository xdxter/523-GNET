#include "GNET_Types.h"

namespace GNET {
	///A class which handles the packet for sequenced UDP (using sequence numbers)
	class SequencedUdp{
	public:
		SequencedUdp();

		///Returns false if a sequenced UDP packet is Accepted.
		bool HandlePacket(Datagram * dat);

		///The sequence number of the packet
		unsigned int seq_num;
	};
}