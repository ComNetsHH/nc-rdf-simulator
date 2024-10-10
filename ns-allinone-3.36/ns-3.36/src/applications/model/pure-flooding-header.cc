#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "pure-flooding-header.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("PureFloodingHeader");

    NS_OBJECT_ENSURE_REGISTERED(PureFloodingHeader);

    PureFloodingHeader::PureFloodingHeader()
        : seq(0),
          ts(Simulator::Now().GetTimeStep()),
          src(0),
          lastHop(0)
    {
        NS_LOG_FUNCTION(this);
    }

    int PureFloodingHeader::GetNumHops()
    {
        return this->numHops;
    }

    void PureFloodingHeader::SetNumHops(uint32_t numHops)
    {
        this->numHops = numHops;
    }

    uint32_t PureFloodingHeader::GetSrc()
    {
        return this->src;
    }

    void PureFloodingHeader::SetSrc(uint32_t src)
    {
        this->src = src;
    }

    void PureFloodingHeader::SetStartPos(Vector pos)
    {
        this->start_pos = pos;
    }

    Vector PureFloodingHeader::GetStartPos()
    {
        return this->start_pos;
    }

    uint32_t PureFloodingHeader::GetLastHop()
    {
        return this->lastHop;
    }

    void PureFloodingHeader::SetLastHop(uint32_t lastHop)
    {
        this->lastHop = lastHop;
    }

    void PureFloodingHeader::SetSeq(uint32_t seq)
    {
        this->seq = seq;
    }
    uint32_t PureFloodingHeader::GetSeq(void) const
    {
        return seq;
    }

    Time PureFloodingHeader::GetTs(void) const
    {
        return TimeStep(ts);
    }

    TypeId PureFloodingHeader::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::PureFloodingHeader")
                                .SetParent<Header>()
                                .SetGroupName("Applications")
                                .AddConstructor<PureFloodingHeader>();
        return tid;
    }

    TypeId PureFloodingHeader::GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }
    void PureFloodingHeader::Print(std::ostream &os) const
    {
        NS_LOG_FUNCTION(this << &os);
        os << "(seq=" << seq << " time=" << TimeStep(ts).As(Time::S) << " src=" << src << " lastHop=" << lastHop << " numHops=" << numHops << ")";
    }
    uint32_t PureFloodingHeader::GetSerializedSize(void) const
    {
        NS_LOG_FUNCTION(this);
        return 4    // seq
               + 8  // ts
               + 8  // src
               + 8  // lastHop
               + 8 // numHops
               + 3 * 8;  // startPos
    }

    void PureFloodingHeader::Serialize(Buffer::Iterator start) const
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        i.WriteHtonU32(seq);
        i.WriteHtonU64(ts);
        i.WriteHtonU32(src);
        i.WriteHtonU32(lastHop);
        i.WriteHtonU32(numHops);

        uint32_t start_pos_x = (uint32_t)start_pos.x;
        uint32_t start_pos_y = (uint32_t)start_pos.y;
        uint32_t start_pos_z = (uint32_t)start_pos.z;

        i.WriteHtonU32(start_pos_x);
        i.WriteHtonU32(start_pos_y);
        i.WriteHtonU32(start_pos_z);

    }

    uint32_t PureFloodingHeader::Deserialize(Buffer::Iterator start)
    {
        NS_LOG_FUNCTION(this << &start);
        Buffer::Iterator i = start;
        seq = i.ReadNtohU32();
        ts = i.ReadNtohU64();
        src = i.ReadNtohU32();
        lastHop = i.ReadNtohU32();
        numHops = i.ReadNtohU32();

        uint32_t start_pos_x = i.ReadNtohU32();
        uint32_t start_pos_y = i.ReadNtohU32();
        uint32_t start_pos_z = i.ReadNtohU32();
        start_pos = Vector(start_pos_x, start_pos_y, start_pos_z);

        return GetSerializedSize();
    }

} // namespace ns3
