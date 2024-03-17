// Copyright (c) 2014 Piotr Gawlowicz
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>

#ifndef NR_FR_NO_OP_ALGORITHM_H
#define NR_FR_NO_OP_ALGORITHM_H

#include "nr-ffr-algorithm.h"
#include "nr-ffr-rrc-sap.h"
#include "nr-ffr-sap.h"
#include "nr-rrc-sap.h"

namespace ns3
{

/**
 * \brief FR algorithm implementation which simply does nothing.
 *
 * Selecting this FR algorithm is equivalent to disabling FFR.
 * This is the default choice.
 *
 * To enable FR, please select another FR algorithm, i.e.,
 * another child class of NrFfrAlgorithm.
 */
class NrFrNoOpAlgorithm : public NrFfrAlgorithm
{
  public:
    /**
     * \brief Creates a NoOP FR algorithm instance.
     */
    NrFrNoOpAlgorithm();

    ~NrFrNoOpAlgorithm() override;

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
    friend class MemberNrFfrSapProvider<NrFrNoOpAlgorithm>;
    /// let the forwarder class access the protected and private members
    friend class MemberNrFfrRrcSapProvider<NrFrNoOpAlgorithm>;

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
    // FFR SAP
    NrFfrSapUser* m_ffrSapUser;         ///< FFR SAP user
    NrFfrSapProvider* m_ffrSapProvider; ///< FFR SAP provider

    // FFR RRF SAP
    NrFfrRrcSapUser* m_ffrRrcSapUser;         ///< FFR RRC SAP user
    NrFfrRrcSapProvider* m_ffrRrcSapProvider; ///< FFR RRC SAP provider

}; // end of class NrFrNoOpAlgorithm

} // end of namespace ns3

#endif /* NR_FFR_NO_OP_ALGORITHM_H */
