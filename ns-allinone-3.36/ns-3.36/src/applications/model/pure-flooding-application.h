/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PURE_FLOODING_APP_H
#define PURE_FLOODING_APP_H

#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/seq-ts-header.h"
#include "ns3/traced-callback.h"
#include "ns3/pure-flooding-header.h"
#include "ns3/core-module.h"

namespace ns3
{

  class Socket;
  class Packet;

  class PureFloodingApp : public Application
  {
  public:
    static TypeId GetTypeId(void);
    PureFloodingApp();
    virtual ~PureFloodingApp();

    void LogPerformance() {
      NS_LOG_UNCOND("PERFORMANCE: " << seenNodes.size() << " " << numUpdatesReceivedInTime << " " << numUpdatesReceivedLate);
    };

    int GetNumUpdatesReceivedInTime();

    int GetNumUpdatesReceivedLate();

    int GetNumSeenNodes();

    int GetNumSent();
    int GetNumFwd();
    int GetNumRcvd();

    void ResetStats();

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void ScheduleTransmit(Time dt);

    void Send(void);

    void Forward(Ptr<Packet> packet);

    void HandleRead(Ptr<Socket> socket);

    Ptr<UniformRandomVariable> jitter;

    int seqNo = 0;

    double m_forwardingProbability = 1.0;

    Time m_aoiThreshold = Seconds(0.73573573573);

    Time m_sendInterval = Seconds(1);
    Time m_forwardingJitter = Seconds(0.1);

    uint16_t m_ttl = 999;
    uint32_t m_dataSize = 100;                              //!< packet payload size (must be equal to m_size)
    uint16_t m_port;                                        //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;                                   //!< IPv4 Socket
    Address m_local;                                        //!< local multicast address
    EventId m_sendEvent;                                    //!< Event to send the next packet
    Address m_peerAddress = Ipv4Address("255.255.255.255"); //!< Remote peer address
    uint16_t m_peerPort = 300;                              //!< Remote peer port
    std::vector<std::string> seenSeqNos;
    std::map<uint32_t, Time> lastReceived;

    // Metrics
    std::set<uint32_t> seenNodes;
    int numUpdatesReceivedInTime = 0;
    int numUpdatesReceivedLate = 0;
    int numSent = 0;
    int numReceived = 0;
    int numForwarded = 0;

    /// Callbacks for tracing the packet Rx events
    TracedCallback<Ptr<const Packet>, uint32_t> m_rxTrace;

    /// Callbacks for tracing the packet Tx events
    TracedCallback<Ptr<const Packet>, uint32_t> m_txTrace;

    /// Callbacks for tracing the packet Fwd events
    TracedCallback<Ptr<const Packet>, uint32_t> m_fwdTrace;

    /// Callbacks for tracing the packet Rx events, includes source and destination addresses
    TracedCallback<Ptr<const Packet>, const Address &, const Address &> m_rxTraceWithAddresses;
  };

} // namespace ns3

#endif /* PURE_FLOODING_APP_H */
