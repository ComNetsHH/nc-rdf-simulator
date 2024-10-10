/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
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
 *
 */

// This script configures two nodes on an 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to the other node.  The physical layer is configured
// to receive at a fixed RSS (regardless of the distance and transmit
// power); therefore, changing position of the nodes has no effect.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc --help"
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when rss drops below -97 dBm.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --rss=-97 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-98 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-99 --numPackets=20"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the documentation.
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc --verbose=1"
//
// When you are done, you will notice two pcap trace files in your directory.
// If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
//

#include "ns3/core-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/seq-ts-header.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"

#include "ns3/wifi-standards.h"
#include "ns3/rectangle.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/application-container.h"
#include "ns3/pure-flooding-application.h"
#include "ns3/flooding-helper.h"
#include "ns3/pure-flooding-header.h"
#include "CsvLogger.h"

using namespace ns3;
using namespace std;
using namespace std::string_literals;

NS_LOG_COMPONENT_DEFINE("WifiSimpleAdhoc");

CsvLogger resLogger = CsvLogger();
CsvLogger courseLogger = CsvLogger();

void CourseChange (Ptr<const MobilityModel> mobility, uint32_t nodeId) {
  courseLogger.CreateCourse(nodeId, mobility);
  Simulator::Schedule (Seconds (0.1), &CourseChange, mobility, nodeId);
}

void OnPacketReceive(std::string context, Ptr<const Packet> pkt, uint32_t nodeId)
{
    PureFloodingHeader header;
    pkt->PeekHeader(header);

    double current = Simulator::Now().GetSeconds();
    double sent = header.GetTs().GetSeconds();
    double delay = (current - sent) * 1000;

    uint32_t src = header.GetSrc();
    uint32_t lastHop = header.GetLastHop();
    uint32_t numHops = header.GetNumHops();

    string seqNo = to_string(nodeId) + "-" + to_string(header.GetSeq());

    string type = "PktRcvd";
    resLogger.CreateEntry(nodeId, seqNo, type, to_string(src), to_string(lastHop), to_string(delay), to_string(numHops));
    // uint32_t node_id, string eventType, string src, string lastHop, string delay, string numHops
}

void OnPacketSent(std::string context, Ptr<const Packet> pkt, uint32_t nodeId)
{
    PureFloodingHeader header;
    pkt->PeekHeader(header);

    uint32_t src = header.GetSrc();
    uint32_t lastHop = header.GetLastHop();
    uint32_t numHops = header.GetNumHops();

    string seqNo = to_string(nodeId) + "-" + to_string(header.GetSeq());
    string type = "PktSent";
    resLogger.CreateEntry(nodeId, seqNo, type, to_string(src), to_string(lastHop), "-1", to_string(numHops));
}

void OnPacketForward(std::string context, Ptr<const Packet> pkt, uint32_t nodeId)
{
    PureFloodingHeader header;
    pkt->PeekHeader(header);

    double current = Simulator::Now().GetSeconds();
    double sent = header.GetTs().GetSeconds();
    double delay = (current - sent) * 1000;

    uint32_t src = header.GetSrc();
    uint32_t lastHop = header.GetLastHop();
    uint32_t numHops = header.GetNumHops();

    string seqNo = to_string(nodeId) + "-" + to_string(header.GetSeq());
    string type = "PktFwd";
    resLogger.CreateEntry(nodeId, seqNo, type, to_string(src), to_string(lastHop), to_string(delay), to_string(numHops));
}

int main(int argc, char *argv[])
{
    NS_LOG_UNCOND("START");
    int simTime = 180;         // seconds
    uint32_t packetSize = 100; // bytes
    uint32_t seed = 0;
    int numNodes = 10;
    int version = 7;
    double interval = 1; // seconds
    bool verbose = false;
    int size = 5000;

    Packet::EnablePrinting();

    CommandLine cmd(__FILE__);
    cmd.AddValue("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue("interval", "interval (seconds) between packets", interval);
    cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
    cmd.AddValue("seed", "seed", seed);
    cmd.AddValue("numNodes", "numNodes", numNodes);
    cmd.AddValue("size", "size", size);
    cmd.AddValue("v", "v", version);
    cmd.Parse(argc, argv);

    resLogger.SetFile("res/v" + to_string(version) + "/pf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_r" + to_string(seed) + ".csv");
    courseLogger.SetFile("res/v" + to_string(version) + "/course_pf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_r" + to_string(seed) + ".csv");

    ns3::SeedManager::SetSeed(seed + 10);

    // Convert to time object
    Time interPacketInterval = Seconds(interval);

    NodeContainer c;
    c.Create(numNodes);

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;
    if (verbose)
    {
        wifi.EnableLogComponents(); // Turn on all Wifi logging
    }

    string phyMode = "OfdmRate3MbpsBW10MHz";
    wifi.SetStandard(WIFI_STANDARD_80211p);

    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("RxGain", DoubleValue(0));
    wifiPhy.Set("RxSensitivity", DoubleValue(-89));
    wifiPhy.Set("ChannelWidth", UintegerValue(10));
    wifiPhy.Set("TxPowerStart", DoubleValue(20));
    wifiPhy.Set("TxPowerEnd", DoubleValue(20));
    // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
    // wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5.90e9));
    wifiPhy.SetChannel(wifiChannel.Create());

    // Add a mac and disable rate control
    WifiMacHelper wifiMac;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue(phyMode), "ControlMode", StringValue(phyMode));
    // Set it to adhoc mode
    wifiMac.SetType("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

    // Setup Mobility Model
    Ptr<UniformRandomVariable> uniformPos = CreateObject<UniformRandomVariable>();
    uniformPos->SetAttribute("Min", DoubleValue(0));
    uniformPos->SetAttribute("Max", DoubleValue(size));

    RandomRectanglePositionAllocator pos;
    pos.SetX(uniformPos);
    pos.SetY(uniformPos);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0, size, 0, size)),
                              "Speed", StringValue("ns3::ConstantRandomVariable[Constant=60]"),
                              "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    mobility.SetPositionAllocator(&pos);
    mobility.Install(c);

    // mobility.Install (c);

    InternetStackHelper internet;
    internet.Install(c);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO("Assign IP Addresses.");
    ipv4.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer i = ipv4.Assign(devices);

    PureFloodingAppHelper client(3000, interPacketInterval, interPacketInterval, packetSize, 1.0);

    Ptr<UniformRandomVariable> startTimeRNG = CreateObject<UniformRandomVariable>();

    for (int i = 0; i < numNodes; i++)
    {
        ApplicationContainer apps = client.Install(c.Get(i));
        apps.Start(Seconds(startTimeRNG->GetValue(0.0, 2.0)));
        Simulator::ScheduleWithContext (c.Get(i)->GetId(), Seconds (0), &CourseChange, c.Get(i)->GetObject<MobilityModel>(), c.Get(i)->GetId());
    }

    Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Rx", MakeCallback(&OnPacketReceive));
    Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Tx", MakeCallback(&OnPacketSent));
    Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Fwd", MakeCallback(&OnPacketForward));

    Simulator::Stop(Seconds(simTime));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_UNCOND("END");

    return 0;
}
