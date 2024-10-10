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

#ifndef RATE_DECAY_FLOODING_NC_HEADER_H
#define RATE_DECAY_FLOODING_NC_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"

namespace ns3
{
    class RateDecayFloodingNcHeader : public Header
    {
    public:
        RateDecayFloodingNcHeader();

        int GetNumHopsLeft();
        int GetNumHopsRight();
        void SetNumHopsLeft(uint32_t hops);
        void SetNumHopsRight(uint32_t hops);
        void SetSrcLeft(uint32_t src);
        void SetSrcRight(uint32_t src);
        void SetLastHop(uint32_t lastHop);
        uint32_t GetSrcLeft();
        uint32_t GetSrcRight();
        uint32_t GetLastHop();

        void SetTsLeft(Time ts);
        void SetTsRight(Time ts);
        Time GetTsLeft (void) const;
        Time GetTsRight (void) const;
        uint32_t GetSeqLeft (void) const;
        uint32_t GetSeqRight (void) const;
        void SetSeqLeft (uint32_t seq);
        void SetSeqRight (uint32_t seq);

        void SetStartPosLeft (Vector pos);
        Vector GetStartPosLeft ();

        void SetStartPosRight (Vector pos);
        Vector GetStartPosRight ();

        void SetLastPos (Vector pos);
        Vector GetLastPos ();

        static TypeId GetTypeId(void);

        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual uint32_t GetSerializedSize(void) const;
        virtual void Serialize(Buffer::Iterator start) const;
        virtual uint32_t Deserialize(Buffer::Iterator start);

    private:
        uint32_t numHopsLeft = 0;
        uint32_t numHopsRight = 0;
        uint32_t srcLeft = 0;
        uint32_t srcRight = 0;
        uint32_t lastHop = 0;
        uint32_t seqLeft = 0;
        uint32_t seqRight = 0;
        uint64_t tsLeft = 0;
        uint64_t tsRight = 0;
        Vector startPosLeft = Vector(0, 0, 0);
        Vector startPosRight = Vector(0, 0, 0);
        Vector lastPos = Vector(0, 0, 0);

        
    };

} // namespace ns3

#endif /* RATE_DECAY_FLOODING_NC_HEADER_H */
