// Copyright (c) 2014 Piotr Gawlowicz
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>

#ifndef NR_FR_HARD_ALGORITHM_H
#define NR_FR_HARD_ALGORITHM_H

#include "nr-ffr-algorithm.h"
#include "nr-ffr-rrc-sap.h"
#include "nr-ffr-sap.h"
#include "nr-rrc-sap.h"

namespace ns3
{

/**
 * \brief Hard Frequency Reuse algorithm implementation which uses only 1 sub-band.
 */
class NrFrHardAlgorithm : public NrFfrAlgorithm
{
  public:
    /**
     * \brief Creates a trivial ffr algorithm instance.
     */
    NrFrHardAlgorithm();

    ~NrFrHardAlgorithm() override;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    // inherited from NrFfrAlgorithm
    void SetNrFfrSapUser(NrFfrSapUser* s) override;
    NrFfrSapProvider* GetNrFfrSapProvider() override;

    void SetNrFfrRrcSapUser(NrFfrRrcSapUser* s) override;
    NrFfrRrcSapProvider* GetNrFfrRrcSapProvider() override;

    /// let the forwarder class access the protected and private members
    friend class MemberNrFfrSapProvider<NrFrHardAlgorithm>;
    /// let the forwarder class access the protected and private members
    friend class MemberNrFfrRrcSapProvider<NrFrHardAlgorithm>;

  protected:
    // inherited from Object
    void DoInitialize() override;
    void DoDispose() override;

    void Reconfigure() override;

    // FFR SAP PROVIDER IMPLEMENTATION
    std::vector<bool> DoGetAvailableDlRbg() override;
    bool DoIsDlRbgAvailableForUe(int i, uint16_t rnti) override;
    std::vector<bool> DoGetAvailableUlRbg() override;
    bool DoIsUlRbgAvailableForUe(int i, uint16_t rnti) override;
    void DoReportDlCqiInfo(
        const FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) override;
    void DoReportUlCqiInfo(
        const FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) override;
    void DoReportUlCqiInfo(std::map<uint16_t, std::vector<double>> ulCqiMap) override;
    uint8_t DoGetTpc(uint16_t rnti) override;
    uint16_t DoGetMinContinuousUlBandwidth() override;

    // FFR SAP RRC PROVIDER IMPLEMENTATION
    void DoReportUeMeas(uint16_t rnti, NrRrcSap::MeasResults measResults) override;
    void DoRecvLoadInformation(NrEpcX2Sap::LoadInformationParams params) override;

  private:
    /**
     * Set downlink configuration
     *
     * \param cellId the cell ID
     * \param bandwidth the bandwidth
     */
    void SetDownlinkConfiguration(uint16_t cellId, uint8_t bandwidth);
    /**
     * Set uplink configuration
     *
     * \param cellId the cell ID
     * \param bandwidth the bandwidth
     */
    void SetUplinkConfiguration(uint16_t cellId, uint8_t bandwidth);
    /**
     * Initialize downlink rbg maps
     */
    void InitializeDownlinkRbgMaps();
    /**
     * Initialize uplink rbg maps
     */
    void InitializeUplinkRbgMaps();

    // FFR SAP
    NrFfrSapUser* m_ffrSapUser;         ///< FFR SAP user
    NrFfrSapProvider* m_ffrSapProvider; ///< FFR SAP provider

    // FFR RRF SAP
    NrFfrRrcSapUser* m_ffrRrcSapUser;         ///< FFR RRC SAP user
    NrFfrRrcSapProvider* m_ffrRrcSapProvider; ///< FFR RRC SAP provider

    uint8_t m_dlOffset;  ///< DL offset
    uint8_t m_dlSubBand; ///< DL subband

    uint8_t m_ulOffset;  ///< UL offset
    uint8_t m_ulSubBand; ///< UL subband

    std::vector<bool> m_dlRbgMap; ///< DL RBG Map
    std::vector<bool> m_ulRbgMap; ///< UL RBG Map

}; // end of class NrFrHardAlgorithm

} // end of namespace ns3

#endif /* NR_FR_HARD_ALGORITHM_H */
