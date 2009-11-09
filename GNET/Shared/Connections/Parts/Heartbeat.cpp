#include "GNET_Types.h"
#include "GNET_Peer.h"
#include "Connections/Parts/Heartbeat.h"

using namespace GNET;

	Heartbeat::Heartbeat(int ms_silence_time, int ms_delay, int max_attempts, Peer* peer, Connection* connection)
	{
		this->ms_silence_time = ms_silence_time;
		this->ms_delay = ms_delay;
		this->max_attempts = max_attempts;
		this->peer = peer;
		this->connection = connection;

		this->Reset();					
	}

	bool Heartbeat::HandlePacket(Datagram *data) {
		switch (data->pack->GetType()) {
			case MARCO_PACKET:
				{
					dd("Received Marco Packet");
					PoloPack pack;
					peer->Send(&pack, &(connection->remote), false);
					return true;
				}
			case POLO_PACKET:
				{	
					dd("Received Polo Packet");
					// We don't need to do anything here because
					// Reset should have already been called by our 
					// Connection
					return true;
				}
		}
		return false;
	}

	void Heartbeat::Update() {
		if (timer.Finished()) {	
			dd("Sending Marco Packet");
			if (pinging) {
				if (attempts++ > max_attempts)
					connection->Disconnect();
			} else
				pinging = true;
			MarcoPack pack;
			peer->Send(&pack, &(connection->remote), false);
			timer.Reset(ms_delay);
		}
	}

	void Heartbeat::Reset() {
		if (pinging) {
			pinging = false;
			attempts = 0;
		}
		timer.Reset(ms_silence_time);
	}