/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <ns3/core-module.h>

#include "v2x-kpi.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("V2xKpi");

V2xKpi::V2xKpi ()
{}

V2xKpi::~V2xKpi ()
{
  sqlite3_close (m_db);
  m_db = nullptr;
}

void
V2xKpi::DeleteWhere (uint32_t seed, uint32_t run, const std::string &table)
{
  int rc;
  sqlite3_stmt *stmt;
  std::string cmd = "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;";
  rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Could not prepare correctly the delete statement. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, seed) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, run) == SQLITE_OK);

  rc = sqlite3_step (stmt);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the finalize statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::SetDbPath (std::string dbPath)
{
  m_dbPath = dbPath + ".db";
}

void
V2xKpi::SetTxAppDuration (double duration)
{
  m_txAppDuration = duration;
}

void
V2xKpi::WriteKpis ()
{
  SavePktTxData ();
  SavePktRxData ();
  SaveAvrgPir ();
  SaveThput ();
  ComputePsschTxStats ();
  ComputePsschTbCorruptionStats ();
}

void
V2xKpi::ConsiderAllTx (bool allTx)
{
  m_considerAllTx = allTx;
}

void
V2xKpi::SavePktRxData ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM pktTxRx WHERE txRx = 'rx' AND txRx IS NOT NULL AND SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      uint32_t nodeId = sqlite3_column_int (stmt, 2);
      auto nodeIt = m_rxDataMap.find (nodeId);
      if (nodeIt == m_rxDataMap.end ())
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " not found creating new entry");
          std::map <std::string, std::vector <PktTxRxData> > secondMap;
          std::string srcIp = std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5)));
          std::vector <PktTxRxData> data;
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))));
          data.push_back (result);
          secondMap.insert (std::make_pair (srcIp, data));
          m_rxDataMap.insert (std::make_pair (nodeId, secondMap));
        }
      else
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " found");
          std::string srcIp = std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5)));
          auto txIt = nodeIt->second.find (srcIp);
          if (txIt == nodeIt->second.end ())
            {
              //NS_LOG_UNCOND ("srcIp = " << srcIp << " not found creating new entry");
              std::vector <PktTxRxData> data;
              PktTxRxData result (sqlite3_column_double (stmt, 0),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                                  sqlite3_column_int (stmt, 2),
                                  sqlite3_column_int (stmt, 3),
                                  sqlite3_column_int (stmt, 4),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))));
              data.push_back (result);
              nodeIt->second.insert (std::make_pair (srcIp, data));
            }
          else
            {
              //NS_LOG_UNCOND ("srcIp = " << srcIp << " found");
              PktTxRxData result (sqlite3_column_double (stmt, 0),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                                  sqlite3_column_int (stmt, 2),
                                  sqlite3_column_int (stmt, 3),
                                  sqlite3_column_int (stmt, 4),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))));
              txIt->second.push_back (result);
            }
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::SaveAvrgPir ()
{
  std::string tableName = "avrgPir";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "srcIp TEXT NOT NULL,"
                      "dstIp TEXT NOT NULL,"
                      "avrgPirSec DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);

  for (const auto &it:m_rxDataMap)
    {
      for (const auto &it2:it.second)
        {
          double avrgPir = ComputeAvrgPir (it2.second);
          if (avrgPir == -1.0)
            {
              //It may happen that a node would rxed only one pkt from a
              //particular tx node. In that case, PIR can not be computed.
              //Therefore, we do not log the PIR.
              continue;
            }
          //NS_LOG_UNCOND ("Avrg PIR " << avrgPir);
          PktTxRxData data = it2.second.at (0);
          sqlite3_stmt *stmt;
          std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
          rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4,it2.first.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 5, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 6, avrgPir) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, RngSeedManager::GetSeed ()) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 8, RngSeedManager::GetRun ()) == SQLITE_OK);

          rc = sqlite3_step (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
          rc = sqlite3_finalize (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
        }
    }
}
double
V2xKpi::ComputeAvrgPir (std::vector <PktTxRxData> data)
{
  uint64_t pirCounter = 0;
  double lastPktRxTime = 0.0;
  double pir = 0.0;

  //NS_LOG_UNCOND ("Packet Vector size " << data.size () << " to compute average PIR");

  for (const auto &it:data)
    {
      if (pirCounter == 0 && lastPktRxTime == 0.0)
        {
          //this is the first packet, just store the time and get out
          lastPktRxTime = it.time;
          continue;
        }

      // NS_LOG_UNCOND ("it.time - lastPktRxTime " << it.time - lastPktRxTime);
      pir = pir + (it.time - lastPktRxTime);
      lastPktRxTime = it.time;
      pirCounter++;
    }
  double avrgPir = 0.0;
  if (pirCounter == 0)
    {
      //It may happen that a node would rxed only one pkt from a
      //particular tx node. In that case, PIR can not be computed.
      avrgPir = -1.0;
    }
  else
    {
      avrgPir = pir / pirCounter;
    }
  return avrgPir;
}

void
V2xKpi::SaveThput ()
{
  std::string tableName = "thput";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "srcIp TEXT NOT NULL,"
                      "totalPktTxed int NOT NULL,"
                      "dstIp TEXT NOT NULL,"
                      "totalPktRxed int NOT NULL,"
                      "thputKbps DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);

  for (const auto &it:m_rxDataMap)
    {
      for (const auto &it2:it.second)
        {
          double thput = ComputeThput (it2.second);
          //NS_LOG_UNCOND ("thput " << thput << " kbps");
          PktTxRxData data = it2.second.at (0);
          sqlite3_stmt *stmt;
          std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
          rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4, it2.first.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, GetTotalTxPkts (it2.first)) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, static_cast <uint64_t> (it2.second.size ())) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 8, thput) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetSeed ()) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 10, RngSeedManager::GetRun ()) == SQLITE_OK);

          rc = sqlite3_step (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
          rc = sqlite3_finalize (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
        }

      //if (report stats for all TX AND total number of tx from whom the receiver rxed the packets is less the total Tx - 1)
      NS_LOG_DEBUG ("m_considerAllTx flag value" << m_considerAllTx);
      NS_LOG_DEBUG ("Num of transmitters this receiver able to rxed data " << it.second.size ());
      NS_LOG_DEBUG ("Total number of transmitters " << m_txDataMap.size ());
      if (m_considerAllTx && (it.second.size () < (m_txDataMap.size () - 1)))
        {
          for (const auto &itTx:m_txDataMap)
            {
              if (it.second.find (itTx.second.at (0).ipAddrs) == it.second.end ())
                {
                  //we didnt find the TX in our m_rxDataMap. Lets read the first
                  //entry of our m_rxDataMap just to read some info of the RX
                  //node.
                  PktTxRxData data = it.second.begin ()->second.at (0);
                  //avoid my own IP
                  if (itTx.second.at (0).ipAddrs != data.ipAddrs)
                    {
                      sqlite3_stmt *stmt;
                      std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
                      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4, itTx.second.at (0).ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, GetTotalTxPkts (itTx.second.at (0).ipAddrs)) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, 0) == SQLITE_OK); // zero rxed pkets
                      NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 8, 0) == SQLITE_OK); // zero thput
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetSeed ()) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 10, RngSeedManager::GetRun ()) == SQLITE_OK);

                      rc = sqlite3_step (stmt);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
                      rc = sqlite3_finalize (stmt);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
                    }
                }
            }
        }
    }
}

double
V2xKpi::ComputeThput (std::vector <PktTxRxData> data)
{
  NS_ABORT_MSG_IF (m_txAppDuration == 0.0, "Can not compute throughput with " << m_txAppDuration << " duration");
  uint64_t rxByteCounter = 0;

  //NS_LOG_UNCOND ("Packet Vector size " << data.size () << " to throughput");
  for (const auto &it:data)
    {
      rxByteCounter += it.pktSize;
    }

  //thput in kpbs
  double thput = (rxByteCounter * 8) / m_txAppDuration / 1000.0;
  return thput;
}

void
V2xKpi::SavePktTxData ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM pktTxRx WHERE txRx = 'tx' AND txRx IS NOT NULL AND SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      uint32_t nodeId = sqlite3_column_int (stmt, 2);
      auto nodeIt = m_txDataMap.find (nodeId);
      if (nodeIt == m_txDataMap.end ())
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " not found creating new entry");
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5))));
          std::vector <PktTxRxData> data;
          data.push_back (result);
          m_txDataMap.insert (std::make_pair (nodeId, data));
        }
      else
        {
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5))));
          nodeIt->second.push_back (result);
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

uint64_t
V2xKpi::GetTotalTxPkts (std::string srcIpAddrs)
{
  uint64_t totalTxPkts = 0;
  for (const auto &it:m_txDataMap)
    {
      if (it.second.at (0).ipAddrs == srcIpAddrs)
        {
          totalTxPkts = it.second.size ();
          break;
        }
    }

  return totalTxPkts;
}

void
V2xKpi::ComputePsschTxStats ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM psschTxUeMac WHERE SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);
  uint32_t rowCount = 0;
  std::vector <PsschTxData> nonOverLapPsschTx;
  std::vector <PsschTxData> overLapPsschTx;
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      rowCount++;
      PsschTxData result (sqlite3_column_int (stmt, 5),
                          sqlite3_column_int (stmt, 6),
                          sqlite3_column_int (stmt, 7),
                          sqlite3_column_int (stmt, 8),
                          sqlite3_column_int (stmt, 9),
                          sqlite3_column_int (stmt, 11),
                          sqlite3_column_int (stmt, 12));

      if (nonOverLapPsschTx.size () == 0 && nonOverLapPsschTx.size () == 0)
        {
          nonOverLapPsschTx.push_back (result);
        }
      else
        {
          auto it = std::find (nonOverLapPsschTx.begin (), nonOverLapPsschTx.end (), result);
          if (it != nonOverLapPsschTx.end ())
            {
              overLapPsschTx.push_back (result);
              overLapPsschTx.push_back (*it);
              nonOverLapPsschTx.erase (it);
            }
          else
            {
              auto it = std::find (overLapPsschTx.begin (), overLapPsschTx.end (), result);
              if (it != overLapPsschTx.end ())
                {
                  overLapPsschTx.push_back (result);
                }
              else
                {
                  nonOverLapPsschTx.push_back (result);
                }
            }
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));


  //NS_LOG_UNCOND ("Non-overlapping Tx " << nonOverLapPsschTx.size ());
  //NS_LOG_UNCOND ("overlapping Tx " << overLapPsschTx.size ());
  //NS_LOG_UNCOND ("Total rows " << rowCount);
  SaveSimultPsschTxStats (rowCount, nonOverLapPsschTx.size (), overLapPsschTx.size ());
}

void
V2xKpi::SaveSimultPsschTxStats (uint32_t totalPsschTx, uint32_t numNonOverLapPsschTx, uint32_t numOverLapPsschTx)
{
  std::string tableName = "simulPsschTx";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "totalTx INTEGER NOT NULL,"
                      "numNonOverlapping INTEGER NOT NULL,"
                      "numOverlapping INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);


  sqlite3_stmt *stmt;
  std::string sql = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?);";
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 1, totalPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 2, numNonOverLapPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 3, numOverLapPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 4, RngSeedManager::GetSeed ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 5, RngSeedManager::GetRun ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_step (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::ComputePsschTbCorruptionStats ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM psschRxUePhy WHERE SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);
  uint32_t rowCount = 0;
  uint32_t psschSuccessCount = 0;
  uint32_t sci2SuccessCount = 0;


  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      rowCount++;
      uint8_t psschcorrupt = sqlite3_column_int (stmt, 21);
      uint8_t sci2corrupt = sqlite3_column_int (stmt, 23);

      if (!psschcorrupt)
        {
          ++psschSuccessCount;
        }

      if (!sci2corrupt)
        {
          ++sci2SuccessCount;
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));


  //NS_LOG_UNCOND ("psschSuccessCount " << psschSuccessCount);
  //NS_LOG_UNCOND ("sci2SuccessCount " << sci2SuccessCount);
  //NS_LOG_UNCOND ("Total rows " << rowCount);
  SavePsschTbCorruptionStats (rowCount, psschSuccessCount, sci2SuccessCount);
}

void
V2xKpi::SavePsschTbCorruptionStats (uint32_t totalTbRx, uint32_t psschSuccessCount, uint32_t sci2SuccessCount)
{
  std::string tableName = "PsschTbRx";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "totalRx INTEGER NOT NULL,"
                      "psschSuccessCount INTEGER NOT NULL,"
                      "psschFailCount INTEGER NOT NULL,"
                      "sci2SuccessCount INTEGER NOT NULL,"
                      "sci2FailCount INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);


  sqlite3_stmt *stmt;
  std::string sql = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?);";
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 1, totalTbRx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 2, psschSuccessCount) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 3, (totalTbRx - psschSuccessCount)) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 4, sci2SuccessCount) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 5, (totalTbRx - sci2SuccessCount)) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 6, RngSeedManager::GetSeed ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 7, RngSeedManager::GetRun ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_step (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}


} // namespace ns3