/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#include "ns3/simulator.h"
#include <algorithm>
#include <cmath>
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "uav-mobility-model.h"
#include "position-allocator.h"

namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("UavMobilityModel");

  NS_OBJECT_ENSURE_REGISTERED(UavMobilityModel);

  TypeId
  UavMobilityModel::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::UavMobilityModel")
                            .SetParent<MobilityModel>()
                            .SetGroupName("Mobility")
                            .AddConstructor<UavMobilityModel>()
                            .AddAttribute("Bounds", "The 2d bounding area",
                                          RectangleValue(Rectangle(-100, 100, -100, 100)),
                                          MakeRectangleAccessor(&UavMobilityModel::m_bounds),
                                          MakeRectangleChecker())
                            .AddAttribute("Speed", "A random variable to control the speed (m/s).",
                                          StringValue("ns3::UniformRandomVariable[Min=1.0|Max=2.0]"),
                                          MakePointerAccessor(&UavMobilityModel::m_speed),
                                          MakePointerChecker<RandomVariableStream>())
                            .AddAttribute("Pause", "A random variable to control the pause (s).",
                                          StringValue("ns3::ConstantRandomVariable[Constant=2.0]"),
                                          MakePointerAccessor(&UavMobilityModel::m_pause),
                                          MakePointerChecker<RandomVariableStream>())
                            .AddAttribute("PositionAllocator",
                                          "The position model used to pick a destination point.",
                                          PointerValue(),
                                          MakePointerAccessor(&UavMobilityModel::m_position),
                                          MakePointerChecker<PositionAllocator>());
    return tid;
  }

  UavMobilityModel::UavMobilityModel()
  {
    m_direction = CreateObject<UniformRandomVariable>();
  }

  void
  UavMobilityModel::DoDispose(void)
  {
    // chain up.
    MobilityModel::DoDispose();
  }
  void
  UavMobilityModel::DoInitialize(void)
  {
    DoInitializePrivate();
    MobilityModel::DoInitialize();
  }

  void
  UavMobilityModel::DoInitializePrivate(void)
  {

    double direction = m_direction->GetValue(0, 2 * M_PI);
    home = m_helper.GetCurrentPosition();
    SetDirectionAndSpeed(direction);
  }

  void
  UavMobilityModel::BeginPause(void)
  {
    m_helper.Update();
    m_helper.Pause();
    Time pause = Seconds(m_pause->GetValue());
    m_event.Cancel();
    m_event = Simulator::Schedule(pause, &UavMobilityModel::ResetDirectionAndSpeed, this);
    NotifyCourseChange();
  }

  void
  UavMobilityModel::SetDirectionAndSpeed(double direction)
  {
    NS_LOG_FUNCTION_NOARGS();
    m_helper.UpdateWithBounds(m_bounds);
    Vector position = m_helper.GetCurrentPosition();

    Time delay;
    double speed = m_speed->GetValue();

    if (CalculateDistance(home, position) >= 0.1)
    {
      // return home
      double dx = (home.x - position.x);
      double dy = (home.y - position.y);
      double dz = (home.z - position.z);
      double k = speed / std::sqrt(dx * dx + dy * dy + dz * dz);

      m_helper.SetVelocity(Vector(k * dx, k * dy, k * dz));
      m_helper.Unpause();
      double dist = CalculateDistance(home, position);
      delay = Seconds(dist / speed);
    }
    else
    {
      const Vector vector(std::cos(direction) * speed,
                          std::sin(direction) * speed,
                          0.0);
      m_helper.SetVelocity(vector);
      m_helper.Unpause();
      Vector next = m_bounds.CalculateIntersection(position, vector);
      double dist = std::min(CalculateDistance(position, next), range);
      delay = Seconds(dist / speed);
    }

    m_event.Cancel();
    m_event = Simulator::Schedule(delay,
                                  &UavMobilityModel::BeginPause, this);
    NotifyCourseChange();
  }
  void
  UavMobilityModel::ResetDirectionAndSpeed(void)
  {
    double direction = m_direction->GetValue(0, M_PI);

    m_helper.UpdateWithBounds(m_bounds);
    Vector position = m_helper.GetCurrentPosition();
    switch (m_bounds.GetClosestSide(position))
    {
    case Rectangle::RIGHT:
      direction += M_PI / 2;
      break;
    case Rectangle::LEFT:
      direction += -M_PI / 2;
      break;
    case Rectangle::TOP:
      direction += M_PI;
      break;
    case Rectangle::BOTTOM:
      direction += 0.0;
      break;
    }
    SetDirectionAndSpeed(direction);
  }
  Vector
  UavMobilityModel::DoGetPosition(void) const
  {
    m_helper.UpdateWithBounds(m_bounds);
    return m_helper.GetCurrentPosition();
  }
  void
  UavMobilityModel::DoSetPosition(const Vector &position)
  {
    m_helper.SetPosition(position);
    m_event.Cancel();
    m_event = Simulator::ScheduleNow(&UavMobilityModel::DoInitializePrivate, this);
  }
  Vector
  UavMobilityModel::DoGetVelocity(void) const
  {
    return m_helper.GetVelocity();
  }
  int64_t
  UavMobilityModel::DoAssignStreams(int64_t stream)
  {
    m_direction->SetStream(stream);
    m_speed->SetStream(stream + 1);
    m_pause->SetStream(stream + 2);
    return 3;
  }

} // namespace ns3
