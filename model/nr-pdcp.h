// Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#ifndef NR_PDCP_H
#define NR_PDCP_H

#include "nr-pdcp-sap.h"
#include "nr-rlc-sap.h"

#include "ns3/object.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

namespace ns3
{

/**
 * LTE PDCP entity, see 3GPP TS 36.323
 */
class NrPdcp : public Object // SimpleRefCount<NrPdcp>
{
    /// allow NrPdcpSpecificNrRlcSapUser class friend access
    friend class NrPdcpSpecificNrRlcSapUser;
    /// allow NrPdcpSpecificNrPdcpSapProvider<NrPdcp> class friend access
    friend class NrPdcpSpecificNrPdcpSapProvider<NrPdcp>;

  public:
    NrPdcp();
    ~NrPdcp() override;
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    void DoDispose() override;

    /**
     *
     *
     * @param rnti
     */
    void SetRnti(uint16_t rnti);

    /**
     *
     *
     * @param lcId
     */
    void SetLcId(uint8_t lcId);

    /**
     *
     *
     * @param s the PDCP SAP user to be used by this NR_PDCP
     */
    void SetNrPdcpSapUser(NrPdcpSapUser* s);

    /**
     *
     *
     * @return the PDCP SAP Provider interface offered to the RRC by this NR_PDCP
     */
    NrPdcpSapProvider* GetNrPdcpSapProvider();

    /**
     *
     *
     * @param s the RLC SAP Provider to be used by this NR_PDCP
     */
    void SetNrRlcSapProvider(NrRlcSapProvider* s);

    /**
     *
     *
     * @return the RLC SAP User interface offered to the RLC by this NR_PDCP
     */
    NrRlcSapUser* GetNrRlcSapUser();

    /// maximum PDCP SN
    static const uint16_t MAX_PDCP_SN = 4096;

    /**
     * Status variables of the PDCP
     */
    struct Status
    {
        uint16_t txSn; ///< TX sequence number
        uint16_t rxSn; ///< RX sequence number
    };

    /**
     *
     * @return the current status of the PDCP
     */
    Status GetStatus() const;

    /**
     * Set the status of the PDCP
     *
     * @param s
     */
    void SetStatus(Status s);

    /**
     * TracedCallback for PDU transmission event.
     *
     * @param [in] rnti The C-RNTI identifying the UE.
     * @param [in] lcid The logical channel id corresponding to
     *             the sending RLC instance.
     * @param [in] size Packet size.
     */
    typedef void (*PduTxTracedCallback)(uint16_t rnti, uint8_t lcid, uint32_t size);

    /**
     * TracedCallback signature for PDU receive event.
     *
     * @param [in] rnti The C-RNTI identifying the UE.
     * @param [in] lcid The logical channel id corresponding to
     *             the sending RLC instance.
     * @param [in] size Packet size.
     * @param [in] delay Delay since packet sent, in ns..
     */
    typedef void (*PduRxTracedCallback)(const uint16_t rnti,
                                        const uint8_t lcid,
                                        const uint32_t size,
                                        const uint64_t delay);

  protected:
    /**
     * Interface provided to upper RRC entity
     *
     * @param params the TransmitPdcpSduParameters
     */
    virtual void DoTransmitPdcpSdu(NrPdcpSapProvider::TransmitPdcpSduParameters params);

    NrPdcpSapUser* m_pdcpSapUser;         ///< PDCP SAP user
    NrPdcpSapProvider* m_pdcpSapProvider; ///< PDCP SAP provider

    /**
     * Interface provided to lower RLC entity
     *
     * @param p packet
     */
    virtual void DoReceivePdu(Ptr<Packet> p);

    NrRlcSapUser* m_rlcSapUser;         ///< RLC SAP user
    NrRlcSapProvider* m_rlcSapProvider; ///< RLC SAP provider

    uint16_t m_rnti; ///< RNTI
    uint8_t m_lcid;  ///< LCID

    /**
     * Used to inform of a PDU delivery to the RLC SAP provider.
     * The parameters are RNTI, LCID and bytes delivered
     */
    TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
    /**
     * Used to inform of a PDU reception from the RLC SAP user.
     * The parameters are RNTI, LCID, bytes delivered and delivery delay in nanoseconds.
     */
    TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

  private:
    /**
     * State variables. See section 7.1 in TS 36.323
     */
    uint16_t m_txSequenceNumber;
    /**
     * State variables. See section 7.1 in TS 36.323
     */
    uint16_t m_rxSequenceNumber;

    /**
     * Constants. See section 7.2 in TS 36.323
     */
    static const uint16_t m_maxPdcpSn = 4095;
};

} // namespace ns3

#endif // NR_PDCP_H
