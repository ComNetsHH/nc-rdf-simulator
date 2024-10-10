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
#include "ns3/opengym-module.h"

#include "ns3/wifi-standards.h"
#include "ns3/rectangle.h"
#include "ns3/constant-velocity-mobility-model.h"
#include "ns3/application-container.h"
#include "ns3/pure-flooding-application.h"
#include "ns3/flooding-helper.h"
#include "ns3/contention-based-flooding-header.h"
#include "ns3/rate-decay-flooding-application.h"
#include "KpiLogger.h"
#include "RdfEnvironment.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("OpenGym");

KpiLogger kpiLogger = KpiLogger();

int monitoringInterval = 1;
uint32_t simSeed = 1;
double simulationTime = 1; // seconds
double envStepTime = 0.1;  // seconds, ns3gym env step time interval
uint32_t openGymPort = 5555;
uint32_t testArg = 0;

///
Ptr<OpenGymSpace> GetActionSpace(void)
{
  // Should be i/q combination
  uint32_t numActions = 4;
  /**
    0 = i=0.24, q=1.8
    1 = i=0.24, q=1.2
    2 = i=0.36, q=1.8
    3 = i=0.36, q=1.2
  **/
  Ptr<OpenGymDiscreteSpace> space = CreateObject<OpenGymDiscreteSpace>(numActions);
  return space;
}

bool GetGameOver(void)
{
  return false;
}

Ptr<OpenGymSpace> GetObservationSpace(void)
{
  float low = 0.0;
  float high = 500.0;
  std::vector<uint32_t> shape = {
      1,
  };
  std::string dtype = TypeNameGet<uint32_t>();
  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace>(low, high, shape, dtype);
  NS_LOG_UNCOND("MyGetObservationSpace: " << space);
  return space;
}

Ptr<OpenGymDataContainer> GetObservation(void)
{
  std::vector<uint32_t> shape = {
      1,
  };
  Ptr<OpenGymBoxContainer<uint32_t>> box = CreateObject<OpenGymBoxContainer<uint32_t>>(shape);
  box->AddValue(200);

  NS_LOG_UNCOND("MyGetObservation: " << box);
  return box;
}

float GetReward(void)
{
  static float reward = 0.0;
  reward += 1;
  return reward;
}

std::string GetExtraInfo(void)
{
  std::string myInfo = "Waddup brudi";
  NS_LOG_UNCOND("MyGetExtraInfo: " << myInfo);
  return myInfo;
}

void ScheduleNextStateRead(double envStepTime, Ptr<OpenGymInterface> openGym)
{
  Simulator::Schedule(Seconds(envStepTime), &ScheduleNextStateRead, envStepTime, openGym);
  openGym->NotifyCurrentState();
}

///

void GetKPIs(NodeContainer c, int numNodes)
{

  double sumNodesSeen = 0;
  double sumInTime = 0;
  double sumLate = 0;
  for (int i = 0; i < numNodes; i++)
  {
    auto rdfApp = c.Get(i)->GetApplication(0)->GetObject<RateDecayFloodingApp>();
    sumNodesSeen += rdfApp->GetNumSeenNodes();
    sumInTime += rdfApp->GetNumUpdatesReceivedInTime();
    sumLate += rdfApp->GetNumUpdatesReceivedLate();
  }

  double pd = sumNodesSeen / (numNodes * (numNodes - 1));
  double pe500 = sumLate / (sumInTime + sumLate);
  kpiLogger.CreateEntry(pd, pe500);
  NS_LOG_UNCOND(Simulator::Now().As(Time::S) << " P_D = " << pd << ", P_E,500 = " << pe500);

  Simulator::Schedule(Seconds(monitoringInterval), &GetKPIs, c, numNodes);
}

bool ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer>(action);
  NS_LOG_UNCOND("MyExecuteActions: " << action);
  return true;
}

int main(int argc, char *argv[])
{
  NS_LOG_UNCOND("START");
  int simTime = 180;         // seconds
  uint32_t packetSize = 100; // bytes
  uint32_t seed = 0;
  int numNodes = 10;
  int version = 12;
  double interval = 1; // seconds
  double decayFactor = 1.0;
  bool verbose = false;
  int size = 0;
  double speed = 30.0;

  Packet::EnablePrinting();

  CommandLine cmd(__FILE__);
  cmd.AddValue("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue("interval", "interval (seconds) between packets", interval);
  cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue("seed", "seed", seed);
  cmd.AddValue("numNodes", "numNodes", numNodes);
  cmd.AddValue("size", "size", size);
  cmd.AddValue("decayFactor", "decayFactor", decayFactor);
  cmd.AddValue("v", "v", version);
  cmd.AddValue("simTime", "simTime", simTime);
  cmd.AddValue("speed", "speed", speed);
  cmd.Parse(argc, argv);

  kpiLogger.SetFile("res/v" + to_string(version) + "/kpi_rdf_n" + to_string(numNodes) + "_i" + to_string(int(interval * 1000)) + "_q" + to_string(int(decayFactor * 100)) + "_r" + to_string(seed) + ".csv");

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

  string phyMode = "OfdmRate3MbpsBW10MHz"; // OfdmRate3MbpsBW10MHz
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
                            "Speed", StringValue("ns3::ConstantRandomVariable[Constant=" + to_string(speed) + "]"),
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

  RateDecayFloodingAppHelper client(3000, interPacketInterval, interPacketInterval, packetSize, 509.003, decayFactor);

  Ptr<UniformRandomVariable> startTimeRNG = CreateObject<UniformRandomVariable>();

  for (int i = 0; i < numNodes; i++)
  {
    ApplicationContainer apps = client.Install(c.Get(i));
    apps.Start(Seconds(startTimeRNG->GetValue(0.0, 2.0)));
  }

  Simulator::Schedule(Seconds(0), &GetKPIs, c, numNodes);
  // OpenGym Env
  Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface>(openGymPort);
  // Ptr<RdfEnvironment> rdfEnv = CreateObject<RdfEnvironment>(Seconds(envStepTime));
  RdfEnvironment * rdfEnv = new RdfEnvironment();
  rdfEnv->SetOpenGymInterface(openGymInterface);
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  openGymInterface->NotifySimulationEnd();
  Simulator::Destroy();
  NS_LOG_UNCOND("END");

  return 0;
}
