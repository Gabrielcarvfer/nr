// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#include "nr-epc-x2-sap.h"

namespace ns3
{

NrEpcX2Sap::~NrEpcX2Sap()
{
}

NrEpcX2Sap::ErabToBeSetupItem::ErabToBeSetupItem()
    : erabLevelQosParameters(NrEpsBearer(NrEpsBearer::GBR_CONV_VOICE))
{
}

NrEpcX2SapProvider::~NrEpcX2SapProvider()
{
}

NrEpcX2SapUser::~NrEpcX2SapUser()
{
}

} // namespace ns3
