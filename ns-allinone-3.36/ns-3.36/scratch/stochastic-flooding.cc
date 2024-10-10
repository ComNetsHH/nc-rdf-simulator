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
#include "KpiLogger.h"

using namespace ns3;
using namespace std;
using namespace std::string_literals;

NS_LOG_COMPONENT_DEFINE("SF");

CsvLogger resLogger = CsvLogger();
CsvLogger courseLogger = CsvLogger();
KpiLogger kpiLogger = KpiLogger();

void CourseChange(Ptr<const MobilityModel> mobility, uint32_t nodeId)
{
  courseLogger.CreateCourse(nodeId, mobility);
  Simulator::Schedule(Seconds(0.1), &CourseChange, mobility, nodeId);
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

  string seqNo = to_string(src) + "-" + to_string(header.GetSeq());

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

  string seqNo = to_string(src) + "-" + to_string(header.GetSeq());

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

  string seqNo = to_string(src) + "-" + to_string(header.GetSeq());

  string type = "PktFwd";
  resLogger.CreateEntry(nodeId, seqNo, type, to_string(src), to_string(lastHop), to_string(delay), to_string(numHops));
}

void LogProgress()
{
  NS_LOG_UNCOND(Simulator::Now().As(Time::S));
  Simulator::Schedule(Seconds(5), &LogProgress);
}

void GetKPIs(NodeContainer c, int numNodes)
{

    double sumNodesSeen = 0;
    double sumInTime = 0;
    double sumLate = 0;

    double sumSent = 0;
    double sumRcvd = 0;
    double sumFwd = 0;
    for (int i = 0; i < numNodes; i++)
    {
        auto rdfApp = c.Get(i)->GetApplication(0)->GetObject<PureFloodingApp>();
        sumNodesSeen += rdfApp->GetNumSeenNodes();
        sumInTime += rdfApp->GetNumUpdatesReceivedInTime();
        sumLate += rdfApp->GetNumUpdatesReceivedLate();

        sumSent += rdfApp->GetNumSent();
        sumRcvd += rdfApp->GetNumRcvd();
        sumFwd += rdfApp->GetNumFwd();
    }

    double pd = sumNodesSeen / (numNodes * (numNodes - 1));
    double pe500 = sumLate / (sumInTime + sumLate);
    kpiLogger.CreateEntry(pd, pe500, sumSent, sumRcvd, sumFwd);
    NS_LOG_UNCOND(Simulator::Now().As(Time::S) << " P_D = " << pd << ", P_EX = " << pe500);
}

void ResetStats(Ptr<PureFloodingApp> app) {
    app->ResetStats();
}

int main(int argc, char *argv[])
{
    NS_LOG_UNCOND("START");
    double simTime = 180;         // seconds
    uint32_t packetSize = 100; // bytes
    uint32_t seed = 0;
    int numNodes = 10;
    int version = 12;
    double interval = 1; // seconds
    double forwardingProbability = 1.0;
    double size = 0;
    double speedMin = -1.0;
    double speedMax = -1.0;
  bool tracing = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("packetSize", "size of application packet sent", packetSize);
    cmd.AddValue("interval", "interval (seconds) between packets", interval);
    cmd.AddValue("seed", "seed", seed);
    cmd.AddValue("numNodes", "numNodes", numNodes);
    cmd.AddValue("size", "size", size);
    cmd.AddValue("forwardingProbability", "forwardingProbability", forwardingProbability);
    cmd.AddValue("v", "v", version);
    cmd.AddValue("simTime", "simTime", simTime);
    cmd.AddValue("speedMax", "speedMax", speedMax);
    cmd.AddValue("speedMin", "speedMin", speedMin);
  cmd.AddValue("tracing", "tracing", tracing);
    cmd.Parse(argc, argv);

    kpiLogger.SetFile("res/v" + to_string(version) + "/kpi_sf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_p" + to_string(int(forwardingProbability * 100)) + "_r" + to_string(seed) + ".csv");

  if (tracing)
  {
    resLogger.SetFile("res/v" + to_string(version) + "/sf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_p" + to_string(int(forwardingProbability * 100)) + "_r" + to_string(seed) + ".csv");
    courseLogger.SetFile("res/v" + to_string(version) + "/course_sf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_p" + to_string(int(forwardingProbability * 100)) + "_r" + to_string(seed) + ".csv");
  }

    ns3::SeedManager::SetSeed(seed + 10);

    // Convert to time object
    Time interPacketInterval = Seconds(interval);

    NodeContainer c;
    c.Create(numNodes);

    // The below set of helpers will help us to put together the wifi NICs we want
    WifiHelper wifi;

    string phyMode = "OfdmRate3MbpsBW10MHz";
    wifi.SetStandard(WIFI_STANDARD_80211p);

    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode));

    YansWifiPhyHelper wifiPhy;
    wifiPhy.Set("RxGain", DoubleValue(0));
    wifiPhy.Set("RxSensitivity", DoubleValue(-85));
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

    ObjectFactory pos;
    pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
    pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=" + to_string(size) + "]"));
    pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=" + to_string(size) + "]"));

    Ptr<PositionAllocator> posAlloc = pos.Create()->GetObject<PositionAllocator>();

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel",
                              "Bounds", RectangleValue(Rectangle(0, size, 0, size)),
                              "Speed", StringValue("ns3::UniformRandomVariable[Min="+ to_string(speedMin) +"|Max=" + to_string(speedMax) + "]"),
                              "Pause", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));

    mobility.SetPositionAllocator(posAlloc);
    mobility.Install(c);

    InternetStackHelper internet;
    internet.Install(c);

    Ipv4AddressHelper ipv4;
    NS_LOG_INFO("Assign IP Addresses.");
    ipv4.SetBase("10.1.0.0", "255.255.0.0");
    Ipv4InterfaceContainer i = ipv4.Assign(devices);

    PureFloodingAppHelper client(3000, interPacketInterval, Seconds(0.01), packetSize, forwardingProbability);

    Ptr<UniformRandomVariable> startTimeRNG = CreateObject<UniformRandomVariable>();

    for (int i = 0; i < numNodes; i++)
    {
        ApplicationContainer apps = client.Install(c.Get(i));
        apps.Start(Seconds(startTimeRNG->GetValue(0.0, 5.0))); 
        Simulator::ScheduleWithContext(c.Get(i)->GetId(), Seconds (5.0), &ResetStats, c.Get(i)->GetApplication(0)->GetObject<PureFloodingApp>());
        if(tracing){
      Simulator::ScheduleWithContext(c.Get(i)->GetId(), Seconds(0), &CourseChange, c.Get(i)->GetObject<MobilityModel>(), c.Get(i)->GetId());
    }
    }

      if (tracing)
        {
            Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Rx", MakeCallback(&OnPacketReceive));
            Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Tx", MakeCallback(&OnPacketSent));
            Config::Connect("/NodeList/*/ApplicationList/0/$ns3::PureFloodingApp/Fwd", MakeCallback(&OnPacketForward));
        }


    Simulator::Stop(Seconds(simTime));
    Simulator::Run();

    GetKPIs(c, numNodes);

    Simulator::Destroy();
    NS_LOG_UNCOND("END");

    return 0;
}
