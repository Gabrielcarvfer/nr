// Copyright (c) 2015 Danilo Abrignani
// Copyright (c) 2016 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Authors: Danilo Abrignani <danilo.abrignani@unibo.it>
//          Biljana Bojovic <biljana.bojovic@cttc.es>
//
///

#include "nr-no-op-component-carrier-manager.h"

#include "nr-common.h"

#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrNoOpComponentCarrierManager");
NS_OBJECT_ENSURE_REGISTERED(NrNoOpComponentCarrierManager);

NrNoOpComponentCarrierManager::NrNoOpComponentCarrierManager()
{
    NS_LOG_FUNCTION(this);
    m_ccmRrcSapProvider = new MemberNrCcmRrcSapProvider<NrNoOpComponentCarrierManager>(this);
    m_ccmMacSapUser = new MemberNrCcmMacSapUser<NrNoOpComponentCarrierManager>(this);
    m_macSapProvider = new GnbMacMemberNrMacSapProvider<NrNoOpComponentCarrierManager>(this);
    m_ccmRrcSapUser = nullptr;
}

NrNoOpComponentCarrierManager::~NrNoOpComponentCarrierManager()
{
    NS_LOG_FUNCTION(this);
}

void
NrNoOpComponentCarrierManager::DoDispose()
{
    NS_LOG_FUNCTION(this);
    delete m_ccmRrcSapProvider;
    delete m_ccmMacSapUser;
    delete m_macSapProvider;
}

TypeId
NrNoOpComponentCarrierManager::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrNoOpComponentCarrierManager")
                            .SetParent<NrGnbComponentCarrierManager>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrNoOpComponentCarrierManager>();
    return tid;
}

void
NrNoOpComponentCarrierManager::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    NrGnbComponentCarrierManager::DoInitialize();
}

//////////////////////////////////////////////
// MAC SAP
/////////////////////////////////////////////

void
NrNoOpComponentCarrierManager::DoTransmitPdu(NrMacSapProvider::TransmitPduParameters params)
{
    NS_LOG_FUNCTION(this);
    auto it = m_macSapProvidersMap.find(params.componentCarrierId);
    NS_ASSERT_MSG(it != m_macSapProvidersMap.end(),
                  "could not find Sap for NrComponentCarrier " << params.componentCarrierId);
    // with this algorithm all traffic is on Primary Carrier
    it->second->TransmitPdu(params);
}

void
NrNoOpComponentCarrierManager::DoTransmitBufferStatusReport(
    NrMacSapProvider::BufferStatusReportParameters params)
{
    NS_LOG_FUNCTION(this);
    auto ueManager = m_ccmRrcSapUser->GetUeManager(params.rnti);
    auto it = m_macSapProvidersMap.find(ueManager->GetComponentCarrierId());
    NS_ASSERT_MSG(it != m_macSapProvidersMap.end(), "could not find Sap for NrComponentCarrier ");
    it->second->BufferStatusReport(params);
}

void
NrNoOpComponentCarrierManager::DoNotifyTxOpportunity(
    NrMacSapUser::TxOpportunityParameters txOpParams)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG(this << " rnti= " << txOpParams.rnti << " lcid= " << +txOpParams.lcid << " layer= "
                      << +txOpParams.layer << " ccId=" << +txOpParams.componentCarrierId);
    m_ueInfo.at(txOpParams.rnti).m_ueAttached.at(txOpParams.lcid)->NotifyTxOpportunity(txOpParams);
}

void
NrNoOpComponentCarrierManager::DoReceivePdu(NrMacSapUser::ReceivePduParameters rxPduParams)
{
    NS_LOG_FUNCTION(this);
    auto lcidIt = m_ueInfo.at(rxPduParams.rnti).m_ueAttached.find(rxPduParams.lcid);
    if (lcidIt != m_ueInfo.at(rxPduParams.rnti).m_ueAttached.end())
    {
        lcidIt->second->ReceivePdu(rxPduParams);
    }
}

void
NrNoOpComponentCarrierManager::DoNotifyHarqDeliveryFailure()
{
    NS_LOG_FUNCTION(this);
}

void
NrNoOpComponentCarrierManager::DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults)
{
    NS_LOG_FUNCTION(this << rnti << (uint16_t)measResults.measId);
}

void
NrNoOpComponentCarrierManager::DoAddUe(uint16_t rnti, uint8_t state)
{
    NS_LOG_FUNCTION(this << rnti << (uint16_t)state);
    auto ueInfoIt = m_ueInfo.find(rnti);
    if (ueInfoIt == m_ueInfo.end())
    {
        NS_LOG_DEBUG(this << " UE " << rnti << " was not found, now it is added in the map");
        NrUeInfo info;
        info.m_ueState = state;

        // the Primary carrier (PC) is enabled by default
        // on the PC the SRB0 and SRB1 are enabled when the Ue is connected
        // these are hard-coded and the configuration not pass through the
        // Component Carrier Manager which is responsible of configure
        // only DataRadioBearer on the different Component Carrier
        info.m_enabledComponentCarrier = 1;
        m_ueInfo.emplace(rnti, info);
    }
    else
    {
        NS_LOG_DEBUG(this << " UE " << rnti << "found, updating the state from "
                          << +ueInfoIt->second.m_ueState << " to " << +state);
        ueInfoIt->second.m_ueState = state;
    }
}

void
NrNoOpComponentCarrierManager::DoAddLc(NrGnbCmacSapProvider::LcInfo lcInfo, NrMacSapUser* msu)
{
    NS_LOG_FUNCTION(this);
    m_ueInfo.at(lcInfo.rnti).m_rlcLcInstantiated.emplace(lcInfo.lcId, lcInfo);
}

void
NrNoOpComponentCarrierManager::DoRemoveUe(uint16_t rnti)
{
    NS_LOG_FUNCTION(this);
    auto rntiIt = m_ueInfo.find(rnti);
    NS_ASSERT_MSG(rntiIt != m_ueInfo.end(), "request to remove UE info with unknown RNTI " << rnti);
    m_ueInfo.erase(rntiIt);
}

std::vector<NrCcmRrcSapProvider::LcsConfig>
NrNoOpComponentCarrierManager::DoSetupDataRadioBearer(NrEpsBearer bearer,
                                                      uint8_t bearerId,
                                                      uint16_t rnti,
                                                      uint8_t lcid,
                                                      uint8_t lcGroup,
                                                      NrMacSapUser* msu)
{
    NS_LOG_FUNCTION(this << rnti);
    auto rntiIt = m_ueInfo.find(rnti);
    NS_ASSERT_MSG(rntiIt != m_ueInfo.end(), "SetupDataRadioBearer on unknown RNTI " << rnti);

    // enable by default all carriers
    rntiIt->second.m_enabledComponentCarrier = m_noOfComponentCarriers;

    std::vector<NrCcmRrcSapProvider::LcsConfig> res;
    NrCcmRrcSapProvider::LcsConfig entry;
    NrGnbCmacSapProvider::LcInfo lcinfo;
    // NS_LOG_DEBUG (this << " componentCarrierEnabled " << (uint16_t) eccIt->second);
    for (uint16_t ncc = 0; ncc < m_noOfComponentCarriers; ncc++)
    {
        // NS_LOG_DEBUG (this << " res size " << (uint16_t) res.size ());
        NrGnbCmacSapProvider::LcInfo lci;
        lci.rnti = rnti;
        lci.lcId = lcid;
        lci.lcGroup = lcGroup;
        lci.qci = bearer.qci;
        if (ncc == 0)
        {
            lci.resourceType = bearer.GetResourceType();
            lci.mbrUl = bearer.gbrQosInfo.mbrUl;
            lci.mbrDl = bearer.gbrQosInfo.mbrDl;
            lci.gbrUl = bearer.gbrQosInfo.gbrUl;
            lci.gbrDl = bearer.gbrQosInfo.gbrDl;
        }
        else
        {
            lci.resourceType = 0;
            lci.mbrUl = 0;
            lci.mbrDl = 0;
            lci.gbrUl = 0;
            lci.gbrDl = 0;
        } // data flows only on PC
        NS_LOG_DEBUG(this << " RNTI " << lci.rnti << "Lcid " << (uint16_t)lci.lcId << " lcGroup "
                          << (uint16_t)lci.lcGroup);
        entry.componentCarrierId = ncc;
        entry.lc = lci;
        entry.msu = m_ccmMacSapUser;
        res.push_back(entry);
    } // end for

    auto lcidIt = rntiIt->second.m_rlcLcInstantiated.find(lcid);
    if (lcidIt == rntiIt->second.m_rlcLcInstantiated.end())
    {
        lcinfo.rnti = rnti;
        lcinfo.lcId = lcid;
        lcinfo.lcGroup = lcGroup;
        lcinfo.qci = bearer.qci;
        lcinfo.resourceType = bearer.GetResourceType();
        lcinfo.mbrUl = bearer.gbrQosInfo.mbrUl;
        lcinfo.mbrDl = bearer.gbrQosInfo.mbrDl;
        lcinfo.gbrUl = bearer.gbrQosInfo.gbrUl;
        lcinfo.gbrDl = bearer.gbrQosInfo.gbrDl;
        rntiIt->second.m_rlcLcInstantiated.emplace(lcinfo.lcId, lcinfo);
        rntiIt->second.m_ueAttached.emplace(lcinfo.lcId, msu);
    }
    else
    {
        NS_LOG_ERROR("LC already exists");
    }
    return res;
}

std::vector<uint8_t>
NrNoOpComponentCarrierManager::DoReleaseDataRadioBearer(uint16_t rnti, uint8_t lcid)
{
    NS_LOG_FUNCTION(this << rnti << +lcid);

    // Here we receive directly the RNTI and the LCID, instead of only DRB ID
    // DRB ID are mapped as DRBID = LCID + 2
    auto rntiIt = m_ueInfo.find(rnti);
    NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                  "request to Release Data Radio Bearer on UE with unknown RNTI " << rnti);

    NS_LOG_DEBUG(this << " remove LCID " << +lcid << " for RNTI " << rnti);
    std::vector<uint8_t> res;
    for (uint16_t i = 0; i < rntiIt->second.m_enabledComponentCarrier; i++)
    {
        res.insert(res.end(), i);
    }

    auto lcIt = rntiIt->second.m_ueAttached.find(lcid);
    NS_ASSERT_MSG(lcIt != rntiIt->second.m_ueAttached.end(), "Logical Channel not found");
    rntiIt->second.m_ueAttached.erase(lcIt);

    auto rlcInstancesIt = rntiIt->second.m_rlcLcInstantiated.find(lcid);
    NS_ASSERT_MSG(rlcInstancesIt != rntiIt->second.m_rlcLcInstantiated.end(),
                  "Logical Channel not found");
    rntiIt->second.m_rlcLcInstantiated.erase(rlcInstancesIt);

    return res;
}

NrMacSapUser*
NrNoOpComponentCarrierManager::DoConfigureSignalBearer(NrGnbCmacSapProvider::LcInfo lcinfo,
                                                       NrMacSapUser* msu)
{
    NS_LOG_FUNCTION(this);

    auto rntiIt = m_ueInfo.find(lcinfo.rnti);
    NS_ASSERT_MSG(rntiIt != m_ueInfo.end(),
                  "request to add a signal bearer to unknown RNTI " << lcinfo.rnti);

    auto lcidIt = rntiIt->second.m_ueAttached.find(lcinfo.lcId);
    if (lcidIt == rntiIt->second.m_ueAttached.end())
    {
        rntiIt->second.m_ueAttached.emplace(lcinfo.lcId, msu);
    }
    else
    {
        NS_LOG_ERROR("LC already exists");
    }

    return m_ccmMacSapUser;
}

void
NrNoOpComponentCarrierManager::DoNotifyPrbOccupancy(double prbOccupancy, uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Update PRB occupancy:" << prbOccupancy
                                         << " at carrier:" << (uint32_t)componentCarrierId);
    m_ccPrbOccupancy.insert(std::pair<uint8_t, double>(componentCarrierId, prbOccupancy));
}

void
NrNoOpComponentCarrierManager::DoUlReceiveMacCe(nr::MacCeListElement_s bsr,
                                                uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(bsr.m_macCeType == nr::MacCeListElement_s::BSR,
                  "Received a Control Message not allowed " << bsr.m_macCeType);
    if (bsr.m_macCeType == nr::MacCeListElement_s::BSR)
    {
        nr::MacCeListElement_s newBsr;
        newBsr.m_rnti = bsr.m_rnti;
        newBsr.m_macCeType = bsr.m_macCeType;
        newBsr.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
        newBsr.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
        newBsr.m_macCeValue.m_bufferStatus = std::vector<uint8_t>(4, 0);
        for (uint16_t i = 0; i < 4; i++)
        {
            uint8_t bsrId = bsr.m_macCeValue.m_bufferStatus.at(i);
            uint32_t buffer = nr::BufferSizeLevelBsr::BsrId2BufferSize(bsrId);
            // here the buffer should be divide among the different sap
            // since the buffer status report are compressed information
            // it is needed to use BsrId2BufferSize to uncompress
            // after the split over all component carriers is is needed to
            // compress again the information to fit MacCeListEkement_s structure
            // verify how many Component Carrier are enabled per UE
            // in this simple code the BufferStatus will be notify only
            // to the primary carrier component
            newBsr.m_macCeValue.m_bufferStatus.at(i) =
                nr::BufferSizeLevelBsr::BufferSize2BsrId(buffer);
        }
        auto sapIt = m_ccmMacSapProviderMap.find(componentCarrierId);
        if (sapIt == m_ccmMacSapProviderMap.end())
        {
            NS_FATAL_ERROR("Sap not found in the CcmMacSapProviderMap");
        }
        else
        {
            // in the current implementation bsr in uplink is forwarded only to the primary carrier.
            // above code demonstrates how to resize buffer status if more carriers are being used
            // in future
            sapIt->second->ReportMacCeToScheduler(newBsr);
        }
    }
    else
    {
        NS_FATAL_ERROR("Expected BSR type of message.");
    }
}

void
NrNoOpComponentCarrierManager::DoUlReceiveSr(uint16_t rnti, uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this);

    auto sapIt = m_ccmMacSapProviderMap.find(componentCarrierId);
    NS_ABORT_MSG_IF(sapIt == m_ccmMacSapProviderMap.end(),
                    "Sap not found in the CcmMacSapProviderMap");

    sapIt->second->ReportSrToScheduler(rnti);
}

//////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(NrRrComponentCarrierManager);

NrRrComponentCarrierManager::NrRrComponentCarrierManager()
{
    NS_LOG_FUNCTION(this);
}

NrRrComponentCarrierManager::~NrRrComponentCarrierManager()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrRrComponentCarrierManager::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrRrComponentCarrierManager")
                            .SetParent<NrNoOpComponentCarrierManager>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrRrComponentCarrierManager>();
    return tid;
}

void
NrRrComponentCarrierManager::DoTransmitBufferStatusReport(
    NrMacSapProvider::BufferStatusReportParameters params)
{
    NS_LOG_FUNCTION(this);

    uint32_t numberOfCarriersForUe = m_ueInfo.at(params.rnti).m_enabledComponentCarrier;
    if (params.lcid == 0 || params.lcid == 1 || numberOfCarriersForUe == 1)
    {
        NS_LOG_INFO("Buffer status forwarded to the primary carrier.");
        auto ueManager = m_ccmRrcSapUser->GetUeManager(params.rnti);
        m_macSapProvidersMap.at(ueManager->GetComponentCarrierId())->BufferStatusReport(params);
    }
    else
    {
        params.retxQueueSize /= numberOfCarriersForUe;
        params.txQueueSize /= numberOfCarriersForUe;
        for (uint32_t i = 0; i < numberOfCarriersForUe; i++)
        {
            NS_ASSERT_MSG(m_macSapProvidersMap.find(i) != m_macSapProvidersMap.end(),
                          "Mac sap provider does not exist.");
            m_macSapProvidersMap.at(i)->BufferStatusReport(params);
        }
    }
}

void
NrRrComponentCarrierManager::DoUlReceiveMacCe(nr::MacCeListElement_s bsr,
                                              uint8_t componentCarrierId)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(componentCarrierId == 0,
                  "Received BSR from a NrComponentCarrier not allowed, ComponentCarrierId = "
                      << componentCarrierId);
    NS_ASSERT_MSG(bsr.m_macCeType == nr::MacCeListElement_s::BSR,
                  "Received a Control Message not allowed " << bsr.m_macCeType);

    // split traffic in uplink equally among carriers
    uint32_t numberOfCarriersForUe = m_ueInfo.at(bsr.m_rnti).m_enabledComponentCarrier;

    if (bsr.m_macCeType == nr::MacCeListElement_s::BSR)
    {
        nr::MacCeListElement_s newBsr;
        newBsr.m_rnti = bsr.m_rnti;
        // mac control element type, values can be BSR, PHR, CRNTI
        newBsr.m_macCeType = bsr.m_macCeType;
        // the power headroom, 64 means no valid phr is available
        newBsr.m_macCeValue.m_phr = bsr.m_macCeValue.m_phr;
        // indicates that the CRNTI MAC CE was received. The value is not used.
        newBsr.m_macCeValue.m_crnti = bsr.m_macCeValue.m_crnti;
        // and value 64 means that the buffer status should not be updated
        newBsr.m_macCeValue.m_bufferStatus = std::vector<uint8_t>(4, 0);
        // always all 4 LCGs are present see 6.1.3.1 of 3GPP TS 36.321.
        for (uint16_t i = 0; i < 4; i++)
        {
            uint8_t bsrStatusId = bsr.m_macCeValue.m_bufferStatus.at(i);
            uint32_t bufferSize = nr::BufferSizeLevelBsr::BsrId2BufferSize(bsrStatusId);
            // here the buffer should be divide among the different sap
            // since the buffer status report are compressed information
            // it is needed to use BsrId2BufferSize to uncompress
            // after the split over all component carriers is is needed to
            // compress again the information to fit nr::MacCeListElement_s structure
            // verify how many Component Carrier are enabled per UE
            newBsr.m_macCeValue.m_bufferStatus.at(i) =
                nr::BufferSizeLevelBsr::BufferSize2BsrId(bufferSize / numberOfCarriersForUe);
        }
        // notify MAC of each component carrier that is enabled for this UE
        for (uint32_t i = 0; i < numberOfCarriersForUe; i++)
        {
            NS_ASSERT_MSG(m_ccmMacSapProviderMap.find(i) != m_ccmMacSapProviderMap.end(),
                          "Mac sap provider does not exist.");
            m_ccmMacSapProviderMap.find(i)->second->ReportMacCeToScheduler(newBsr);
        }
    }
    else
    {
        auto ueManager = m_ccmRrcSapUser->GetUeManager(bsr.m_rnti);
        m_ccmMacSapProviderMap.at(ueManager->GetComponentCarrierId())->ReportMacCeToScheduler(bsr);
    }
}

void
NrRrComponentCarrierManager::DoUlReceiveSr(uint16_t rnti, uint8_t /* componentCarrierId */)
{
    NS_LOG_FUNCTION(this);
    // split traffic in uplink equally among carriers
    uint32_t numberOfCarriersForUe = m_ueInfo.at(rnti).m_enabledComponentCarrier;

    m_ccmMacSapProviderMap.find(m_lastCcIdForSr)->second->ReportSrToScheduler(rnti);

    m_lastCcIdForSr++;
    if (m_lastCcIdForSr > numberOfCarriersForUe - 1)
    {
        m_lastCcIdForSr = 0;
    }
}

} // end of namespace ns3
