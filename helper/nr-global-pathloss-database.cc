// Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#include "nr-global-pathloss-database.h"

#include "ns3/nr-gnb-net-device.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/nr-ue-net-device.h"

#include <limits>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrGlobalPathlossDatabase");

NrGlobalPathlossDatabase::~NrGlobalPathlossDatabase()
{
}

void
NrGlobalPathlossDatabase::Print()
{
    NS_LOG_FUNCTION(this);
    for (auto cellIdIt = m_pathlossMap.begin(); cellIdIt != m_pathlossMap.end(); ++cellIdIt)
    {
        for (auto imsiIt = cellIdIt->second.begin(); imsiIt != cellIdIt->second.end(); ++imsiIt)
        {
            std::cout << "CellId: " << cellIdIt->first << " IMSI: " << imsiIt->first
                      << " pathloss: " << imsiIt->second << " dB" << std::endl;
        }
    }
}

double
NrGlobalPathlossDatabase::GetPathloss(uint16_t cellId, uint64_t imsi)
{
    NS_LOG_FUNCTION(this);
    auto cellIt = m_pathlossMap.find(cellId);
    if (cellIt == m_pathlossMap.end())
    {
        return std::numeric_limits<double>::infinity();
    }
    auto ueIt = cellIt->second.find(imsi);
    if (ueIt == cellIt->second.end())
    {
        return std::numeric_limits<double>::infinity();
    }
    return ueIt->second;
}

void
DownlinkNrGlobalPathlossDatabase::UpdatePathloss(std::string context,
                                                 Ptr<const SpectrumPhy> txPhy,
                                                 Ptr<const SpectrumPhy> rxPhy,
                                                 double lossDb)
{
    NS_LOG_FUNCTION(this << lossDb);
    uint16_t cellId = txPhy->GetDevice()->GetObject<NrGnbNetDevice>()->GetCellId();
    uint16_t imsi = rxPhy->GetDevice()->GetObject<NrUeNetDevice>()->GetImsi();
    m_pathlossMap[cellId][imsi] = lossDb;
}

void
UplinkNrGlobalPathlossDatabase::UpdatePathloss(std::string context,
                                               Ptr<const SpectrumPhy> txPhy,
                                               Ptr<const SpectrumPhy> rxPhy,
                                               double lossDb)
{
    NS_LOG_FUNCTION(this << lossDb);
    uint16_t imsi = txPhy->GetDevice()->GetObject<NrUeNetDevice>()->GetImsi();
    uint16_t cellId = rxPhy->GetDevice()->GetObject<NrGnbNetDevice>()->GetCellId();
    m_pathlossMap[cellId][imsi] = lossDb;
}

} // namespace ns3
