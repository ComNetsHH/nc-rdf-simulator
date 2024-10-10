/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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

#ifndef CONTENTION_BASED_FLOODING_HEADER_H
#define CONTENTION_BASED_FLOODING_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"

namespace ns3
{
    class ContentionBasedFloodingHeader : public Header
    {
    public:
        ContentionBasedFloodingHeader();

        int GetNumHops();
        void SetNumHops(uint32_t hops);
        void SetSrc(uint32_t src);
        void SetLastHop(uint32_t lastHop);
        uint32_t GetSrc();
        uint32_t GetLastHop();
        void SetTs (Time ts);
        Time GetTs (void) const;
        uint32_t GetSeq (void) const;
        void SetSeq (uint32_t seq);

        void SetStartPos (Vector pos);
        Vector GetStartPos ();

        void SetLastPos (Vector pos);
        Vector GetLastPos ();

        static TypeId GetTypeId(void);

        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual uint32_t GetSerializedSize(void) const;
        virtual void Serialize(Buffer::Iterator start) const;
        virtual uint32_t Deserialize(Buffer::Iterator start);

    private:
        uint32_t numHops = 0;
        uint32_t src = 0;
        uint32_t lastHop = 0;
        uint32_t seq = 0;
        uint64_t ts = 0;
        Vector start_pos = Vector(0, 0, 0);
        Vector last_pos = Vector(0, 0, 0);

        
    };

} // namespace ns3

#endif /* CONTENTION_BASED_FLOODING_HEADER_H */
