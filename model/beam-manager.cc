// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "beam-manager.h"

#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("BeamManager");
NS_OBJECT_ENSURE_REGISTERED(BeamManager);

BeamManager::BeamManager()
{
    // TODO Auto-generated constructor stub
}

void
BeamManager::Configure(const Ptr<UniformPlanarArray>& antennaArray)
{
    m_antennaArray = antennaArray;
    ChangeToQuasiOmniBeamformingVector();
}

PhasedArrayModel::ComplexVector
BeamManager::GetVector(const BeamformingVector& v) const
{
    return v.first;
}

BeamId
BeamManager::GetBeamId(const BeamformingVector& v) const
{
    return v.second;
}

Ptr<const UniformPlanarArray>
BeamManager::GetAntenna() const
{
    return m_antennaArray;
}

BeamManager::~BeamManager()
{
    // TODO Auto-generated destructor stub
}

TypeId
BeamManager::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::BeamManager").SetParent<Object>().AddConstructor<BeamManager>();
    return tid;
}

void
BeamManager::SetPredefinedBeam(PhasedArrayModel::ComplexVector predefinedBeam)
{
    NS_LOG_FUNCTION(this);
    NS_ABORT_MSG_IF(predefinedBeam.GetSize() == 0, "Cannot assign an empty predefined beam");
    NS_ABORT_MSG_IF(predefinedBeam.GetSize() != m_antennaArray->GetNumElems(),
                    "Cannot assign a predefined beamforming vector whose dimension is not "
                    "compatible with antenna array");
    m_predefinedDirTxRxW = std::make_pair(predefinedBeam, PREDEFINED_BEAM_ID);
}

void
BeamManager::SetPredefinedBeam(uint16_t sector, double elevation)
{
    NS_LOG_FUNCTION(this);
    m_predefinedDirTxRxW = std::make_pair(CreateDirectionalBfv(m_antennaArray, sector, elevation),
                                          BeamId(sector, elevation));
}

void
BeamManager::SaveBeamformingVector(const BeamformingVector& bfv, const Ptr<const NetDevice>& device)
{
    NS_LOG_INFO("Save beamforming vector toward device with node id:"
                << device->GetNode()->GetId() << " with BeamId:" << bfv.second);

    if (m_predefinedDirTxRxW.first.GetSize() != 0)
    {
        NS_LOG_WARN("Saving beamforming vector for device, while there is also a predefined "
                    "beamforming vector defined to be used for all transmissions.");
    }

    if (device != nullptr)
    {
        BeamformingStorage::iterator iter = m_beamformingVectorMap.find(device);
        if (iter != m_beamformingVectorMap.end())
        {
            (*iter).second = bfv;
        }
        else
        {
            m_beamformingVectorMap.insert(std::make_pair(device, bfv));
        }
    }
}

void
BeamManager::ChangeBeamformingVector(const Ptr<const NetDevice>& device)
{
    NS_LOG_FUNCTION(this);

    BeamformingStorage::iterator it = m_beamformingVectorMap.find(device);
    if (it == m_beamformingVectorMap.end())
    {
        NS_LOG_INFO("Could not find the beamforming vector for the provided device");

        // if there is no beam defined for this specific device then use a
        // predefined beam if specified and if not, then use quasi omni
        if (m_predefinedDirTxRxW.first.GetSize() != 0)
        {
            m_antennaArray->SetBeamformingVector(m_predefinedDirTxRxW.first);
        }
        else
        {
            ChangeToQuasiOmniBeamformingVector();
        }
    }
    else
    {
        NS_LOG_INFO("Beamforming vector found");
        m_antennaArray->SetBeamformingVector(it->second.first);
    }
}

PhasedArrayModel::ComplexVector
BeamManager::GetCurrentBeamformingVector()
{
    return m_antennaArray->GetBeamformingVector();
}

void
BeamManager::ChangeToQuasiOmniBeamformingVector()
{
    NS_LOG_FUNCTION(this);

    UintegerValue numRows;
    UintegerValue numColumns;
    m_antennaArray->GetAttribute("NumRows", numRows);
    m_antennaArray->GetAttribute("NumColumns", numColumns);

    /**
     * Before configuring omni beamforming vector,
     * we want to make sure that it is being calculated
     * with the actual number of antenna rows and columns.
     * We want to avoid recalculations, if these numbers didn't
     * change. Which will normally be the case.
     */
    if (numRows.Get() != m_numRows || numColumns.Get() != m_numColumns ||
        m_isPolDual != m_antennaArray->IsDualPol())
    {
        m_isPolDual = m_antennaArray->IsDualPol();
        m_numPortElems = m_antennaArray->GetNumElemsPerPort();
        m_numRows = numRows.Get();
        m_numColumns = numColumns.Get();
        m_omniTxRxW = std::make_pair(CreateQuasiOmniBfv(m_antennaArray), OMNI_BEAM_ID);
    }

    m_antennaArray->SetBeamformingVector(m_omniTxRxW.first);
}

PhasedArrayModel::ComplexVector
BeamManager::GetBeamformingVector(const Ptr<NetDevice>& device) const
{
    NS_LOG_FUNCTION(this);
    PhasedArrayModel::ComplexVector beamformingVector;
    BeamformingStorage::const_iterator it = m_beamformingVectorMap.find(device);
    if (it != m_beamformingVectorMap.end())
    {
        beamformingVector = it->second.first;
    }
    else
    {
        // it there is no specific beam saved for this device, check
        // whether we have a predefined beam set, if yes return its vector
        if (m_predefinedDirTxRxW.first.GetSize() != 0)
        {
            beamformingVector = m_predefinedDirTxRxW.first;
        }
        else
        {
            beamformingVector = m_antennaArray->GetBeamformingVector();
        }
    }
    return beamformingVector;
}

BeamId
BeamManager::GetBeamId(const Ptr<NetDevice>& device) const
{
    BeamId beamId;
    BeamformingStorage::const_iterator it = m_beamformingVectorMap.find(device);
    if (it != m_beamformingVectorMap.end())
    {
        beamId = it->second.second;
    }
    else
    {
        // it there is no specific beam saved for this device, check
        // whether we have a predefined beam set, if yes return its ID
        if (m_predefinedDirTxRxW.first.GetSize() != 0)
        {
            beamId = m_predefinedDirTxRxW.second;
        }
        else
        {
            beamId = OMNI_BEAM_ID;
        }
    }
    return beamId;
}

void
BeamManager::SetSector(double sector, double elevation) const
{
    NS_LOG_INFO("Set sector to : " << (unsigned)sector << ", and elevation to: " << elevation);
    m_antennaArray->SetBeamformingVector(CreateDirectionalBfv(m_antennaArray, sector, elevation));
}

} /* namespace ns3 */
