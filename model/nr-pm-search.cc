/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search.h"

#include "ns3/double.h"
#include "ns3/enum.h"

#include <numeric>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearch");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearch);

TypeId
NrPmSearch::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPmSearch")
            .SetParent<Object>()
            .AddAttribute("RankLimit",
                          "Max MIMO rank is minimum of num UE ports, num gNB ports, and RankLimit",
                          UintegerValue(UINT8_MAX),
                          MakeUintegerAccessor(&NrPmSearch::m_rankLimit),
                          MakeUintegerChecker<uint8_t>())
            .AddAttribute("RankAlgorithm",
                          "Algorithm used to determine the MIMO Rank",
                          EnumValue(NrPmSearch::RankAlgorithm::SVD),
                          MakeEnumAccessor<RankAlgorithm>(&NrPmSearch::m_rankAlgorithm),
                          MakeEnumChecker(RankAlgorithm::SVD,
                                          "SVD",
                                          RankAlgorithm::WaterFilling,
                                          "WaterFilling"))
            .AddAttribute("RankThreshold",
                          "Rank threshold for SVD selection",
                          DoubleValue(std::numeric_limits<double>::epsilon()),
                          MakeDoubleAccessor(&NrPmSearch::m_rankThreshold),
                          MakeDoubleChecker<double>());
    return tid;
}

void
NrPmSearch::SetAmc(Ptr<const NrAmc> amc)
{
    m_amc = amc;
}

void
NrPmSearch::SetGnbParams(bool isDualPol, size_t numHPorts, size_t numVPorts)
{
    m_nGnbPorts = isDualPol ? 2 * numHPorts * numVPorts : numHPorts * numVPorts;
    m_isGnbDualPol = isDualPol;
    m_nGnbHPorts = numHPorts;
    m_nGnbVPorts = numVPorts;
}

void
NrPmSearch::SetUeParams(size_t numTotalPorts)
{
    m_nRxPorts = numTotalPorts;
}

void
NrPmSearch::SetSubbandSize(size_t subbandSize)
{
    m_subbandSize = subbandSize;
}

size_t
NrPmSearch::GetSubbandSize() const
{
    return m_subbandSize;
}

uint8_t
NrPmSearch::SelectRank(NrIntfNormChanMat& channelMatrix) const
{
    uint8_t maxRank = 0;

    // First determine the maximum supported rank
    switch (m_rankAlgorithm)
    {
    case RankAlgorithm::SVD:
        maxRank = channelMatrix.GetEigenWidebandRank(m_rankThreshold);
        break;
    case RankAlgorithm::WaterFilling:
        maxRank = channelMatrix.GetWaterfillingWidebandRank(*m_ranks.rbegin());
        break;
    default:
        NS_ABORT_MSG("Unknown rank algorithm");
    }

    // If our maxRank is not supported, then take the maximum supported
    // todo: find nearest rank, rounding downwards
    if (std::lower_bound(m_ranks.begin(), m_ranks.end(), maxRank) == m_ranks.end())
    {
        maxRank = *m_ranks.rbegin();
    }
    return maxRank;
}

} // namespace ns3
