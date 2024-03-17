// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#include "nr-ff-mac-scheduler.h"

#include <ns3/enum.h>
#include <ns3/log.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrFfMacScheduler");

NS_OBJECT_ENSURE_REGISTERED(NrFfMacScheduler);

NrFfMacScheduler::NrFfMacScheduler()
    : m_ulCqiFilter(SRS_UL_CQI)
{
    NS_LOG_FUNCTION(this);
}

NrFfMacScheduler::~NrFfMacScheduler()
{
    NS_LOG_FUNCTION(this);
}

void
NrFfMacScheduler::DoDispose()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrFfMacScheduler::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrFfMacScheduler")
            .SetParent<Object>()
            .SetGroupName("Nr")
            .AddAttribute("UlCqiFilter",
                          "The filter to apply on UL CQIs received",
                          EnumValue(NrFfMacScheduler::SRS_UL_CQI),
                          MakeEnumAccessor<UlCqiFilter_t>(&NrFfMacScheduler::m_ulCqiFilter),
                          MakeEnumChecker(NrFfMacScheduler::SRS_UL_CQI,
                                          "SRS_UL_CQI",
                                          NrFfMacScheduler::PUSCH_UL_CQI,
                                          "PUSCH_UL_CQI"));
    return tid;
}

} // namespace ns3
