#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "rate-decay-flooding-nc-header.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("RateDecayFloodingNcHeader");

    NS_OBJECT_ENSURE_REGISTERED(RateDecayFloodingNcHeader);

    RateDecayFloodingNcHeader::RateDecayFloodingNcHeader()
        : seqLeft(0),
          seqRight(0),
          tsLeft(Simulator::Now().GetTimeStep()),
          tsRight(Simulator::Now().GetTimeStep()),
          srcLeft(0),
          srcRight(0),
          lastHop(0)
    {
        NS_LOG_FUNCTION(this);
    }

    int RateDecayFloodingNcHeader::GetNumHopsLeft()
    {
        return this->numHopsLeft;
    }

    int RateDecayFloodingNcHeader::GetNumHopsRight()
    {
        return this->numHopsRight;
    }

    void RateDecayFloodingNcHeader::SetNumHopsLeft(uint32_t numHops)
    {
        this->numHopsLeft = numHops;
    }

    void RateDecayFloodingNcHeader::SetNumHopsRight(uint32_t numHops)
    {
        this->numHopsRight = numHops;
    }

    uint32_t RateDecayFloodingNcHeader::GetSrcLeft()
    {
        return this->srcLeft;
    }

    uint32_t RateDecayFloodingNcHeader::GetSrcRight()
    {
        return this->srcRight;
    }

    void RateDecayFloodingNcHeader::SetSrcLeft(uint32_t src)
    {
        this->srcLeft = src;
    }

    void RateDecayFloodingNcHeader::SetSrcRight(uint32_t src)
    {
        this->srcRight = src;
    }

    uint32_t RateDecayFloodingNcHeader::GetLastHop()
    {
        return this->lastHop;
    }

    void RateDecayFloodingNcHeader::SetStartPosLeft(Vector pos)
    {
        this->startPosLeft = pos;
    }

        void RateDecayFloodingNcHeader::SetStartPosRight(Vector pos)
    {
        this->startPosRight = pos;
    }

    Vector RateDecayFloodingNcHeader::GetStartPosLeft()
    {
        return this->startPosLeft;
    }

    Vector RateDecayFloodingNcHeader::GetStartPosRight()
    {
        return this->startPosRight;
    }

    void RateDecayFloodingNcHeader::SetLastPos(Vector pos)
    {
        this->lastPos = pos;
    }

    Vector RateDecayFloodingNcHeader::GetLastPos()
    {
        return this->lastPos;
    }

    void RateDecayFloodingNcHeader::SetLastHop(uint32_t lastHop)
    {
        this->lastHop = lastHop;
    }

    void RateDecayFloodingNcHeader::SetSeqLeft(uint32_t seq)
    {
        this->seqLeft = seq;
    }

    void RateDecayFloodingNcHeader::SetSeqRight(uint32_t seq)
    {
        this->seqRight = seq;
    }

    uint32_t RateDecayFloodingNcHeader::GetSeqLeft(void) const
    {
        return seqLeft;
    }

    uint32_t RateDecayFloodingNcHeader::GetSeqRight(void) const
    {
        return seqRight;
    }

    void RateDecayFloodingNcHeader::SetTsLeft (Time ts) {
        this->tsLeft = ts.GetTimeStep();
    }

    void RateDecayFloodingNcHeader::SetTsRight (Time ts) {
        this->tsRight = ts.GetTimeStep();
    }

    Time RateDecayFloodingNcHeader::GetTsLeft(void) const
    {
        return TimeStep(tsLeft);
    }

    Time RateDecayFloodingNcHeader::GetTsRight(void) const
    {
        return TimeStep(tsRight);
    }

    TypeId RateDecayFloodingNcHeader::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::RateDecayFloodingNcHeader")
                                .SetParent<Header>()
                                .SetGroupName("Applications")
                                .AddConstructor<RateDecayFloodingNcHeader>();
        return tid;
    }

    TypeId RateDecayFloodingNcHeader::GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }
    void RateDecayFloodingNcHeader::Print(std::ostream &os) const
    {
        // NS_LOG_FUNCTION(this << &os);
        // os << "(startPos=" << start_pos << " seq=" << seq << " time=" << TimeStep(ts).As(Time::S) << " src=" << src << " lastHop=" << lastHop << " numHops=" << numHops << ")";
    }
    uint32_t RateDecayFloodingNcHeader::GetSerializedSize(void) const
    {
        NS_LOG_FUNCTION(this);
        return (4        // seq
               + 8      // ts
               + 8      // src
               + 8      // numHops
               + 3 * 8)  // startPos
               * 2 // for Left & Right
               + 8      // lastHop
               + 3 * 8; // lastPos
    }

    void RateDecayFloodingNcHeader::Serialize(Buffer::Iterator start) const
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        i.WriteHtonU32(seqLeft);
        i.WriteHtonU32(seqRight);
        i.WriteHtonU64(tsLeft);
        i.WriteHtonU64(tsRight);
        i.WriteHtonU32(srcLeft);
        i.WriteHtonU32(srcRight);
        i.WriteHtonU32(numHopsLeft);
        i.WriteHtonU32(numHopsRight);


        i.WriteHtonU32(lastHop);

        uint32_t startPosLeftX = (uint32_t)startPosLeft.x;
        uint32_t startPosLeftY = (uint32_t)startPosLeft.y;
        uint32_t startPosLeftZ = (uint32_t)startPosLeft.z;

        i.WriteHtonU32(startPosLeftX);
        i.WriteHtonU32(startPosLeftY);
        i.WriteHtonU32(startPosLeftZ);

        uint32_t startPosRightX = (uint32_t)startPosRight.x;
        uint32_t startPosRightY = (uint32_t)startPosRight.y;
        uint32_t startPosRightZ = (uint32_t)startPosRight.z;

        i.WriteHtonU32(startPosRightX);
        i.WriteHtonU32(startPosRightY);
        i.WriteHtonU32(startPosRightZ);

        uint32_t last_pos_x = (uint32_t)lastPos.x;
        uint32_t last_pos_y = (uint32_t)lastPos.y;
        uint32_t last_pos_z = (uint32_t)lastPos.z;

        i.WriteHtonU32(last_pos_x);
        i.WriteHtonU32(last_pos_y);
        i.WriteHtonU32(last_pos_z);
    }

    uint32_t RateDecayFloodingNcHeader::Deserialize(Buffer::Iterator start)
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        seqLeft = i.ReadNtohU32();
        seqRight = i.ReadNtohU32();
        tsLeft = i.ReadNtohU64();
        tsRight = i.ReadNtohU64();
        srcLeft = i.ReadNtohU32();
        srcRight = i.ReadNtohU32();
        numHopsLeft = i.ReadNtohU32();
        numHopsRight = i.ReadNtohU32();

        lastHop = i.ReadNtohU32();

        uint32_t startPosLeftX = i.ReadNtohU32();
        uint32_t startPosLeftY = i.ReadNtohU32();
        uint32_t startPosLeftZ = i.ReadNtohU32();
        startPosLeft = Vector(startPosLeftX, startPosLeftY, startPosLeftZ);

        uint32_t startPosRightX = i.ReadNtohU32();
        uint32_t startPosRightY = i.ReadNtohU32();
        uint32_t startPosRightZ = i.ReadNtohU32();
        startPosRight = Vector(startPosRightX, startPosRightY, startPosRightZ);

        uint32_t last_pos_x = i.ReadNtohU32();
        uint32_t last_pos_y = i.ReadNtohU32();
        uint32_t last_pos_z = i.ReadNtohU32();
        lastPos = Vector(last_pos_x, last_pos_y, last_pos_z);

        return GetSerializedSize();
    }

} // namespace ns3
