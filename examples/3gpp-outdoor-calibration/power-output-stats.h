// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef POWER_OUTPUT_STATS_H
#define POWER_OUTPUT_STATS_H

#include "ns3/nstime.h"
#include "ns3/sfnsf.h"
#include "ns3/spectrum-value.h"
#include "ns3/sqlite-output.h"

#include <vector>

namespace ns3
{

/**
 * @brief Class to collect and store the transmission power values obtained from a simulation
 *
 * The class is meant to store in a database the values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * @see SetDb
 * @see SavePower
 * @see EmptyCache
 */
class PowerOutputStats
{
  public:
    /**
     * @brief Constructor
     */
    PowerOutputStats();

    /**
     * @brief Install the output database.
     * @param db database pointer
     * @param tableName name of the table where the values will be stored
     *
     * The db pointer must be valid through all the lifespan of the class. The
     * method creates, if not exists, a table for storing the values. The table
     * will contain the following columns:
     *
     * - "Frame INTEGER NOT NULL, "
     * - "SubFrame INTEGER NOT NULL,"
     * - "Slot INTEGER NOT NULL,"
     * - "Rnti INTEGER NOT NULL,"
     * - "Imsi INTEGER NOT NULL,"
     * - "BwpId INTEGER NOT NULL,"
     * - "CellId INTEGER NOT NULL,"
     * - "txPsdSum DOUBLE NOT NULL,"
     * - "Seed INTEGER NOT NULL,"
     * - "Run INTEGER NOT NULL);"
     *
     * Please note that this method, if the db already contains a table with
     * the same name, also clean existing values that has the same
     * Seed/Run pair.
     */
    void SetDb(SQLiteOutput* db, const std::string& tableName = "power");

    /**
     * @brief Store power values
     * @param sfnSf Slot number
     * @param txPsd TxPsd
     * @param t Time for the transmission
     * @param rnti RNTI
     * @param imsi IMSI
     * @param bwpId BWP ID
     * @param cellId cell ID
     *
     * Please note that the values in txPsd will be summed before storing.
     */
    void SavePower(const SfnSf& sfnSf,
                   Ptr<const SpectrumValue> txPsd,
                   const Time& t,
                   uint16_t rnti,
                   uint64_t imsi,
                   uint16_t bwpId,
                   uint16_t cellId);

    /**
     * @brief Force the cache write to disk, emptying the cache itself.
     */
    void EmptyCache();

  private:
    struct PowerResultCache
    {
        uint16_t frame;
        uint8_t subFrame;
        uint16_t slot;
        uint16_t rnti;
        uint64_t imsi;
        uint16_t bwpId;
        uint16_t cellId;
        double txPowerRb;
        double txPowerTotal;
        uint32_t rbNumActive;
        uint32_t rbNumTotal;
    };

    static void DeleteWhere(SQLiteOutput* p, uint32_t seed, uint32_t run, const std::string& table);

    void WriteCache();

    SQLiteOutput* m_db;                         //!< DB pointer
    std::vector<PowerResultCache> m_powerCache; //!< Result cache
    std::string m_tableName;                    //!< Table name
};

} // namespace ns3

#endif // POWER_OUTPUT_STATS_H
