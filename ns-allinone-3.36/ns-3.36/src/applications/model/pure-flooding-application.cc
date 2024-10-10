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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "pure-flooding-application.h"

using namespace std;

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("PureFloodingApp");

  NS_OBJECT_ENSURE_REGISTERED(PureFloodingApp);

  TypeId
  PureFloodingApp::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::PureFloodingApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<PureFloodingApp>()
                            .AddAttribute("Port", "Port on which we listen for incoming packets.",
                                          UintegerValue(9),
                                          MakeUintegerAccessor(&PureFloodingApp::m_port),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("SendInterval", "Time between the sending of two packets",
                                          TimeValue(Seconds(1)),
                                          MakeTimeAccessor(&PureFloodingApp::m_sendInterval),
                                          MakeTimeChecker())
                            .AddAttribute("ForwardingJitter", "Time between the sending of two packets",
                                          TimeValue(Seconds(0.11)),
                                          MakeTimeAccessor(&PureFloodingApp::m_forwardingJitter),
                                          MakeTimeChecker())
                            .AddAttribute("PacketSize", "The size of packets transmitted.",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&PureFloodingApp::m_dataSize),
                                          MakeUintegerChecker<uint32_t>(1))
                            .AddAttribute("TTL", "Max Hop count of a packet",
                                          UintegerValue(9999),
                                          MakeUintegerAccessor(&PureFloodingApp::m_ttl),
                                          MakeUintegerChecker<uint32_t>(0))
                            .AddAttribute("ForwardingProbability", "The probability to forward a packet",
                                          DoubleValue(1.0),
                                          MakeDoubleAccessor(&PureFloodingApp::m_forwardingProbability),
                                          MakeDoubleChecker<double>(0.0))
                            .AddTraceSource("Rx", "A packet has been received",
                                            MakeTraceSourceAccessor(&PureFloodingApp::m_rxTrace),
                                            "ns3::Packet::TracedCallback")
                            .AddTraceSource("RxWithAddresses", "A packet has been received",
                                            MakeTraceSourceAccessor(&PureFloodingApp::m_rxTraceWithAddresses),
                                            "ns3::Packet::TwoAddressTracedCallback")
                            .AddTraceSource("Tx", "A packet has been Sent",
                                            MakeTraceSourceAccessor(&PureFloodingApp::m_txTrace),
                                            "ns3::Packet::TracedCallback")
                            .AddTraceSource("Fwd", "A packet has been Forwarded",
                                            MakeTraceSourceAccessor(&PureFloodingApp::m_fwdTrace),
                                            "ns3::Packet::TracedCallback");
    return tid;
  }

  PureFloodingApp::PureFloodingApp()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    jitter = CreateObject<UniformRandomVariable>();
    jitter->SetAttribute("Min", DoubleValue(0));
    jitter->SetAttribute("Max", DoubleValue(m_forwardingJitter.GetSeconds()));
  }

  PureFloodingApp::~PureFloodingApp()
  {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
  }

  void
  PureFloodingApp::DoDispose(void)
  {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
  }

  void
  PureFloodingApp::StartApplication(void)
  {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket(GetNode(), tid);
      InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
      if (m_socket->Bind(local) == -1)
      {
        NS_FATAL_ERROR("Failed to bind socket");
      }
      if (addressUtils::IsMulticast(m_local))
      {
        Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket>(m_socket);
        if (udpSocket)
        {
          // equivalent to setsockopt (MCAST_JOIN_GROUP)
          udpSocket->MulticastJoinGroup(0, m_local);
        }
        else
        {
          NS_FATAL_ERROR("Error: Failed to join multicast group");
        }
      }

      InetSocketAddress remote = InetSocketAddress(Ipv4Address("255.255.255.255"), 3000);
      m_socket->SetAllowBroadcast(true);
      m_socket->Connect(remote);
      ScheduleTransmit(m_sendInterval);
    }

    m_socket->SetRecvCallback(MakeCallback(&PureFloodingApp::HandleRead, this));
  }

  void
  PureFloodingApp::StopApplication()
  {
    NS_LOG_FUNCTION(this);

    if (m_socket != 0)
    {
      m_socket->Close();
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
  }

  void
  PureFloodingApp::ScheduleTransmit(Time dt)
  {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &PureFloodingApp::Send, this);
  }

  void PureFloodingApp::Forward(Ptr<Packet> packet)
  {
    auto p = CreateObject<UniformRandomVariable>();
    p->SetAttribute("Min", DoubleValue(0));
    p->SetAttribute("Max", DoubleValue(1.0));

    if (p->GetValue() <= m_forwardingProbability)
    {
      m_socket->Send(packet);
      m_fwdTrace(packet, GetNode()->GetId());
      numForwarded++;
    }
  }

  void
  PureFloodingApp::Send(void)
  {
    NS_LOG_FUNCTION(this);

    NS_ASSERT(m_sendEvent.IsExpired());
    Address localAddress;
    m_socket->GetSockName(localAddress);

    Ptr<Packet> p = Create<Packet>(m_dataSize);
    uint32_t nodeId = GetNode()->GetId();

    Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();

    PureFloodingHeader header;
    header.SetSeq(this->seqNo++);
    header.SetSrc(nodeId);
    header.SetLastHop(nodeId);
    header.SetStartPos(nodePos);
    p->AddHeader(header);
    m_txTrace(p, GetNode()->GetId());
    numSent++;
    m_socket->Send(p);
    ScheduleTransmit(m_sendInterval);

    string pkt_id = to_string(GetNode()->GetId()) + "-" + to_string(header.GetSeq());
    seenSeqNos.push_back(pkt_id);
  }

  void
  PureFloodingApp::HandleRead(Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
      socket->GetSockName(localAddress);
      Ptr<Packet> packetCopy = packet->Copy();
      packetCopy->RemoveAllPacketTags();
      packetCopy->RemoveAllByteTags();
      PureFloodingHeader header;
      packetCopy->RemoveHeader(header);

      uint32_t src = header.GetSrc();
      Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();
      double dist_sender = CalculateDistance(header.GetStartPos(), nodePos);

      header.SetNumHops(header.GetNumHops() + 1);
      header.SetLastHop(GetNode()->GetId());

      packetCopy->AddHeader(header);

      string pkt_id = to_string(header.GetSrc()) + "-" + to_string(header.GetSeq());

      if (std::find(seenSeqNos.begin(), seenSeqNos.end(), pkt_id) == seenSeqNos.end())
      {
        seenNodes.insert(src);
        if (lastReceived.count(src) > 0 && dist_sender <= 509.003)
        {
          Time aoi = Simulator::Now() - lastReceived[src];
          if (aoi > m_aoiThreshold)
          {
            numUpdatesReceivedLate++;
          }
          else
          {
            numUpdatesReceivedInTime++;
          }
        }
        lastReceived[src] = header.GetTs();
        m_rxTrace(packet, GetNode()->GetId());
        m_rxTraceWithAddresses(packet, from, localAddress);
        numReceived++;
        if (header.GetNumHops() < m_ttl)
        {
          Simulator::Schedule(Seconds(jitter->GetValue()), &PureFloodingApp::Forward, this, packetCopy);
        }
        seenSeqNos.push_back(pkt_id);
      }
    }
  }

  int PureFloodingApp::GetNumUpdatesReceivedInTime()
  {
    return numUpdatesReceivedInTime;
  }

  int PureFloodingApp::GetNumUpdatesReceivedLate()
  {
    return numUpdatesReceivedLate;
  }

  int PureFloodingApp::GetNumSeenNodes()
  {
    return seenNodes.size();
  }

  int PureFloodingApp::GetNumSent()
  {
    return numSent;
  }

  int PureFloodingApp::GetNumFwd()
  {
    return numForwarded;
  }

  int PureFloodingApp::GetNumRcvd()
  {
    return numReceived;
  }

  void PureFloodingApp::ResetStats()
  {
        numUpdatesReceivedInTime = 0;
        numUpdatesReceivedLate = 0;
        seenNodes.clear();
        numSent = 0;
        numReceived = 0;
        numForwarded = 0;
  }

} // Namespace ns3
