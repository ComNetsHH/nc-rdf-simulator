#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "contention-based-flooding-header.h"

namespace ns3
{

    NS_LOG_COMPONENT_DEFINE("ContentionBasedFloodingHeader");

    NS_OBJECT_ENSURE_REGISTERED(ContentionBasedFloodingHeader);

    ContentionBasedFloodingHeader::ContentionBasedFloodingHeader()
        : seq(0),
          ts(Simulator::Now().GetTimeStep()),
          src(0),
          lastHop(0)
    {
        NS_LOG_FUNCTION(this);
    }

    int ContentionBasedFloodingHeader::GetNumHops()
    {
        return this->numHops;
    }

    void ContentionBasedFloodingHeader::SetNumHops(uint32_t numHops)
    {
        this->numHops = numHops;
    }

    uint32_t ContentionBasedFloodingHeader::GetSrc()
    {
        return this->src;
    }

    void ContentionBasedFloodingHeader::SetSrc(uint32_t src)
    {
        this->src = src;
    }

    uint32_t ContentionBasedFloodingHeader::GetLastHop()
    {
        return this->lastHop;
    }

    void ContentionBasedFloodingHeader::SetStartPos(Vector pos)
    {
        this->start_pos = pos;
    }

    Vector ContentionBasedFloodingHeader::GetStartPos()
    {
        return this->start_pos;
    }

    void ContentionBasedFloodingHeader::SetLastPos(Vector pos)
    {
        this->last_pos = pos;
    }

    Vector ContentionBasedFloodingHeader::GetLastPos()
    {
        return this->last_pos;
    }

    void ContentionBasedFloodingHeader::SetLastHop(uint32_t lastHop)
    {
        this->lastHop = lastHop;
    }

    void ContentionBasedFloodingHeader::SetSeq(uint32_t seq)
    {
        this->seq = seq;
    }
    uint32_t ContentionBasedFloodingHeader::GetSeq(void) const
    {
        return seq;
    }

    Time ContentionBasedFloodingHeader::GetTs(void) const
    {
        return TimeStep(ts);
    }

    void ContentionBasedFloodingHeader::SetTs (Time ts) {
        this->ts = ts.GetTimeStep();
    }

    TypeId ContentionBasedFloodingHeader::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ContentionBasedFloodingHeader")
                                .SetParent<Header>()
                                .SetGroupName("Applications")
                                .AddConstructor<ContentionBasedFloodingHeader>();
        return tid;
    }

    TypeId ContentionBasedFloodingHeader::GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }
    void ContentionBasedFloodingHeader::Print(std::ostream &os) const
    {
        NS_LOG_FUNCTION(this << &os);
        os << "(startPos=" << start_pos << " seq=" << seq << " time=" << TimeStep(ts).As(Time::S) << " src=" << src << " lastHop=" << lastHop << " numHops=" << numHops << ")";
    }
    uint32_t ContentionBasedFloodingHeader::GetSerializedSize(void) const
    {
        NS_LOG_FUNCTION(this);
        return 4        // seq
               + 8      // ts
               + 8      // src
               + 8      // lastHop
               + 8      // numHops
               + 3 * 8  // startPos
               + 3 * 8; // lastPos
    }

    void ContentionBasedFloodingHeader::Serialize(Buffer::Iterator start) const
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

        uint32_t last_pos_x = (uint32_t)last_pos.x;
        uint32_t last_pos_y = (uint32_t)last_pos.y;
        uint32_t last_pos_z = (uint32_t)last_pos.z;

        i.WriteHtonU32(last_pos_x);
        i.WriteHtonU32(last_pos_y);
        i.WriteHtonU32(last_pos_z);
    }

    uint32_t ContentionBasedFloodingHeader::Deserialize(Buffer::Iterator start)
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

        uint32_t last_pos_x = i.ReadNtohU32();
        uint32_t last_pos_y = i.ReadNtohU32();
        uint32_t last_pos_z = i.ReadNtohU32();
        last_pos = Vector(last_pos_x, last_pos_y, last_pos_z);

        return GetSerializedSize();
    }

} // namespace ns3
