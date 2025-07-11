// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef FLOW_MONITOR_OUTPUT_STATS_H
#define FLOW_MONITOR_OUTPUT_STATS_H

#include "ns3/flow-monitor-helper.h"
#include "ns3/sqlite-output.h"

namespace ns3
{

/**
 * @brief Class to store the flow monitor values obtained from a simulation
 *
 * The class is meant to store in a database the e2e values for
 * a simulation.
 *
 * @see SetDb
 * @see Save
 */
class FlowMonitorOutputStats
{
  public:
    /**
     * @brief FlowMonitorOutputStats constructor
     */
    FlowMonitorOutputStats();

    /**
     * @brief Install the output database.
     * @param db database pointer
     * @param tableName name of the table where the values will be stored
     *
     * The db pointer must be valid through all the lifespan of the class. The
     * method creates, if not exists, a table for storing the values. The table
     * will contain the following columns:
     *
     * - "FlowId INTEGER NOT NULL, "
     * - "TxPackets INTEGER NOT NULL,"
     * - "TxBytes INTEGER NOT NULL,"
     * - "TxOfferedMbps DOUBLE NOT NULL,"
     * - "RxBytes INTEGER NOT NULL,"
     * - "ThroughputMbps DOUBLE NOT NULL, "
     * - "MeanDelayMs DOUBLE NOT NULL, "
     * - "MeanJitterMs DOUBLE NOT NULL, "
     * - "RxPackets INTEGER NOT NULL, "
     * - "SEED INTEGER NOT NULL,"
     * - "RUN INTEGER NOT NULL,"
     * - "PRIMARY KEY(FlowId,SEED,RUN)"
     *
     * Please note that this method, if the db already contains a table with
     * the same name, also clean existing values that has the same
     * Seed/Run pair.
     */
    void SetDb(SQLiteOutput* db, const std::string& tableName);

    /**
     * @brief Store the flow monitor output in the database
     * @param monitor Flow Monitor
     * @param flowmonHelper Flow Monitor Helper
     * @param filename filename for a text output
     * @param addressesToConsider An optional parameter that contains the addresses
     * of the flows which will be saved into a database, others will be filtered out.
     */
    void Save(const Ptr<FlowMonitor>& monitor,
              FlowMonitorHelper& flowmonHelper,
              const std::string& filename,
              std::set<Ipv4Address> addressesToConsider = {});

  private:
    static void DeleteWhere(SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string& table);

    SQLiteOutput* m_db;
    std::string m_tableName;
};

} // namespace ns3

#endif // FLOW_MONITOR_OUTPUT_STATS_H
