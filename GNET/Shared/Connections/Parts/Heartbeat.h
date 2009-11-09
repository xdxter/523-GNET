#include "Base/Timer.h"

#pragma once

namespace GNET {
	class Peer;
	class Connection;

	class Heartbeat {
	public:
		Heartbeat(int ms_silence_time, int ms_delay, int max_attempts, Peer* peer, Connection* connection);
		bool HandlePacket(Datagram *data);
		void Update();
		void Reset();
	
	protected:
		Peer* peer;
		Connection* connection;

		Timer timer;
		bool pinging;
		int attempts;

		int ms_silence_time,
			ms_delay,
			max_attempts;
	};
}