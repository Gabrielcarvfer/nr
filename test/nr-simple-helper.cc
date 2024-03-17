/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 * (Based on lte-helper.cc)
 */

#include "nr-simple-helper.h"

#include "nr-simple-net-device.h"
#include "nr-test-entities.h"

#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/error-model.h"
#include "ns3/log.h"
#include "ns3/simple-channel.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrSimpleHelper");

NS_OBJECT_ENSURE_REGISTERED(NrSimpleHelper);

NrSimpleHelper::NrSimpleHelper()
{
    NS_LOG_FUNCTION(this);
    m_enbDeviceFactory.SetTypeId(NrSimpleNetDevice::GetTypeId());
    m_ueDeviceFactory.SetTypeId(NrSimpleNetDevice::GetTypeId());
}

void
NrSimpleHelper::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    m_phyChannel = CreateObject<SimpleChannel>();

    Object::DoInitialize();
}

NrSimpleHelper::~NrSimpleHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrSimpleHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrSimpleHelper")
            .SetParent<Object>()
            .AddConstructor<NrSimpleHelper>()
            .AddAttribute("RlcEntity",
                          "Specify which type of RLC will be used. ",
                          EnumValue(RLC_UM),
                          MakeEnumAccessor<NrRlcEntityType_t>(&NrSimpleHelper::m_lteRlcEntityType),
                          MakeEnumChecker(RLC_UM, "RlcUm", RLC_AM, "RlcAm"));
    return tid;
}

void
NrSimpleHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_phyChannel = nullptr;

    m_enbMac->Dispose();
    m_enbMac = nullptr;
    m_ueMac->Dispose();
    m_ueMac = nullptr;

    Object::DoDispose();
}

NetDeviceContainer
NrSimpleHelper::InstallGnbDevice(NodeContainer c)
{
    NS_LOG_FUNCTION(this);
    Initialize(); // will run DoInitialize () if necessary
    NetDeviceContainer devices;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<NetDevice> device = InstallSingleEnbDevice(node);
        devices.Add(device);
    }
    return devices;
}

NetDeviceContainer
NrSimpleHelper::InstallUeDevice(NodeContainer c)
{
    NS_LOG_FUNCTION(this);
    NetDeviceContainer devices;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<NetDevice> device = InstallSingleUeDevice(node);
        devices.Add(device);
    }
    return devices;
}

Ptr<NetDevice>
NrSimpleHelper::InstallSingleEnbDevice(Ptr<Node> n)
{
    NS_LOG_FUNCTION(this);

    m_enbRrc = CreateObject<NrTestRrc>();
    m_enbPdcp = CreateObject<NrPdcp>();

    if (m_lteRlcEntityType == RLC_UM)
    {
        m_enbRlc = CreateObject<NrRlcUm>();
    }
    else // m_lteRlcEntityType == RLC_AM
    {
        m_enbRlc = CreateObject<NrRlcAm>();
    }

    m_enbRlc->SetRnti(11);
    m_enbRlc->SetLcId(12);

    Ptr<NrSimpleNetDevice> enbDev = m_enbDeviceFactory.Create<NrSimpleNetDevice>();
    enbDev->SetAddress(Mac48Address::Allocate());
    enbDev->SetChannel(m_phyChannel);

    n->AddDevice(enbDev);

    m_enbMac = CreateObject<NrTestMac>();
    m_enbMac->SetDevice(enbDev);

    m_enbRrc->SetDevice(enbDev);

    enbDev->SetReceiveCallback(MakeCallback(&NrTestMac::Receive, m_enbMac));

    // Connect SAPs: RRC <-> PDCP <-> RLC <-> MAC

    m_enbRrc->SetNrPdcpSapProvider(m_enbPdcp->GetNrPdcpSapProvider());
    m_enbPdcp->SetNrPdcpSapUser(m_enbRrc->GetNrPdcpSapUser());

    m_enbPdcp->SetNrRlcSapProvider(m_enbRlc->GetNrRlcSapProvider());
    m_enbRlc->SetNrRlcSapUser(m_enbPdcp->GetNrRlcSapUser());

    m_enbRlc->SetNrMacSapProvider(m_enbMac->GetNrMacSapProvider());
    m_enbMac->SetNrMacSapUser(m_enbRlc->GetNrMacSapUser());

    return enbDev;
}

Ptr<NetDevice>
NrSimpleHelper::InstallSingleUeDevice(Ptr<Node> n)
{
    NS_LOG_FUNCTION(this);

    m_ueRrc = CreateObject<NrTestRrc>();
    m_uePdcp = CreateObject<NrPdcp>();

    if (m_lteRlcEntityType == RLC_UM)
    {
        m_ueRlc = CreateObject<NrRlcUm>();
    }
    else // m_lteRlcEntityType == RLC_AM
    {
        m_ueRlc = CreateObject<NrRlcAm>();
    }

    m_ueRlc->SetRnti(21);
    m_ueRlc->SetLcId(22);

    Ptr<NrSimpleNetDevice> ueDev = m_ueDeviceFactory.Create<NrSimpleNetDevice>();
    ueDev->SetAddress(Mac48Address::Allocate());
    ueDev->SetChannel(m_phyChannel);

    n->AddDevice(ueDev);

    m_ueMac = CreateObject<NrTestMac>();
    m_ueMac->SetDevice(ueDev);

    ueDev->SetReceiveCallback(MakeCallback(&NrTestMac::Receive, m_ueMac));

    // Connect SAPs: RRC <-> PDCP <-> RLC <-> MAC

    m_ueRrc->SetNrPdcpSapProvider(m_uePdcp->GetNrPdcpSapProvider());
    m_uePdcp->SetNrPdcpSapUser(m_ueRrc->GetNrPdcpSapUser());

    m_uePdcp->SetNrRlcSapProvider(m_ueRlc->GetNrRlcSapProvider());
    m_ueRlc->SetNrRlcSapUser(m_uePdcp->GetNrRlcSapUser());

    m_ueRlc->SetNrMacSapProvider(m_ueMac->GetNrMacSapProvider());
    m_ueMac->SetNrMacSapUser(m_ueRlc->GetNrMacSapUser());

    return ueDev;
}

void
NrSimpleHelper::EnableLogComponents()
{
    auto level = (LogLevel)(LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_PREFIX_FUNC);

    LogComponentEnable("Config", level);
    LogComponentEnable("NrSimpleHelper", level);
    LogComponentEnable("NrTestEntities", level);
    LogComponentEnable("NrPdcp", level);
    LogComponentEnable("NrRlc", level);
    LogComponentEnable("NrRlcUm", level);
    LogComponentEnable("NrRlcAm", level);
    LogComponentEnable("NrSimpleNetDevice", level);
    LogComponentEnable("SimpleNetDevice", level);
    LogComponentEnable("SimpleChannel", level);
}

void
NrSimpleHelper::EnableTraces()
{
    //   EnableMacTraces ();
    EnableRlcTraces();
    EnablePdcpTraces();
}

void
NrSimpleHelper::EnableRlcTraces()
{
    EnableDlRlcTraces();
    EnableUlRlcTraces();
}

/**
 * DL transmit PDU callback
 *
 * \param rlcStats the stats calculator
 * \param path
 * \param rnti the RNTI
 * \param lcid the LCID
 * \param packetSize the packet size
 */
void
NrSimpleHelperDlTxPduCallback(Ptr<NrBearerStatsCalculator> rlcStats,
                              std::string path,
                              uint16_t rnti,
                              uint8_t lcid,
                              uint32_t packetSize)
{
    NS_LOG_FUNCTION(rlcStats << path << rnti << (uint16_t)lcid << packetSize);
    uint64_t imsi = 111;
    uint16_t cellId = 222;
    rlcStats->DlTxPdu(cellId, imsi, rnti, lcid, packetSize);
}

/**
 * DL receive PDU callback
 *
 * \param rlcStats the stats calculator
 * \param path
 * \param rnti the RNTI
 * \param lcid the LCID
 * \param packetSize the packet size
 * \param delay the delay
 */
void
NrSimpleHelperDlRxPduCallback(Ptr<NrBearerStatsCalculator> rlcStats,
                              std::string path,
                              uint16_t rnti,
                              uint8_t lcid,
                              uint32_t packetSize,
                              uint64_t delay)
{
    NS_LOG_FUNCTION(rlcStats << path << rnti << (uint16_t)lcid << packetSize << delay);
    uint64_t imsi = 333;
    uint16_t cellId = 555;
    rlcStats->DlRxPdu(cellId, imsi, rnti, lcid, packetSize, delay);
}

void
NrSimpleHelper::EnableDlRlcTraces()
{
    NS_LOG_FUNCTION_NOARGS();

    //   Config::Connect ("/NodeList/*/DeviceList/*/NrRlc/TxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperDlTxPduCallback, m_rlcStats));
    //   Config::Connect ("/NodeList/*/DeviceList/*/NrRlc/RxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperDlRxPduCallback, m_rlcStats));
}

/**
 * UL transmit PDU callback
 *
 * \param rlcStats the stats calculator
 * \param path
 * \param rnti the RNTI
 * \param lcid the LCID
 * \param packetSize the packet size
 */
void
NrSimpleHelperUlTxPduCallback(Ptr<NrBearerStatsCalculator> rlcStats,
                              std::string path,
                              uint16_t rnti,
                              uint8_t lcid,
                              uint32_t packetSize)
{
    NS_LOG_FUNCTION(rlcStats << path << rnti << (uint16_t)lcid << packetSize);
    uint64_t imsi = 1111;
    uint16_t cellId = 555;
    rlcStats->UlTxPdu(cellId, imsi, rnti, lcid, packetSize);
}

/**
 * UL receive PDU callback
 *
 * \param rlcStats the stats calculator
 * \param path
 * \param rnti the RNTI
 * \param lcid the LCID
 * \param packetSize the packet size
 * \param delay the delay
 */
void
NrSimpleHelperUlRxPduCallback(Ptr<NrBearerStatsCalculator> rlcStats,
                              std::string path,
                              uint16_t rnti,
                              uint8_t lcid,
                              uint32_t packetSize,
                              uint64_t delay)
{
    NS_LOG_FUNCTION(rlcStats << path << rnti << (uint16_t)lcid << packetSize << delay);
    uint64_t imsi = 444;
    uint16_t cellId = 555;
    rlcStats->UlRxPdu(cellId, imsi, rnti, lcid, packetSize, delay);
}

void
NrSimpleHelper::EnableUlRlcTraces()
{
    NS_LOG_FUNCTION_NOARGS();

    //   Config::Connect ("/NodeList/*/DeviceList/*/NrRlc/TxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperUlTxPduCallback, m_rlcStats));
    //   Config::Connect ("/NodeList/*/DeviceList/*/NrRlc/RxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperUlRxPduCallback, m_rlcStats));
}

void
NrSimpleHelper::EnablePdcpTraces()
{
    EnableDlPdcpTraces();
    EnableUlPdcpTraces();
}

void
NrSimpleHelper::EnableDlPdcpTraces()
{
    NS_LOG_FUNCTION_NOARGS();

    //   Config::Connect ("/NodeList/*/DeviceList/*/NrPdcp/TxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperDlTxPduCallback, m_pdcpStats));
    //   Config::Connect ("/NodeList/*/DeviceList/*/NrPdcp/RxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperDlRxPduCallback, m_pdcpStats));
}

void
NrSimpleHelper::EnableUlPdcpTraces()
{
    NS_LOG_FUNCTION_NOARGS();

    //   Config::Connect ("/NodeList/*/DeviceList/*/NrPdcp/TxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperUlTxPduCallback, m_pdcpStats));
    //   Config::Connect ("/NodeList/*/DeviceList/*/NrPdcp/RxPDU",
    //                    MakeBoundCallback (&NrSimpleHelperUlRxPduCallback, m_pdcpStats));
}

} // namespace ns3
