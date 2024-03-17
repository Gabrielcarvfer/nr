// Copyright 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#include "nr-hex-grid-enb-topology-helper.h"

#include "nr-epc-helper.h"

#include <ns3/abort.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/pointer.h>

#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrHexGridEnbTopologyHelper");

NS_OBJECT_ENSURE_REGISTERED(NrHexGridEnbTopologyHelper);

NrHexGridEnbTopologyHelper::NrHexGridEnbTopologyHelper()
{
    NS_LOG_FUNCTION(this);
}

NrHexGridEnbTopologyHelper::~NrHexGridEnbTopologyHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrHexGridEnbTopologyHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrHexGridEnbTopologyHelper")
            .SetParent<Object>()
            .AddConstructor<NrHexGridEnbTopologyHelper>()
            .AddAttribute("InterSiteDistance",
                          "The distance [m] between nearby sites",
                          DoubleValue(500),
                          MakeDoubleAccessor(&NrHexGridEnbTopologyHelper::m_d),
                          MakeDoubleChecker<double>())
            .AddAttribute("SectorOffset",
                          "The offset [m] in the position for the node of each sector with respect "
                          "to the center of the three-sector site",
                          DoubleValue(0.5),
                          MakeDoubleAccessor(&NrHexGridEnbTopologyHelper::m_offset),
                          MakeDoubleChecker<double>())
            .AddAttribute("SiteHeight",
                          "The height [m] of each site",
                          DoubleValue(30),
                          MakeDoubleAccessor(&NrHexGridEnbTopologyHelper::m_siteHeight),
                          MakeDoubleChecker<double>())
            .AddAttribute("MinX",
                          "The x coordinate where the hex grid starts.",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&NrHexGridEnbTopologyHelper::m_xMin),
                          MakeDoubleChecker<double>())
            .AddAttribute("MinY",
                          "The y coordinate where the hex grid starts.",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&NrHexGridEnbTopologyHelper::m_yMin),
                          MakeDoubleChecker<double>())
            .AddAttribute(
                "GridWidth",
                "The number of sites in even rows (odd rows will have one additional site).",
                UintegerValue(1),
                MakeUintegerAccessor(&NrHexGridEnbTopologyHelper::m_gridWidth),
                MakeUintegerChecker<uint32_t>());
    return tid;
}

void
NrHexGridEnbTopologyHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
NrHexGridEnbTopologyHelper::SetNrHelper(Ptr<NrHelper> h)
{
    NS_LOG_FUNCTION(this << h);
    m_lteHelper = h;
}

NetDeviceContainer
NrHexGridEnbTopologyHelper::SetPositionAndInstallEnbDevice(NodeContainer c)
{
    NS_LOG_FUNCTION(this);
    NetDeviceContainer enbDevs;
    const double xydfactor = std::sqrt(0.75);
    double yd = xydfactor * m_d;
    for (uint32_t n = 0; n < c.GetN(); ++n)
    {
        uint32_t currentSite = n / 3;
        uint32_t biRowIndex = (currentSite / (m_gridWidth + m_gridWidth + 1));
        uint32_t biRowRemainder = currentSite % (m_gridWidth + m_gridWidth + 1);
        uint32_t rowIndex = biRowIndex * 2;
        uint32_t colIndex = biRowRemainder;
        if (biRowRemainder >= m_gridWidth)
        {
            ++rowIndex;
            colIndex -= m_gridWidth;
        }
        NS_LOG_LOGIC("node " << n << " site " << currentSite << " rowIndex " << rowIndex
                             << " colIndex " << colIndex << " biRowIndex " << biRowIndex
                             << " biRowRemainder " << biRowRemainder);
        double y = m_yMin + yd * rowIndex;
        double x;
        double antennaOrientation;
        if ((rowIndex % 2) == 0)
        {
            x = m_xMin + m_d * colIndex;
        }
        else // row is odd
        {
            x = m_xMin - (0.5 * m_d) + m_d * colIndex;
        }

        switch (n % 3)
        {
        case 0:
            antennaOrientation = 0;
            x += m_offset;
            m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue(1));
            break;

        case 1:
            antennaOrientation = 120;
            x -= m_offset / 2.0;
            y += m_offset * xydfactor;
            m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue(2));
            break;

        case 2:
            antennaOrientation = -120;
            x -= m_offset / 2.0;
            y -= m_offset * xydfactor;
            m_lteHelper->SetFfrAlgorithmAttribute("FrCellTypeId", UintegerValue(3));
            break;

            // no default, n%3 = 0, 1, 2
        }
        Ptr<Node> node = c.Get(n);
        Ptr<MobilityModel> mm = node->GetObject<MobilityModel>();
        Vector pos(x, y, m_siteHeight);
        NS_LOG_LOGIC("node " << n << " at " << pos << " antennaOrientation " << antennaOrientation);
        mm->SetPosition(Vector(x, y, m_siteHeight));
        m_lteHelper->SetEnbAntennaModelAttribute("Orientation", DoubleValue(antennaOrientation));
        enbDevs.Add(m_lteHelper->InstallEnbDevice(node));
    }
    return enbDevs;
}

} // namespace ns3
