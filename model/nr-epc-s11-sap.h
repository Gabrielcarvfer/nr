// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.cat>

#ifndef NR_EPC_S11_SAP_H
#define NR_EPC_S11_SAP_H

#include "nr-epc-tft.h"
#include "nr-eps-bearer.h"

#include "ns3/address.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3
{

/**
 * NrEpcS11Sap
 */
class NrEpcS11Sap
{
  public:
    virtual ~NrEpcS11Sap() = default;

    /**
     * GTPC message
     */
    struct GtpcMessage
    {
        uint32_t teid; ///< TEID
    };

    /**
     * Fully-qualified TEID, see 3GPP TS 29.274 section 8.22
     */
    struct Fteid
    {
        uint32_t teid;       ///< TEID
        Ipv4Address address; ///< IP address
    };

    /**
     * TS 29.274 8.21  User Location Information (ULI)
     */
    struct Uli
    {
        uint16_t gci; ///< GCI
    };
};

/**
 * @ingroup nr
 *
 * MME side of the S11 Service Access Point (SAP), provides the MME
 * methods to be called when an S11 message is received by the MME.
 */
class NrEpcS11SapMme : public NrEpcS11Sap
{
  public:
    /**
     * 3GPP TS 29.274 version 8.3.1 Release 8 section 8.28
     */
    struct BearerContextCreated
    {
        NrEpcS11Sap::Fteid sgwFteid; ///< EPC FTEID
        uint8_t epsBearerId;         ///< EPS bearer ID
        NrEpsBearer bearerLevelQos;  ///< EPS bearer
        Ptr<NrEpcTft> tft;           ///< traffic flow template
    };

    /**
     * Create Session Response message, see 3GPP TS 29.274 7.2.2
     */
    struct CreateSessionResponseMessage : public GtpcMessage
    {
        std::list<BearerContextCreated> bearerContextsCreated; ///< bearer contexts created
    };

    /**
     * send a Create Session Response message
     *
     * @param msg the message
     */
    virtual void CreateSessionResponse(CreateSessionResponseMessage msg) = 0;

    /**
     * Bearer Context Removed structure
     */
    struct BearerContextRemoved
    {
        uint8_t epsBearerId; ///< EPS bearer ID
    };

    /**
     * Delete Bearer Request message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.9.2
     */
    struct DeleteBearerRequestMessage : public GtpcMessage
    {
        std::list<BearerContextRemoved> bearerContextsRemoved; ///< list of bearer context removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Request message shall be sent
     * on the S11 interface by PGW to SGW and from SGW to MME
     * @param msg the message
     */
    virtual void DeleteBearerRequest(DeleteBearerRequestMessage msg) = 0;

    /**
     * Modify Bearer Response message, see 3GPP TS 29.274 7.2.7
     */
    struct ModifyBearerResponseMessage : public GtpcMessage
    {
        /// Cause enumeration
        enum Cause
        {
            REQUEST_ACCEPTED = 0,
            REQUEST_ACCEPTED_PARTIALLY,
            REQUEST_REJECTED,
            CONTEXT_NOT_FOUND
        };

        Cause cause; ///< the cause
    };

    /**
     * Send a Modify Bearer Response message
     *
     * @param msg the message
     */
    virtual void ModifyBearerResponse(ModifyBearerResponseMessage msg) = 0;
};

/**
 * @ingroup nr
 *
 * SGW side of the S11 Service Access Point (SAP), provides the SGW
 * methods to be called when an S11 message is received by the SGW.
 */
class NrEpcS11SapSgw : public NrEpcS11Sap
{
  public:
    /// BearerContextToBeCreated structure
    struct BearerContextToBeCreated
    {
        NrEpcS11Sap::Fteid sgwFteid; ///< FTEID
        uint8_t epsBearerId;         ///< EPS bearer ID
        NrEpsBearer bearerLevelQos;  ///< bearer QOS level
        Ptr<NrEpcTft> tft;           ///< traffic flow template
    };

    /**
     * Create Session Request message, see 3GPP TS 29.274 7.2.1
     */
    struct CreateSessionRequestMessage : public GtpcMessage
    {
        uint64_t imsi; ///< IMSI
        Uli uli;       ///< ULI
        std::list<BearerContextToBeCreated>
            bearerContextsToBeCreated; ///< list of bearer contexts to be created
    };

    /**
     * Send a Create Session Request message
     *
     * @param msg the message
     */
    virtual void CreateSessionRequest(CreateSessionRequestMessage msg) = 0;

    /// BearerContextToBeCreated structure
    struct BearerContextToBeRemoved
    {
        uint8_t epsBearerId; ///< EPS bearer ID
    };

    /**
     * Delete Bearer Command message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.17.1
     */
    struct DeleteBearerCommandMessage : public GtpcMessage
    {
        std::list<BearerContextToBeRemoved>
            bearerContextsToBeRemoved; ///< list of bearer contexts to be removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Command message shall be sent
     * on the S11 interface by the MME to the SGW
     * @param msg the DeleteBearerCommandMessage
     */
    virtual void DeleteBearerCommand(DeleteBearerCommandMessage msg) = 0;

    /// BearerContextRemovedSgwPgw structure
    struct BearerContextRemovedSgwPgw
    {
        uint8_t epsBearerId; ///< EPS bearer ID
    };

    /**
     * Delete Bearer Response message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.10.2
     */
    struct DeleteBearerResponseMessage : public GtpcMessage
    {
        std::list<BearerContextRemovedSgwPgw>
            bearerContextsRemoved; ///< list of bearer contexts removed
    };

    /**
     * @brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Command message shall be sent
     * on the S11 interface by the MME to the SGW
     * @param msg the message
     */
    virtual void DeleteBearerResponse(DeleteBearerResponseMessage msg) = 0;

    /**
     * Modify Bearer Request message, see 3GPP TS 29.274 7.2.7
     */
    struct ModifyBearerRequestMessage : public GtpcMessage
    {
        Uli uli; ///< ULI
    };

    /**
     * Send a Modify Bearer Request message
     *
     * @param msg the message
     */
    virtual void ModifyBearerRequest(ModifyBearerRequestMessage msg) = 0;
};

/**
 * Template for the implementation of the NrEpcS11SapMme as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcS11SapMme : public NrEpcS11SapMme
{
  public:
    /**
     * Constructor
     *
     * @param owner the owner class
     */
    NrMemberEpcS11SapMme(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcS11SapMme() = delete;

    // inherited from NrEpcS11SapMme
    void CreateSessionResponse(CreateSessionResponseMessage msg) override;
    void ModifyBearerResponse(ModifyBearerResponseMessage msg) override;
    void DeleteBearerRequest(DeleteBearerRequestMessage msg) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcS11SapMme<C>::NrMemberEpcS11SapMme(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcS11SapMme<C>::CreateSessionResponse(CreateSessionResponseMessage msg)
{
    m_owner->DoCreateSessionResponse(msg);
}

template <class C>
void
NrMemberEpcS11SapMme<C>::DeleteBearerRequest(DeleteBearerRequestMessage msg)
{
    m_owner->DoDeleteBearerRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapMme<C>::ModifyBearerResponse(ModifyBearerResponseMessage msg)
{
    m_owner->DoModifyBearerResponse(msg);
}

/**
 * Template for the implementation of the NrEpcS11SapSgw as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class NrMemberEpcS11SapSgw : public NrEpcS11SapSgw
{
  public:
    /**
     * Constructor
     *
     * @param owner the owner class
     */
    NrMemberEpcS11SapSgw(C* owner);

    // Delete default constructor to avoid misuse
    NrMemberEpcS11SapSgw() = delete;

    // inherited from NrEpcS11SapSgw
    void CreateSessionRequest(CreateSessionRequestMessage msg) override;
    void ModifyBearerRequest(ModifyBearerRequestMessage msg) override;
    void DeleteBearerCommand(DeleteBearerCommandMessage msg) override;
    void DeleteBearerResponse(DeleteBearerResponseMessage msg) override;

  private:
    C* m_owner; ///< owner class
};

template <class C>
NrMemberEpcS11SapSgw<C>::NrMemberEpcS11SapSgw(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::CreateSessionRequest(CreateSessionRequestMessage msg)
{
    m_owner->DoCreateSessionRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::ModifyBearerRequest(ModifyBearerRequestMessage msg)
{
    m_owner->DoModifyBearerRequest(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::DeleteBearerCommand(DeleteBearerCommandMessage msg)
{
    m_owner->DoDeleteBearerCommand(msg);
}

template <class C>
void
NrMemberEpcS11SapSgw<C>::DeleteBearerResponse(DeleteBearerResponseMessage msg)
{
    m_owner->DoDeleteBearerResponse(msg);
}

} // namespace ns3

#endif /* NR_EPC_S11_SAP_H */
