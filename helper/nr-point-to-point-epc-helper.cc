// Copyright (c) 2011-2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Authors:
//   Jaume Nin <jnin@cttc.es>
//   Nicola Baldo <nbaldo@cttc.es>
//   Manuel Requena <manuel.requena@cttc.es>
//   (most of the code refactored to no-backhaul-epc-helper.cc)

#include "nr-point-to-point-epc-helper.h"

#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/string.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPointToPointEpcHelper");

NS_OBJECT_ENSURE_REGISTERED(NrPointToPointEpcHelper);

NrPointToPointEpcHelper::NrPointToPointEpcHelper()
    : NrNoBackhaulEpcHelper()
{
    NS_LOG_FUNCTION(this);
    // To access the attribute value within the constructor
    ObjectBase::ConstructSelf(AttributeConstructionList());

    // since we use point-to-point links for the backhaul links,
    // we use a /30 subnet which can hold exactly two addresses
    // (remember that net broadcast and null address are not valid)
    m_s1uIpv4AddressHelper.SetBase("10.0.0.0", "255.255.255.252");
    m_s1apIpv4AddressHelper.SetBase("11.0.0.0", "255.255.255.252");
}

NrPointToPointEpcHelper::~NrPointToPointEpcHelper()
{
    NS_LOG_FUNCTION(this);
}

TypeId
NrPointToPointEpcHelper::GetTypeId()
{
    NS_LOG_FUNCTION_NOARGS();
    static TypeId tid =
        TypeId("ns3::NrPointToPointEpcHelper")
            .SetParent<NrNoBackhaulEpcHelper>()
            .SetGroupName("Nr")
            .AddConstructor<NrPointToPointEpcHelper>()
            .AddAttribute("S1uLinkDataRate",
                          "The data rate to be used for the next S1-U link to be created",
                          DataRateValue(DataRate("10Gb/s")),
                          MakeDataRateAccessor(&NrPointToPointEpcHelper::m_s1uLinkDataRate),
                          MakeDataRateChecker())
            .AddAttribute("S1uLinkDelay",
                          "The delay to be used for the next S1-U link to be created",
                          TimeValue(Seconds(0)),
                          MakeTimeAccessor(&NrPointToPointEpcHelper::m_s1uLinkDelay),
                          MakeTimeChecker())
            .AddAttribute("S1uLinkMtu",
                          "The MTU of the next S1-U link to be created. Note that, because of the "
                          "additional GTP/UDP/IP tunneling overhead, you need a MTU larger than "
                          "the end-to-end MTU that you want to support.",
                          UintegerValue(2000),
                          MakeUintegerAccessor(&NrPointToPointEpcHelper::m_s1uLinkMtu),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("S1uLinkPcapPrefix",
                          "Prefix for Pcap generated by S1-U link",
                          StringValue("s1u"),
                          MakeStringAccessor(&NrPointToPointEpcHelper::m_s1uLinkPcapPrefix),
                          MakeStringChecker())
            .AddAttribute("S1uLinkEnablePcap",
                          "Enable Pcap for X2 link",
                          BooleanValue(false),
                          MakeBooleanAccessor(&NrPointToPointEpcHelper::m_s1uLinkEnablePcap),
                          MakeBooleanChecker());
    return tid;
}

void
NrPointToPointEpcHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    NrNoBackhaulEpcHelper::DoDispose();
}

void
NrPointToPointEpcHelper::AddGnb(Ptr<Node> gnb,
                                Ptr<NetDevice> nrGnbNetDevice,
                                std::vector<uint16_t> cellIds)
{
    NS_LOG_FUNCTION(this << gnb << nrGnbNetDevice << cellIds.size());

    NrNoBackhaulEpcHelper::AddGnb(gnb, nrGnbNetDevice, cellIds);

    // create a point to point link between the gNB and the SGW with
    // the corresponding new NetDevices on each side
    Ptr<Node> sgw = GetSgwNode();

    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(m_s1uLinkDataRate));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(m_s1uLinkMtu));
    p2ph.SetChannelAttribute("Delay", TimeValue(m_s1uLinkDelay));
    NetDeviceContainer gnbSgwDevices = p2ph.Install(gnb, sgw);
    NS_LOG_LOGIC("Ipv4 ifaces of the gNB after installing p2p dev: "
                 << gnb->GetObject<Ipv4>()->GetNInterfaces());

    if (m_s1uLinkEnablePcap)
    {
        p2ph.EnablePcapAll(m_s1uLinkPcapPrefix);
    }

    m_s1uIpv4AddressHelper.NewNetwork();
    Ipv4InterfaceContainer gnbSgwIpIfaces = m_s1uIpv4AddressHelper.Assign(gnbSgwDevices);
    NS_LOG_LOGIC("number of Ipv4 ifaces of the gNB after assigning Ipv4 addr to S1 dev: "
                 << gnb->GetObject<Ipv4>()->GetNInterfaces());

    Ipv4Address gnbS1uAddress = gnbSgwIpIfaces.GetAddress(0);
    Ipv4Address sgwS1uAddress = gnbSgwIpIfaces.GetAddress(1);

    NrNoBackhaulEpcHelper::AddS1Interface(gnb, gnbS1uAddress, sgwS1uAddress, cellIds);
}

} // namespace ns3
