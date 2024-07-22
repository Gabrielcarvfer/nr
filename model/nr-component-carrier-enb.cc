// Copyright (c) 2015 Danilo Abrignani
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Danilo Abrignani <danilo.abrignani@unibo.it>

#include "nr-component-carrier-enb.h"

#include "nr-ff-mac-scheduler.h"
#include "nr-ffr-algorithm.h"
#include "nr-gnb-mac.h"
#include "nr-gnb-phy.h"

#include <ns3/abort.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrComponentCarrierEnb");
NS_OBJECT_ENSURE_REGISTERED(NrComponentCarrierEnb);

TypeId
NrComponentCarrierEnb::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrComponentCarrierEnb")
            .SetParent<NrComponentCarrier>()
            .AddConstructor<NrComponentCarrierEnb>()
            .AddAttribute("NrGnbPhy",
                          "The PHY associated to this EnbNetDevice",
                          PointerValue(),
                          MakePointerAccessor(&NrComponentCarrierEnb::m_phy),
                          MakePointerChecker<NrGnbPhy>())
            .AddAttribute("NrGnbMac",
                          "The MAC associated to this EnbNetDevice",
                          PointerValue(),
                          MakePointerAccessor(&NrComponentCarrierEnb::m_mac),
                          MakePointerChecker<NrGnbMac>())
            .AddAttribute("NrFfMacScheduler",
                          "The scheduler associated to this EnbNetDevice",
                          PointerValue(),
                          MakePointerAccessor(&NrComponentCarrierEnb::m_scheduler),
                          MakePointerChecker<NrFfMacScheduler>())
            .AddAttribute("NrFfrAlgorithm",
                          "The FFR algorithm associated to this EnbNetDevice",
                          PointerValue(),
                          MakePointerAccessor(&NrComponentCarrierEnb::m_ffrAlgorithm),
                          MakePointerChecker<NrFfrAlgorithm>());
    return tid;
}

NrComponentCarrierEnb::NrComponentCarrierEnb()
{
    NS_LOG_FUNCTION(this);
}

NrComponentCarrierEnb::~NrComponentCarrierEnb()
{
    NS_LOG_FUNCTION(this);
}

void
NrComponentCarrierEnb::DoDispose()
{
    NS_LOG_FUNCTION(this);
    if (m_phy)
    {
        m_phy->Dispose();
        m_phy = nullptr;
    }
    if (m_mac)
    {
        m_mac->Dispose();
        m_mac = nullptr;
    }
    if (m_scheduler)
    {
        m_scheduler->Dispose();
        m_scheduler = nullptr;
    }
    if (m_ffrAlgorithm)
    {
        m_ffrAlgorithm->Dispose();
        m_ffrAlgorithm = nullptr;
    }

    Object::DoDispose();
}

void
NrComponentCarrierEnb::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    m_phy->Initialize();
    m_mac->Initialize();
    m_ffrAlgorithm->Initialize();
    m_scheduler->Initialize();
}

Ptr<NrGnbPhy>
NrComponentCarrierEnb::GetPhy()
{
    NS_LOG_FUNCTION(this);
    return m_phy;
}

void
NrComponentCarrierEnb::SetPhy(Ptr<NrGnbPhy> s)
{
    NS_LOG_FUNCTION(this);
    m_phy = s;
}

Ptr<NrGnbMac>
NrComponentCarrierEnb::GetMac()
{
    NS_LOG_FUNCTION(this);
    return m_mac;
}

void
NrComponentCarrierEnb::SetMac(Ptr<NrGnbMac> s)
{
    NS_LOG_FUNCTION(this);
    m_mac = s;
}

Ptr<NrFfrAlgorithm>
NrComponentCarrierEnb::GetFfrAlgorithm()
{
    NS_LOG_FUNCTION(this);
    return m_ffrAlgorithm;
}

void
NrComponentCarrierEnb::SetFfrAlgorithm(Ptr<NrFfrAlgorithm> s)
{
    NS_LOG_FUNCTION(this);
    m_ffrAlgorithm = s;
}

Ptr<NrFfMacScheduler>
NrComponentCarrierEnb::GetFfMacScheduler()
{
    NS_LOG_FUNCTION(this);
    return m_scheduler;
}

void
NrComponentCarrierEnb::SetFfMacScheduler(Ptr<NrFfMacScheduler> s)
{
    NS_LOG_FUNCTION(this);
    m_scheduler = s;
}

} // namespace ns3
