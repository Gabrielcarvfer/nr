// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Jaume Nin <jnin@cttc.es>
// modified by: Marco Miozzo <mmiozzo@cttc.es>
//        Convert NrMacStatsCalculator in NrPhyTxStatsCalculator

#ifndef NR_PHY_TX_STATS_CALCULATOR_H_
#define NR_PHY_TX_STATS_CALCULATOR_H_

#include "nr-stats-calculator.h"

#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include <ns3/nr-common.h>

#include <fstream>
#include <string>

namespace ns3
{

/**
 * \ingroup nr
 *
 * Takes care of storing the information generated at PHY layer regarding
 * transmission. Metrics saved are:
 *
 *   - Timestamp (in seconds)
 *   - Frame index
 *   - Subframe index
 *   - C-RNTI
 *   - MCS for transport block 1
 *   - Size of transport block 1
 *   - MCS for transport block 2 (0 if not used)
 *   - Size of transport block 2 (0 if not used)
 */
class NrPhyTxStatsCalculator : public NrStatsCalculator
{
  public:
    /**
     * Constructor
     */
    NrPhyTxStatsCalculator();

    /**
     * Destructor
     */
    ~NrPhyTxStatsCalculator() override;

    // Inherited from ns3::Object
    /**
     * Register this type.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Set the name of the file where the UL Tx PHY statistics will be stored.
     *
     * @param outputFilename string with the name of the file
     */
    void SetUlTxOutputFilename(std::string outputFilename);

    /**
     * Get the name of the file where the UL RX PHY statistics will be stored.
     * @return the name of the file where the UL RX PHY statistics will be stored
     */
    std::string GetUlTxOutputFilename();

    /**
     * Set the name of the file where the DL TX PHY statistics will be stored.
     *
     * @param outputFilename string with the name of the file
     */
    void SetDlTxOutputFilename(std::string outputFilename);

    /**
     * Get the name of the file where the DL TX PHY statistics will be stored.
     * @return the name of the file where the DL TX PHY statistics will be stored
     */
    std::string GetDlTxOutputFilename();

    /**
     * Notifies the stats calculator that an downlink transmission has occurred.
     * @param params Trace information regarding PHY transmission stats
     */
    void DlPhyTransmission(nr::PhyTransmissionStatParameters params);

    /**
     * Notifies the stats calculator that an uplink transmission has occurred.
     * @param params Trace information regarding PHY transmission stats
     */
    void UlPhyTransmission(nr::PhyTransmissionStatParameters params);

    /**
     * trace sink
     *
     * \param phyTxStats
     * \param path
     * \param params
     */
    static void DlPhyTransmissionCallback(Ptr<NrPhyTxStatsCalculator> phyTxStats,
                                          std::string path,
                                          nr::PhyTransmissionStatParameters params);

    /**
     * trace sink
     *
     * \param phyTxStats
     * \param path
     * \param params
     */
    static void UlPhyTransmissionCallback(Ptr<NrPhyTxStatsCalculator> phyTxStats,
                                          std::string path,
                                          nr::PhyTransmissionStatParameters params);

  private:
    /**
     * When writing DL TX PHY statistics first time to file,
     * columns description is added. Then next lines are
     * appended to file. This value is true if output
     * files have not been opened yet
     */
    bool m_dlTxFirstWrite;

    /**
     * When writing UL TX PHY statistics first time to file,
     * columns description is added. Then next lines are
     * appended to file. This value is true if output
     * files have not been opened yet
     */
    bool m_ulTxFirstWrite;

    /**
     * DL TX PHY statistics output trace file
     */
    std::ofstream m_dlTxOutFile;

    /**
     * UL TX PHY statistics output trace file
     */
    std::ofstream m_ulTxOutFile;
};

} // namespace ns3

#endif /* NR_PHY_TX_STATS_CALCULATOR_H_ */
