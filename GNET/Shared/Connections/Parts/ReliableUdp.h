#include "GNET_Types.h"
#include "Base/Timer.h"

#pragma once

namespace GNET {
	struct RudpItem;

	///A map type to hold RUDP info
	typedef std::map<int, RudpItem*> ReliableTable;
	typedef std::pair<int, RudpItem*> ReliableTablePair;	
	typedef std::map<int, RudpItem*>::iterator ReliableTableIter;

	///Defined Macro for the time out interval
	#define RUDP_TIMEOUT 1000

	///Defined Macro for the reservation of the Time out ID 
	///Note: ID can't be reused in one minute
	#define RUDP_ID_RESERVATION_TIMEOUT 60000	//id can't be reused in one minute

	/// This is used by the ReliableTracker to keep track of stuff.
	struct RudpItem{
		Datagram * dat;
		///Used for both resend and ID reservation
		Timer timer;
	};
	
	///A class which adds the functionality of reliable UDP through the concept of sequence numbers.
	class ReliableUdp {	
	public:
		///Constructor
		ReliableUdp(Peer * _peer) {
			peer = _peer;
		}

		///Destructor
		~ReliableUdp(){}

		///This function resends the packet if the timout occurs on a particular sent packet.
		void Update();

		///This function adds a relaible datapacket with a sequence number.
		bool SendingPacket(Datagram * dgram);

		///This function handles a reliable data pack on receive.
		///It also drops a packet if it has already been received. (i.e.Repititive packet)
		///It also resets the timer.
		bool HandlePacket(Datagram * dat);

	private:
		///Contains data about the packets which have been sent so that the handle packet function 
		///can check for this entry, when a particular ACK is received
		ReliableTable out_list;

		///Contains data about the packets which have been received so that the handle packet function 
		///can check for the sequence number, when a particular packet is received
		ReliableTable in_list;

		///Used to iterate the 'out_list' or 'in_list'
		ReliableTableIter it;

		void SendAckPack(SOCKADDR_IN* sock, unsigned int seq_num);

		Peer * peer;
	};
	
}