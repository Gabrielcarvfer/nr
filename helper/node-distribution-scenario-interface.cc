// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "node-distribution-scenario-interface.h"

#include <cmath> // cos, sin, M_PI (non-standard)

namespace ns3
{

NodeDistributionScenarioInterface::~NodeDistributionScenarioInterface()
{
}

const NodeContainer&
NodeDistributionScenarioInterface::GetBaseStations() const
{
    return m_bs;
}

const NodeContainer&
NodeDistributionScenarioInterface::GetUserTerminals() const
{
    return m_ut;
}

void
NodeDistributionScenarioInterface::SetSitesNumber(std::size_t n)
{
    NS_ASSERT_MSG(m_sectorization != SiteSectorizationType::NONE, "Must set sectorization first.");
    m_numSites = n;
    auto sectors = static_cast<std::size_t>(m_sectorization);
    m_numBs = m_numSites * sectors;
}

void
NodeDistributionScenarioInterface::SetBsNumber(std::size_t n)
{
    NS_ASSERT_MSG(m_sectorization != SiteSectorizationType::NONE, "Must set sectorization first.");
    auto sectors = static_cast<std::size_t>(m_sectorization);
    // Compute the number of sites, then set that,
    // so m_numBs is consistent with sectorization
    std::size_t sites = n / sectors;
    SetSitesNumber(sites);
}

void
NodeDistributionScenarioInterface::SetUtNumber(std::size_t n)
{
    m_numUt = n;
}

std::size_t
NodeDistributionScenarioInterface::GetNumSites() const
{
    return m_numSites;
}

std::size_t
NodeDistributionScenarioInterface::GetNumCells() const
{
    return m_numBs;
}

double
NodeDistributionScenarioInterface::GetAntennaOrientationDegrees(std::size_t cellId) const
{
    double orientation = 0.0;
    if (m_sectorization == TRIPLE)
    {
        auto sectors = static_cast<std::size_t>(m_sectorization);
        std::size_t sector = cellId % sectors;
        double sectorSize = 360.0 / sectors;
        orientation = sectorSize * (sector + 0.25); // First sector starts at 0
    }
    return orientation;
}

double
NodeDistributionScenarioInterface::GetAntennaOrientationRadians(std::size_t cellId) const
{
    double orientationRads = GetAntennaOrientationDegrees(cellId) * M_PI / 180;
    if (orientationRads > M_PI)
    {
        orientationRads -= 2 * M_PI;
    }

    return orientationRads;
}

uint16_t
NodeDistributionScenarioInterface::GetSiteIndex(std::size_t cellId) const
{
    auto sectors = static_cast<std::size_t>(m_sectorization);
    return cellId / sectors;
}

uint16_t
NodeDistributionScenarioInterface::GetSectorIndex(std::size_t cellId) const
{
    auto sectors = static_cast<std::size_t>(m_sectorization);
    return cellId % sectors;
}

uint16_t
NodeDistributionScenarioInterface::GetCellIndex(std::size_t ueId) const
{
    return ueId % m_numBs;
}

Vector
NodeDistributionScenarioInterface::GetAntennaPosition(const Vector& sitePos, uint16_t cellId) const
{
    Vector pos(sitePos);

    double angle = GetAntennaOrientationDegrees(cellId);
    pos.x += m_antennaOffset * cos(angle * M_PI / 180);
    pos.y += m_antennaOffset * sin(angle * M_PI / 180);
    return pos;
}

} // namespace ns3
