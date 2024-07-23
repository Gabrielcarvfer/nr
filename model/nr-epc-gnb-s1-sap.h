// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#ifndef NR_EPC_ENB_S1_SAP_H
#define NR_EPC_ENB_S1_SAP_H

#include "nr-eps-bearer.h"

#include <ns3/ipv4-address.h>

#include <list>

namespace ns3
{

/**
 * This class implements the Service Access Point (SAP) between the
 * NrGnbRrc and the NrEpcEnbApplication. In particular, this class implements the
 * Provider part of the SAP, i.e., the methods exported by the
 * NrEpcEnbApplication and called by the NrGnbRrc.
 */
class NrEpcEnbS1SapProvider
{
  public:
    virtual ~NrEpcEnbS1SapProvider();

    /**
     * Initial UE message.
     *
     * \param imsi IMSI
     * \param rnti RNTI
     */
    virtual void InitialUeMessage(uint64_t imsi, uint16_t rnti) = 0;

    /**
     * \brief Triggers epc-enb-application to send ERAB Release Indication message towards MME
     * \param imsi the UE IMSI
     * \param rnti the UE RNTI
     * \param bearerId Bearer Identity which is to be de-activated
     */
    virtual void DoSendReleaseIndication(uint64_t imsi, uint16_t rnti, uint8_t bearerId) = 0;

    /// BearerToBeSwitched structure
    struct BearerToBeSwitched
    {
        uint8_t epsBearerId; ///< Bearer ID
        uint32_t teid;       ///< TEID
    };

    /// PathSwitchRequestParameters structure
    struct PathSwitchRequestParameters
    {
        uint16_t rnti;      ///< RNTI
        uint16_t cellId;    ///< cell ID
        uint32_t mmeUeS1Id; ///< mmeUeS1Id in practice, we use the IMSI
        std::list<BearerToBeSwitched> bearersToBeSwitched; ///< list of bearers to be switched
    };

    /**
     * Path Switch Request
     *
     * \param params
     */
    virtual void PathSwitchRequest(PathSwitchRequestParameters params) = 0;

    /**
     * Release UE context at the S1 Application of the source eNB after
     * reception of the UE CONTEXT RELEASE X2 message from the target eNB
     * during X2-based handover
     *
     * \param rnti RNTI
     */
    virtual void UeContextRelease(uint16_t rnti) = 0;
};

/**
 * This class implements the Service Access Point (SAP) between the
 * NrGnbRrc and the NrEpcEnbApplication. In particular, this class implements the
 * User part of the SAP, i.e., the methods exported by the NrGnbRrc
 * and called by the NrEpcEnbApplication.
 */
class NrEpcEnbS1SapUser
{
  public:
    virtual ~NrEpcEnbS1SapUser();

    /**
     * Parameters passed to InitialContextSetupRequest ()
     */
    struct InitialContextSetupRequestParameters
    {
        uint16_t rnti; /**< the RNTI identifying the UE */
    };

    /**
     * Initial context setup request
     *
     * \param params Parameters
     */
    virtual void InitialContextSetupRequest(InitialContextSetupRequestParameters params) = 0;

    /**
     * Parameters passed to DataRadioBearerSetupRequest ()
     */
    struct DataRadioBearerSetupRequestParameters
    {
        uint16_t rnti;      /**< the RNTI identifying the UE for which the
                                 DataRadioBearer is to be created */
        NrEpsBearer bearer; /**< the characteristics of the bearer to be setup */
        uint8_t bearerId;   /**< the EPS Bearer Identifier */
        uint32_t gtpTeid;   /**< S1-bearer GTP tunnel endpoint identifier, see 36.423 9.2.1 */
        Ipv4Address transportLayerAddress; /**< IP Address of the SGW, see 36.423 9.2.1 */
    };

    /**
     * Request the setup of a DataRadioBearer
     *
     * \param params Parameters
     */
    virtual void DataRadioBearerSetupRequest(DataRadioBearerSetupRequestParameters params) = 0;

    /// PathSwitchRequestAcknowledgeParameters structure
    struct PathSwitchRequestAcknowledgeParameters
    {
        uint16_t rnti; ///< RNTI
    };

    /**
     * Request a path switch acknowledge
     *
     * \param params Parameters
     */
    virtual void PathSwitchRequestAcknowledge(PathSwitchRequestAcknowledgeParameters params) = 0;
};

/**
 * Template for the implementation of the NrEpcEnbS1SapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcEnbS1SapProvider : public NrEpcEnbS1SapProvider
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    NrMemberEpcEnbS1SapProvider(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcEnbS1SapProvider() = delete;

    // inherited from NrEpcEnbS1SapProvider
    void InitialUeMessage(uint64_t imsi, uint16_t rnti) override;
    void DoSendReleaseIndication(uint64_t imsi, uint16_t rnti, uint8_t bearerId) override;

    void PathSwitchRequest(PathSwitchRequestParameters params) override;
    void UeContextRelease(uint16_t rnti) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcEnbS1SapProvider<C>::NrMemberEpcEnbS1SapProvider(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcEnbS1SapProvider<C>::InitialUeMessage(uint64_t imsi, uint16_t rnti)
{
    m_owner->DoInitialUeMessage(imsi, rnti);
}

template <class C>
void
NrMemberEpcEnbS1SapProvider<C>::DoSendReleaseIndication(uint64_t imsi,
                                                        uint16_t rnti,
                                                        uint8_t bearerId)
{
    m_owner->DoReleaseIndication(imsi, rnti, bearerId);
}

template <class C>
void
NrMemberEpcEnbS1SapProvider<C>::PathSwitchRequest(PathSwitchRequestParameters params)
{
    m_owner->DoPathSwitchRequest(params);
}

template <class C>
void
NrMemberEpcEnbS1SapProvider<C>::UeContextRelease(uint16_t rnti)
{
    m_owner->DoUeContextRelease(rnti);
}

/**
 * Template for the implementation of the NrEpcEnbS1SapUser as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcEnbS1SapUser : public NrEpcEnbS1SapUser
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    NrMemberEpcEnbS1SapUser(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcEnbS1SapUser() = delete;

    // inherited from NrEpcEnbS1SapUser
    void InitialContextSetupRequest(InitialContextSetupRequestParameters params) override;
    void DataRadioBearerSetupRequest(DataRadioBearerSetupRequestParameters params) override;
    void PathSwitchRequestAcknowledge(PathSwitchRequestAcknowledgeParameters params) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcEnbS1SapUser<C>::NrMemberEpcEnbS1SapUser(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcEnbS1SapUser<C>::InitialContextSetupRequest(InitialContextSetupRequestParameters params)
{
    m_owner->DoInitialContextSetupRequest(params);
}

template <class C>
void
NrMemberEpcEnbS1SapUser<C>::DataRadioBearerSetupRequest(
    DataRadioBearerSetupRequestParameters params)
{
    m_owner->DoDataRadioBearerSetupRequest(params);
}

template <class C>
void
NrMemberEpcEnbS1SapUser<C>::PathSwitchRequestAcknowledge(
    PathSwitchRequestAcknowledgeParameters params)
{
    m_owner->DoPathSwitchRequestAcknowledge(params);
}

} // namespace ns3

#endif // NR_EPC_ENB_S1_SAP_H
