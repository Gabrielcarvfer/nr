// Copyright (c) 2022 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_RADIO_BEARER_STATS_SIMPLE_H_
#define NR_RADIO_BEARER_STATS_SIMPLE_H_

#include "nr-stats-calculator.h"

#include "ns3/basic-data-calculators.h"
#include "ns3/nr-common.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"

#include <fstream>
#include <string>

namespace ns3
{

/*
 * @ingroup helpers
 *
 * Defines the minimum set of functions that RLC or PDC stats class should implement.
 * See also NrBearerStatsSimple and NrBearerStatsSimple.
 */
class NrBearerStatsBase : public Object
{
  public:
    // Inherited from ns3::Object
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();
    void DoDispose() override;
    /**
     * Notifies the stats calculator that an UL transmission has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who transmitted the PDU
     * @param rnti C-RNTI of the UE who transmitted the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     */
    virtual void UlTxPdu(uint16_t cellId,
                         uint64_t imsi,
                         uint16_t rnti,
                         uint8_t lcid,
                         uint32_t packetSize) = 0;
    /**
     * Notifies the stats calculator that an UL reception has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who transmitted the PDU
     * @param rnti C-RNTI of the UE who transmitted the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     * @param delay RLC to RLC delay in nanoseconds
     */
    virtual void UlRxPdu(uint16_t cellId,
                         uint64_t imsi,
                         uint16_t rnti,
                         uint8_t lcid,
                         uint32_t packetSize,
                         uint64_t delay) = 0;
    /**
     * Notifies the stats calculator that an DL transmission has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who is receiving the PDU
     * @param rnti C-RNTI of the UE who is receiving the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     */
    virtual void DlTxPdu(uint16_t cellId,
                         uint64_t imsi,
                         uint16_t rnti,
                         uint8_t lcid,
                         uint32_t packetSize) = 0;
    /**
     * Notifies the stats calculator that an downlink reception has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who received the PDU
     * @param rnti C-RNTI of the UE who received the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     * @param delay RLC to RLC delay in nanoseconds
     */
    virtual void DlRxPdu(uint16_t cellId,
                         uint64_t imsi,
                         uint16_t rnti,
                         uint8_t lcid,
                         uint32_t packetSize,
                         uint64_t delay) = 0;
};

/**
 * @ingroup helpers
 *
 * @brief
 *
 * This class writes RLC or PDC statistics to separate files:
 *   - DL TX statistics
 *   - DL RX statistics
 *   - UL TX statistics
 *   - UL RX statistics
 */
class NrBearerStatsSimple : public NrBearerStatsBase
{
  public:
    /**
     * Class constructor
     */
    NrBearerStatsSimple();

    /**
     * Class constructor
     */
    NrBearerStatsSimple(std::string protocolType);

    /**
     * Class destructor
     */
    ~NrBearerStatsSimple() override;

    // Inherited from ns3::Object
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();
    void DoDispose() override;

    /**
     * Get the name of the file where the UL TX (RLC or PDCP) statistics will be stored.
     * @return the name of the file where the uplink statistics will be stored
     */
    std::string GetUlTxOutputFilename();

    /**
     * Get the name of the file where the UL RX (RLC or PDCP) statistics will be stored.
     * @return the name of the file where the uplink statistics will be stored
     */
    std::string GetUlRxOutputFilename();

    /**
     * Get the name of the file where the DL TX (RLC or PDCP) statistics will be stored.
     * @return the name of the file where the downlink statistics will be stored
     */
    std::string GetDlTxOutputFilename();

    /**
     * Get the name of the file where the DL RX (RLC or PDCP) statistics will be stored.
     * @return the name of the file where the downlink statistics will be stored
     */
    std::string GetDlRxOutputFilename();

    /**
     * Notifies the stats calculator that an uplink transmission has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who transmitted the PDU
     * @param rnti C-RNTI of the UE who transmitted the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     */
    void UlTxPdu(uint16_t cellId,
                 uint64_t imsi,
                 uint16_t rnti,
                 uint8_t lcid,
                 uint32_t packetSize) override;

    /**
     * Notifies the stats calculator that an uplink reception has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who received the PDU
     * @param rnti C-RNTI of the UE who received the PDU
     * @param lcid LCID through which the PDU has been received
     * @param packetSize size of the PDU in bytes
     * @param delay RLC to RLC delay in nanoseconds
     */
    void UlRxPdu(uint16_t cellId,
                 uint64_t imsi,
                 uint16_t rnti,
                 uint8_t lcid,
                 uint32_t packetSize,
                 uint64_t delay) override;

    /**
     * Notifies the stats calculator that an downlink transmission has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who is receiving the PDU
     * @param rnti C-RNTI of the UE who is receiving the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     */
    void DlTxPdu(uint16_t cellId,
                 uint64_t imsi,
                 uint16_t rnti,
                 uint8_t lcid,
                 uint32_t packetSize) override;

    /**
     * Notifies the stats calculator that an downlink reception has occurred.
     * @param cellId CellId of the attached Gnb
     * @param imsi IMSI of the UE who received the PDU
     * @param rnti C-RNTI of the UE who received the PDU
     * @param lcid LCID through which the PDU has been transmitted
     * @param packetSize size of the PDU in bytes
     * @param delay RLC to RLC delay in nanoseconds
     */
    void DlRxPdu(uint16_t cellId,
                 uint64_t imsi,
                 uint16_t rnti,
                 uint8_t lcid,
                 uint32_t packetSize,
                 uint64_t delay) override;

  private:
    /**
     * Protocol type, by default RLC
     */
    std::string m_protocolType;
    std::string m_dlRlcTxOutputFilename;  //!< Output file name for DL RLC TX traces
    std::string m_dlRlcRxOutputFilename;  //!< Output file name for DL RLC RX traces
    std::string m_ulRlcTxOutputFilename;  //!< Output file name for UL RLC TX traces
    std::string m_ulRlcRxOutputFilename;  //!< Output file name for UL RLC RX traces
    std::string m_dlPdcpTxOutputFilename; //!< Output file name for UL PDCP TX traces
    std::string m_dlPdcpRxOutputFilename; //!< Output file name for UL PDCP RX traces
    std::string m_ulPdcpTxOutputFilename; //!< Output file name for UL PDCP RX traces
    std::string m_ulPdcpRxOutputFilename; //!< Output file name for UL PDCP RX traces
    std::ofstream m_dlTxOutFile; //!< Output file stream to which DL RLC TX stats will be written
    std::ofstream m_dlRxOutFile; //!< Output file stream to which DL RLC RX stats will be written
    std::ofstream m_ulTxOutFile; //!< Output file stream to which UL RLC TX stats will be written
    std::ofstream m_ulRxOutFile; //!< Output file stream to which UL RLC RX stats will be written
};

} // namespace ns3

#endif /* NR_RADIO_BEARER_STATS_SIMPLE_H_ */
