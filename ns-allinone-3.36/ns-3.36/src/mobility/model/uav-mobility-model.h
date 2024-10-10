#ifndef UAV_MOBILITY_MODEL_H
#define UAV_MOBILITY_MODEL_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/rectangle.h"
#include "ns3/random-variable-stream.h"
#include "mobility-model.h"
#include "constant-velocity-helper.h"
#include "position-allocator.h"

namespace ns3 {

/**
 * \ingroup mobility
 * \brief Random waypoint mobility model.
 *
 * Each object starts by pausing at time zero for the duration governed
 * by the random variable "Pause".  After pausing, the object will pick 
 * a new waypoint (via the PositionAllocator) and a new random speed 
 * via the random variable "Speed", and will begin moving towards the 
 * waypoint at a constant speed.  When it reaches the destination, 
 * the process starts over (by pausing).
 *
 * This mobility model enforces no bounding box by itself; the 
 * PositionAllocator assigned to this object will bound the movement.
 * If the user fails to provide a pointer to a PositionAllocator to
 * be used to pick waypoints, the simulation program will assert.
 *
 * The implementation of this model is not 2d-specific. i.e. if you provide
 * a 3d random waypoint position model to this mobility model, the model 
 * will still work. There is no 3d position allocator for now but it should
 * be trivial to add one.
 */
class UavMobilityModel : public MobilityModel
{
public:
  /**
   * Register this type with the TypeId system.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  UavMobilityModel ();

private:
  /**
   * Set a new direction and speed
   */
  void ResetDirectionAndSpeed (void);
  /**
   * Pause, cancel currently scheduled event, schedule end of pause event
   */
  void BeginPause (void);
  /**
   * Set new velocity and direction, and schedule next pause event  
   * \param direction (radians)
   */
  void SetDirectionAndSpeed (double direction);
  /**
   * Sets a new random direction and calls SetDirectionAndSpeed
   */
  void DoInitializePrivate (void);
  virtual void DoDispose (void);
  virtual void DoInitialize (void);
  virtual Vector DoGetPosition (void) const;
  virtual void DoSetPosition (const Vector &position);
  virtual Vector DoGetVelocity (void) const;
  virtual int64_t DoAssignStreams (int64_t);

  Vector home;

  double range = 1000.0;

  Ptr<UniformRandomVariable> m_direction; //!< rv to control direction
  Rectangle m_bounds; //!< the 2D bounding area
  Ptr<RandomVariableStream> m_speed; //!< a random variable to control speed
  Ptr<RandomVariableStream> m_pause; //!< a random variable to control pause 
  EventId m_event; //!< event ID of next scheduled event
  ConstantVelocityHelper m_helper; //!< helper for velocity computations
  Ptr<PositionAllocator> m_position; //!< pointer to position allocator
};

} // namespace ns3

#endif /* RANDOM_WAYPOINT_MOBILITY_MODEL_H */
