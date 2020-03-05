/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef SRC_NR_MODEL_BEAM_MANAGER_H_
#define SRC_NR_MODEL_BEAM_MANAGER_H_

#include "ideal-beamforming-algorithm.h"
#include "ns3/event-id.h"
#include <ns3/nstime.h>
#include <ns3/net-device.h>


namespace ns3 {

class MmWaveUeNetDevice;
class MmWaveEnbNetDevice;
class IdealBeamformingHelper;

/**
 * \ingroup beam-management
 * \brief BeamManager is responsible of installation and configuration of antenna
 * array. Additionally, in the case of gNB it saves the map of beamforming
 * vectors per device.
 */
class BeamManager: public Object {

friend IdealBeamformingHelper;

public:

  BeamManager();

  virtual ~BeamManager();

  /**
   * \brief GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();

  /**
   * \brief Configures quasi-omni beamforming vector and sets up the expire timer
   * for beamforming
   * \param antennaNumDim1 the first antenna dimension in number of elements
   * \param antennaNumDim2 the second antenna dimension in number of elements
   */
  void Configure (const Ptr<ThreeGppAntennaArrayModel>& antennaArray, uint32_t antennaNumDim1, uint32_t antennaNumDim2);

  /**
   * \brief Get weight vector from a BeamformingVector
   * \param v the BeamformingVector
   * \return the weight vector
   */
  complexVector_t GetVector (const BeamformingVector& v) const;

  /**
   * \brief Extract the beam id from the beamforming vector specified
   * \return the beam id
   * \param v the beamforming vector
   */
  BeamId GetBeamId (const BeamformingVector &v) const;


  typedef std::map<const Ptr<const NetDevice>, BeamformingVector> BeamformingStorage; //!< BeamformingStorage type used to save the map of beamforming vectors per device

  /**
   * \brief Function that saves the beamforming weights of the antenna
   * for transmission or reception to/from a specified connected device.
   * \param antennaWeights the weights of the beamforming vector
   * \param beamId the unique identifier of the beam
   * \param device device to which it is being transmitted, or from which is
   * being received
   */
public:
  virtual void SaveBeamformingVector (const BeamformingVector& bfv,
                                      const Ptr<const NetDevice>& device);
  /**
   * \brief Change the beamforming vector for tx/rx to/from specified device
   * \param device Device to change the beamforming vector for
   */
  virtual void ChangeBeamformingVector (const Ptr<const NetDevice>& device);

  /**
   * \brief Change the antenna model to omnidirectional (ignoring the beams) TODO check this
   */
  virtual void ChangeToOmniTx ();

  /**
   * \brief Function that returns the beamforming vector that is currently being
   * used by the antenna.
   * \return the current beamforming vector
   */
  virtual complexVector_t GetCurrentBeamformingVector ();

  /**
   * \brief Function that returns the beamforming vector weights that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual complexVector_t GetBeamformingVector (const Ptr<NetDevice>& device) const;


  /**
   * \brief Function that returns the beamId of the beam that is used to
   * communicated with a specified device
   * \return the current beamforming vector
   */
  virtual BeamId GetBeamId (const Ptr<NetDevice>& device) const;

  /**
   * \brief Generate a omni directional beamforming vector
   * \param antennaNumDim1 First dimension of the antenna
   * \param antennaNumDim2 Second dimension of the antenna
   * \return the beamforming vector
   */
  BeamformingVector GenerateOmniTxRxW (uint32_t antennaNumDim1, uint32_t antennaNumDim2) const;


  void SetSector (uint16_t sector, double elevation) const;

private:

  Ptr<ThreeGppAntennaArrayModel> m_antennaArray;  // the antenna array instance for which is responsible this BeamManager
  BeamformingVector m_omniTxRxW; //!< Beamforming vector that emulates omnidirectional transmission and reception
  BeamformingStorage m_beamformingVectorMap; //!< device to beamforming vector mapping

};

} /* namespace ns3 */

#endif /* SRC_NR_MODEL_BEAM_MANAGER_H_ */
