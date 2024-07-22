// Copyright (c) 2011-2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Authors:
//   Jaume Nin <jnin@cttc.es>
//   Nicola Baldo <nbaldo@cttc.es>
//   Manuel Requena <manuel.requena@cttc.es>

#include "emu-epc-helper.h"

#include "ns3/emu-fd-net-device-helper.h"
#include "ns3/epc-x2.h"
#include "ns3/log.h"
#include "ns3/lte-enb-net-device.h"
#include "ns3/nr-gnb-rrc.h"
#include "ns3/string.h"

#include <iomanip>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("EmunrEpcHelper");

NS_OBJECT_ENSURE_REGISTERED(EmunrEpcHelper);

EmunrEpcHelper::EmunrEpcHelper()
    : NrNoBackhaulEpcHelper()
{
    NS_LOG_FUNCTION(this);
    // To access the attribute value within the constructor
    ObjectBase::ConstructSelf(AttributeConstructionList());

    // Create EmuFdNetDevice for SGW
    EmuFdNetDeviceHelper emu;
    NS_LOG_LOGIC("SGW device: " << m_sgwDeviceName);
    emu.SetDeviceName(m_sgwDeviceName);

    Ptr<Node> sgw = GetSgwNode();
    NetDeviceContainer sgwDevices = emu.Install(sgw);
    Ptr<NetDevice> sgwDevice = sgwDevices.Get(0);
    NS_LOG_LOGIC("SGW MAC address: " << m_sgwMacAddress);
    sgwDevice->SetAttribute("Address", Mac48AddressValue(m_sgwMacAddress.c_str()));

    // Address of the SGW: 10.0.0.1
    m_epcIpv4AddressHelper.SetBase("10.0.0.0", "255.255.255.0", "0.0.0.1");
    m_sgwIpIfaces = m_epcIpv4AddressHelper.Assign(sgwDevices);

    // Address of the first eNB: 10.0.0.101
    m_epcIpv4AddressHelper.SetBase("10.0.0.0", "255.255.255.0", "0.0.0.101");
}

EmunrEpcHelper::~EmuNrEpcHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
EmuNrEpcHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::EmuNrEpcHelper")
            .SetParent<NrEpcHelper>()
            .SetGroupName("Nr")
            .AddConstructor<EmuNrEpcHelper>()
            .AddAttribute("SgwDeviceName",
                          "The name of the device used for the S1-U interface of the SGW",
                          StringValue("veth0"),
                          MakeStringAccessor(&EmuNrEpcHelper::m_sgwDeviceName),
                          MakeStringChecker())
            .AddAttribute("EnbDeviceName",
                          "The name of the device used for the S1-U interface of the eNB",
                          StringValue("veth1"),
                          MakeStringAccessor(&EmuNrEpcHelper::m_enbDeviceName),
                          MakeStringChecker())
            .AddAttribute("SgwMacAddress",
                          "MAC address used for the SGW",
                          StringValue("00:00:00:59:00:aa"),
                          MakeStringAccessor(&EmuNrEpcHelper::m_sgwMacAddress),
                          MakeStringChecker())
            .AddAttribute("EnbMacAddressBase",
                          "First 5 bytes of the eNB MAC address base",
                          StringValue("00:00:00:eb:00"),
                          MakeStringAccessor(&EmuNrEpcHelper::m_enbMacAddressBase),
                          MakeStringChecker());
    return tid;
}

TypeId
EmuNrEpcHelper::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
EmuNrEpcHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    NrNoBackhaulEpcHelper::DoDispose();
}

void
EmuNrEpcHelper::AddEnb(Ptr<Node> enb, Ptr<NetDevice> nrGnbNetDevice, std::vector<uint16_t> cellIds)
{
    NS_LOG_FUNCTION(this << enb << nrGnbNetDevice << cellIds.size());

    NrNoBackhaulEpcHelper::AddEnb(enb, nrGnbNetDevice, cellIds);

    // Create an EmuFdNetDevice for the eNB to connect with the SGW and other eNBs
    EmuFdNetDeviceHelper emu;
    NS_LOG_LOGIC("eNB cellId: " << cellIds.at(0));
    NS_LOG_LOGIC("eNB device: " << m_enbDeviceName);
    emu.SetDeviceName(m_enbDeviceName);
    NetDeviceContainer enbDevices = emu.Install(enb);

    std::ostringstream enbMacAddress;
    enbMacAddress << m_enbMacAddressBase << ":" << std::hex << std::setfill('0') << std::setw(2)
                  << cellIds.at(0);
    NS_LOG_LOGIC("eNB MAC address: " << enbMacAddress.str());
    Ptr<NetDevice> enbDev = enbDevices.Get(0);
    enbDev->SetAttribute("Address", Mac48AddressValue(enbMacAddress.str().c_str()));

    // emu.EnablePcap ("enbDevice", enbDev);

    NS_LOG_LOGIC("number of Ipv4 ifaces of the eNB after installing emu dev: "
                 << enb->GetObject<Ipv4>()->GetNInterfaces());
    Ipv4InterfaceContainer enbIpIfaces = m_epcIpv4AddressHelper.Assign(enbDevices);
    NS_LOG_LOGIC("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to S1 dev: "
                 << enb->GetObject<Ipv4>()->GetNInterfaces());

    Ipv4Address enbAddress = enbIpIfaces.GetAddress(0);
    Ipv4Address sgwAddress = m_sgwIpIfaces.GetAddress(0);

    NrNoBackhaulEpcHelper::AddS1Interface(enb, enbAddress, sgwAddress, cellIds);
}

void
EmuNrEpcHelper::AddX2Interface(Ptr<Node> enb1, Ptr<Node> enb2)
{
    NS_LOG_FUNCTION(this << enb1 << enb2);

    NS_LOG_WARN("X2 support still untested");

    // for X2, we reuse the same device and IP address of the S1-U interface
    Ptr<Ipv4> enb1Ipv4 = enb1->GetObject<Ipv4>();
    Ptr<Ipv4> enb2Ipv4 = enb2->GetObject<Ipv4>();
    NS_LOG_LOGIC("number of Ipv4 ifaces of the eNB #1: " << enb1Ipv4->GetNInterfaces());
    NS_LOG_LOGIC("number of Ipv4 ifaces of the eNB #2: " << enb2Ipv4->GetNInterfaces());
    NS_LOG_LOGIC("number of NetDevices of the eNB #1: " << enb1->GetNDevices());
    NS_LOG_LOGIC("number of NetDevices of the eNB #2: " << enb2->GetNDevices());

    // 0 is the LTE device, 1 is localhost, 2 is the EPC NetDevice
    Ptr<NetDevice> enb1EpcDev = enb1->GetDevice(2);
    Ptr<NetDevice> enb2EpcDev = enb2->GetDevice(2);

    int32_t enb1Interface = enb1Ipv4->GetInterfaceForDevice(enb1EpcDev);
    int32_t enb2Interface = enb2Ipv4->GetInterfaceForDevice(enb2EpcDev);
    NS_ASSERT(enb1Interface >= 0);
    NS_ASSERT(enb2Interface >= 0);
    NS_ASSERT(enb1Ipv4->GetNAddresses(enb1Interface) == 1);
    NS_ASSERT(enb2Ipv4->GetNAddresses(enb2Interface) == 1);
    Ipv4Address enb1Addr = enb1Ipv4->GetAddress(enb1Interface, 0).GetLocal();
    Ipv4Address enb2Addr = enb2Ipv4->GetAddress(enb2Interface, 0).GetLocal();
    NS_LOG_LOGIC(" eNB 1 IP address: " << enb1Addr);
    NS_LOG_LOGIC(" eNB 2 IP address: " << enb2Addr);

    // Add X2 interface to both eNBs' X2 entities
    Ptr<NrEpcX2> enb1X2 = enb1->GetObject<NrEpcX2>();
    Ptr<NrGnbNetDevice> enb1NrDev = enb1->GetDevice(0)->GetObject<NrGnbNetDevice>();
    std::vector<uint16_t> enb1CellIds = enb1NrDev->GetCellIds();
    uint16_t enb1CellId = enb1CellIds.at(0);
    NS_LOG_LOGIC("NrGnbNetDevice #1 = " << enb1NrDev << " - CellId = " << enb1CellId);

    Ptr<NrEpcX2> enb2X2 = enb2->GetObject<NrEpcX2>();
    Ptr<NrGnbNetDevice> enb2NrDev = enb2->GetDevice(0)->GetObject<NrGnbNetDevice>();
    std::vector<uint16_t> enb2CellIds = enb2NrDev->GetCellIds();
    uint16_t enb2CellId = enb2CellIds.at(0);
    NS_LOG_LOGIC("NrGnbNetDevice #2 = " << enb2NrDev << " - CellId = " << enb2CellId);

    enb1X2->AddX2Interface(enb1CellId, enb1Addr, enb2CellIds, enb2Addr);
    enb2X2->AddX2Interface(enb2CellId, enb2Addr, enb1CellIds, enb1Addr);

    enb1NrDev->GetRrc()->AddX2Neighbour(enb2NrDev->GetCellId());
    enb2NrDev->GetRrc()->AddX2Neighbour(enb1NrDev->GetCellId());
}

} // namespace ns3
