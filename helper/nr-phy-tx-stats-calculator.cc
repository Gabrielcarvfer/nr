// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Jaume Nin <jnin@cttc.es>
// modified by: Marco Miozzo <mmiozzo@cttc.es>
//        Convert MacStatsCalculator in NrPhyTxStatsCalculator

#include "nr-phy-tx-stats-calculator.h"

#include "ns3/string.h"
#include <ns3/log.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPhyTxStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED(NrPhyTxStatsCalculator);

NrPhyTxStatsCalculator::NrPhyTxStatsCalculator()
    : m_dlTxFirstWrite(true),
      m_ulTxFirstWrite(true)
{
    NS_LOG_FUNCTION(this);
}

NrPhyTxStatsCalculator::~NrPhyTxStatsCalculator()
{
    NS_LOG_FUNCTION(this);
    if (m_dlTxOutFile.is_open())
    {
        m_dlTxOutFile.close();
    }

    if (m_ulTxOutFile.is_open())
    {
        m_ulTxOutFile.close();
    }
}

TypeId
NrPhyTxStatsCalculator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrPhyTxStatsCalculator")
            .SetParent<NrStatsCalculator>()
            .SetGroupName("Nr")
            .AddConstructor<NrPhyTxStatsCalculator>()
            .AddAttribute("DlTxOutputFilename",
                          "Name of the file where the downlink results will be saved.",
                          StringValue("DlTxPhyStats.txt"),
                          MakeStringAccessor(&NrPhyTxStatsCalculator::SetDlTxOutputFilename),
                          MakeStringChecker())
            .AddAttribute("UlTxOutputFilename",
                          "Name of the file where the uplink results will be saved.",
                          StringValue("UlTxPhyStats.txt"),
                          MakeStringAccessor(&NrPhyTxStatsCalculator::SetUlTxOutputFilename),
                          MakeStringChecker());
    return tid;
}

void
NrPhyTxStatsCalculator::SetUlTxOutputFilename(std::string outputFilename)
{
    NrStatsCalculator::SetUlOutputFilename(outputFilename);
}

std::string
NrPhyTxStatsCalculator::GetUlTxOutputFilename()
{
    return NrStatsCalculator::GetUlOutputFilename();
}

void
NrPhyTxStatsCalculator::SetDlTxOutputFilename(std::string outputFilename)
{
    NrStatsCalculator::SetDlOutputFilename(outputFilename);
}

std::string
NrPhyTxStatsCalculator::GetDlTxOutputFilename()
{
    return NrStatsCalculator::GetDlOutputFilename();
}

void
NrPhyTxStatsCalculator::DlPhyTransmission(nr::PhyTransmissionStatParameters params)
{
    NS_LOG_FUNCTION(this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti
                         << params.m_layer << params.m_mcs << params.m_size << params.m_rv
                         << params.m_ndi);
    NS_LOG_INFO("Write DL Tx Phy Stats in " << GetDlTxOutputFilename());

    if (m_dlTxFirstWrite)
    {
        m_dlTxOutFile.open(GetDlOutputFilename());
        if (!m_dlTxOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetDlTxOutputFilename());
            return;
        }
        m_dlTxFirstWrite = false;
        m_dlTxOutFile << "% time\tcellId\tIMSI\tRNTI\tlayer\tmcs\tsize\trv\tndi\tccId";
        m_dlTxOutFile << "\n";
    }

    m_dlTxOutFile << params.m_timestamp << "\t";
    m_dlTxOutFile << (uint32_t)params.m_cellId << "\t";
    m_dlTxOutFile << params.m_imsi << "\t";
    m_dlTxOutFile << params.m_rnti << "\t";
    // m_dlTxOutFile << (uint32_t) params.m_txMode << "\t"; // txMode is not available at dl tx side
    m_dlTxOutFile << (uint32_t)params.m_layer << "\t";
    m_dlTxOutFile << (uint32_t)params.m_mcs << "\t";
    m_dlTxOutFile << params.m_size << "\t";
    m_dlTxOutFile << (uint32_t)params.m_rv << "\t";
    m_dlTxOutFile << (uint32_t)params.m_ndi << "\t";
    m_dlTxOutFile << (uint32_t)params.m_ccId << std::endl;
}

void
NrPhyTxStatsCalculator::UlPhyTransmission(nr::PhyTransmissionStatParameters params)
{
    NS_LOG_FUNCTION(this << params.m_cellId << params.m_imsi << params.m_timestamp << params.m_rnti
                         << params.m_layer << params.m_mcs << params.m_size << params.m_rv
                         << params.m_ndi);
    NS_LOG_INFO("Write UL Tx Phy Stats in " << GetUlTxOutputFilename());

    if (m_ulTxFirstWrite)
    {
        m_ulTxOutFile.open(GetUlTxOutputFilename());
        if (!m_ulTxOutFile.is_open())
        {
            NS_LOG_ERROR("Can't open file " << GetUlTxOutputFilename());
            return;
        }
        m_ulTxFirstWrite = false;
        // m_ulTxOutFile << "% time\tcellId\tIMSI\tRNTI\ttxMode\tlayer\tmcs\tsize\trv\tndi";
        m_ulTxOutFile << "% time\tcellId\tIMSI\tRNTI\tlayer\tmcs\tsize\trv\tndi\tccId";
        m_ulTxOutFile << "\n";
    }

    m_ulTxOutFile << params.m_timestamp << "\t";
    m_ulTxOutFile << (uint32_t)params.m_cellId << "\t";
    m_ulTxOutFile << params.m_imsi << "\t";
    m_ulTxOutFile << params.m_rnti << "\t";
    // m_ulTxOutFile << (uint32_t) params.m_txMode << "\t";
    m_ulTxOutFile << (uint32_t)params.m_layer << "\t";
    m_ulTxOutFile << (uint32_t)params.m_mcs << "\t";
    m_ulTxOutFile << params.m_size << "\t";
    m_ulTxOutFile << (uint32_t)params.m_rv << "\t";
    m_ulTxOutFile << (uint32_t)params.m_ndi << "\t";
    m_ulTxOutFile << (uint32_t)params.m_ccId << std::endl;
}

void
NrPhyTxStatsCalculator::DlPhyTransmissionCallback(Ptr<NrPhyTxStatsCalculator> phyTxStats,
                                                  std::string path,
                                                  nr::PhyTransmissionStatParameters params)
{
    NS_LOG_FUNCTION(phyTxStats << path);
    uint64_t imsi = 0;
    std::ostringstream pathAndRnti;
    std::string pathEnb = path.substr(0, path.find("/ComponentCarrierMap"));
    pathAndRnti << pathEnb << "/NrGnbRrc/UeMap/" << params.m_rnti;
    if (phyTxStats->ExistsImsiPath(pathAndRnti.str()))
    {
        imsi = phyTxStats->GetImsiPath(pathAndRnti.str());
    }
    else
    {
        imsi = FindImsiFromGnbRlcPath(pathAndRnti.str());
        phyTxStats->SetImsiPath(pathAndRnti.str(), imsi);
    }

    params.m_imsi = imsi;
    phyTxStats->DlPhyTransmission(params);
}

void
NrPhyTxStatsCalculator::UlPhyTransmissionCallback(Ptr<NrPhyTxStatsCalculator> phyTxStats,
                                                  std::string path,
                                                  nr::PhyTransmissionStatParameters params)
{
    NS_LOG_FUNCTION(phyTxStats << path);
    uint64_t imsi = 0;
    std::ostringstream pathAndRnti;
    pathAndRnti << path << "/" << params.m_rnti;
    std::string pathUePhy = path.substr(0, path.find("/ComponentCarrierMapUe"));
    if (phyTxStats->ExistsImsiPath(pathAndRnti.str()))
    {
        imsi = phyTxStats->GetImsiPath(pathAndRnti.str());
    }
    else
    {
        imsi = FindImsiFromNrUeNetDevice(pathUePhy);
        phyTxStats->SetImsiPath(pathAndRnti.str(), imsi);
    }

    params.m_imsi = imsi;
    phyTxStats->UlPhyTransmission(params);
}

} // namespace ns3
