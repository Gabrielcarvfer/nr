// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#ifndef NR_EPC_X2_SAP_H
#define NR_EPC_X2_SAP_H

#include "nr-eps-bearer.h"

#include "ns3/ipv4-address.h"
#include "ns3/packet.h"

#include <bitset>

namespace ns3
{

class Node;

/**
 * The X2 SAP defines the service between the X2 entity and the RRC entity.
 *
 * The X2 SAP follows the specification 3GPP TS 36.423: "X2 application protocol (X2AP)"
 *
 * The service primitives corresponds to the X2AP procedures and messages and
 * the service parameters corresponds to the Information Elements
 *
 * Note: Any reference in this file refers to the 3GPP TS 36.423 specification
 */

/**
 * @brief Common structures for NrEpcX2SapProvider and NrEpcX2SapUser
 */
class NrEpcX2Sap
{
  public:
    virtual ~NrEpcX2Sap() = default;

    /**
     * E-RABs to be setup item as
     * it is used in the HANDOVER REQUEST message.
     * See section 9.1.1.1 for further info about the parameters
     */
    struct ErabToBeSetupItem
    {
        uint16_t erabId;                    ///< E-RAB ID
        NrEpsBearer erabLevelQosParameters; ///< E-RAB level QOS parameters
        bool dlForwarding;                  ///< DL forwarding
        Ipv4Address transportLayerAddress;  ///< transport layer address
        uint32_t gtpTeid;                   ///< TEID

        ErabToBeSetupItem()
            : erabLevelQosParameters(NrEpsBearer(NrEpsBearer::GBR_CONV_VOICE))
        {
        }
    };

    /**
     * E-RABs admitted item as
     * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
     * See section 9.1.1.2 for further info about the parameters
     */
    struct ErabAdmittedItem
    {
        uint16_t erabId;    ///< E-RAB ID
        uint32_t ulGtpTeid; ///< uplink GTP TEID
        uint32_t dlGtpTeid; ///< downlink GTP TEID
    };

    /**
     * E-RABs not admitted item as
     * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
     * See section 9.1.1.2 for further info about the parameters
     */
    struct ErabNotAdmittedItem
    {
        uint16_t erabId; ///< E-RAB ID
        uint16_t cause;  ///< cause
    };

    /**
     * E-RABs subject to status transfer item as
     * it is used in the SN STATUS TRANSFER message.
     * See section 9.1.1.4 for further info about the parameters
     */
    static const uint16_t m_maxPdcpSn = 4096;

    /// ErabsSubjectToStatusTransferItem structure
    struct ErabsSubjectToStatusTransferItem
    {
        uint16_t erabId;                                    ///< ERAB ID
        std::bitset<m_maxPdcpSn> receiveStatusOfUlPdcpSdus; ///< receive status of UL PDCP SDUs
        uint16_t ulPdcpSn;                                  ///< UL PDCP SN
        uint32_t ulHfn;                                     ///< UL HFN
        uint16_t dlPdcpSn;                                  ///< DL PDCP SN
        uint32_t dlHfn;                                     ///< DL HFN
    };

    /**
     * UL Interference OverloadIndication as
     * it is used in the LOAD INFORMATION message.
     * See section 9.2.17 for further info about the values
     */
    enum UlInterferenceOverloadIndicationItem
    {
        HighInterference,
        MediumInterference,
        LowInterference
    };

    /**
     * UL High Interference Information as
     * it is used in the LOAD INFORMATION message.
     * See section 9.1.2.1 for further info about the parameters
     */
    struct UlHighInterferenceInformationItem
    {
        uint16_t targetCellId; ///< target cell ID
        std::vector<bool>
            ulHighInterferenceIndicationList; ///< UL high interference indication list
    };

    /**
     * Relative Narrowband Tx Power (RNTP) as
     * it is used in the LOAD INFORMATION message.
     * See section 9.2.19 for further info about the parameters
     *
     * Note: You can use INT16_MIN value for -infinite value
     *       in the rntpThreshold field
     */
    struct RelativeNarrowbandTxBand
    {
        std::vector<bool> rntpPerPrbList;             ///< RNTP per prb list
        int16_t rntpThreshold{INT16_MIN};             ///< RNTP threshold
        uint16_t antennaPorts{UINT16_MAX};            ///< antenna ports
        uint16_t pB{UINT16_MAX};                      ///< PB
        uint16_t pdcchInterferenceImpact{UINT16_MAX}; ///< PDC channel interference list
    };

    /**
     * Cell Information Item as
     * it is used in the LOAD INFORMATION message.
     * See section 9.1.2.1 for further info about the parameters
     */
    struct CellInformationItem
    {
        uint16_t sourceCellId; ///< source cell ID
        std::vector<UlInterferenceOverloadIndicationItem>
            ulInterferenceOverloadIndicationList; ///< UL interference overload indication list
        std::vector<UlHighInterferenceInformationItem>
            ulHighInterferenceInformationList; ///< UL high interference information list
        RelativeNarrowbandTxBand relativeNarrowbandTxBand; ///< relative narrow transmit band
    };

    /**
     * Load Indicator as
     * it is used in the RESOURCE STATUS UPDATE message.
     * See section 9.2.36 for further info about the value
     */
    enum LoadIndicator
    {
        LowLoad,
        MediumLoad,
        HighLoad,
        Overload
    };

    /**
     * Composite Available Capacity as
     * it is used in the RESOURCE STATUS UPDATE message.
     * See section 9.2.45 for further info about the parameters
     */
    struct CompositeAvailCapacity
    {
        uint16_t cellCapacityClassValue; ///< cell capacity class value
        uint16_t capacityValue;          ///< capacity value
    };

    /**
     * Cell Measurement Result Item as
     * it is used in the RESOURCE STATUS UPDATE message.
     * See section 9.1.2.14 for further info about the parameters
     */
    struct CellMeasurementResultItem
    {
        uint16_t sourceCellId; ///< source cell id

        LoadIndicator dlHardwareLoadIndicator; ///< DL hardware load indicator
        LoadIndicator ulHardwareLoadIndicator; ///< UL hardware load indicator

        LoadIndicator dlS1TnlLoadIndicator; ///< DL S1 TNL load indicator
        LoadIndicator ulS1TnlLoadIndicator; ///< UL S1 TNL load indicator

        uint16_t dlGbrPrbUsage;    ///< DL GBR PRB usage
        uint16_t ulGbrPrbUsage;    ///< UL GBR PRB usage
        uint16_t dlNonGbrPrbUsage; ///< DL Non GBR PRB usage
        uint16_t ulNonGbrPrbUsage; ///< UL Non GBR PRB usage
        uint16_t dlTotalPrbUsage;  ///< DL Total PRB usage
        uint16_t ulTotalPrbUsage;  ///< UL Total PRB usage

        CompositeAvailCapacity dlCompositeAvailableCapacity; ///< DL composite available capacity
        CompositeAvailCapacity ulCompositeAvailableCapacity; ///< UL composite available capacity
    };

    /// Cause ID enumeration
    enum IdCause
    {
        HandoverDesirableForRadioReason,
        TimeCriticalHandover
    };

    /**
     * @brief Parameters of the HANDOVER REQUEST message.
     *
     * See section 9.1.1.1 for further info about the parameters
     */
    struct HandoverRequestParams
    {
        uint16_t oldGnbUeX2apId;                ///< old gNB UE X2 AP ID
        uint16_t cause;                         ///< cause
        uint16_t sourceCellId;                  ///< source cell ID
        uint16_t targetCellId;                  ///< target cell ID
        uint32_t mmeUeS1apId;                   ///< MME UE S1 AP ID
        uint64_t ueAggregateMaxBitRateDownlink; ///< UE aggregate max bit rate downlink
        uint64_t ueAggregateMaxBitRateUplink;   ///< UE aggregate max bit rate uplink
        std::vector<ErabToBeSetupItem> bearers; ///< bearers
        Ptr<Packet> rrcContext;                 ///< RRC context
    };

    /**
     * @brief Parameters of the HANDOVER REQUEST ACKNOWLEDGE message.
     *
     * See section 9.1.1.2 for further info about the parameters
     */
    struct HandoverRequestAckParams
    {
        uint16_t oldGnbUeX2apId;                             ///< old gNB UE X2 AP ID
        uint16_t newGnbUeX2apId;                             ///< new gNB UE X2 AP ID
        uint16_t sourceCellId;                               ///< source cell ID
        uint16_t targetCellId;                               ///< target cell ID
        std::vector<ErabAdmittedItem> admittedBearers;       ///< admitted bearers
        std::vector<ErabNotAdmittedItem> notAdmittedBearers; ///< not admitted bearers
        Ptr<Packet> rrcContext;                              ///< RRC context
    };

    /**
     * @brief Parameters of the HANDOVER PREPARATION FAILURE message.
     *
     * See section 9.1.1.3 for further info about the parameters
     */
    struct HandoverPreparationFailureParams
    {
        uint16_t oldGnbUeX2apId;         ///< old gNB UE X2 AP ID
        uint16_t sourceCellId;           ///< source cell ID
        uint16_t targetCellId;           ///< target cell ID
        uint16_t cause;                  ///< cause
        uint16_t criticalityDiagnostics; ///< criticality diagnostics
    };

    /**
     * @brief Parameters of the SN STATUS TRANSFER message.
     *
     * See section 9.1.1.4 for further info about the parameters
     */
    struct SnStatusTransferParams
    {
        uint16_t oldGnbUeX2apId; ///< old gNB UE X2 AP ID
        uint16_t newGnbUeX2apId; ///< new gNB UE X2 AP ID
        uint16_t sourceCellId;   ///< source cell ID
        uint16_t targetCellId;   ///< target cell ID
        std::vector<ErabsSubjectToStatusTransferItem>
            erabsSubjectToStatusTransferList; ///< ERABs subject to status transfer list
    };

    /**
     * @brief Parameters of the UE CONTEXT RELEASE message.
     *
     * See section 9.1.1.5 for further info about the parameters
     */
    struct UeContextReleaseParams
    {
        uint16_t oldGnbUeX2apId; ///< old gNB UE X2 AP ID
        uint16_t newGnbUeX2apId; ///< new gNB UE X2 AP ID
        uint16_t sourceCellId;   ///< source cell ID
        uint16_t targetCellId;   ///< target cell ID
    };

    /**
     * @brief Parameters of the LOAD INFORMATION message.
     *
     * See section 9.1.2.1 for further info about the parameters
     */
    struct LoadInformationParams
    {
        uint16_t targetCellId{UINT16_MAX};                    ///< target cell ID
        std::vector<CellInformationItem> cellInformationList; ///< cell information list
    };

    /**
     * @brief Parameters of the RESOURCE STATUS UPDATE message.
     *
     * See section 9.1.2.14 for further info about the parameters
     */
    struct ResourceStatusUpdateParams
    {
        uint16_t targetCellId;      ///< target cell ID
        uint16_t gnb1MeasurementId; ///< gNB1 measurement ID
        uint16_t gnb2MeasurementId; ///< gNB2 measurement ID
        std::vector<CellMeasurementResultItem>
            cellMeasurementResultList; ///< cell measurement result list
    };

    /**
     * @brief Parameters of the UE DATA primitive
     *
     * Forward UE data during the handover procedure from source gNB (sourceCellId)
     * to target gNB (targetCellId) using a GTP-U tunnel (gtpTeid)
     */
    struct UeDataParams
    {
        uint16_t sourceCellId; ///< source cell ID
        uint16_t targetCellId; ///< target cell ID
        uint32_t gtpTeid;      ///< GTP TEID
        Ptr<Packet> ueData;    ///< UE data
    };

    /**
     * @brief Parameters of the HANDOVER CANCEL message.
     *
     * See section 9.1.1.6 for further info about the parameters
     */
    struct HandoverCancelParams
    {
        uint16_t oldGnbUeX2apId; ///< old gNB UE X2 AP ID
        uint16_t newGnbUeX2apId; ///< new gNB UE X2 AP ID
        uint16_t sourceCellId;   ///< source cell ID
        uint16_t targetCellId;   ///< target cell ID
        uint16_t cause;          ///< cause
    };
};

/**
 * These service primitives of this part of the X2 SAP
 * are provided by the X2 entity and issued by RRC entity
 */
class NrEpcX2SapProvider : public NrEpcX2Sap
{
  public:
    //
    // Service primitives
    //

    /**
     * Send handover request function
     * @param params handover request parameters
     */
    virtual void SendHandoverRequest(HandoverRequestParams params) = 0;

    /**
     * Send handover request ack function
     * @param params the handover request ack parameters
     */
    virtual void SendHandoverRequestAck(HandoverRequestAckParams params) = 0;

    /**
     * Send handover preparation failure function
     * @param params the handover preparation failure
     */
    virtual void SendHandoverPreparationFailure(HandoverPreparationFailureParams params) = 0;

    /**
     * Send SN status transfer function
     * @param params the SN status transfer parameters
     */
    virtual void SendSnStatusTransfer(SnStatusTransferParams params) = 0;

    /**
     * Send UE context release function
     * @param params the UE context release parameters
     */
    virtual void SendUeContextRelease(UeContextReleaseParams params) = 0;

    /**
     * Send load information function
     * @param params the load information parameters
     */
    virtual void SendLoadInformation(LoadInformationParams params) = 0;

    /**
     * Send resource status update function
     * @param params the resource statue update parameters
     */
    virtual void SendResourceStatusUpdate(ResourceStatusUpdateParams params) = 0;

    /**
     * Send UE data function
     * @param params the UE data parameters
     */
    virtual void SendUeData(UeDataParams params) = 0;

    /**
     * @brief Send handover Cancel to the target gNB
     * @param params the handover cancel parameters
     */
    virtual void SendHandoverCancel(HandoverCancelParams params) = 0;
};

/**
 * These service primitives of this part of the X2 SAP
 * are provided by the RRC entity and issued by the X2 entity
 */
class NrEpcX2SapUser : public NrEpcX2Sap
{
  public:
    /*
     * Service primitives
     */

    /**
     * Receive handover request function
     * @param params the handover request parameters
     */
    virtual void RecvHandoverRequest(HandoverRequestParams params) = 0;

    /**
     * Receive handover request ack function
     * @param params the handover request ack parameters
     */
    virtual void RecvHandoverRequestAck(HandoverRequestAckParams params) = 0;

    /**
     * Receive handover preparation failure function
     * @param params the handover preparation failure parameters
     */
    virtual void RecvHandoverPreparationFailure(HandoverPreparationFailureParams params) = 0;

    /**
     * Receive SN status transfer function
     * @param params the SN status transfer parameters
     */
    virtual void RecvSnStatusTransfer(SnStatusTransferParams params) = 0;

    /**
     * Receive UE context release function
     * @param params the receive UE context release parameters
     */
    virtual void RecvUeContextRelease(UeContextReleaseParams params) = 0;

    /**
     * Receive load information function
     * @param params the load information parameters
     */
    virtual void RecvLoadInformation(LoadInformationParams params) = 0;

    /**
     * Receive resource status update function
     * @param params the resource status update parameters
     */
    virtual void RecvResourceStatusUpdate(ResourceStatusUpdateParams params) = 0;

    /**
     * Receive UE data function
     * @param params UE data parameters
     */
    virtual void RecvUeData(UeDataParams params) = 0;

    /**
     * Receive handover cancel function
     * @param params the receive handover cancel parameters
     *
     */
    virtual void RecvHandoverCancel(HandoverCancelParams params) = 0;
};

///////////////////////////////////////

/**
 * NrEpcX2SpecificEpcX2SapProvider
 */
template <class C>
class NrEpcX2SpecificEpcX2SapProvider : public NrEpcX2SapProvider
{
  public:
    /**
     * Constructor
     *
     * @param x2 the owner class
     */
    NrEpcX2SpecificEpcX2SapProvider(C* x2);

    // Delete default constructor to avoid misuse
    NrEpcX2SpecificEpcX2SapProvider() = delete;

    //
    // Interface implemented from NrEpcX2SapProvider
    //

    /**
     * Send handover request function
     * @param params the handover request parameters
     */
    void SendHandoverRequest(HandoverRequestParams params) override;

    /**
     * Send handover request ack function
     * @param params the handover request ack parameters
     */
    void SendHandoverRequestAck(HandoverRequestAckParams params) override;

    /**
     * Send handover preparation failure function
     * @param params the handover preparation failure parameters
     */
    void SendHandoverPreparationFailure(HandoverPreparationFailureParams params) override;

    /**
     * Send SN status transfer function
     * @param params the SN status transfer parameters
     */
    void SendSnStatusTransfer(SnStatusTransferParams params) override;

    /**
     * Send UE context release function
     * @param params the UE context release parameters
     */
    void SendUeContextRelease(UeContextReleaseParams params) override;

    /**
     * Send load information function
     * @param params the load information parameters
     */
    void SendLoadInformation(LoadInformationParams params) override;

    /**
     * Send resource status update function
     * @param params the resource status update parameters
     */
    void SendResourceStatusUpdate(ResourceStatusUpdateParams params) override;

    /**
     * Send UE data function
     * @param params the UE data parameters
     */
    void SendUeData(UeDataParams params) override;

    /**
     * @brief Send handover Cancel to the target gNB
     * @param params the handover cancel parameters
     */
    void SendHandoverCancel(HandoverCancelParams params) override;

  private:
    C* m_x2; ///< owner class
};

template <class C>
NrEpcX2SpecificEpcX2SapProvider<C>::NrEpcX2SpecificEpcX2SapProvider(C* x2)
    : m_x2(x2)
{
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendHandoverRequest(HandoverRequestParams params)
{
    m_x2->DoSendHandoverRequest(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendHandoverRequestAck(HandoverRequestAckParams params)
{
    m_x2->DoSendHandoverRequestAck(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendHandoverPreparationFailure(
    HandoverPreparationFailureParams params)
{
    m_x2->DoSendHandoverPreparationFailure(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendSnStatusTransfer(SnStatusTransferParams params)
{
    m_x2->DoSendSnStatusTransfer(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendUeContextRelease(UeContextReleaseParams params)
{
    m_x2->DoSendUeContextRelease(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendLoadInformation(LoadInformationParams params)
{
    m_x2->DoSendLoadInformation(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendResourceStatusUpdate(ResourceStatusUpdateParams params)
{
    m_x2->DoSendResourceStatusUpdate(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendUeData(UeDataParams params)
{
    m_x2->DoSendUeData(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapProvider<C>::SendHandoverCancel(HandoverCancelParams params)
{
    m_x2->DoSendHandoverCancel(params);
}

/**
 * NrEpcX2SpecificEpcX2SapUser
 */
template <class C>
class NrEpcX2SpecificEpcX2SapUser : public NrEpcX2SapUser
{
  public:
    /**
     * Constructor
     *
     * @param rrc RRC
     */
    NrEpcX2SpecificEpcX2SapUser(C* rrc);

    // Delete default constructor to avoid misuse
    NrEpcX2SpecificEpcX2SapUser() = delete;

    //
    // Interface implemented from NrEpcX2SapUser
    //

    /**
     * Receive handover request function
     * @param params the receive handover request parameters
     */
    void RecvHandoverRequest(HandoverRequestParams params) override;

    /**
     * Receive handover request ack function
     * @param params the receive handover request ack parameters
     */
    void RecvHandoverRequestAck(HandoverRequestAckParams params) override;

    /**
     * Receive handover preparation failure function
     * @param params the receive handover preparation failure parameters
     */
    void RecvHandoverPreparationFailure(HandoverPreparationFailureParams params) override;

    /**
     * Receive SN status transfer function
     * @param params the SN status transfer parameters
     */
    void RecvSnStatusTransfer(SnStatusTransferParams params) override;

    /**
     * Receive UE context release function
     * @param params the UE context release parameters
     */
    void RecvUeContextRelease(UeContextReleaseParams params) override;

    /**
     * Receive load information function
     * @param params the load information parameters
     */
    void RecvLoadInformation(LoadInformationParams params) override;

    /**
     * Receive resource status update function
     * @param params the receive resource status update
     */
    void RecvResourceStatusUpdate(ResourceStatusUpdateParams params) override;

    /**
     * Receive UE data function
     * @param params the UE data parameters
     */
    void RecvUeData(UeDataParams params) override;

    /**
     * Receive handover cancel function
     * @param params the receive handover cancel parameters
     *
     */
    void RecvHandoverCancel(HandoverCancelParams params) override;

  private:
    C* m_rrc; ///< owner class
};

template <class C>
NrEpcX2SpecificEpcX2SapUser<C>::NrEpcX2SpecificEpcX2SapUser(C* rrc)
    : m_rrc(rrc)
{
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvHandoverRequest(HandoverRequestParams params)
{
    m_rrc->DoRecvHandoverRequest(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvHandoverRequestAck(HandoverRequestAckParams params)
{
    m_rrc->DoRecvHandoverRequestAck(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvHandoverPreparationFailure(
    HandoverPreparationFailureParams params)
{
    m_rrc->DoRecvHandoverPreparationFailure(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvSnStatusTransfer(SnStatusTransferParams params)
{
    m_rrc->DoRecvSnStatusTransfer(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvUeContextRelease(UeContextReleaseParams params)
{
    m_rrc->DoRecvUeContextRelease(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvLoadInformation(LoadInformationParams params)
{
    m_rrc->DoRecvLoadInformation(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvResourceStatusUpdate(ResourceStatusUpdateParams params)
{
    m_rrc->DoRecvResourceStatusUpdate(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvUeData(UeDataParams params)
{
    m_rrc->DoRecvUeData(params);
}

template <class C>
void
NrEpcX2SpecificEpcX2SapUser<C>::RecvHandoverCancel(HandoverCancelParams params)
{
    m_rrc->DoRecvHandoverCancel(params);
}

} // namespace ns3

#endif // NR_EPC_X2_SAP_H
