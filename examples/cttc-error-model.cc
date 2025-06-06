// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-module.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"

#include <chrono>

/**
 * @file cttc-error-model.cc
 * @ingroup examples
 * @brief Error model example with fixed MCS: 1 gNB and 1 UE, multiple packets with varying fading
 * conditions.
 *
 * This example allows the user to test the end-to-end performance with the new
 * NR PHY abstraction model for error modeling by using a fixed MCS. It allows the user to set the
 * MCS, the gNB-UE distance, the MCS table, the error model type, and the HARQ method.
 *
 * The NR error model can be set as "--errorModel=ns3::NrEesmCcT1", for HARQ-CC and MCS Table1,
 * while "--errorModel=ns3::NrLteMiErrorModel" configures the LTE error model.
 * For NR, you can choose between different types of error model, which use
 * different tables and different methods to process the HARQ history, e.g.,
 * "--errorModel=ns3::NrEesmIrT1", for HARQ-IR and MCS Table2.
 * You can fix also the MCS index to use with "--mcs=7" (7 in this case), which refers
 * to the configured MCS table.
 *
 * The scenario consists of a single gNB and a single UE, placed at positions (0.0, 0.0, 10), and
 * (0.0, ueY, 1.5), respectively. ueY can be configured by the user, e.g. "ueY=20", and defaults
 * to 30 m.
 *
 * By default, the program uses the 3GPP channel model, Urban Micro scenario, without shadowing and
 * with probabilistic line of sight / non-line of sight option. The program runs for 50 seconds and
 * one packet is transmitted every 200 ms from gNB to UE (donwlink direction). The packet size can
 * be configured by using the following parameter: "--packetSize=1000". The channel update period is
 * 150 ms, so that every packet encounters a different fading condition.
 *
 * This simulation prints the output to the terminal. The output statistics are
 * averaged among all the transmitted packets.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./ns3 run cttc-error-model
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CttcErrorModelExample");

static Ptr<ListPositionAllocator>
GetGnbPositions(double gNbHeight = 10.0)
{
    Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
    pos->Add(Vector(0.0, 0.0, gNbHeight));

    return pos;
}

static Ptr<ListPositionAllocator>
GetUePositions(double ueY, double ueHeight = 1.5)
{
    Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator>();
    pos->Add(Vector(0.0, ueY, ueHeight));

    return pos;
}

static std::vector<uint64_t> packetsTime;

static void
PrintRxPkt([[maybe_unused]] std::string context, Ptr<const Packet> pkt)
{
    // ASSUMING ONE UE
    SeqTsHeader seqTs;
    pkt->PeekHeader(seqTs);
    packetsTime.push_back((Simulator::Now() - seqTs.GetTs()).GetMicroSeconds());
}

int
main(int argc, char* argv[])
{
    uint32_t mcs = 13;
    const uint8_t gNbNum = 1;
    const uint8_t ueNum = 1;
    double totalTxPower = 4;
    uint16_t numerologyBwp = 4;
    double centralFrequencyBand = 28e9;
    double bandwidthBand = 100e6;
    double ueY = 30.0;

    double simTime = 10.0; // 50 seconds: to take statistics
    uint32_t pktSize = 500;
    Time udpAppStartTime = MilliSeconds(1000);
    Time packetInterval = MilliSeconds(200);
    Time updateChannelInterval = MilliSeconds(150);
    bool isUl = false;

    std::string errorModel = "ns3::NrEesmCcT1";

    CommandLine cmd(__FILE__);

    cmd.AddValue("simTime", "Simulation time", simTime);
    cmd.AddValue("mcs", "The MCS that will be used in this example", mcs);
    cmd.AddValue("errorModelType",
                 "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1, "
                 "ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
                 errorModel);
    cmd.AddValue("ueY", "Y position of any UE", ueY);
    cmd.AddValue("pktSize", "Packet Size", pktSize);
    cmd.AddValue("isUl", "Is this an UL transmission?", isUl);

    cmd.Parse(argc, argv);

    uint32_t packets = (simTime - udpAppStartTime.GetSeconds()) / packetInterval.GetSeconds();
    NS_ABORT_IF(packets == 0);

    /*
     * Default values for the simulation. We are progressively removing all
     * the instances of SetDefault, but we need it for legacy code (LTE)
     */
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    Config::SetDefault("ns3::NrAmc::ErrorModelType", TypeIdValue(TypeId::LookupByName(errorModel)));
    Config::SetDefault("ns3::NrAmc::AmcModel",
                       EnumValue(NrAmc::ShannonModel)); // NOT USED in this example. MCS is fixed.

    // create base stations and mobile terminals
    NodeContainer gNbNodes;
    NodeContainer ueNodes;
    MobilityHelper mobility;

    double gNbHeight = 10.0;
    double ueHeight = 1.5;

    gNbNodes.Create(gNbNum);
    ueNodes.Create(ueNum);

    Ptr<ListPositionAllocator> gNbPositionAlloc = GetGnbPositions(gNbHeight);
    Ptr<ListPositionAllocator> uePositionAlloc = GetUePositions(ueY, ueHeight);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(gNbPositionAlloc);
    mobility.Install(gNbNodes);

    mobility.SetPositionAllocator(uePositionAlloc);
    mobility.Install(ueNodes);

    /*
     * Setup the NR module. We create the various helpers needed for the
     * NR simulation:
     * - nrEpcHelper, which will setup the core network
     * - IdealBeamformingHelper, which takes care of the beamforming part
     * - NrHelper, which takes care of creating and connecting the various
     * part of the NR stack
     * - NrChannelHelper, which will setup the spectrum channel
     */
    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();

    // Put the pointers inside nrHelper
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);
    nrHelper->SetEpcHelper(nrEpcHelper);

    // Set the channel using UMi scenario
    channelHelper->ConfigureFactories("UMi", "Default", "ThreeGpp");
    /*
     * Spectrum division. We create one operational band, with one CC, and the CC with a single
     * bandwidth part.
     */
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    const uint8_t numCcPerBand = 1;

    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequencyBand,
                                                   bandwidthBand,
                                                   numCcPerBand);
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    // Set attributes of the channel
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(updateChannelInterval));
    channelHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

    // Set and create the channel in the band
    channelHelper->AssignChannelsToBands({band});
    allBwps = CcBwpCreator::GetAllBwps({band});

    Packet::EnableChecking();
    Packet::EnablePrinting();

    /*
     *  Case (i): Attributes valid for all the nodes
     */
    // Beamforming method
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(DirectPathBeamforming::GetTypeId()));

    // Core latency
    nrEpcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<IsotropicAntennaModel>()));

    // Scheduler
    nrHelper->SetSchedulerAttribute("FixedMcsDl", BooleanValue(true));
    nrHelper->SetSchedulerAttribute("FixedMcsUl", BooleanValue(true));
    nrHelper->SetSchedulerAttribute("StartingMcsDl", UintegerValue(mcs));
    nrHelper->SetSchedulerAttribute("StartingMcsUl", UintegerValue(mcs));

    // Error Model: UE and GNB with same spectrum error model.
    nrHelper->SetUlErrorModel(errorModel);
    nrHelper->SetDlErrorModel(errorModel);

    // Both DL and UL AMC will have the same model behind.
    // Note: NOT USED in this example. MCS is fixed.
    nrHelper->SetGnbDlAmcAttribute("AmcModel", EnumValue(NrAmc::ShannonModel));
    nrHelper->SetGnbUlAmcAttribute("AmcModel", EnumValue(NrAmc::ShannonModel));

    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(totalTxPower));

    uint32_t bwpId = 0;

    // gNb routing between Bearer and bandwidh part
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpId));

    // Ue routing between Bearer and bandwidth part
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpId));

    NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice(gNbNodes, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes, allBwps);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gnbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);

    /*
     * Case (iii): Go node for node and change the attributes we have to setup
     * per-node.
     */

    // Get the first netdevice (gnbNetDev.Get (0)) and the first bandwidth part (0)
    // and set the attribute.
    nrHelper->GetGnbPhy(gnbNetDev.Get(0), 0)
        ->SetAttribute("Numerology", UintegerValue(numerologyBwp));
    nrHelper->GetGnbPhy(gnbNetDev.Get(0), 0)->SetAttribute("TxPower", DoubleValue(totalTxPower));

    // create the internet and install the IP stack on the UEs
    // get SGW/PGW and create a single RemoteHost
    auto [remoteHost, remoteHostIpv4Address] =
        nrEpcHelper->SetupRemoteHost("100Gb/s", 2500, Seconds(0.000));
    NodeContainer remoteHostContainer(remoteHost);

    InternetStackHelper internet;
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));

    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    UdpServerHelper dlPacketSinkHelper(dlPort);
    ApplicationContainer txApps;
    ApplicationContainer sinkApps;
    NodeContainer txNodes;
    NodeContainer sinkNodes;
    std::vector<Ipv4Address> sinkIps;

    if (isUl)
    {
        sinkIps.push_back(remoteHostIpv4Address);
        sinkNodes = remoteHostContainer;
        txNodes = ueNodes;
    }
    else
    {
        sinkIps.resize(ueIpIface.GetN());
        for (uint32_t i = 0; i < ueIpIface.GetN(); i++)
        {
            sinkIps[i] = ueIpIface.GetAddress(i);
        }
        sinkNodes = ueNodes;
        txNodes = remoteHostContainer;
    }

    // configure here UDP traffic
    for (uint32_t i = 0; i < txNodes.GetN(); ++i)
    {
        for (uint32_t j = 0; j < sinkNodes.GetN(); ++j)
        {
            UdpClientHelper dlClient(sinkIps[j], dlPort);
            dlClient.SetAttribute("MaxPackets", UintegerValue(packets));
            dlClient.SetAttribute("PacketSize", UintegerValue(pktSize));
            dlClient.SetAttribute("Interval", TimeValue(packetInterval));

            txApps.Add(dlClient.Install(txNodes.Get(i)));
        }
    }

    sinkApps.Add(dlPacketSinkHelper.Install(sinkNodes));
    for (uint32_t j = 0; j < sinkApps.GetN(); ++j)
    {
        Ptr<UdpServer> client = DynamicCast<UdpServer>(sinkApps.Get(j));
        NS_ASSERT(client != nullptr);
        std::stringstream ss;
        ss << j;
        client->TraceConnect("Rx", ss.str(), MakeCallback(&PrintRxPkt));
    }

    // start UDP server and client apps
    sinkApps.Start(udpAppStartTime);
    txApps.Start(udpAppStartTime);
    sinkApps.Stop(Seconds(simTime));
    txApps.Stop(Seconds(simTime));

    // attach UEs to the closest g
    nrHelper->AttachToClosestGnb(ueNetDev, gnbNetDev);

    // enable the traces provided by the nr module
    // nrHelper->EnableTraces();

    Simulator::Stop(Seconds(simTime));

    auto start = std::chrono::steady_clock::now();

    Simulator::Run();

    auto end = std::chrono::steady_clock::now();

    uint64_t sum = 0;
    uint32_t cont = 0;
    for (auto& v : packetsTime)
    {
        if (v < 100000)
        {
            sum += v;
            cont++;
        }
    }
    std::cout << "Packets received: " << packetsTime.size() << std::endl;
    std::cout << "Counter (packets not affected by reordering): " << +cont << std::endl;

    if (!packetsTime.empty() && cont > 0)
    {
        std::cout << "Average e2e latency (over all received packets): " << sum / packetsTime.size()
                  << " us" << std::endl;
        std::cout << "Average e2e latency (over counter): " << sum / cont << " us" << std::endl;
    }
    else
    {
        std::cout << "Average e2e latency: Not Available" << std::endl;
    }

    for (auto it = sinkApps.Begin(); it != sinkApps.End(); ++it)
    {
        uint64_t recv = DynamicCast<UdpServer>(*it)->GetReceived();
        std::cout << "Sent: " << packets << " Recv: " << recv << " Lost: " << packets - recv
                  << " pkts, ( " << (static_cast<double>(packets - recv) / packets) * 100.0
                  << " % )" << std::endl;
    }

    Simulator::Destroy();

    std::cout << "Running time: "
              << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " s."
              << std::endl;
    return 0;
}
