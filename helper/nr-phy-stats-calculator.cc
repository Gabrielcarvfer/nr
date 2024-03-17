// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Jaume Nin <jnin@cttc.es>
//         Danilo Abrignani <danilo.abrignani@unibo.it> (Modification due to new Architecture -
// Carrier Aggregation - GSoC 2015)

#include "nr-phy-stats-calculator.h"

#include "ns3/string.h"
#include <ns3/log.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPhyStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED(NrPhyStatsCalculator);

NrPhyStatsCalculator::NrPhyStatsCalculator()
    : m_RsrpSinrFirstWrite(true),
      m_UeSinrFirstWrite(true),
      m_InterferenceFirstWrite(true)
{
    NS_LOG_FUNCTION(this);
}

NrPhyStatsCalculator::~NrPhyStatsCalculator()
{
    NS_LOG_FUNCTION(this);
    if (m_interferenceOutFile.is_open())
    {
        m_interferenceOutFile.close();
    }

    if (m_rsrpOutFile.is_open())
    {
        m_rsrpOutFile.close();
    }

    if (m_ueSinrOutFile.is_open())
    {
        m_ueSinrOutFile.close();
    }
}

TypeId
NrPhyStatsCalculator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPhyStatsCalculator")
            .SetParent<NrStatsCalculator>()
            .SetGroupName("Nr")
            .AddConstructor<NrPhyStatsCalculator>()
            .AddAttribute("DlRsrpSinrFilename",
                          "Name of the file where the RSRP/SINR statistics will be saved.",
                          StringValue("DlRsrpSinrStats.txt"),
                          MakeStringAccessor(&NrPhyStatsCalculator::SetCurrentCellRsrpSinrFilename),
                          MakeStringChecker())
            .AddAttribute("UlSinrFilename",
                          "Name of the file where the UE SINR statistics will be saved.",
                          StringValue("UlSinrStats.txt"),
                          MakeStringAccessor(&NrPhyStatsCalculator::SetUeSinrFilename),
                          MakeStringChecker())
            .AddAttribute("UlInterferenceFilename",
                          "Name of the file where the interference statistics will be saved.",
                          StringValue("UlInterferenceStats.txt"),
                          MakeStringAccessor(&NrPhyStatsCalculator::SetInterferenceFilename),
                          MakeStringChecker());
    return tid;
}

void
NrPhyStatsCalculator::SetCurrentCellRsrpSinrFilename(std::string filename)
{
    m_RsrpSinrFilename = filename;
}

std::string
NrPhyStatsCalculator::GetCurrentCellRsrpSinrFilename()
{
    return m_RsrpSinrFilename;
}

void
NrPhyStatsCalculator::SetUeSinrFilename(std::string filename)
{
    m_ueSinrFilename = filename;
}

std::string
NrPhyStatsCalculator::GetUeSinrFilename()
{
    return m_ueSinrFilename;
}

void
NrPhyStatsCalculator::SetInterferenceFilename(std::string filename)
{
    m_interferenceFilename = filename;
}

std::string
NrPhyStatsCalculator::GetInterferenceFilename()
{
    return m_interferenceFilename;
}

void
NrPhyStatsCalculator::ReportCurrentCellRsrpSinr(uint16_t cellId,
                                                uint64_t imsi,
                                                uint16_t rnti,
                                                double rsrp,
                                                double sinr,
                                                uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this << cellId << imsi << rnti << rsrp << sinr);
    NS_LOG_INFO("Write RSRP/SINR Phy Stats in " << GetCurrentCellRsrpSinrFilename());

    if (m_RsrpSinrFirstWrite)
    {
        m_rsrpOutFile.open(GetCurrentCellRsrpSinrFilename());
        if (!m_rsrpOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetCurrentCellRsrpSinrFilename());
            return;
        }
        m_RsrpSinrFirstWrite = false;
        m_rsrpOutFile << "% time\tcellId\tIMSI\tRNTI\trsrp\tsinr\tComponentCarrierId";
        m_rsrpOutFile << "\n";
    }

    m_rsrpOutFile << Simulator::Now().GetSeconds() << "\t";
    m_rsrpOutFile << cellId << "\t";
    m_rsrpOutFile << imsi << "\t";
    m_rsrpOutFile << rnti << "\t";
    m_rsrpOutFile << rsrp << "\t";
    m_rsrpOutFile << sinr << "\t";
    m_rsrpOutFile << (uint32_t)componentCarrierId << std::endl;
}

void
NrPhyStatsCalculator::ReportUeSinr(uint16_t cellId,
                                   uint64_t imsi,
                                   uint16_t rnti,
                                   double sinrLinear,
                                   uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this << cellId << imsi << rnti << sinrLinear);
    NS_LOG_INFO("Write SINR Linear Phy Stats in " << GetUeSinrFilename());

    if (m_UeSinrFirstWrite)
    {
        m_ueSinrOutFile.open(GetUeSinrFilename());
        if (!m_ueSinrOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetUeSinrFilename());
            return;
        }
        m_UeSinrFirstWrite = false;
        m_ueSinrOutFile << "% time\tcellId\tIMSI\tRNTI\tsinrLinear\tcomponentCarrierId";
        m_ueSinrOutFile << "\n";
    }
    m_ueSinrOutFile << Simulator::Now().GetSeconds() << "\t";
    m_ueSinrOutFile << cellId << "\t";
    m_ueSinrOutFile << imsi << "\t";
    m_ueSinrOutFile << rnti << "\t";
    m_ueSinrOutFile << sinrLinear << "\t";
    m_ueSinrOutFile << (uint32_t)componentCarrierId << std::endl;
}

void
NrPhyStatsCalculator::ReportInterference(uint16_t cellId, Ptr<SpectrumValue> interference)
{
    NS_LOG_FUNCTION(this << cellId << interference);
    NS_LOG_INFO("Write Interference Phy Stats in " << GetInterferenceFilename());

    if (m_InterferenceFirstWrite)
    {
        m_interferenceOutFile.open(GetInterferenceFilename());
        if (!m_interferenceOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetInterferenceFilename());
            return;
        }
        m_InterferenceFirstWrite = false;
        m_interferenceOutFile << "% time\tcellId\tInterference";
        m_interferenceOutFile << "\n";
    }

    m_interferenceOutFile << Simulator::Now().GetSeconds() << "\t";
    m_interferenceOutFile << cellId << "\t";
    m_interferenceOutFile << *interference;
}

void
NrPhyStatsCalculator::ReportCurrentCellRsrpSinrCallback(Ptr<NrPhyStatsCalculator> phyStats,
                                                        std::string path,
                                                        uint16_t cellId,
                                                        uint16_t rnti,
                                                        double rsrp,
                                                        double sinr,
                                                        uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(phyStats << path);
    uint64_t imsi = 0;
    std::string pathUePhy = path.substr(0, path.find("/ComponentCarrierMapUe"));
    if (phyStats->ExistsImsiPath(pathUePhy))
    {
        imsi = phyStats->GetImsiPath(pathUePhy);
    }
    else
    {
        imsi = FindImsiFromNrUeNetDevice(pathUePhy);
        phyStats->SetImsiPath(pathUePhy, imsi);
    }

    phyStats->ReportCurrentCellRsrpSinr(cellId, imsi, rnti, rsrp, sinr, componentCarrierId);
}

void
NrPhyStatsCalculator::ReportUeSinr(Ptr<NrPhyStatsCalculator> phyStats,
                                   std::string path,
                                   uint16_t cellId,
                                   uint16_t rnti,
                                   double sinrLinear,
                                   uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(phyStats << path);

    uint64_t imsi = 0;
    std::ostringstream pathAndRnti;
    pathAndRnti << path << "/" << rnti;
    std::string pathEnbMac = path.substr(0, path.find("/ComponentCarrierMap"));
    pathEnbMac += "/NrEnbMac/DlScheduling";
    if (phyStats->ExistsImsiPath(pathAndRnti.str()))
    {
        imsi = phyStats->GetImsiPath(pathAndRnti.str());
    }
    else
    {
        imsi = FindImsiFromGnbMac(pathEnbMac, rnti);
        phyStats->SetImsiPath(pathAndRnti.str(), imsi);
    }

    phyStats->ReportUeSinr(cellId, imsi, rnti, sinrLinear, componentCarrierId);
}

void
NrPhyStatsCalculator::ReportInterference(Ptr<NrPhyStatsCalculator> phyStats,
                                         std::string path,
                                         uint16_t cellId,
                                         Ptr<SpectrumValue> interference)
{
    NS_LOG_FUNCTION(phyStats << path);
    phyStats->ReportInterference(cellId, interference);
}

} // namespace ns3
