// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Jaume Nin <jnin@cttc.es>
// Modified by: Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
//              Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)

#include "nr-mac-stats-calculator.h"

#include "ns3/string.h"
#include <ns3/log.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrMacStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED(NrMacStatsCalculator);

NrMacStatsCalculator::NrMacStatsCalculator()
    : m_dlFirstWrite(true),
      m_ulFirstWrite(true)
{
    NS_LOG_FUNCTION(this);
}

NrMacStatsCalculator::~NrMacStatsCalculator()
{
    NS_LOG_FUNCTION(this);
    if (m_dlOutFile.is_open())
    {
        m_dlOutFile.close();
    }

    if (m_ulOutFile.is_open())
    {
        m_ulOutFile.close();
    }
}

TypeId
NrMacStatsCalculator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrMacStatsCalculator")
            .SetParent<NrStatsCalculator>()
            .SetGroupName("Nr")
            .AddConstructor<NrMacStatsCalculator>()
            .AddAttribute("DlOutputFilename",
                          "Name of the file where the downlink results will be saved.",
                          StringValue("DlMacStats.txt"),
                          MakeStringAccessor(&NrMacStatsCalculator::SetDlOutputFilename),
                          MakeStringChecker())
            .AddAttribute("UlOutputFilename",
                          "Name of the file where the uplink results will be saved.",
                          StringValue("UlMacStats.txt"),
                          MakeStringAccessor(&NrMacStatsCalculator::SetUlOutputFilename),
                          MakeStringChecker());
    return tid;
}

void
NrMacStatsCalculator::SetUlOutputFilename(std::string outputFilename)
{
    NrStatsCalculator::SetUlOutputFilename(outputFilename);
}

std::string
NrMacStatsCalculator::GetUlOutputFilename()
{
    return NrStatsCalculator::GetUlOutputFilename();
}

void
NrMacStatsCalculator::SetDlOutputFilename(std::string outputFilename)
{
    NrStatsCalculator::SetDlOutputFilename(outputFilename);
}

std::string
NrMacStatsCalculator::GetDlOutputFilename()
{
    return NrStatsCalculator::GetDlOutputFilename();
}

void
NrMacStatsCalculator::DlScheduling(uint16_t cellId,
                                   uint64_t imsi,
                                   nr::DlSchedulingCallbackInfo dlSchedulingCallbackInfo)
{
    NS_LOG_FUNCTION(
        this << cellId << imsi << dlSchedulingCallbackInfo.frameNo
             << dlSchedulingCallbackInfo.subframeNo << dlSchedulingCallbackInfo.rnti
             << (uint32_t)dlSchedulingCallbackInfo.mcsTb1 << dlSchedulingCallbackInfo.sizeTb1
             << (uint32_t)dlSchedulingCallbackInfo.mcsTb2 << dlSchedulingCallbackInfo.sizeTb2);
    NS_LOG_INFO("Write DL Mac Stats in " << GetDlOutputFilename());

    if (m_dlFirstWrite)
    {
        m_dlOutFile.open(GetDlOutputFilename());
        if (!m_dlOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetDlOutputFilename());
            return;
        }
        m_dlFirstWrite = false;
        m_dlOutFile
            << "% time\tcellId\tIMSI\tframe\tsframe\tRNTI\tmcsTb1\tsizeTb1\tmcsTb2\tsizeTb2\tccId";
        m_dlOutFile << "\n";
    }

    m_dlOutFile << Simulator::Now().GetSeconds() << "\t";
    m_dlOutFile << (uint32_t)cellId << "\t";
    m_dlOutFile << imsi << "\t";
    m_dlOutFile << dlSchedulingCallbackInfo.frameNo << "\t";
    m_dlOutFile << dlSchedulingCallbackInfo.subframeNo << "\t";
    m_dlOutFile << dlSchedulingCallbackInfo.rnti << "\t";
    m_dlOutFile << (uint32_t)dlSchedulingCallbackInfo.mcsTb1 << "\t";
    m_dlOutFile << dlSchedulingCallbackInfo.sizeTb1 << "\t";
    m_dlOutFile << (uint32_t)dlSchedulingCallbackInfo.mcsTb2 << "\t";
    m_dlOutFile << dlSchedulingCallbackInfo.sizeTb2 << "\t";
    m_dlOutFile << (uint32_t)dlSchedulingCallbackInfo.componentCarrierId << std::endl;
}

void
NrMacStatsCalculator::UlScheduling(uint16_t cellId,
                                   uint64_t imsi,
                                   uint32_t frameNo,
                                   uint32_t subframeNo,
                                   uint16_t rnti,
                                   uint8_t mcsTb,
                                   uint16_t size,
                                   uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this << cellId << imsi << frameNo << subframeNo << rnti << (uint32_t)mcsTb
                         << size);
    NS_LOG_INFO("Write UL Mac Stats in " << GetUlOutputFilename());

    if (m_ulFirstWrite)
    {
        m_ulOutFile.open(GetUlOutputFilename());
        if (!m_ulOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetUlOutputFilename());
            return;
        }
        m_ulFirstWrite = false;
        m_ulOutFile << "% time\tcellId\tIMSI\tframe\tsframe\tRNTI\tmcs\tsize\tccId";
        m_ulOutFile << "\n";
    }

    m_ulOutFile << Simulator::Now().GetSeconds() << "\t";
    m_ulOutFile << (uint32_t)cellId << "\t";
    m_ulOutFile << imsi << "\t";
    m_ulOutFile << frameNo << "\t";
    m_ulOutFile << subframeNo << "\t";
    m_ulOutFile << rnti << "\t";
    m_ulOutFile << (uint32_t)mcsTb << "\t";
    m_ulOutFile << size << "\t";
    m_ulOutFile << (uint32_t)componentCarrierId << std::endl;
}

void
NrMacStatsCalculator::DlSchedulingCallback(Ptr<NrMacStatsCalculator> macStats,
                                           std::string path,
                                           nr::DlSchedulingCallbackInfo dlSchedulingCallbackInfo)
{
    NS_LOG_FUNCTION(macStats << path);
    uint64_t imsi = 0;
    std::ostringstream pathAndRnti;
    std::string pathEnb = path.substr(0, path.find("/ComponentCarrierMap"));
    pathAndRnti << pathEnb << "/NrGnbRrc/UeMap/" << dlSchedulingCallbackInfo.rnti;
    if (macStats->ExistsImsiPath(pathAndRnti.str()))
    {
        imsi = macStats->GetImsiPath(pathAndRnti.str());
    }
    else
    {
        imsi = FindImsiFromGnbRlcPath(pathAndRnti.str());
        macStats->SetImsiPath(pathAndRnti.str(), imsi);
    }
    uint16_t cellId = 0;
    if (macStats->ExistsCellIdPath(pathAndRnti.str()))
    {
        cellId = macStats->GetCellIdPath(pathAndRnti.str());
    }
    else
    {
        cellId = FindCellIdFromGnbRlcPath(pathAndRnti.str());
        macStats->SetCellIdPath(pathAndRnti.str(), cellId);
    }

    macStats->DlScheduling(cellId, imsi, dlSchedulingCallbackInfo);
}

void
NrMacStatsCalculator::UlSchedulingCallback(Ptr<NrMacStatsCalculator> macStats,
                                           std::string path,
                                           uint32_t frameNo,
                                           uint32_t subframeNo,
                                           uint16_t rnti,
                                           uint8_t mcs,
                                           uint16_t size,
                                           uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(macStats << path);

    uint64_t imsi = 0;
    std::ostringstream pathAndRnti;
    std::string pathEnb = path.substr(0, path.find("/ComponentCarrierMap"));
    pathAndRnti << pathEnb << "/NrGnbRrc/UeMap/" << rnti;
    if (macStats->ExistsImsiPath(pathAndRnti.str()))
    {
        imsi = macStats->GetImsiPath(pathAndRnti.str());
    }
    else
    {
        imsi = FindImsiFromGnbRlcPath(pathAndRnti.str());
        macStats->SetImsiPath(pathAndRnti.str(), imsi);
    }
    uint16_t cellId = 0;
    if (macStats->ExistsCellIdPath(pathAndRnti.str()))
    {
        cellId = macStats->GetCellIdPath(pathAndRnti.str());
    }
    else
    {
        cellId = FindCellIdFromGnbRlcPath(pathAndRnti.str());
        macStats->SetCellIdPath(pathAndRnti.str(), cellId);
    }

    macStats->UlScheduling(cellId, imsi, frameNo, subframeNo, rnti, mcs, size, componentCarrierId);
}

} // namespace ns3
