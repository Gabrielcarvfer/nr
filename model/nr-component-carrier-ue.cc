// Copyright (c) 2015 Danilo Abrignani
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Danilo Abrignani <danilo.abrignani@unibo.it>

#include "component-carrier-ue.h"
#include "nr-ue-mac.h"
#include "nr-ue-phy.h"

#include <ns3/abort.h>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ComponentCarrierUe");

NS_OBJECT_ENSURE_REGISTERED(ComponentCarrierUe);

TypeId
ComponentCarrierUe::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ComponentCarrierUe")
                            .SetParent<NrComponentCarrier>()
                            .AddConstructor<ComponentCarrierUe>()
                            .AddAttribute("NrUePhy",
                                          "The PHY associated to this EnbNetDevice",
                                          PointerValue(),
                                          MakePointerAccessor(&ComponentCarrierUe::m_phy),
                                          MakePointerChecker<NrUePhy>())
                            .AddAttribute("NrUeMac",
                                          "The MAC associated to this UeNetDevice",
                                          PointerValue(),
                                          MakePointerAccessor(&ComponentCarrierUe::m_mac),
                                          MakePointerChecker<NrUeMac>());
    return tid;
}

ComponentCarrierUe::ComponentCarrierUe()
{
    NS_LOG_FUNCTION(this);
}

ComponentCarrierUe::~ComponentCarrierUe()
{
    NS_LOG_FUNCTION(this);
}

void
ComponentCarrierUe::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_phy->Dispose();
    m_phy = nullptr;
    m_mac->Dispose();
    m_mac = nullptr;
    Object::DoDispose();
}

void
ComponentCarrierUe::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    m_phy->Initialize();
    m_mac->Initialize();
}

void
ComponentCarrierUe::SetPhy(Ptr<NrUePhy> s)
{
    NS_LOG_FUNCTION(this);
    m_phy = s;
}

Ptr<NrUePhy>
ComponentCarrierUe::GetPhy() const
{
    NS_LOG_FUNCTION(this);
    return m_phy;
}

void
ComponentCarrierUe::SetMac(Ptr<NrUeMac> s)
{
    NS_LOG_FUNCTION(this);
    m_mac = s;
}

Ptr<NrUeMac>
ComponentCarrierUe::GetMac() const
{
    NS_LOG_FUNCTION(this);
    return m_mac;
}

} // namespace ns3
