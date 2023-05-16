// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-fh-control.h"

#include <ns3/core-module.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrFhControl");
NS_OBJECT_ENSURE_REGISTERED(NrFhControl);

static constexpr uint32_t
Cantor(uint16_t x1, uint16_t x2)
{
    return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
}

TypeId
NrFhControl::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrFhControl")
            .SetParent<Object>()
            .AddConstructor<NrFhControl>()
            .SetGroupName("Nr")
            .AddAttribute(
                "FhControlMethod",
                "The FH Control method defines the model that the fhControl will use"
                "to limit the capacity. There are four FH Control methods: "
                "a) Dropping. When CTRL channels are sent, PHY asks the FhControl whether"
                "the allocation fits. If not, it drops the DCI + data."
                "b) Postponing. When tdma/ofdma have allocated the RBs/symbols to all the"
                "UEs, it iterates through all the UEs and asks the FhControl whether the"
                "allocation fits. If not, it sets the assigned RBGs to 0 and therefore the"
                "sending of the data is postponed (DCI is not created – data stays in RLC queue)"
                "c) Optimize MCS. When tdma/ofdma have allocated the RBs/symbols to all the UEs,"
                "it iterates through all the UEs (with data in their queues and resources"
                "allocated during the scheduling process) and asks CI for the max MCS. It"
                "assigns the min among the allocated one and the max MCS."
                "d) Optimize RBs. When tdma/ofdma are allocating the RBs/symbols to a UE,"
                "it calls the CI to provide the max RBs that can be assigned.",
                EnumValue(NrFhControl::Dropping),
                MakeEnumAccessor<FhControlMethod>(&NrFhControl::SetFhControlMethod,
                                             &NrFhControl::GetFhControlMethod),
                MakeEnumChecker(NrFhControl::Dropping,
                                "Dropping",
                                NrFhControl::Postponing,
                                "Postponing",
                                NrFhControl::OptimizeMcs,
                                "OptimizeMcs",
                                NrFhControl::OptimizeRBs,
                                "OptimizeRBs"))
            .AddAttribute("FhCapacity",
                          "The available fronthaul capacity (in MHz)",
                          UintegerValue(1000),
                          MakeUintegerAccessor(&NrFhControl::SetFhCapacity),
                          MakeUintegerChecker<uint16_t>(0, 50000))
            .AddAttribute("OverheadDyn",
                          "The overhead for dynamic adaptation (in bits)",
                          UintegerValue(32),
                          MakeUintegerAccessor(&NrFhControl::SetOverheadDyn),
                          MakeUintegerChecker<uint8_t>(0, 100))

        ;
    return tid;
}

NrFhControl::NrFhControl()
    : m_physicalCellId(0),
      m_fhPhySapUser(0),
      m_fhSchedSapUser(0)
{
    NS_LOG_FUNCTION(this);
    m_fhPhySapProvider = new MemberNrFhPhySapProvider<NrFhControl>(this);
    m_fhSchedSapProvider = new MemberNrFhSchedSapProvider<NrFhControl>(this);
}

NrFhControl::~NrFhControl()
{
}

void
NrFhControl::SetNrFhPhySapUser(NrFhPhySapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    m_fhPhySapUser = s;
}

NrFhPhySapProvider*
NrFhControl::GetNrFhPhySapProvider()
{
    NS_LOG_FUNCTION(this);

    return m_fhPhySapProvider;
}

void
NrFhControl::SetNrFhSchedSapUser(NrFhSchedSapUser* s)
{
    NS_LOG_FUNCTION(this << s);
    m_fhSchedSapUser = s;
}

NrFhSchedSapProvider*
NrFhControl::GetNrFhSchedSapProvider()
{
    NS_LOG_FUNCTION(this);
    return m_fhSchedSapProvider;
}

void
NrFhControl::SetFhControlMethod(FhControlMethod model)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Set the Fh Control Limit Model to: " << model);
    m_fhControlMethod = model;
}

NrFhControl::FhControlMethod
NrFhControl::GetFhControlMethod() const
{
    NS_LOG_FUNCTION(this);
    return m_fhControlMethod;
}

uint8_t
NrFhControl::DoGetFhControlMethod() const
{
    NS_LOG_FUNCTION(this);
    return m_fhControlMethod;
}

void
NrFhControl::SetFhCapacity(uint16_t capacity)
{
    NS_LOG_FUNCTION(this);
    m_fhCapacity = capacity;
}

void
NrFhControl::SetOverheadDyn(uint8_t overhead)
{
    NS_LOG_FUNCTION(this);
    m_overheadDyn = overhead;
}

void
NrFhControl::SetPhysicalCellId(uint16_t physicalCellId)
{
    NS_LOG_FUNCTION(this);
    m_physicalCellId = physicalCellId;
    NS_LOG_DEBUG("NrFhControl initialized for cell Id: " << m_physicalCellId);
}

uint16_t
NrFhControl::DoGetPhysicalCellId() const
{
    return m_physicalCellId;
}

void
NrFhControl::DoSetActiveUe(uint16_t bwpId, uint16_t rnti, uint32_t bytes)
{
    uint32_t c1 = Cantor(bwpId, rnti);
    if (m_activeUes.find(bwpId) == m_activeUes.end()) // bwpId not in the map
    {
        m_activeUes.insert(std::make_pair(bwpId, rnti));
    }

    if (m_rntiQueueSize.find(c1) == m_rntiQueueSize.end()) // UE not in the map
    {
        NS_LOG_DEBUG("Cell: " << m_physicalCellId << " Creating pair " << c1 << " for bwpId: "
                              << bwpId << " and rnti: " << rnti << " with bytes: " << bytes);

        m_rntiQueueSize.insert(std::make_pair(c1, bytes));
    }
    else
    {
        NS_LOG_DEBUG("Cell: " << m_physicalCellId << " Updating pair " << c1 << " for bwpId: "
                              << bwpId << " and rnti: " << rnti << " with bytes: " << bytes);
        m_rntiQueueSize.at(c1) = bytes;
    }
}

void
NrFhControl::DoUpdateActiveUesMap(uint16_t bwpId, const std::deque<VarTtiAllocInfo>& allocation)
{
    NS_LOG_DEBUG("Cell: " << m_physicalCellId << " We got called for reset for " << bwpId);

    for (const auto& alloc : allocation)
    {
        if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL ||
            alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
            continue;
        }

        uint16_t rnti = alloc.m_dci->m_rnti;
        uint32_t c1 = Cantor(bwpId, rnti);
        uint32_t numRbs =
            static_cast<uint32_t>(
                std::count(alloc.m_dci->m_rbgBitmask.begin(), alloc.m_dci->m_rbgBitmask.end(), 1)) *
            static_cast<uint32_t>(m_fhSchedSapUser->GetNumRbPerRbgFromSched());

        NS_LOG_DEBUG("Get num of RBs per RBG from sched: "
                     << m_fhSchedSapUser->GetNumRbPerRbgFromSched() << " numRbs = " << numRbs);

        // TODO: Add traces!

        // update stored maps
        if (m_rntiQueueSize.size() == 0)
        {
            NS_LOG_DEBUG("empty MAP");
            NS_ABORT_MSG_IF(m_activeUes.size() > 0, "No UE in map, but something in activeUes map");
            continue;
        }

        NS_LOG_DEBUG("Looking for key " << c1 << " map size " << m_rntiQueueSize.size());

        if (m_rntiQueueSize.at(c1) > (alloc.m_dci->m_tbSize - 3)) // 3 bytes MAC subPDU header
        {
            m_rntiQueueSize.at(c1) = m_rntiQueueSize.at(c1) - (alloc.m_dci->m_tbSize - 3);
            NS_LOG_DEBUG("Updating queue size for cell: " << m_physicalCellId << " bwpId: " << bwpId
                                                          << " RNTI: " << rnti << " to "
                                                          << m_rntiQueueSize.at(c1));
        }
        else
        {
            NS_LOG_DEBUG("Removing UE because we served it. RLC queue size: "
                         << m_rntiQueueSize.at(c1)
                         << " and allocation of: " << (alloc.m_dci->m_tbSize - 3));
            m_rntiQueueSize.erase(c1);
            m_activeUes.erase(bwpId);
        }
    }
}

void
NrFhControl::DoGetDoesAllocationFit()
{
    // NS_LOG_UNCOND("NrFhControl::DoGetDoesAllocationFit for cell: " << m_physicalCellId);
}

} // namespace ns3
