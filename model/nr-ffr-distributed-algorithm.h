// Copyright (c) 2014 Piotr Gawlowicz
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>

#ifndef NR_FFR_DISTRIBUTED_ALGORITHM_H
#define NR_FFR_DISTRIBUTED_ALGORITHM_H

#include "nr-ffr-algorithm.h"
#include "nr-ffr-rrc-sap.h"
#include "nr-ffr-sap.h"
#include "nr-rrc-sap.h"

namespace ns3
{

/**
 * \brief Distributed Fractional Frequency Reuse algorithm implementation.
 */
class NrFfrDistributedAlgorithm : public NrFfrAlgorithm
{
  public:
    NrFfrDistributedAlgorithm();
    ~NrFfrDistributedAlgorithm() override;

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
    friend class MemberNrFfrSapProvider<NrFfrDistributedAlgorithm>;
    /// let the forwarder class access the protected and private members
    friend class MemberNrFfrRrcSapProvider<NrFfrDistributedAlgorithm>;

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
     * Set down link configuration function
     *
     * \param cellId cell ID
     * \param bandwidth the bandwidth
     */
    void SetDownlinkConfiguration(uint16_t cellId, uint8_t bandwidth);
    /**
     * Set up link configuration function
     *
     * \param cellId cell ID
     * \param bandwidth the bandwidth
     */
    void SetUplinkConfiguration(uint16_t cellId, uint8_t bandwidth);
    /**
     * Initialize down link RBG maps function
     */
    void InitializeDownlinkRbgMaps();
    /**
     * Initialize up link RBG maps function
     */
    void InitializeUplinkRbgMaps();

    /**
     * Initialize up link RBG maps function
     *
     * \param rnti the RNTI
     * \param cellId the cell ID
     * \param rsrp the RSRP
     * \param rsrq the RSRQ
     */
    void UpdateNeighbourMeasurements(uint16_t rnti, uint16_t cellId, uint8_t rsrp, uint8_t rsrq);

    /// Calculate function
    void Calculate();
    /**
     * Send load information function
     *
     * \param targetCellId the cell ID
     */
    void SendLoadInformation(uint16_t targetCellId);

    // FFR SAP
    NrFfrSapUser* m_ffrSapUser;         ///< FFR SAP User
    NrFfrSapProvider* m_ffrSapProvider; ///< FFR SAP Provider

    // FFR RRF SAP
    NrFfrRrcSapUser* m_ffrRrcSapUser;         ///< FFR RRC SAP User
    NrFfrRrcSapProvider* m_ffrRrcSapProvider; ///< FFR RRC SAP Provider

    std::vector<bool> m_dlRbgMap; ///< DL RBG map
    std::vector<bool> m_ulRbgMap; ///< UL RBG map

    uint8_t m_edgeRbNum;              ///< edge RB number
    std::vector<bool> m_dlEdgeRbgMap; ///< DL edge RBG map
    std::vector<bool> m_ulEdgeRbgMap; ///< UL edge RBG map

    /// UePosition enumeration
    enum UePosition
    {
        AreaUnset,
        CenterArea,
        EdgeArea
    };

    std::map<uint16_t, uint8_t> m_ues; ///< UEs map

    uint8_t m_edgeSubBandRsrqThreshold; ///< edge sub band RSRQ threshold

    uint8_t m_centerPowerOffset; ///< center power offset
    uint8_t m_edgePowerOffset;   ///< edge power offset

    uint8_t m_centerAreaTpc; ///< center area TPC
    uint8_t m_edgeAreaTpc;   ///< edge area TCP

    Time m_calculationInterval; ///< calculation interval
    EventId m_calculationEvent; ///< calculation event

    // The expected measurement identity
    uint8_t m_rsrqMeasId; ///< RSRQ measurement ID
    uint8_t m_rsrpMeasId; ///< RSRP measurement ID

    /**
     * \brief Measurements reported by a UE for a cell ID.
     *
     * The values are quantized according 3GPP TS 36.133 section 9.1.4 and 9.1.7.
     */
    class UeMeasure : public SimpleRefCount<UeMeasure>
    {
      public:
        uint16_t m_cellId; ///< Cell ID
        uint8_t m_rsrp;    ///< RSRP
        uint8_t m_rsrq;    ///< RSRQ
    };

    /// Cell Id is used as the key for the following map
    typedef std::map<uint16_t, Ptr<UeMeasure>> MeasurementRow_t;
    /// RNTI is used as the key for the following map
    typedef std::map<uint16_t, MeasurementRow_t> MeasurementTable_t;
    MeasurementTable_t m_ueMeasures; ///< UE measures

    std::vector<uint16_t> m_neighborCell; ///< neighbor cell

    uint8_t m_rsrpDifferenceThreshold; ///< RSRP difference threshold

    std::map<uint16_t, uint32_t> m_cellWeightMap; ///< cell weight map

    std::map<uint16_t, std::vector<bool>> m_rntp; ///< RNTP

}; // end of class NrFfrDistributedAlgorithm

} // end of namespace ns3

#endif /* NR_FR_DISTRIBUTED_ALGORITHM_H */
