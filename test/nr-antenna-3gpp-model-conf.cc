// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/test.h"

using namespace ns3;

/**
 * @ingroup test
 * @file test-antenna-3gpp-model-conf.cc
 *
 * @brief This test case checks if the throughput/SINR/MCS
 * obtained is as expected for the configured antenna model and for
 * different positions of UE. The test scenario consists of a scenario in
 * which a single UE is attached to a gNB.
 * UE performs a UDP full buffer downlink traffic.
 * gNB is configured to have 1 bandwidth part.
 * Currently there are 2 types of antenna elements: omni and 3gpp directional.
 *
 */

class TestAntenna3gppModelConf : public TestCase
{
  public:
    enum DirectionGnbUeXYAngle
    {
        DirectionGnbUe_45,
        DirectionGnbUe_135,
        DirectionGnbUe_225,
        DirectionGnbUe_315,
        DirectionGnbUe_0,
        DirectionGnbUe_90,
        DirectionGnbUe_180,
        DirectionGnbUe_270,
    };

    TestAntenna3gppModelConf(const std::string& name,
                             DirectionGnbUeXYAngle conf,
                             bool gNbOmniAntennaElem,
                             bool ueOmniAntennaElem,
                             uint8_t ueNoOfAntennas,
                             std::string losCondition);
    ~TestAntenna3gppModelConf() override;
    void UeReception(RxPacketTraceParams params);

  private:
    void DoRun() override;
    std::string m_name;
    DirectionGnbUeXYAngle m_conf;
    bool m_ueOmniAntennaElem;
    bool m_gNbOmniAntennaElem;

    uint8_t m_ueNoOfAntennas;
    std::string m_losCondition;
    Ptr<MinMaxAvgTotalCalculator<double>> m_sinrCell1;
    Ptr<MinMaxAvgTotalCalculator<double>> m_sinrCell2;
    Ptr<MinMaxAvgTotalCalculator<double>> m_mcsCell1;
    Ptr<MinMaxAvgTotalCalculator<double>> m_mcsCell2;
    Ptr<MinMaxAvgTotalCalculator<double>> m_rbNumCell1;
    Ptr<MinMaxAvgTotalCalculator<double>> m_rbNumCell2;
};

void
UETraceReception(TestAntenna3gppModelConf* test, RxPacketTraceParams params)
{
    test->UeReception(params);
}

void
TestAntenna3gppModelConf::UeReception(RxPacketTraceParams params)
{
    if (params.m_cellId == 1)
    {
        m_sinrCell1->Update(params.m_sinr);
        m_mcsCell1->Update(params.m_mcs);
        m_rbNumCell1->Update(params.m_rbAssignedNum);
    }
    else if (params.m_cellId == 2)
    {
        m_sinrCell2->Update(params.m_sinr);
        m_mcsCell2->Update(params.m_mcs);
        m_rbNumCell2->Update(params.m_rbAssignedNum);
    }
    else
    {
        NS_ABORT_MSG("Cell does not exist ... ");
    }
}

TestAntenna3gppModelConf::TestAntenna3gppModelConf(const std::string& name,
                                                   DirectionGnbUeXYAngle conf,
                                                   bool gNbOmniAntennaElem,
                                                   bool ueOmniAntennaElem,
                                                   uint8_t ueNoOfAntennas,
                                                   std::string losCondition)
    : TestCase(name)
{
    m_name = name;
    m_conf = conf;
    m_gNbOmniAntennaElem = gNbOmniAntennaElem;
    m_ueOmniAntennaElem = ueOmniAntennaElem;
    m_ueNoOfAntennas = ueNoOfAntennas;
    m_losCondition = losCondition;
    m_sinrCell1 = CreateObject<MinMaxAvgTotalCalculator<double>>();
    m_sinrCell2 = CreateObject<MinMaxAvgTotalCalculator<double>>();
    m_mcsCell1 = CreateObject<MinMaxAvgTotalCalculator<double>>();
    m_mcsCell2 = CreateObject<MinMaxAvgTotalCalculator<double>>();
    m_rbNumCell1 = CreateObject<MinMaxAvgTotalCalculator<double>>();
    m_rbNumCell2 = CreateObject<MinMaxAvgTotalCalculator<double>>();
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
TestAntenna3gppModelConf::~TestAntenna3gppModelConf()
{
}

void
TestAntenna3gppModelConf::DoRun()
{
    std::cout << "\n\n\n" << m_name << std::endl;
    // set simulation time and mobility
    Time simTime = MilliSeconds(800);
    Time udpAppStartTimeDl = MilliSeconds(400);
    Time udpAppStopTimeDl = MilliSeconds(800);
    uint32_t packetSize = 1000;
    DataRate udpRate = DataRate("2Mbps");

    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));
    Config::SetDefault("ns3::NrEpsBearer::Release", UintegerValue(15));

    // create base stations and mobile terminals
    NodeContainer gNbNodes;
    NodeContainer ueNodes;
    MobilityHelper mobility;

    double gNbHeight = 1.5;
    double ueHeight = 1.5;
    gNbNodes.Create(1);
    ueNodes.Create(1);

    Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator>();
    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();

    gNbPositionAlloc->Add(Vector(0, 0, gNbHeight));

    if (m_conf == DirectionGnbUe_45)
    {
        uePositionAlloc->Add(Vector(20, 20, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_135)
    {
        uePositionAlloc->Add(Vector(-20, 20, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_225)
    {
        uePositionAlloc->Add(Vector(-20, -20, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_315)
    {
        uePositionAlloc->Add(Vector(20, -20, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_0)
    {
        uePositionAlloc->Add(Vector(20, 0, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_90)
    {
        uePositionAlloc->Add(Vector(0, 20, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_180)
    {
        uePositionAlloc->Add(Vector(-20, 0, ueHeight));
    }
    else if (m_conf == DirectionGnbUe_270)
    {
        uePositionAlloc->Add(Vector(0, -20, ueHeight));
    }

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(gNbPositionAlloc);
    mobility.Install(gNbNodes);

    mobility.SetPositionAllocator(uePositionAlloc);
    mobility.Install(ueNodes);

    Ptr<NrPointToPointEpcHelper> nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
    // Put the pointers inside nrHelper
    idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                         TypeIdValue(CellScanBeamforming::GetTypeId()));
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);

    // set the number of antenna elements of UE
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(sqrt(m_ueNoOfAntennas)));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(sqrt(m_ueNoOfAntennas)));
    if (m_ueOmniAntennaElem)
    {
        nrHelper->SetUeAntennaAttribute("AntennaElement",
                                        PointerValue(CreateObject<IsotropicAntennaModel>()));
    }
    else
    {
        nrHelper->SetUeAntennaAttribute("AntennaElement",
                                        PointerValue(CreateObject<ThreeGppAntennaModel>()));
    }
    // set the number of antenna elements of gNbs
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    if (m_gNbOmniAntennaElem)
    {
        nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                         PointerValue(CreateObject<IsotropicAntennaModel>()));
    }
    else
    {
        nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                         PointerValue(CreateObject<ThreeGppAntennaModel>()));
    }

    // UE transmit power
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(20.0));

    // gNB transmit power
    nrHelper->SetGnbPhyAttribute("TxPower", DoubleValue(44.0));

    nrHelper->SetGnbPhyAttribute("Numerology", UintegerValue(3.0));

    nrHelper->SetEpcHelper(nrEpcHelper);

    /*
     * Spectrum division. We create two operational bands, each of them containing
     * one component carrier, and each CC containing a single bandwidth part
     * centered at the frequency specified by the input parameters.
     * Each spectrum part length is, as well, specified by the input parameters.
     * Both operational bands will use the StreetCanyon channel modeling.
     */
    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    double centralFrequency = 28e9;
    double bandwidth = 20e6;
    const uint8_t numCcPerBand = 1;
    CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, bandwidth, numCcPerBand);

    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    // Set the spectrum channel using
    channelHelper->ConfigureFactories("UMi", m_losCondition);
    // Shadowing
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    channelHelper->AssignChannelsToBands({band});
    allBwps = CcBwpCreator::GetAllBwps({band});

    uint32_t bwpIdForLowLat = 0;
    // gNb routing between Bearer and bandwidh part
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB",
                                                 UintegerValue(bwpIdForLowLat));
    // UE routing between Bearer and bandwidh part
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_LOW_LAT_EMBB", UintegerValue(bwpIdForLowLat));

    // install nr net devices
    NetDeviceContainer gNbDevs = nrHelper->InstallGnbDevice(gNbNodes, allBwps);
    NetDeviceContainer ueNetDevs = nrHelper->InstallUeDevice(ueNodes, allBwps);

    // create the internet and install the IP stack on the UEs
    // get SGW/PGW and create a single RemoteHost
    auto [remoteHost, remoteHostIpv4Address] =
        nrEpcHelper->SetupRemoteHost("100Gb/s", 2500, Seconds(0.000));

    InternetStackHelper internet;
    internet.Install(ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDevs));

    // attach UEs to the closest gNB
    nrHelper->AttachToClosestGnb(ueNetDevs, gNbDevs);

    // assign IP address to UEs, and install UDP downlink applications
    uint16_t dlPort = 1234;
    ApplicationContainer clientAppsDl;
    ApplicationContainer serverAppsDl;

    Time udpInterval =
        Time::FromDouble((packetSize * 8) / static_cast<double>(udpRate.GetBitRate()), Time::S);

    UdpServerHelper dlPacketSinkHelper(dlPort);
    serverAppsDl.Add(dlPacketSinkHelper.Install(ueNodes));

    UdpClientHelper dlClient(ueIpIface.GetAddress(0), dlPort);
    dlClient.SetAttribute("PacketSize", UintegerValue(packetSize));
    dlClient.SetAttribute("Interval", TimeValue(udpInterval));
    dlClient.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    clientAppsDl.Add(dlClient.Install(remoteHost));

    Ptr<NrEpcTft> tft = Create<NrEpcTft>();
    NrEpcTft::PacketFilter dlpf;
    dlpf.localPortStart = dlPort;
    dlpf.localPortEnd = dlPort;
    tft->Add(dlpf);

    NrEpsBearer bearer(NrEpsBearer::NGBR_LOW_LAT_EMBB);
    nrHelper->ActivateDedicatedEpsBearer(ueNetDevs.Get(0), bearer, tft);

    // start UDP server and client apps
    serverAppsDl.Start(udpAppStartTimeDl);
    clientAppsDl.Start(udpAppStartTimeDl);

    serverAppsDl.Stop(udpAppStopTimeDl);
    clientAppsDl.Stop(udpAppStopTimeDl);

    Ptr<NrSpectrumPhy> ue1SpectrumPhy = nrHelper->GetUePhy(ueNetDevs.Get(0), 0)->GetSpectrumPhy();
    ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe",
                                               MakeBoundCallback(&UETraceReception, this));

    // nrHelper->EnableTraces();
    Simulator::Stop(simTime);
    Simulator::Run();

    std::cout << serverAppsDl.GetN() << std::endl;
    Ptr<UdpServer> serverApp1 = serverAppsDl.Get(0)->GetObject<UdpServer>();
    //  double throughput1 = (serverApp1->GetReceived () * packetSize *
    //  8)/(udpAppStopTimeDl-udpAppStartTimeDl).GetSeconds ();
    double throughput1 = (serverApp1->GetReceived() * (packetSize + 28) * 8) /
                         (udpAppStopTimeDl - udpAppStartTimeDl).GetSeconds();

    std::cout << "\n UE:  " << throughput1 / 1e6 << " Mbps"
              << "\t Avg.SINR:" << 10 * log10(m_sinrCell1->getMean())
              << "\t Avg.MCS:" << m_mcsCell1->getMean()
              << "\t Avg. RB Num:" << m_rbNumCell1->getMean();

    Simulator::Destroy();
}

// The TestSuite class names the TestNrSystemTestOfdmaTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class Antenna3gppModelConfTestSuite : public TestSuite
{
  public:
    Antenna3gppModelConfTestSuite();
};

Antenna3gppModelConfTestSuite::Antenna3gppModelConfTestSuite()
    : TestSuite("nr-antenna-3gpp-model-conf", Type::SYSTEM)
{
    std::list<TestAntenna3gppModelConf::DirectionGnbUeXYAngle> conf = {
        TestAntenna3gppModelConf::DirectionGnbUe_45,
        TestAntenna3gppModelConf::DirectionGnbUe_135,
        TestAntenna3gppModelConf::DirectionGnbUe_225,
        TestAntenna3gppModelConf::DirectionGnbUe_315,
        TestAntenna3gppModelConf::DirectionGnbUe_0,
        TestAntenna3gppModelConf::DirectionGnbUe_90,
        TestAntenna3gppModelConf::DirectionGnbUe_180,
        TestAntenna3gppModelConf::DirectionGnbUe_270};

    std::list<uint8_t> ueNoOfAntennas = {16};

    std::list<std::string> losConditions = {"LOS"};

    //  std::list<TypeId> gNbantennaArrayModelTypes = {AntennaArrayModel::GetTypeId(),
    //  AntennaArray3gppModel::GetTypeId ()};
    std::list<bool> gNbOmniAntennaElement = {false, true};

    //  std::list<TypeId> ueAntennaArrayModelTypes = {AntennaArrayModel::GetTypeId(),
    //  AntennaArray3gppModel::GetTypeId ()};
    std::list<bool> ueOmniAntennaElement = {false, true};

    for (const auto& losCondition : losConditions)
    {
        for (const auto& c : conf)
        {
            for (const auto& oaaGnb : gNbOmniAntennaElement)
            {
                for (const auto& oaaUe : ueOmniAntennaElement)
                {
                    for (const auto& n : ueNoOfAntennas)
                    {
                        std::stringstream ss;
                        ss << " Test: ";

                        if (c == TestAntenna3gppModelConf::DirectionGnbUe_45)
                        {
                            ss << "DirectionGnbUe_45";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_135)
                        {
                            ss << "DirectionGnbUe_135";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_225)
                        {
                            ss << "DirectionGnbUe_225";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_315)
                        {
                            ss << "DirectionGnbUe_315";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_0)
                        {
                            ss << "DirectionGnbUe_0";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_90)
                        {
                            ss << "DirectionGnbUe_90";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_180)
                        {
                            ss << "DirectionGnbUe_180";
                        }
                        else if (c == TestAntenna3gppModelConf::DirectionGnbUe_270)
                        {
                            ss << "DirectionGnbUe_270";
                        }

                        ss << " , channelCondition: " << losCondition;

                        ss << " , UE number of antennas:" << (unsigned)n;

                        if (oaaGnb)
                        {
                            ss << " , gNB antenna element type: omni";
                        }
                        else
                        {
                            ss << " , gNB antenna element type: 3gpp";
                        }

                        if (oaaUe)
                        {
                            ss << " , UE antenna element type: omni";
                        }
                        else
                        {
                            ss << " , UE antenna element type: 3gpp";
                        }

                        AddTestCase(new TestAntenna3gppModelConf(ss.str(),
                                                                 c,
                                                                 oaaGnb,
                                                                 oaaUe,
                                                                 n,
                                                                 losCondition),
                                    Duration::QUICK);
                    }
                }
            }
        }
    }
}

// Do not forget to allocate an instance of this TestSuite
static Antenna3gppModelConfTestSuite testSuite;
