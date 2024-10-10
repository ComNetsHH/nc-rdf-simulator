/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "flooding-helper.h"
#include "ns3/pure-flooding-application.h"
#include "ns3/contention-based-flooding-application.h"
#include "ns3/rate-decay-flooding-application.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

PureFloodingAppHelper::PureFloodingAppHelper (uint16_t port, Time sendInterval, Time forwardingJitter, uint32_t packetSize, double forwardingProbability, uint16_t ttl)
{
  m_factory.SetTypeId (PureFloodingApp::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("SendInterval", TimeValue (sendInterval));
  SetAttribute ("ForwardingJitter", TimeValue (forwardingJitter));
  SetAttribute ("PacketSize", UintegerValue (packetSize));
  SetAttribute ("ForwardingProbability", DoubleValue (forwardingProbability));
  SetAttribute ("TTL", UintegerValue (ttl));
}

void 
PureFloodingAppHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
PureFloodingAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
PureFloodingAppHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
PureFloodingAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
PureFloodingAppHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<PureFloodingApp> ();
  node->AddApplication (app);

  return app;
}

ContentionBasedFloodingAppHelper::ContentionBasedFloodingAppHelper (uint16_t port, Time sendInterval, Time forwardingJitter, uint32_t packetSize, double maxDistance)
{
  m_factory.SetTypeId (ContentionBasedFloodingApp::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("SendInterval", TimeValue (sendInterval));
  SetAttribute ("ForwardingJitter", TimeValue (forwardingJitter));
  SetAttribute ("PacketSize", UintegerValue (packetSize));
  SetAttribute ("MaxDistance", DoubleValue (maxDistance));
}

void 
ContentionBasedFloodingAppHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
ContentionBasedFloodingAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ContentionBasedFloodingAppHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
ContentionBasedFloodingAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
ContentionBasedFloodingAppHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<ContentionBasedFloodingApp> ();
  node->AddApplication (app);

  return app;
}

RateDecayFloodingAppHelper::RateDecayFloodingAppHelper (uint16_t port, Time sendInterval, Time forwardingJitter, uint32_t packetSize, double maxDistance, double decayFactor, double minGain, double lossRate)
{
  m_factory.SetTypeId (RateDecayFloodingApp::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("SendInterval", TimeValue (sendInterval));
  SetAttribute ("ForwardingJitter", TimeValue (forwardingJitter));
  SetAttribute ("PacketSize", UintegerValue (packetSize));
  SetAttribute ("MaxDistance", DoubleValue (maxDistance));
  SetAttribute ("MaxDistance", DoubleValue (maxDistance));
  SetAttribute ("DecayFactor", DoubleValue(decayFactor));
  SetAttribute ("MinGain", DoubleValue(minGain));
  SetAttribute ("LossRate", DoubleValue(lossRate));
  
}

void 
RateDecayFloodingAppHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
RateDecayFloodingAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RateDecayFloodingAppHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RateDecayFloodingAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
RateDecayFloodingAppHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<RateDecayFloodingApp> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
