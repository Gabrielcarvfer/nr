// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Jaume Nin <jnin@cttc.es>
// Modified by: Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
//              Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)

#ifndef NR_MAC_STATS_CALCULATOR_H_
#define NR_MAC_STATS_CALCULATOR_H_

#include "nr-stats-calculator.h"

#include "ns3/nr-common.h"
#include "ns3/nr-gnb-mac.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"

#include <fstream>
#include <string>

namespace ns3
{

/**
 * \ingroup nr
 *
 * Takes care of storing the information generated at MAC layer. Metrics saved are:
 *   - Timestamp (in seconds)
 *   - Frame index
 *   - Subframe index
 *   - C-RNTI
 *   - MCS for transport block 1
 *   - Size of transport block 1
 *   - MCS for transport block 2 (0 if not used)
 *   - Size of transport block 2 (0 if not used)
 */
class NrMacStatsCalculator : public NrStatsCalculator
{
  public:
    /**
     * Constructor
     */
    NrMacStatsCalculator();

    /**
     * Destructor
     */
    ~NrMacStatsCalculator() override;

    // Inherited from ns3::Object
    /**
     * Register this type.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Set the name of the file where the uplink statistics will be stored.
     *
     * \param outputFilename string with the name of the file
     */
    void SetUlOutputFilename(std::string outputFilename);

    /**
     * Get the name of the file where the uplink statistics will be stored.
     * @return the name of the file where the uplink statistics will be stored
     */
    std::string GetUlOutputFilename();

    /**
     * Set the name of the file where the downlink statistics will be stored.
     *
     * @param outputFilename string with the name of the file
     */
    void SetDlOutputFilename(std::string outputFilename);

    /**
     * Get the name of the file where the downlink statistics will be stored.
     * @return the name of the file where the downlink statistics will be stored
     */
    std::string GetDlOutputFilename();

    /**
     * Notifies the stats calculator that an downlink scheduling has occurred.
     * @param cellId Cell ID of the attached Gnb
     * @param imsi IMSI of the scheduled UE
     * @param dlSchedulingCallbackInfo the structure that contains downlink scheduling fields:
     * frameNo Frame number
     * subframeNo Subframe number
     * rnti C-RNTI scheduled
     * mcsTb1 MCS for transport block 1
     * sizeTb1 Size of transport block 1
     * mcsTb2 MCS for transport block 2 (0 if not used)
     * sizeTb2 Size of transport block 2 (0 if not used)
     * componentCarrierId component carrier ID
     */

    void DlScheduling(uint16_t cellId,
                      uint64_t imsi,
                      nr::DlSchedulingCallbackInfo dlSchedulingCallbackInfo);

    /**
     * Notifies the stats calculator that an uplink scheduling has occurred.
     * @param cellId Cell ID of the attached Gnb
     * @param imsi IMSI of the scheduled UE
     * @param frameNo Frame number
     * @param subframeNo Subframe number
     * @param rnti C-RNTI scheduled
     * @param mcsTb MCS for transport block
     * @param sizeTb Size of transport block
     * @param componentCarrierId component carrier ID
     */
    void UlScheduling(uint16_t cellId,
                      uint64_t imsi,
                      uint32_t frameNo,
                      uint32_t subframeNo,
                      uint16_t rnti,
                      uint8_t mcsTb,
                      uint16_t sizeTb,
                      uint8_t componentCarrierId);

    /**
     * Trace sink for the ns3::NrGnbMac::DlScheduling trace source
     *
     * \param macStats
     * \param path
     * \param dlSchedulingCallbackInfo nr::DlSchedulingCallbackInfo structure containing all
     * downlink information that is generated what DlScheduling traces is fired
     */
    static void DlSchedulingCallback(Ptr<NrMacStatsCalculator> macStats,
                                     std::string path,
                                     nr::DlSchedulingCallbackInfo dlSchedulingCallbackInfo);

    /**
     * Trace sink for the ns3::NrGnbMac::UlScheduling trace source
     *
     * \param macStats
     * \param path
     * \param frameNo
     * \param subframeNo
     * \param rnti
     * \param mcs
     * \param size
     * \param componentCarrierId
     */
    static void UlSchedulingCallback(Ptr<NrMacStatsCalculator> macStats,
                                     std::string path,
                                     uint32_t frameNo,
                                     uint32_t subframeNo,
                                     uint16_t rnti,
                                     uint8_t mcs,
                                     uint16_t size,
                                     uint8_t componentCarrierId);

  private:
    /**
     * When writing DL MAC statistics first time to file,
     * columns description is added. Then next lines are
     * appended to file. This value is true if output
     * files have not been opened yet
     */
    bool m_dlFirstWrite;

    /**
     * When writing UL MAC statistics first time to file,
     * columns description is added. Then next lines are
     * appended to file. This value is true if output
     * files have not been opened yet
     */
    bool m_ulFirstWrite;

    /**
     * Downlink output trace file
     */
    std::ofstream m_dlOutFile;

    /**
     * Uplink output trace file
     */
    std::ofstream m_ulOutFile;
};

} // namespace ns3

#endif /* NR_MAC_STATS_CALCULATOR_H_ */
