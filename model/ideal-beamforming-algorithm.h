// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_

#include "beam-id.h"
#include "beamforming-vector.h"

#include <ns3/object.h>

namespace ns3
{

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;
class NrSpectrumPhy;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * IdealBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices.
 *
 * Algorithms that inherit this class assume a perfect knowledge of the channel,
 * because of which this group of algorithms is called "ideal".
 */
class IdealBeamformingAlgorithm : public Object
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief Function that generates the beamforming vectors for a pair of communicating devices
     * \param [in] gnbSpectrumPhy gNb spectrum phy instance
     * \param [in] ueSpectrumPhy UE spectrum phy instance
     */
    virtual BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const = 0;
};

/**
 * \ingroup gnb-phy
 * \brief The CellScanBeamforming class
 */
class CellScanBeamforming : public IdealBeamformingAlgorithm
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \return Gets value of BeamSearchAngleStep attribute
     */
    double GetBeamSearchAngleStep() const;

    /**
     * \brief Sets the value of BeamSearchAngleStep attribute
     */
    void SetBeamSearchAngleStep(double beamSearchAngleStep);

    /**
     * \brief constructor
     */
    CellScanBeamforming() = default;

    /**
     * \brief destructor
     */
    ~CellScanBeamforming() override = default;

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using cell scan method
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE device
     * \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;

  private:
    double m_beamSearchAngleStep{30}; //!< the beam search angle step attribute
};

/**
 * \ingroup gnb-phy
 * \brief The CellScanBeamformingAzimuthZenith class
 */
class CellScanBeamformingAzimuthZenith : public IdealBeamformingAlgorithm
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief constructor
     */
    CellScanBeamformingAzimuthZenith() = default;

    /**
     * \brief destructor
     */
    ~CellScanBeamformingAzimuthZenith() override = default;

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using cell scan method
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE
     * \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;

  private:
    std::vector<double> m_azimuth{-56.25, -33.75, -11.25, 11.25, 33.75, 56.25};
    std::vector<double> m_zenith{112.5, 157.5};
};

/**
 * \ingroup gnb-phy
 * \brief The CellScanQuasiOmniBeamforming class
 */
class CellScanQuasiOmniBeamforming : public IdealBeamformingAlgorithm
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \return Gets value of BeamSearchAngleStep attribute
     */
    double GetBeamSearchAngleStep() const;

    /**
     * \brief Sets the value of BeamSearchAngleStep attribute
     */
    void SetBeamSearchAngleStep(double beamSearchAngleStep);

    /**
     * \brief constructor
     */
    CellScanQuasiOmniBeamforming() = default;

    /**
     * \brief destructor
     */
    ~CellScanQuasiOmniBeamforming() override = default;

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using cell scan method at gNB and a fixed quasi-omni beamforming
     * vector at UE \param [in] gnbSpectrumPhy the spectrum phy of the gNB \param [in] ueSpectrumPhy
     * the spectrum phy of the UE \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;

  private:
    double m_beamSearchAngleStep{30};
};

/**
 * \ingroup gnb-phy
 * \brief The DirectPathBeamforming class
 */
class DirectPathBeamforming : public IdealBeamformingAlgorithm
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using the direct path direction
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE
     * \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;
};

/**
 * \ingroup gnb-phy
 * \brief The QuasiOmniDirectPathBeamforming class
 */
class QuasiOmniDirectPathBeamforming : public DirectPathBeamforming
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using the quasi omni beamforming vector for gNB
     * and direct path beamforming vector for UEs
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE
     * \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;
};

/**
 * \ingroup gnb-phy
 * \brief The QuasiOmniDirectPathBeamforming class
 */
class DirectPathQuasiOmniBeamforming : public DirectPathBeamforming
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using the direct-path beamforming vector for gNB
     * and quasi-omni beamforming vector for UEs
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE
     * \return the beamforming vector pair of the gNB and the UE
     *
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;
};

/**
 * \ingroup gnb-phy
 * \brief The OptimalCovMatrixBeamforming class not implemented yet.
 * TODO The idea was to port one of the initial beamforming methods that
 * were implemented in NYU/University of Padova mmwave module.
 * Method is based on a long term covariation matrix.
 */
class OptimalCovMatrixBeamforming : public IdealBeamformingAlgorithm
{
  public:
    /**
     * \brief Get the type id
     * \return the type id of the class
     */
    static TypeId GetTypeId();

    /**
     * \brief Function that generates the beamforming vectors for a pair of
     * communicating devices by using the direct-path beamforming vector for gNB
     * and quasi-omni beamforming vector for UEs
     * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
     * \param [in] ueSpectrumPhy the spectrum phy of the UE
     * \return the beamforming vector pair of the gNB and the UE
     */
    BeamformingVectorPair GetBeamformingVectors(
        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;
};

} // namespace ns3
#endif
