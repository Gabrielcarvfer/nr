// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#ifndef NR_MAC_SAP_H
#define NR_MAC_SAP_H

#include "ns3/packet.h"

namespace ns3
{

/**
 * Service Access Point (SAP) offered by the MAC to the RLC
 * See Femto Forum MAC Scheduler Interface Specification v 1.11, Figure 1
 *
 * This is the MAC SAP Provider, i.e., the part of the SAP that contains the MAC methods called by
 * the RLC
 */
class NrMacSapProvider
{
  public:
    virtual ~NrMacSapProvider() = default;

    /**
     * Parameters for NrMacSapProvider::TransmitPdu
     *
     */
    struct TransmitPduParameters
    {
        Ptr<Packet> pdu; /**< the RLC PDU */
        uint16_t rnti;   /**< the C-RNTI identifying the UE */
        uint8_t lcid;    /**< the logical channel id corresponding to the sending RLC instance */
        uint8_t layer;   /**< the layer value that was passed by the MAC in the call to
                            NotifyTxOpportunity that generated this PDU */
        uint8_t harqProcessId; /**< the HARQ process id that was passed by the MAC in the call to
                                  NotifyTxOpportunity that generated this PDU */
        uint8_t componentCarrierId; /**< the component carrier id corresponding to the sending Mac
                                       instance */
    };

    /**
     * send an RLC PDU to the MAC for transmission. This method is to be
     * called as a response to NrMacSapUser::NotifyTxOpportunity
     *
     * @param params TransmitPduParameters
     */
    virtual void TransmitPdu(TransmitPduParameters params) = 0;

    /**
     * Parameters for NrMacSapProvider::BufferStatusReport
     */
    struct BufferStatusReportParameters
    {
        uint16_t rnti; /**< the C-RNTI identifying the UE */
        uint8_t lcid;  /**< the logical channel id corresponding to the sending RLC instance */
        uint32_t txQueueSize;     /**< the current size of the RLC transmission queue */
        uint16_t txQueueHolDelay; /**< the Head Of Line delay of the transmission queue */
        uint32_t retxQueueSize;   /**<  the current size of the RLC retransmission queue in bytes */
        uint16_t retxQueueHolDelay; /**<  the Head Of Line delay of the retransmission queue */
        uint16_t
            statusPduSize; /**< the current size of the pending STATUS RLC  PDU message in bytes */
        bool expBsrTimer;
    };

    /**
     * Report the RLC buffer status to the MAC
     *
     * @param params BufferStatusReportParameters
     */
    virtual void BufferStatusReport(BufferStatusReportParameters params) = 0;
};

/**
 * Service Access Point (SAP) offered by the MAC to the RLC
 * See Femto Forum MAC Scheduler Interface Specification v 1.11, Figure 1
 *
 * This is the MAC SAP User, i.e., the part of the SAP that contains the RLC methods called by the
 * MAC
 */
class NrMacSapUser
{
  public:
    virtual ~NrMacSapUser() = default;

    /**
     * Parameters for NrMacSapUser::NotifyTxOpportunity
     *
     */
    struct TxOpportunityParameters
    {
        /**
         * @brief TxOpportunityParameters constructor
         * @param bytes Bytes
         * @param layer Layer
         * @param harqId HarqID
         * @param ccId Component carrier ID
         * @param rnti RNTI
         * @param lcId Logical Channel ID
         */
        TxOpportunityParameters(uint32_t bytes,
                                uint8_t layer,
                                uint8_t harqId,
                                uint8_t ccId,
                                uint16_t rnti,
                                uint8_t lcId)
        {
            this->bytes = bytes;
            this->layer = layer;
            this->harqId = harqId;
            this->componentCarrierId = ccId;
            this->rnti = rnti;
            this->lcid = lcId;
        }

        /**
         * @brief TxOpportunityParameters default constructor (DEPRECATED)
         */
        TxOpportunityParameters()
        {
        }

        uint32_t bytes;             /**< the number of bytes to transmit */
        uint8_t layer;              /**<  the layer of transmission (MIMO) */
        uint8_t harqId;             /**< the HARQ ID */
        uint8_t componentCarrierId; /**< the component carrier id */
        uint16_t rnti;              /**< the C-RNTI identifying the UE */
        uint8_t lcid;               /**< the logical channel id */
    };

    /**
     * Called by the MAC to notify the RLC that the scheduler granted a
     * transmission opportunity to this RLC instance.
     *
     * @param params the TxOpportunityParameters
     */
    virtual void NotifyTxOpportunity(TxOpportunityParameters params) = 0;

    /**
     * Called by the MAC to notify the RLC that an HARQ process related
     * to this RLC instance has failed
     *
     * @todo eventual parameters to be defined
     */
    virtual void NotifyHarqDeliveryFailure() = 0;

    /**
     * Parameters for NrMacSapUser::ReceivePdu
     *
     */
    struct ReceivePduParameters
    {
        /**
         * @brief ReceivePduParameters default constructor (DEPRECATED)
         */
        ReceivePduParameters()
        {
        }

        /**
         * @brief ReceivePduParameters constructor
         * @param p Packet
         * @param rnti RNTI
         * @param lcid Logical Channel ID
         */
        ReceivePduParameters(const Ptr<Packet>& p, uint16_t rnti, uint8_t lcid)
        {
            this->p = p;
            this->rnti = rnti;
            this->lcid = lcid;
        }

        Ptr<Packet> p; /**< the RLC PDU to be received */
        uint16_t rnti; /**< the C-RNTI identifying the UE */
        uint8_t lcid;  /**< the logical channel id */
    };

    /**
     * Called by the MAC to notify the RLC of the reception of a new PDU
     *
     * @param params the ReceivePduParameters
     */
    virtual void ReceivePdu(ReceivePduParameters params) = 0;
};

/// GnbMacMemberNrMacSapProvider class
template <class C>
class GnbMacMemberNrMacSapProvider : public NrMacSapProvider
{
  public:
    /**
     * Constructor
     *
     * @param mac the MAC class
     */
    GnbMacMemberNrMacSapProvider(C* mac);

    // inherited from NrMacSapProvider
    void TransmitPdu(TransmitPduParameters params) override;
    void BufferStatusReport(BufferStatusReportParameters params) override;

  private:
    C* m_mac; ///< the MAC class
};

template <class C>
GnbMacMemberNrMacSapProvider<C>::GnbMacMemberNrMacSapProvider(C* mac)
    : m_mac(mac)
{
}

template <class C>
void
GnbMacMemberNrMacSapProvider<C>::TransmitPdu(TransmitPduParameters params)
{
    m_mac->DoTransmitPdu(params);
}

template <class C>
void
GnbMacMemberNrMacSapProvider<C>::BufferStatusReport(BufferStatusReportParameters params)
{
    m_mac->DoTransmitBufferStatusReport(params);
}

} // namespace ns3

#endif // NR_MAC_SAP_H
