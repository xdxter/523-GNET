#include "Base/Timer.h"

#pragma once

namespace GNET {
	class Peer;
	class Connection;

	///A class which lets the server know if a particular client is still alive or is dead so that
	///the server always maintains a correct list of the current alive clients.
	class Heartbeat {
	public:
		///Constructor
		Heartbeat(int ms_silence_time, int ms_delay, int max_attempts, Peer* peer, Connection* connection);

		///Function which handles the Marco and Polo packets.
		bool HandlePacket(Datagram *data);

		///A function which sends a Marco packet from the client to let the server know that the client is still alive. 
		void Update();

		///Resets the timer.
		void Reset();
	
	protected:
		Peer* peer;
		Connection* connection;

		Timer timer;

		///A boolean used by the Reset function to start the Marco Polo cycle (By setting attempts variable) 
		bool pinging;

		///A variable which stores the current number of attempts of the Marco Polo cycle.
		int attempts;

		///Variables used for time keeping purposes.
		int ms_silence_time,
			ms_delay,
		///Variable which denotes the maximum number of attempts for the Marco Polo packet cycle.
			max_attempts;
	};
}