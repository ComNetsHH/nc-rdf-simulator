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
#include "ns3/mobility-module.h"

#include "rate-decay-flooding-application.h"

using namespace std;

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("RateDecayFloodingApp");

    NS_OBJECT_ENSURE_REGISTERED(RateDecayFloodingApp);

    TypeId
    RateDecayFloodingApp::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::RateDecayFloodingApp")
                                .SetParent<Application>()
                                .SetGroupName("Applications")
                                .AddConstructor<RateDecayFloodingApp>()
                                .AddAttribute("Port", "Port on which we listen for incoming packets.",
                                              UintegerValue(9),
                                              MakeUintegerAccessor(&RateDecayFloodingApp::m_port),
                                              MakeUintegerChecker<uint16_t>())
                                .AddAttribute("SendInterval", "Time between the sending of two packets",
                                              TimeValue(Seconds(1)),
                                              MakeTimeAccessor(&RateDecayFloodingApp::m_sendInterval),
                                              MakeTimeChecker())
                                .AddAttribute("ForwardingJitter", "Time between the sending of two packets",
                                              TimeValue(Seconds(0.11)),
                                              MakeTimeAccessor(&RateDecayFloodingApp::m_forwardingJitter),
                                              MakeTimeChecker())
                                .AddAttribute("PacketSize", "The size of packets transmitted.",
                                              UintegerValue(100),
                                              MakeUintegerAccessor(&RateDecayFloodingApp::m_dataSize),
                                              MakeUintegerChecker<uint32_t>(1))
                                .AddAttribute("MaxDistance", "Maximum Communication range",
                                              DoubleValue(500.0),
                                              MakeDoubleAccessor(&RateDecayFloodingApp::m_maxDistance),
                                              MakeDoubleChecker<double>(0.0))
                                .AddAttribute("DecayFactor", "The decay factor",
                                              DoubleValue(1.0),
                                              MakeDoubleAccessor(&RateDecayFloodingApp::m_decayFactor),
                                              MakeDoubleChecker<double>(0.0))
                                .AddAttribute("MinGain", "Minimum Gain to create a coded packet",
                                              DoubleValue(0.0),
                                              MakeDoubleAccessor(&RateDecayFloodingApp::m_minGain),
                                              MakeDoubleChecker<double>(0.0))
                                .AddAttribute("LossRate", "Preconfigured Loss Rate",
                                              DoubleValue(0.0),
                                              MakeDoubleAccessor(&RateDecayFloodingApp::m_lossRate),
                                              MakeDoubleChecker<double>(0.0))
                                .AddTraceSource("Rx", "A packet has been received",
                                                MakeTraceSourceAccessor(&RateDecayFloodingApp::m_rxTrace),
                                                "ns3::Packet::TracedCallback")
                                .AddTraceSource("RxWithAddresses", "A packet has been received",
                                                MakeTraceSourceAccessor(&RateDecayFloodingApp::m_rxTraceWithAddresses),
                                                "ns3::Packet::TwoAddressTracedCallback")
                                .AddTraceSource("Tx", "A packet has been Sent",
                                                MakeTraceSourceAccessor(&RateDecayFloodingApp::m_txTrace),
                                                "ns3::Packet::TracedCallback")
                                .AddTraceSource("Fwd", "A packet has been Forwarded",
                                                MakeTraceSourceAccessor(&RateDecayFloodingApp::m_fwdTrace),
                                                "ns3::Packet::TracedCallback");
        return tid;
    }

    RateDecayFloodingApp::RateDecayFloodingApp()
    {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        lossEstimator = new BayesianPacketLossEstimator();
    }

    RateDecayFloodingApp::~RateDecayFloodingApp()
    {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
    }

    void
    RateDecayFloodingApp::DoDispose(void)
    {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    double RateDecayFloodingApp::GetNcGain(Vector e, Vector f, Vector o) {
        double p_e = m_lossRate;
        double p_s = 1 -p_e;
        double rho = 12 / 1e6;

        double R = m_maxDistance;
        double A = M_PI * pow(R,2);
        double d_FO = CalculateDistance(o,f);
        double d_EO = CalculateDistance(o,e);
        double d_FE = CalculateDistance(e,f);

        double alpha_FO = acos(d_FO / (2 * R));
        double alpha_EO = acos(d_EO / (2 * R));
        double alpha_FE = acos(d_FE / (2 * R));

        double overlap_FO = 2 * alpha_FO * pow(R,2) - d_FO * R * sin(alpha_FO);
        double overlap_EO = 2 * alpha_EO * pow(R,2) - d_EO * R * sin(alpha_EO);
        double overlap_FE = 2 * alpha_FE * pow(R,2) - d_FE * R * sin(alpha_FE);

        double A_0 = A - overlap_FE;
        double A_1 = overlap_FE - overlap_FO;
        double A_2 = overlap_EO - overlap_FO;
        double A_3 = overlap_FO;

        double xR = p_s * rho * (A_0 + (A_1 + A_3) * p_e);
        double xR_NC = p_s * rho * ((A_1 + A_2) * p_s + A_3 * 2 * p_e * p_s);

        return xR_NC - xR;
    }

    void
    RateDecayFloodingApp::StartApplication(void)
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
            if (m_lossRate > 1.0) {
                ScheduleLossUpdate(Seconds(0.5));
            }
        }

        m_socket->SetRecvCallback(MakeCallback(&RateDecayFloodingApp::HandleRead, this));
    }

    void
    RateDecayFloodingApp::StopApplication()
    {
        NS_LOG_FUNCTION(this);

        if (m_socket != 0)
        {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }

    void
    RateDecayFloodingApp::ScheduleTransmit(Time dt)
    {
        NS_LOG_FUNCTION(this << dt);
        m_sendEvent = Simulator::Schedule(dt, &RateDecayFloodingApp::Send, this);
    }

    void RateDecayFloodingApp::ScheduleLossUpdate(Time dt) {
        if (numRxFailures + numRxSuccess > 0) {
            lossEstimator->reportObservation(numRxSuccess, numRxFailures);

            // NS_LOG_UNCOND("LOSSES");
            // NS_LOG_UNCOND(numRxFailures);
            // NS_LOG_UNCOND("RECEPTIONS");
            // NS_LOG_UNCOND(numRxSuccess);

            numRxFailures = 0;
            numRxSuccess = 0;

            m_lossRate = lossEstimator->getExpectedLoss();

            // NS_LOG_UNCOND("LOSS");
            // NS_LOG_UNCOND(m_lossRate);
        }

        m_lossUpdateEvent = Simulator::Schedule(dt, &RateDecayFloodingApp::ScheduleLossUpdate, this, dt);

    }

    void RateDecayFloodingApp::RecordLossEvents(uint32_t nodeId, Time newTime, Vector newPosition, uint32_t numHops) {
        if (lastReceivedPosition.count(nodeId) == 0 || lastDirectlyReceived.count(nodeId) == 0) {
            return;
        }

        Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();

        double oldDist = CalculateDistance(lastReceivedPosition[nodeId], nodePos);
        double newDist = CalculateDistance(newPosition, nodePos);

        // if (numHops > 1) {
        //     // this is from too far away
        //     return;
        // }

        if (newDist <= m_maxDistance && numHops > 0) {
            // This is a packet on detour
            return;
        }

        if(oldDist >= m_maxDistance) {
            return;
        }

        Time timeSinceLastReception = Simulator::Now() - lastDirectlyReceived[nodeId];

        int losses = floor(timeSinceLastReception.GetMilliSeconds() / m_sendInterval.GetMilliSeconds()) -1;

        numRxSuccess += 1;
        numRxFailures += losses;
    }

    void RateDecayFloodingApp::Forward(uint32_t src, string pkt_id)
    {

        if (!packetsToForward.count(src)) {
            return;
        }
        Ptr<Packet> packet = packetsToForward[src];
        ContentionBasedFloodingHeader currentHeader;
        packet->PeekHeader(currentHeader);
        Ptr<Packet> encodedPacket = nullptr;
        double largestGain = 0.0;
        vector<Ptr<Packet>> forwardingBuffer;

        for(auto it = packetsToForward.begin(); it != packetsToForward.end(); it++) {
            auto packetCopy = it->second->Copy();
            ContentionBasedFloodingHeader header;
            packetCopy->RemoveHeader(header);
            string c_pkt_id = to_string(it->first) + "-" + to_string(header.GetSeq());
            packetCopy->AddHeader(header);
            if ((src != it->first)&& std::find(twiceSeenSeqNos.begin(), twiceSeenSeqNos.end(), c_pkt_id) == twiceSeenSeqNos.end())
            {
                forwardingBuffer.push_back(packetCopy);
                double gain = GetNcGain(GetNode()->GetObject<MobilityModel>()->GetPosition(), currentHeader.GetLastPos(), header.GetLastPos());
                if(gain > m_minGain && gain > largestGain) {
                    largestGain = gain;
                    encodedPacket = packetCopy;
                }
            }

        }


        if (std::find(twiceSeenSeqNos.begin(), twiceSeenSeqNos.end(), pkt_id) == twiceSeenSeqNos.end())
        {
            Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();
            m_fwdTrace(packet, GetNode()->GetId(), forwardingBuffer, nodePos, m_lossRate);

            ContentionBasedFloodingHeader header;
            packet->RemoveHeader(header);

            RateDecayFloodingNcHeader ncHeader;

            ncHeader.SetSeqLeft(header.GetSeq());
            ncHeader.SetSrcLeft(header.GetSrc());
            ncHeader.SetStartPosLeft(header.GetStartPos());
            ncHeader.SetNumHopsLeft(header.GetNumHops());
            ncHeader.SetTsLeft(header.GetTs());
            ncHeader.SetSeqLeft(header.GetSeq());

            ncHeader.SetLastHop(GetNode()->GetId());
            ncHeader.SetLastPos(nodePos);

            if(encodedPacket != nullptr) {
                encodedPacket->RemoveHeader(header);
                ncHeader.SetSeqRight(header.GetSeq());
                ncHeader.SetSrcRight(header.GetSrc());
                ncHeader.SetStartPosRight(header.GetStartPos());
                ncHeader.SetNumHopsRight(header.GetNumHops());
                ncHeader.SetTsRight(header.GetTs());
                ncHeader.SetSeqRight(header.GetSeq());
                packetsToForward.erase(header.GetSrc());

                numSentCoded++;
            }

            packet->AddHeader(ncHeader);

            m_socket->Send(packet);
            numForwarded++;
        }
        packetsToForward.erase(src);
    }

    void
    RateDecayFloodingApp::Send(void)
    {
        NS_LOG_FUNCTION(this);

        NS_ASSERT(m_sendEvent.IsExpired());
        Address localAddress;
        m_socket->GetSockName(localAddress);

        Ptr<Packet> p = Create<Packet>(m_dataSize);
        uint32_t nodeId = GetNode()->GetId();

        Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();

        // ContentionBasedFloodingHeader header;
        // header.SetSeq(this->seqNo++);
        // header.SetSrc(nodeId);
        // header.SetLastHop(nodeId);
        // header.SetLastPos(nodePos);
        // header.SetStartPos(nodePos);

        RateDecayFloodingNcHeader header;
        header.SetSeqLeft(this->seqNo++);
        header.SetSrcLeft(nodeId);
        header.SetLastHop(nodeId);
        header.SetLastPos(nodePos);
        header.SetStartPosLeft(nodePos);

        p->AddHeader(header);
        m_txTrace(p, GetNode()->GetId(), m_lossRate);
        numSent++;
        m_socket->Send(p);
        ScheduleTransmit(m_sendInterval);

        string pkt_id = to_string(GetNode()->GetId()) + "-" + to_string(header.GetSeqLeft());
        seenSeqNos.push_back(pkt_id);
    }

    void
    RateDecayFloodingApp::HandleRead(Ptr<Socket> socket)
    {
        NS_LOG_FUNCTION(this << socket);
        Ptr<Packet> ncPacket;
        Address from;
        Address localAddress;
        socket->GetSockName(localAddress);

        while ((ncPacket = socket->RecvFrom(from)))
        {
            RateDecayFloodingNcHeader ncHeader;
            ncPacket->PeekHeader(ncHeader);

            bool isEncodedPacket = ncHeader.GetSrcRight() != 0;

            Ptr<Packet> leftPacket = Create<Packet>(m_dataSize);
            ContentionBasedFloodingHeader leftHeader;
            leftHeader.SetSeq(ncHeader.GetSeqLeft());
            leftHeader.SetSrc(ncHeader.GetSrcLeft());
            leftHeader.SetLastHop(ncHeader.GetLastHop());
            leftHeader.SetStartPos(ncHeader.GetStartPosLeft());
            leftHeader.SetNumHops(ncHeader.GetNumHopsLeft());
            leftHeader.SetTs(ncHeader.GetTsLeft());
            leftHeader.SetSeq(ncHeader.GetSeqLeft());

            leftHeader.SetLastPos(ncHeader.GetLastPos());
            leftPacket->AddHeader(leftHeader);

            Ptr<Packet> rightPacket = Create<Packet>(m_dataSize);
            ContentionBasedFloodingHeader rightHeader;
            rightHeader.SetSeq(ncHeader.GetSeqRight());
            rightHeader.SetSrc(ncHeader.GetSrcRight());
            rightHeader.SetLastHop(ncHeader.GetLastHop());
            rightHeader.SetStartPos(ncHeader.GetStartPosRight());
            rightHeader.SetNumHops(ncHeader.GetNumHopsRight());
            rightHeader.SetTs(ncHeader.GetTsLeft());
            rightHeader.SetSeq(ncHeader.GetSeqRight());

            rightHeader.SetLastPos(ncHeader.GetLastPos());
            rightPacket->AddHeader(rightHeader);

            if(!isEncodedPacket) {
                this->OnReceive(leftPacket, from, localAddress, false);
            } else {
                string leftPacketId = to_string(leftHeader.GetSrc()) + "-" + to_string(leftHeader.GetSeq());
                string rightPacketId = to_string(rightHeader.GetSrc()) + "-" + to_string(rightHeader.GetSeq());

                if(find(seenSeqNos.begin(), seenSeqNos.end(), leftPacketId) != seenSeqNos.end()) {
                    // leftPacket was seen already, we can decode the right packet
                    this->OnReceive(rightPacket, from, localAddress, true);
                }
                if(find(seenSeqNos.begin(), seenSeqNos.end(), rightPacketId) != seenSeqNos.end()) {
                    // rightPacket was seen already, we can decode the left packet
                    this->OnReceive(leftPacket, from, localAddress, false);
                }
            }
        }
    }

    void RateDecayFloodingApp::OnReceive(Ptr<Packet> packet, Address from, Address localAddress, bool isEncodedPacket) {
            Ptr<Packet> packetCopy = packet->Copy();
            packetCopy->RemoveAllPacketTags();
            packetCopy->RemoveAllByteTags();
            ContentionBasedFloodingHeader header;
            packetCopy->RemoveHeader(header);

            uint32_t src = header.GetSrc();
            uint32_t numHops = header.GetNumHops();
            Vector nodePos = GetNode()->GetObject<MobilityModel>()->GetPosition();
            double dist_sender = CalculateDistance(header.GetStartPos(), nodePos);
            double dist_sender_lastHop = CalculateDistance(header.GetStartPos(), header.GetLastPos());
            double advance = dist_sender - dist_sender_lastHop;

            header.SetNumHops(numHops + 1);
            // header.SetLastHop(GetNode()->GetId());
            // header.SetLastPos(nodePos);

            packetCopy->AddHeader(header);

            string pkt_id = to_string(src) + "-" + to_string(header.GetSeq());

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

                
                RecordLossEvents(src, header.GetTs(), header.GetStartPos(), numHops);

                lastReceived[src] = header.GetTs();
                if (numHops == 0) {
                    lastDirectlyReceived[src] = header.GetTs();
                }
                lastReceivedPosition[src] = header.GetStartPos();

                m_rxTrace(packet, GetNode()->GetId());
                m_rxTraceWithAddresses(packet, from, localAddress);
                numReceived++;
                double scale = (1.0 - (advance / m_maxDistance));


                // if(isEncodedPacket && packetsToForward.count(src) > 0) {
                //     packetsToForward[src] = packetCopy;
                //     seenSeqNos.push_back(pkt_id);
                //     return;
                // }

                if (scale < 0.0)
                {
                    scale = 0.0;
                }

                Time cbfDelay = m_forwardingJitter * scale;
                if(isEncodedPacket) {
                    cbfDelay = cbfDelay + m_forwardingJitter; // Add additional delay for encoded pkts
                }
                Time rdfDelay = Seconds(0);
                if (lastForwarded.find(src) != lastForwarded.end())
                {
                    Time lastForwardedForSrc = lastForwarded[src];
                    // rdfDelay = lastForwardedForSrc + m_sendInterval * pow(m_decayFactor, numHops + 1) - Simulator::Now();
                    rdfDelay = lastForwardedForSrc + m_sendInterval * pow(numHops + 1, m_decayFactor) - Simulator::Now();
                    if (rdfDelay < Seconds(0))
                    {
                        rdfDelay = Seconds(0);
                    }
                }

                Time delay = cbfDelay + rdfDelay;
                if (isEncodedPacket) {
                    delay += m_sendInterval * pow(numHops, m_decayFactor); // adding the rdf delay of the previous hop that might have been skipped
                }
                packetsToForward[src] = packetCopy;

                if (advance > 0)
                {
                    Simulator::Schedule(delay, &RateDecayFloodingApp::Forward, this, src, pkt_id);
                }
                seenSeqNos.push_back(pkt_id);
                lastForwarded[src] = Simulator::Now() + rdfDelay;
            }
            else
            {
                if (std::find(twiceSeenSeqNos.begin(), twiceSeenSeqNos.end(), pkt_id) == twiceSeenSeqNos.end())
                {
                    if(!isEncodedPacket) {
                        twiceSeenSeqNos.push_back(pkt_id);
                    }
                }
            }
    }

    int RateDecayFloodingApp::GetNumUpdatesReceivedInTime()
    {
        return numUpdatesReceivedInTime;
    }

    int RateDecayFloodingApp::GetNumUpdatesReceivedLate()
    {
        return numUpdatesReceivedLate;
    }

    int RateDecayFloodingApp::GetNumSeenNodes()
    {
        return seenNodes.size();
    }

    int RateDecayFloodingApp::GetNumSent()
    {
        return numSent;
    }

    int RateDecayFloodingApp::GetNumSentCoded() {
        return numSentCoded;
    }

    int RateDecayFloodingApp::GetNumFwd()
    {
        return numForwarded;
    }

    int RateDecayFloodingApp::GetNumRcvd()
    {
        return numReceived;
    }

    void RateDecayFloodingApp::ResetStats()
    {
        numUpdatesReceivedInTime = 0;
        numUpdatesReceivedLate = 0;
        seenNodes.clear();
        numSent = 0;
        numSentCoded = 0;
        numReceived = 0;
        numForwarded = 0;
    }

} // Namespace ns3
