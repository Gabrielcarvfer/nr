// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

/**
 * @ingroup examples
 * @file cttc-fh-compression.cc
 * @brief A multi-cell network deployment with site sectorization
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.901. This example consists of an hexagonal grid deployment
 * consisting on a central site and a number of outer rings of sites around this
 * central site. Each site is sectorized, meaning that a number of three antenna
 * arrays or panels are deployed per gNB. These three antennas are pointing to
 * 30º, 150º and 270º w.r.t. the horizontal axis. We allocate a band to each
 * sector of a site, and the bands are contiguous in frequency.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of outer rings.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 *
 * With the default configuration, the example will create one DL flow per UE.
 * The example will print on-screen the end-to-end result of each flow,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-fh-compression --PrintHelp"
    \endcode
 *
 */

/*
 * Include part. Often, you will have to include the headers for an entire module;
 * do that by including the name of the module you need with the suffix "-module.h".
 */

#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/three-gpp-ftp-m1-helper.h"

#include <algorithm>
#include <iostream>
/*
 * To be able to use LOG_* functions.
 */
#include "ns3/log.h"

/*
 * Use, always, the namespace ns3. All the NR classes are inside such namespace.
 */
using namespace ns3;

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "FhCompression", in this way:
 *
 * $ export NS_LOG="FhCompression=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE("FhCompression");

class RadioNetworkParametersHelper
{
  public:
    /**
     * @brief Set the radio network parameters to LTE.
     * @param freqReuse The cell frequency reuse.
     */
    void SetNetworkToLte(const std::string scenario,
                         const std::string operationMode,
                         uint16_t numCcs);

    /**
     * @brief Set the radio network parameters to NR.
     * @param scenario Urban scenario (UMa or UMi).
     * @param numerology Numerology to use.
     * @param freqReuse The cell frequency reuse.
     */
    void SetNetworkToNr(const std::string scenario,
                        const std::string operationMode,
                        uint16_t numerology,
                        uint16_t numCcs);

    /**
     * @brief Gets the BS transmit power
     * @return Transmit power in dBW
     */
    double GetTxPower() const;

    /**
     * @brief Gets the operation bandwidth
     * @return Bandwidth in Hz
     */
    double GetBandwidth() const;

    /**
     * @brief Gets the central frequency
     * @return Central frequency in Hz
     */
    double GetCentralFrequency() const;

    /**
     * @brief Gets the band numerology
     * @return Numerology
     */
    uint16_t GetNumerology() const;

    /**
     * @brief Converts the maxMcsVectorInput (string) into an std::vector of maximum MCS used per
     * cell \return maxMcsVector
     */
    std::vector<int16_t> GetMcsVectorFromInput();

  private:
    double m_txPower{-1.0};          //!< Transmit power in dBm
    double m_bandwidth{0.0};         //!< System bandwidth in Hz
    double m_centralFrequency{-1.0}; //!< Band central frequency in Hz
    uint16_t m_numerology{0};        //!< Operation band numerology
};

std::vector<int16_t>
GetMcsVectorFromInput(const std::string& pattern)
{
    static std::unordered_map<std::string, int16_t> lookupTable = {
        {"1", 1},   {"2", 2},   {"3", 3},   {"4", 4},   {"5", 5},   {"6", 6},   {"7", 7},
        {"8", 8},   {"9", 9},   {"10", 10}, {"11", 11}, {"12", 12}, {"13", 13}, {"14", 14},
        {"15", 15}, {"16", 16}, {"17", 17}, {"18", 18}, {"19", 19}, {"20", 20}, {"21", 21},
        {"22", 22}, {"23", 23}, {"24", 24}, {"25", 25}, {"26", 26}, {"27", 27}, {"28", 28},
    };

    std::vector<int16_t> vector;
    std::stringstream ss(pattern);
    std::string token;
    std::vector<std::string> extracted;

    while (std::getline(ss, token, '|'))
    {
        extracted.push_back(token);
    }

    for (const auto& v : extracted)
    {
        if (lookupTable.find(v) == lookupTable.end())
        {
            NS_FATAL_ERROR("Not valid MCS input");
        }
        vector.push_back(lookupTable[v]);
    }

    return vector;
}

void
RadioNetworkParametersHelper::SetNetworkToLte(const std::string scenario,
                                              const std::string operationMode,
                                              uint16_t numCcs)
{
    NS_ABORT_MSG_IF(scenario != "UMa" && scenario != "UMi", "Unsupported scenario");

    m_numerology = 0;
    m_centralFrequency = 2e9;
    m_bandwidth = 20e6 * numCcs; // 100 RBs per CC (freqReuse)
    if (operationMode == "FDD")
    {
        m_bandwidth += m_bandwidth;
    }
    if (scenario == "UMa")
    {
        m_txPower = 43;
    }
    else
    {
        m_txPower = 30;
    }
}

void
RadioNetworkParametersHelper::SetNetworkToNr(const std::string scenario,
                                             const std::string operationMode,
                                             uint16_t numerology,
                                             uint16_t numCcs)
{
    NS_ABORT_MSG_IF(scenario != "UMa" && scenario != "UMi", "Unsupported scenario");

    m_numerology = numerology;
    m_centralFrequency = 2e9;
    m_bandwidth = 100e6 * numCcs; // 20e6 = 100 RBs per CC (freqReuse)
    if (operationMode == "FDD")
    {
        m_bandwidth += m_bandwidth;
    }
    if (scenario == "UMa")
    {
        m_txPower = 43;
    }
    else
    {
        m_txPower = 30;
    }
}

double
RadioNetworkParametersHelper::GetTxPower() const
{
    return m_txPower;
}

double
RadioNetworkParametersHelper::GetBandwidth() const
{
    return m_bandwidth;
}

double
RadioNetworkParametersHelper::GetCentralFrequency() const
{
    return m_centralFrequency;
}

uint16_t
RadioNetworkParametersHelper::GetNumerology() const
{
    return m_numerology;
}

void
Set5gLenaSimulatorParameters(HexagonalGridScenarioHelper gridScenario,
                             std::string scenario,
                             std::string radioNetwork,
                             std::string errorModel,
                             std::string operationMode,
                             std::string direction,
                             uint16_t numerology,
                             std::string pattern1,
                             std::string pattern2,
                             bool uniformPattern,
                             NodeContainer gnbSector1Container,
                             NodeContainer gnbSector2Container,
                             NodeContainer gnbSector3Container,
                             NodeContainer ueSector1Container,
                             NodeContainer ueSector2Container,
                             NodeContainer ueSector3Container,
                             Ptr<NrPointToPointEpcHelper>& baseNrEpcHelper,
                             Ptr<NrHelper>& nrHelper,
                             NetDeviceContainer& gnbSector1NetDev,
                             NetDeviceContainer& gnbSector2NetDev,
                             NetDeviceContainer& gnbSector3NetDev,
                             NetDeviceContainer& ueSector1NetDev,
                             NetDeviceContainer& ueSector2NetDev,
                             NetDeviceContainer& ueSector3NetDev,
                             int16_t maxMcsDl1,
                             int16_t maxMcsDl2,
                             std::vector<int16_t>& maxMcsVector,
                             bool uniformMcs,
                             bool uniformLambda)
{
    /*
     * Create the radio network related parameters
     */
    RadioNetworkParametersHelper ranHelper;
    if (radioNetwork == "LTE")
    {
        ranHelper.SetNetworkToLte(scenario, operationMode, 1);
        if (errorModel.empty())
        {
            errorModel = "ns3::LenaErrorModel";
        }
        else if (errorModel != "ns3::NrLteMiErrorModel" && errorModel != "ns3::LenaErrorModel")
        {
            NS_ABORT_MSG("The selected error model is not recommended for LTE");
        }
    }
    else if (radioNetwork == "NR")
    {
        ranHelper.SetNetworkToNr(scenario, operationMode, numerology, 1);
        if (errorModel.empty())
        {
            errorModel = "ns3::NrEesmIrT2";
        }
        else if (errorModel == "ns3::NrLteMiErrorModel")
        {
            NS_ABORT_MSG("The selected error model is not recommended for NR");
        }
    }
    else
    {
        NS_ABORT_MSG("Unrecognized radio network technology");
    }

    /*
     * Setup the NR module. We create the various helpers needed for the
     * NR simulation:
     * - IdealBeamformingHelper, which takes care of the beamforming part
     * - NrHelper, which takes care of creating and connecting the various
     * part of the NR stack
     * - NrChannelHelper, which takes care of the spectrum channel
     */

    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    nrHelper = CreateObject<NrHelper>();

    // Put the pointers inside nrHelper
    nrHelper->SetBeamformingHelper(idealBeamformingHelper);

    Ptr<NrPointToPointEpcHelper> nrEpcHelper =
        DynamicCast<NrPointToPointEpcHelper>(baseNrEpcHelper);
    nrHelper->SetEpcHelper(nrEpcHelper);
    /*
     * Spectrum division. We create one operational band containing three
     * component carriers, and each CC containing a single bandwidth part
     * centered at the frequency specified by the input parameters.
     * Each spectrum part length is, as well, specified by the input parameters.
     * The operational band will use StreetCanyon channel or UrbanMacro modeling.
     */
    BandwidthPartInfoPtrVector allBwps;
    BandwidthPartInfoPtrVector bwps1;
    BandwidthPartInfoPtrVector bwps2;
    BandwidthPartInfoPtrVector bwps3;
    CcBwpCreator ccBwpCreator;
    // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
    // a single BWP per CC. Get the spectrum values from the RadioNetworkParametersHelper
    double centralFrequencyBand = ranHelper.GetCentralFrequency();
    double bandwidthBand = ranHelper.GetBandwidth();
    const uint8_t numCcPerBand = 1; // In this example, each cell will have one CC with one BWP

    NS_ABORT_MSG_UNLESS(scenario == "UMa" || scenario == "UMi", "Unsupported scenario");

    // Error Model: UE and GNB with same spectrum error model.
    nrHelper->SetUlErrorModel(errorModel);
    nrHelper->SetDlErrorModel(errorModel);

    // Both DL and UL AMC will have the same model behind.
    nrHelper->SetGnbDlAmcAttribute(
        "AmcModel",
        EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
    nrHelper->SetGnbUlAmcAttribute(
        "AmcModel",
        EnumValue(NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

    /*
     * Create the necessary operation bands. In this example, each sector operates
     * in a separate band. Each band contains a single component carrier (CC),
     * which is made of one BWP in TDD operation mode or two BWPs in FDD mode.
     * Note that BWPs have the same bandwidth. Therefore, CCs and bands in FDD are
     * twice larger than in TDD.
     *
     * The configured spectrum division for TDD operation is:
     * |---Band1---|---Band2---|---Band3---|
     * |----CC1----|----CC2----|----CC3----|
     * |----BWP1---|----BWP2---|----BWP3---|
     *
     * And the configured spectrum division for FDD operation is:
     * |---------Band1---------|---------Band2---------|---------Band3---------|
     * |----------CC1----------|----------CC2----------|----------CC3----------|
     * |----BWP1---|----BWP2---|----BWP3---|----BWP4---|----BWP5---|----BWP6---|
     */
    double centralFrequencyBand1 = centralFrequencyBand - bandwidthBand;
    double centralFrequencyBand2 = centralFrequencyBand;
    double centralFrequencyBand3 = centralFrequencyBand + bandwidthBand;
    double bandwidthBand1 = bandwidthBand;
    double bandwidthBand2 = bandwidthBand;
    double bandwidthBand3 = bandwidthBand;

    uint8_t numBwpPerCc = 1;
    if (operationMode == "FDD")
    {
        numBwpPerCc = 2; // FDD will have 2 BWPs per CC
        Config::SetDefault("ns3::NrUeNetDevice::PrimaryUlIndex", UintegerValue(1));
    }

    CcBwpCreator::SimpleOperationBandConf bandConf1(centralFrequencyBand1,
                                                    bandwidthBand1,
                                                    numCcPerBand);
    bandConf1.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
    CcBwpCreator::SimpleOperationBandConf bandConf2(centralFrequencyBand2,
                                                    bandwidthBand2,
                                                    numCcPerBand);
    bandConf2.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC
    CcBwpCreator::SimpleOperationBandConf bandConf3(centralFrequencyBand3,
                                                    bandwidthBand3,
                                                    numCcPerBand);
    bandConf3.m_numBwp = numBwpPerCc; // FDD will have 2 BWPs per CC

    // By using the configuration created, it is time to make the operation bands
    OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf1);
    OperationBandInfo band2 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf2);
    OperationBandInfo band3 = ccBwpCreator.CreateOperationBandContiguousCc(bandConf3);
    Ptr<NrChannelHelper> channelHelper = CreateObject<NrChannelHelper>();
    channelHelper->ConfigureFactories(scenario);
    // Set the attributes for the channel model
    Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
    channelHelper->SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
    channelHelper->SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));
    /**
     * Set the channel model for the bands. The channel model is the same for all the bands.
     * Initialize both fading and propagation
     */
    channelHelper->AssignChannelsToBands({band1, band2, band3});
    allBwps = CcBwpCreator::GetAllBwps({band1, band2, band3});
    bwps1 = CcBwpCreator::GetAllBwps({band1});
    bwps2 = CcBwpCreator::GetAllBwps({band2});
    bwps3 = CcBwpCreator::GetAllBwps({band3});

    /*
     * Start to account for the bandwidth used by the example, as well as
     * the total power that has to be divided among the BWPs. Since there is only
     * one band and one BWP occupying the entire band, there is no need to divide
     * power among BWPs.
     */
    double totalTxPower = ranHelper.GetTxPower(); // Convert to mW
    double x = pow(10, totalTxPower / 10);

    /*
     * allBwps contains all the spectrum configuration needed for the nrHelper.
     *
     * Now, we can setup the attributes. We can have three kind of attributes:
     * (i) parameters that are valid for all the bandwidth parts and applies to
     * all nodes, (ii) parameters that are valid for all the bandwidth parts
     * and applies to some node only, and (iii) parameters that are different for
     * every bandwidth parts. The approach is:
     *
     * - for (i): Configure the attribute through the helper, and then install;
     * - for (ii): Configure the attribute through the helper, and then install
     * for the first set of nodes. Then, change the attribute through the helper,
     * and install again;
     * - for (iii): Install, and then configure the attributes by retrieving
     * the pointer needed, and calling "SetAttribute" on top of such pointer.
     *
     */

    Packet::EnableChecking();
    Packet::EnablePrinting();

    /*
     *  Case (i): Attributes valid for all the nodes
     */
    // Beamforming method
    if (radioNetwork == "LTE")
    {
        idealBeamformingHelper->SetAttribute(
            "BeamformingMethod",
            TypeIdValue(QuasiOmniDirectPathBeamforming::GetTypeId()));
    }
    else
    {
        idealBeamformingHelper->SetAttribute("BeamformingMethod",
                                             TypeIdValue(DirectPathBeamforming::GetTypeId()));
    }

    //

    // Scheduler type
    if (radioNetwork == "LTE")
    {
        nrHelper->SetSchedulerTypeId(TypeId::LookupByName("ns3::NrMacSchedulerOfdmaPF"));
        nrHelper->SetSchedulerAttribute("DlCtrlSymbols", UintegerValue(1));
    }
    // nrHelper->SetSchedulerAttribute ("MaxDlMcs", UintegerValue (maxMcsDl1));
    // nrHelper->SetSchedulerAttribute ("MaxDlMcs", UintegerValue (10));  // 27, 19, 10, 4 for
    // mcsT2,
    //  28, 16, 9 for mcsT1

    // Core latency
    nrEpcHelper->SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(1));
    nrHelper->SetUeAntennaAttribute("AntennaElement",
                                    PointerValue(CreateObject<IsotropicAntennaModel>()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
    nrHelper->SetGnbAntennaAttribute("AntennaElement",
                                     PointerValue(CreateObject<ThreeGppAntennaModel>()));

    // UE transmit power
    nrHelper->SetUePhyAttribute("TxPower", DoubleValue(20.0));

    // Set LTE RBG size
    if (radioNetwork == "LTE")
    {
        nrHelper->SetGnbMacAttribute("NumRbPerRbg", UintegerValue(4));
    }

    // We assume a common traffic pattern for all UEs
    uint32_t bwpIdForLowLat = 0;
    if (operationMode == "FDD" && direction == "UL")
    {
        bwpIdForLowLat = 1;
    }

    // gNb routing between Bearer and bandwidth part
    nrHelper->SetGnbBwpManagerAlgorithmAttribute("NGBR_VIDEO_TCP_DEFAULT",
                                                 UintegerValue(bwpIdForLowLat));

    // Ue routing between Bearer and bandwidth part
    nrHelper->SetUeBwpManagerAlgorithmAttribute("NGBR_VIDEO_TCP_DEFAULT",
                                                UintegerValue(bwpIdForLowLat));

    /*
     * We miss many other parameters. By default, not configuring them is equivalent
     * to use the default values. Please, have a look at the documentation to see
     * what are the default values for all the attributes you are not seeing here.
     */

    /*
     * Case (ii): Attributes valid for a subset of the nodes
     */

    // NOT PRESENT IN THIS SIMPLE EXAMPLE

    /*
     * We have configured the attributes we needed. Now, install and get the pointers
     * to the NetDevices, which contains all the NR stack:
     */

    //  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gridScenario.GetBaseStations (),
    //  allBwps);
    gnbSector1NetDev = nrHelper->InstallGnbDevice(gnbSector1Container, bwps1);
    gnbSector2NetDev = nrHelper->InstallGnbDevice(gnbSector2Container, bwps2);
    gnbSector3NetDev = nrHelper->InstallGnbDevice(gnbSector3Container, bwps3);
    ueSector1NetDev = nrHelper->InstallUeDevice(ueSector1Container, bwps1);
    ueSector2NetDev = nrHelper->InstallUeDevice(ueSector2Container, bwps2);
    ueSector3NetDev = nrHelper->InstallUeDevice(ueSector3Container, bwps3);

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams(gnbSector1NetDev, randomStream);
    randomStream += nrHelper->AssignStreams(gnbSector2NetDev, randomStream);
    randomStream += nrHelper->AssignStreams(gnbSector3NetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueSector1NetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueSector2NetDev, randomStream);
    randomStream += nrHelper->AssignStreams(ueSector3NetDev, randomStream);

    /*
     * Case (iii): Go node for node and change the attributes we have to setup
     * per-node.
     */
    std::vector<int16_t> maxMcsPerCell(gridScenario.GetNumCells());
    if (uniformMcs) // if uniformMcs -> same maximum MCS for all cells, input parameter maxMcsDl1
    {
        for (uint32_t i = 0; i < gridScenario.GetNumCells(); ++i)
        {
            maxMcsPerCell[i] = maxMcsDl1;
            std::cout << "Cell: " << i << " mcs (same mcs): " << maxMcsPerCell[i] << std::endl;
        }
    }
    else // Different cells may use different maxMcs (in case of different TDD patterns)
    {
        // algorithms for non-uniform max MCS
        if (uniformPattern &&
            uniformLambda) // if uniformPattern and uniformLambda --> no difference in max MCS
        {
            for (uint32_t i = 0; i < gridScenario.GetNumCells(); ++i)
            {
                maxMcsPerCell[i] = maxMcsDl1;
                std::cout << "Cell: " << i << " mcs (diff mcs): " << maxMcsPerCell[i] << std::endl;
            }
        }
        else if (uniformLambda &&
                 !uniformPattern) // case of using different TDD patterns (per cell)
        {
            for (uint32_t i = 0; i < gridScenario.GetNumCells(); ++i)
            {
                if (i % 2 == 1) // pattern1
                {
                    maxMcsPerCell[i] = maxMcsDl1;
                }
                else // pattern2
                {
                    maxMcsPerCell[i] = maxMcsDl2;
                }
                std::cout << "Cell: " << i << " mcs (diff tdd): " << maxMcsPerCell[i] << std::endl;
            }
        }
        else if (uniformPattern && !uniformLambda) // case of using different lambda (per cell)
        {
            for (uint32_t i = 0; i < gridScenario.GetNumCells(); ++i)
            {
                maxMcsPerCell[i] = maxMcsVector[i];
                std::cout << "Cell: " << i << " mcs (diff lambda): " << maxMcsPerCell[i]
                          << std::endl;
            }
        }
    }

    // Sectors (cells) of a site are pointing at different directions
    double orientationRads = gridScenario.GetAntennaOrientationRadians(0);
    uint32_t globalCellId = 0;
    for (uint32_t numCell = 0; numCell < gnbSector1NetDev.GetN(); ++numCell)
    {
        Ptr<NetDevice> gnb = gnbSector1NetDev.Get(numCell);
        uint32_t numBwps = nrHelper->GetNumberBwp(gnb);
        if (numBwps == 1) // TDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna =
                DynamicCast<UniformPlanarArray>(phy->GetSpectrumPhy()->GetAntenna());
            antenna->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));

            // Set TDD pattern
            if (uniformPattern || (globalCellId % 2 == 1))
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern1));
            }
            else
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern2));
            }

            // Set max MCS
            nrHelper->GetScheduler(gnb, 0)->SetAttribute("MaxDlMcs",
                                                         IntegerValue(maxMcsPerCell[globalCellId]));
            // nrHelper->GetGnbMac (gnb, 0)->SetAttribute ("MaxDlMcs", UintegerValue (10));  // 27,
            // 19, 10, 4 for mcsT2,
            //  28, 16, 9 for mcsT1
        }

        else if (numBwps == 2) // FDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna0 =
                DynamicCast<UniformPlanarArray>(phy0->GetSpectrumPhy()->GetAntenna());
            antenna0->SetAttribute("BearingAngle", DoubleValue(orientationRads));
            Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy(gnb, 1);
            Ptr<UniformPlanarArray> antenna1 =
                DynamicCast<UniformPlanarArray>(phy1->GetSpectrumPhy()->GetAntenna());
            antenna1->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("TxPower", DoubleValue(-30.0));
            // Set TDD pattern
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute(
                "Pattern",
                StringValue("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute(
                "Pattern",
                StringValue("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

            // Link the two FDD BWP
            nrHelper->GetBwpManagerGnb(gnb)->SetOutputLink(1, 0);
        }

        else
        {
            NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
        globalCellId++;
    }

    orientationRads = gridScenario.GetAntennaOrientationRadians(1);
    for (uint32_t numCell = 0; numCell < gnbSector2NetDev.GetN(); ++numCell)
    {
        Ptr<NetDevice> gnb = gnbSector2NetDev.Get(numCell);
        uint32_t numBwps = nrHelper->GetNumberBwp(gnb);
        if (numBwps == 1) // TDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna =
                DynamicCast<UniformPlanarArray>(phy->GetSpectrumPhy()->GetAntenna());
            antenna->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));

            // Set TDD pattern
            if (uniformPattern || (globalCellId % 2 == 1))
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern1));
            }
            else
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern2));
            }

            // Set max MCS
            nrHelper->GetScheduler(gnb, 0)->SetAttribute("MaxDlMcs",
                                                         IntegerValue(maxMcsPerCell[globalCellId]));
            // nrHelper->GetGnbMac (gnb, 0)->SetAttribute ("MaxDlMcs", UintegerValue (10));  // 27,
            // 19, 10, 4 for mcsT2,
            //  28, 16, 9 for mcsT1
        }

        else if (numBwps == 2) // FDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna0 =
                DynamicCast<UniformPlanarArray>(phy0->GetSpectrumPhy()->GetAntenna());
            antenna0->SetAttribute("BearingAngle", DoubleValue(orientationRads));
            Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy(gnb, 1);
            Ptr<UniformPlanarArray> antenna1 =
                DynamicCast<UniformPlanarArray>(phy1->GetSpectrumPhy()->GetAntenna());
            antenna1->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("TxPower", DoubleValue(-30.0));

            // Set TDD pattern
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute(
                "Pattern",
                StringValue("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute(
                "Pattern",
                StringValue("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

            // Link the two FDD BWP
            nrHelper->GetBwpManagerGnb(gnb)->SetOutputLink(1, 0);
        }

        else
        {
            NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
        globalCellId++;
    }

    orientationRads = gridScenario.GetAntennaOrientationRadians(2);
    for (uint32_t numCell = 0; numCell < gnbSector3NetDev.GetN(); ++numCell)
    {
        Ptr<NetDevice> gnb = gnbSector3NetDev.Get(numCell);
        uint32_t numBwps = nrHelper->GetNumberBwp(gnb);
        if (numBwps == 1) // TDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna =
                DynamicCast<UniformPlanarArray>(phy->GetSpectrumPhy()->GetAntenna());
            antenna->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));

            // Set TDD pattern
            if (uniformPattern || (globalCellId % 2 == 1))
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern1));
            }
            else
            {
                nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Pattern", StringValue(pattern2));
            }

            // Set max MCS
            nrHelper->GetScheduler(gnb, 0)->SetAttribute("MaxDlMcs",
                                                         IntegerValue(maxMcsPerCell[globalCellId]));
            // nrHelper->GetGnbMac (gnb, 0)->SetAttribute ("MaxDlMcs", UintegerValue (10));  // 27,
            // 19, 10, 4 for mcsT2,
            //  28, 16, 9 for mcsT1
        }

        else if (numBwps == 2) // FDD
        {
            // Change the antenna orientation
            Ptr<NrGnbPhy> phy0 = nrHelper->GetGnbPhy(gnb, 0);
            Ptr<UniformPlanarArray> antenna0 =
                DynamicCast<UniformPlanarArray>(phy0->GetSpectrumPhy()->GetAntenna());
            antenna0->SetAttribute("BearingAngle", DoubleValue(orientationRads));
            Ptr<NrGnbPhy> phy1 = nrHelper->GetGnbPhy(gnb, 1);
            Ptr<UniformPlanarArray> antenna1 =
                DynamicCast<UniformPlanarArray>(phy1->GetSpectrumPhy()->GetAntenna());
            antenna1->SetAttribute("BearingAngle", DoubleValue(orientationRads));

            // Set numerology
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("Numerology",
                                                      UintegerValue(ranHelper.GetNumerology()));

            // Set TX power
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute("TxPower", DoubleValue(10 * log10(x)));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute("TxPower", DoubleValue(-30.0));

            // Set TDD pattern
            nrHelper->GetGnbPhy(gnb, 0)->SetAttribute(
                "Pattern",
                StringValue("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));
            nrHelper->GetGnbPhy(gnb, 1)->SetAttribute(
                "Pattern",
                StringValue("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

            // Link the two FDD BWP
            nrHelper->GetBwpManagerGnb(gnb)->SetOutputLink(1, 0);
        }

        else
        {
            NS_ABORT_MSG("Incorrect number of BWPs per CC");
        }
        globalCellId++;
    }

    // Set the UE routing:

    if (operationMode == "FDD")
    {
        for (uint32_t i = 0; i < ueSector1NetDev.GetN(); i++)
        {
            nrHelper->GetBwpManagerUe(ueSector1NetDev.Get(i))->SetOutputLink(0, 1);
        }

        for (uint32_t i = 0; i < ueSector2NetDev.GetN(); i++)
        {
            nrHelper->GetBwpManagerUe(ueSector2NetDev.Get(i))->SetOutputLink(0, 1);
        }

        for (uint32_t i = 0; i < ueSector3NetDev.GetN(); i++)
        {
            nrHelper->GetBwpManagerUe(ueSector3NetDev.Get(i))->SetOutputLink(0, 1);
        }
    }
}

template <typename T>
Ptr<T>
CreateLowLatTft(uint16_t start, uint16_t end, std::string dir)
{
    Ptr<T> lowLatTft;
    lowLatTft = Create<T>();
    typename T::PacketFilter dlpfLowLat;
    if (dir == "DL")
    {
        dlpfLowLat.localPortStart = start;
        dlpfLowLat.localPortEnd = end;
        dlpfLowLat.direction = T::DOWNLINK;
    }
    else
    {
        dlpfLowLat.remotePortStart = start;
        dlpfLowLat.remotePortEnd = end;
        dlpfLowLat.direction = T::UPLINK;
    }
    lowLatTft->Add(dlpfLowLat);
    return lowLatTft;
}

template Ptr<ns3::EpcTft> CreateLowLatTft<ns3::EpcTft>(uint16_t, uint16_t, std::string);
template Ptr<ns3::NrEpcTft> CreateLowLatTft<ns3::NrEpcTft>(uint16_t, uint16_t, std::string);

int
main(int argc, char* argv[])
{
    /*
     * Variables that represent the parameters we will accept as input by the
     * command line. Each of them is initialized with a default value.
     */
    // Scenario parameters (that we will use inside this script):
    uint16_t numOuterRings = 0;
    uint16_t ueNumPergNb = 2;
    bool logging = false;
    bool traces = true;
    std::string simulator = "";
    std::string scenario = "UMi";
    std::string radioNetwork = "NR";   // LTE or NR
    std::string operationMode = "TDD"; // TDD or FDD

    // Traffic parameters (that we will use inside this script:)
    uint32_t udpPacketSize = 600; // bytes
    uint32_t lambda = 2000;       // 4000*600*8 = 19.2 Mbps/UE,
                                  // 3000*600*8 = 14.4 Mbps/UE,
                                  // 2000*600*8 = 9.6 Mbps/UE
                                  // 1500*600*8 = 7.2 Mbps/UE
                                  // 1000*600*8 = 4.8 Mbps/UE

    bool ftpM1Enabled = true;
    double ftpLambda = 5;
    uint32_t ftpFileSize = 512000; // in bytes
    uint16_t ftpPortSector1 = 2001;
    uint16_t ftpPortSector2 = 2002;
    uint16_t ftpPortSector3 = 2003;
    uint32_t ftpClientAppStartTimeMs = 400;
    uint32_t ftpServerAppStartTimeMs = 400;
    // Simulation parameters. Please don't use double to indicate seconds, use
    // milliseconds and integers to avoid representation errors.
    uint32_t simTimeMs = 1400;
    uint32_t udpAppStartTimeMs = 400;
    std::string direction = "DL";

    // Spectrum parameters. We will take the input from the command line, and then
    //  we will pass them inside the NR module.
    uint16_t numerologyBwp = 2;
    //  double centralFrequencyBand = 0.0;  // RadioNetworkParametersHelper provides this hard-coded
    //  value double bandwidthBand = 0.0;  // RadioNetworkParametersHelper provides this hard-coded
    //  values
    std::string pattern1 =
        "F|F|F|F|F|F|F|F|F|F|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
    std::string pattern2 = "F|F|F|F|F|UL|UL|UL|UL|UL|";
    bool uniformPattern = true;
    bool uniformMcs = true;
    bool uniformLambda = true;

    // Where we will store the output files.
    std::string simTag = "default";
    std::string outputDir = "./";

    // Error models
    std::string errorModel = "ns3::NrEesmIrT2";

    // Max DL MCS index
    int16_t maxMcs1 = 28;
    int16_t maxMcs2 = 28;
    // std::vector<uint16_t> maxMcsVector ={4,6,8};
    std::string maxMcsVectorInput = "1|2|4";

    /*
     * From here, we instruct the ns3::CommandLine class of all the input parameters
     * that we may accept as input, as well as their description, and the storage
     * variable.
     */
    CommandLine cmd(__FILE__);

    cmd.AddValue("scenario", "The urban scenario string (UMa or UMi)", scenario);
    cmd.AddValue("numRings", "The number of rings around the central site", numOuterRings);
    cmd.AddValue("ueNumPergNb",
                 "The number of UE per cell or gNB in multiple-ue topology",
                 ueNumPergNb);
    cmd.AddValue("logging", "Enable logging", logging);
    cmd.AddValue("traces", "Enable output traces", traces);
    cmd.AddValue("packetSize", "packet size in bytes to be used by UE traffic", udpPacketSize);
    cmd.AddValue("lambda", "Number of UDP packets generated in one second per UE", lambda);
    cmd.AddValue("uniformLambda",
                 "1: Use same lambda (packets/s) for all UEs and cells (equal to 'lambda' input), "
                 "0: use different packet arrival rates (lambdas) among cells",
                 uniformLambda);
    cmd.AddValue("simTimeMs", "Simulation time", simTimeMs);
    cmd.AddValue("numerologyBwp", "The numerology to be used (NR only)", numerologyBwp);
    cmd.AddValue("pattern1", "The TDD pattern to use", pattern1);
    cmd.AddValue("pattern2", "The TDD pattern to use", pattern2);
    cmd.AddValue("uniformPattern",
                 "1: Use same TDD pattern (pattern1) for all cells, 0: use different TDD patterns "
                 "(pattern1 and pattern2) for cells",
                 uniformPattern);
    cmd.AddValue("direction", "The flow direction (DL or UL)", direction);
    //  cmd.AddValue ("centralFrequencyBand",
    //                "The system frequency to be used in band 1",
    //                centralFrequencyBand);
    //  cmd.AddValue ("bandwidthBand",
    //                "The system bandwidth to be used in band 1",
    //                bandwidthBand);
    cmd.AddValue("technology", "The radio access network technology", radioNetwork);
    cmd.AddValue("operationMode", "The network operation mode can be TDD or FDD", operationMode);
    cmd.AddValue("simTag",
                 "tag to be appended to output filenames to distinguish simulation campaigns",
                 simTag);
    cmd.AddValue("outputDir", "directory where to store simulation results", outputDir);
    cmd.AddValue("ftpM1Enabled",
                 "An indicator whether to enable FTP Model 1 traffic model. To enable configure 1, "
                 "to disable 0.",
                 ftpM1Enabled);

    // Parse the command line
    cmd.Parse(argc, argv);

    /*
     * Check if the frequency and numerology are in the allowed range.
     * If you need to add other checks, here is the best position to put them.
     */
    //  NS_ABORT_IF (centralFrequencyBand > 100e9);
    NS_ABORT_IF(numerologyBwp > 4);
    NS_ABORT_MSG_IF(direction != "DL" && direction != "UL", "Flow direction can only be DL or UL");
    NS_ABORT_MSG_IF(operationMode != "TDD" && operationMode != "FDD",
                    "Operation mode can only be TDD or FDD");
    NS_ABORT_MSG_IF(radioNetwork != "LTE" && radioNetwork != "NR",
                    "Unrecognized radio network technology");
    /*
     * If the logging variable is set to true, enable the log of some components
     * through the code. The same effect can be obtained through the use
     * of the NS_LOG environment variable:
     *
     * export NS_LOG="UdpClient=level_info|prefix_time|prefix_func|prefix_node:UdpServer=..."
     *
     * Usually, the environment variable way is preferred, as it is more customizable,
     * and more expressive.
     */
    if (logging)
    {
        LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
        LogComponentEnable("UdpServer", LOG_LEVEL_INFO);
        LogComponentEnable("NrPdcp", LOG_LEVEL_INFO);
        //      LogComponentEnable ("NrMacSchedulerOfdma", LOG_LEVEL_ALL);
    }

    /*
     * Default values for the simulation. We are progressively removing all
     * the instances of SetDefault, but we need it for legacy code (LTE)
     */
    Config::SetDefault("ns3::NrRlcUm::MaxTxBufferSize", UintegerValue(999999999));

    /*
     * Create the scenario. In our examples, we heavily use helpers that setup
     * the gnbs and ue following a pre-defined pattern. Please have a look at the
     * GridScenarioHelper documentation to see how the nodes will be distributed.
     */
    HexagonalGridScenarioHelper gridScenario;
    gridScenario.SetSectorization(HexagonalGridScenarioHelper::TRIPLE);
    gridScenario.SetNumRings(numOuterRings);
    gridScenario.SetScenarioParameters(scenario);
    uint16_t gNbNum = gridScenario.GetNumCells();
    std::cout << "numcells: " << gNbNum << std::endl;
    uint32_t ueNum = ueNumPergNb * gNbNum;
    std::cout << "numUEs: " << ueNum << std::endl;
    gridScenario.SetUtNumber(ueNum);
    gridScenario.AssignStreams(RngSeedManager::GetRun());
    gridScenario.CreateScenario(); //!< Creates and plots the network deployment
    const uint16_t ffr =
        3; // Fractional Frequency Reuse scheme to mitigate intra-site inter-sector interferences

    /*
     * Create different gNB NodeContainer for the different sectors.
     */
    NodeContainer gnbSector1Container;
    NodeContainer gnbSector2Container;
    NodeContainer gnbSector3Container;
    for (uint32_t j = 0; j < gridScenario.GetBaseStations().GetN(); ++j)
    {
        Ptr<Node> gnb = gridScenario.GetBaseStations().Get(j);
        switch (j % ffr)
        {
        case 0:
            gnbSector1Container.Add(gnb);
            break;
        case 1:
            gnbSector2Container.Add(gnb);
            break;
        case 2:
            gnbSector3Container.Add(gnb);
            break;
        default:
            NS_ABORT_MSG("ffr param cannot be larger than 3");
            break;
        }
    }

    /*
     * Create different UE NodeContainer for the different sectors.
     */
    NodeContainer ueSector1Container;
    NodeContainer ueSector2Container;
    NodeContainer ueSector3Container;

    for (uint32_t j = 0; j < gridScenario.GetUserTerminals().GetN(); ++j)
    {
        Ptr<Node> ue = gridScenario.GetUserTerminals().Get(j);
        switch (j % ffr)
        {
        case 0:
            ueSector1Container.Add(ue);
            break;
        case 1:
            ueSector2Container.Add(ue);
            break;
        case 2:
            ueSector3Container.Add(ue);
            break;
        default:
            NS_ABORT_MSG("ffr param cannot be larger than 3");
            break;
        }
    }

    /*
     * Setup the LTE or NR module. We create the various helpers needed inside
     * their respective configuration functions
     */
    Ptr<NrPointToPointEpcHelper> nrEpcHelper;

    NetDeviceContainer gnbSector1NetDev;
    NetDeviceContainer gnbSector2NetDev;
    NetDeviceContainer gnbSector3NetDev;
    NetDeviceContainer ueSector1NetDev;
    NetDeviceContainer ueSector2NetDev;
    NetDeviceContainer ueSector3NetDev;

    Ptr<LteHelper> lteHelper;
    Ptr<NrHelper> nrHelper;

    std::vector<int16_t> maxMcsVector = GetMcsVectorFromInput(maxMcsVectorInput);

    nrEpcHelper = CreateObject<NrPointToPointEpcHelper>();
    Set5gLenaSimulatorParameters(gridScenario,
                                 scenario,
                                 radioNetwork,
                                 errorModel,
                                 operationMode,
                                 direction,
                                 numerologyBwp,
                                 pattern1,
                                 pattern2,
                                 uniformPattern,
                                 gnbSector1Container,
                                 gnbSector2Container,
                                 gnbSector3Container,
                                 ueSector1Container,
                                 ueSector2Container,
                                 ueSector3Container,
                                 nrEpcHelper,
                                 nrHelper,
                                 gnbSector1NetDev,
                                 gnbSector2NetDev,
                                 gnbSector3NetDev,
                                 ueSector1NetDev,
                                 ueSector2NetDev,
                                 ueSector3NetDev,
                                 maxMcs1,
                                 maxMcs2,
                                 maxMcsVector,
                                 uniformMcs,
                                 uniformLambda);

    // From here, it is standard NS3. In the future, we will create helpers
    // for this part as well.

    auto [remoteHost, remoteHostIpv4Address] =
        nrEpcHelper->SetupRemoteHost("100Gb/s", 2500, Seconds(0.000));
    auto remoteHostContainer = NodeContainer(remoteHost);

    InternetStackHelper internet;

    internet.Install(gridScenario.GetUserTerminals());

    Ipv4InterfaceContainer ueSector1IpIface =
        nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueSector1NetDev));
    Ipv4InterfaceContainer ueSector2IpIface =
        nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueSector2NetDev));
    Ipv4InterfaceContainer ueSector3IpIface =
        nrEpcHelper->AssignUeIpv4Address(NetDeviceContainer(ueSector3NetDev));

    // attach UEs to their gNB. Try to attach them per cellId order
    for (uint32_t u = 0; u < ueNum; ++u)
    {
        uint32_t sector = u % ffr;
        uint32_t i = u / ffr;
        if (sector == 0)
        {
            Ptr<NetDevice> gnbNetDev = gnbSector1NetDev.Get(i % gridScenario.GetNumSites());
            Ptr<NetDevice> ueNetDev = ueSector1NetDev.Get(i);
            if (lteHelper != nullptr)
            {
                lteHelper->Attach(ueNetDev, gnbNetDev);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->AttachToGnb(ueNetDev, gnbNetDev);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
            if (logging)
            {
                Vector gnbpos = gnbNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                Vector uepos = ueNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                double distance = CalculateDistance(gnbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
            }
        }
        else if (sector == 1)
        {
            Ptr<NetDevice> gnbNetDev = gnbSector2NetDev.Get(i % gridScenario.GetNumSites());
            Ptr<NetDevice> ueNetDev = ueSector2NetDev.Get(i);
            if (lteHelper != nullptr)
            {
                lteHelper->Attach(ueNetDev, gnbNetDev);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->AttachToGnb(ueNetDev, gnbNetDev);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
            if (logging)
            {
                Vector gnbpos = gnbNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                Vector uepos = ueNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                double distance = CalculateDistance(gnbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
            }
        }
        else if (sector == 2)
        {
            Ptr<NetDevice> gnbNetDev = gnbSector3NetDev.Get(i % gridScenario.GetNumSites());
            Ptr<NetDevice> ueNetDev = ueSector3NetDev.Get(i);
            if (lteHelper != nullptr)
            {
                lteHelper->Attach(ueNetDev, gnbNetDev);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->AttachToGnb(ueNetDev, gnbNetDev);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
            if (logging)
            {
                Vector gnbpos = gnbNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                Vector uepos = ueNetDev->GetNode()->GetObject<MobilityModel>()->GetPosition();
                double distance = CalculateDistance(gnbpos, uepos);
                std::cout << "Distance = " << distance << " meters" << std::endl;
            }
        }
        else
        {
            NS_ABORT_MSG("Number of sector cannot be larger than 3");
        }
    }

    /*
     * Traffic part. Install two kind of traffic: low-latency and voice, each
     * identified by a particular source port.
     */
    uint16_t dlPortLowLat = 1234;

    ApplicationContainer serverApps;

    // The sink will always listen to the specified ports
    UdpServerHelper dlPacketSinkLowLat(dlPortLowLat);

    // The server, that is the application which is listening, is installed in the UE
    if (direction == "DL")
    {
        serverApps.Add(dlPacketSinkLowLat.Install(
            {ueSector1Container, ueSector2Container, ueSector3Container}));
    }
    else
    {
        serverApps.Add(dlPacketSinkLowLat.Install(remoteHost));
    }

    /*
     * Configure attributes for the different generators, using user-provided
     * parameters for generating a CBR traffic
     *
     * Low-Latency configuration and object creation:
     */
    UdpClientHelper dlClientLowLat;
    dlClientLowLat.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));
    dlClientLowLat.SetAttribute("PacketSize", UintegerValue(udpPacketSize));
    // dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0/lambda)));

    // The bearer that will carry low latency traffic
    EpsBearer lowLatBearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
    NrEpsBearer nrLowLatBearer(NrEpsBearer::NGBR_VIDEO_TCP_DEFAULT);

    // The filter for the low-latency traffic
    Ptr<EpcTft> lowLatTft = CreateLowLatTft<EpcTft>(dlPortLowLat, dlPortLowLat, direction);
    Ptr<NrEpcTft> nrLowLatTft = CreateLowLatTft<NrEpcTft>(dlPortLowLat, dlPortLowLat, direction);

    std::vector<uint32_t> lambdaPerCell(gridScenario.GetNumCells());
    if (uniformLambda)
    {
        for (uint32_t bs = 0; bs < gridScenario.GetNumCells(); ++bs)
        {
            lambdaPerCell[bs] = lambda;
            std::cout << "Cell: " << bs << " lambda (same lambda): " << lambdaPerCell[bs]
                      << std::endl;
        }
    }
    else // non-uniform lambda values among the cells!
    {
        for (uint32_t bs = 0; bs < gridScenario.GetNumCells(); ++bs)
        {
            lambdaPerCell[bs] = 1000 + bs * 2000;
            std::cout << "Cell: " << bs << " lambda (diff lambda): " << lambdaPerCell[bs]
                      << std::endl;
        }
    }

    /*
     * Let's install the applications!
     */
    ApplicationContainer clientApps;
    ApplicationContainer ftpClientAppsSector1;
    ApplicationContainer ftpServerAppsSector1;
    ApplicationContainer ftpClientAppsSector2;
    ApplicationContainer ftpServerAppsSector2;
    ApplicationContainer ftpClientAppsSector3;
    ApplicationContainer ftpServerAppsSector3;
    Ptr<ThreeGppFtpM1Helper> ftpHelperSector1;
    Ptr<ThreeGppFtpM1Helper> ftpHelperSector2;
    Ptr<ThreeGppFtpM1Helper> ftpHelperSector3;

    if (ftpM1Enabled)
    {
        // sector 1 FTP M1 applications configuration
        ftpHelperSector1 = CreateObject<ThreeGppFtpM1Helper>(&ftpServerAppsSector1,
                                                             &ftpClientAppsSector1,
                                                             &ueSector1Container,
                                                             &remoteHostContainer,
                                                             &ueSector1IpIface);
        ftpHelperSector1->Configure(ftpPortSector1,
                                    MilliSeconds(ftpServerAppStartTimeMs),
                                    MilliSeconds(ftpClientAppStartTimeMs),
                                    MilliSeconds(simTimeMs),
                                    ftpLambda,
                                    ftpFileSize);
        ftpHelperSector1->Start();

        // sector 2 FTP M1 applications configuration
        ftpHelperSector2 = CreateObject<ThreeGppFtpM1Helper>(&ftpServerAppsSector2,
                                                             &ftpClientAppsSector2,
                                                             &ueSector2Container,
                                                             &remoteHostContainer,
                                                             &ueSector2IpIface);
        ftpHelperSector2->Configure(ftpPortSector2,
                                    MilliSeconds(ftpServerAppStartTimeMs),
                                    MilliSeconds(ftpClientAppStartTimeMs),
                                    MilliSeconds(simTimeMs),
                                    ftpLambda,
                                    ftpFileSize);
        ftpHelperSector2->Start();

        // sector 3 FTP M1 applications configuration
        ftpHelperSector3 = CreateObject<ThreeGppFtpM1Helper>(&ftpServerAppsSector3,
                                                             &ftpClientAppsSector3,
                                                             &ueSector3Container,
                                                             &remoteHostContainer,
                                                             &ueSector3IpIface);
        ftpHelperSector3->Configure(ftpPortSector3,
                                    MilliSeconds(ftpServerAppStartTimeMs),
                                    MilliSeconds(ftpClientAppStartTimeMs),
                                    MilliSeconds(simTimeMs),
                                    ftpLambda,
                                    ftpFileSize);
        ftpHelperSector3->Start();

        clientApps.Add(ftpClientAppsSector1);
        clientApps.Add(ftpClientAppsSector2);
        clientApps.Add(ftpClientAppsSector3);

        serverApps.Add(ftpServerAppsSector1);
        serverApps.Add(ftpServerAppsSector2);
        serverApps.Add(ftpServerAppsSector3);
    }
    else
    {
        for (uint32_t i = 0; i < ueSector1Container.GetN(); ++i)
        {
            dlClientLowLat.SetAttribute(
                "Interval",
                TimeValue(Seconds(1.0 / lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                                      gridScenario.GetNumSectorsPerSite()])));
            std::cout << "ue (sector1): " << i << " index: "
                      << (i % gridScenario.GetNumSites()) * gridScenario.GetNumSectorsPerSite()
                      << " lambda: "
                      << lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                       gridScenario.GetNumSectorsPerSite()]
                      << std::endl;
            Ptr<Node> ue = ueSector1Container.Get(i);
            Ptr<NetDevice> ueDevice = ueSector1NetDev.Get(i);
            Address ueAddress = ueSector1IpIface.GetAddress(i);

            // The client, who is transmitting, is installed in the remote host,
            // with destination address set to the address of the UE
            if (direction == "DL")
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(addressUtils::ConvertToSocketAddress(ueAddress, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(remoteHost));
            }
            else
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(
                        addressUtils::ConvertToSocketAddress(remoteHostIpv4Address, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(ue));
            }
            // Activate a dedicated bearer for the traffic type
            if (lteHelper != nullptr)
            {
                lteHelper->ActivateDedicatedEpsBearer(ueDevice, lowLatBearer, lowLatTft);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->ActivateDedicatedEpsBearer(ueDevice, nrLowLatBearer, nrLowLatTft);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
        }

        for (uint32_t i = 0; i < ueSector2Container.GetN(); ++i)
        {
            dlClientLowLat.SetAttribute(
                "Interval",
                TimeValue(Seconds(1.0 / lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                                          gridScenario.GetNumSectorsPerSite() +
                                                      1])));
            std::cout << "ue (sector2): " << i << " index: "
                      << (i % gridScenario.GetNumSites()) * gridScenario.GetNumSectorsPerSite() + 1
                      << " lambda: "
                      << lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                           gridScenario.GetNumSectorsPerSite() +
                                       1]
                      << std::endl;
            Ptr<Node> ue = ueSector2Container.Get(i);
            Ptr<NetDevice> ueDevice = ueSector2NetDev.Get(i);
            Address ueAddress = ueSector2IpIface.GetAddress(i);

            // The client, who is transmitting, is instaviso entonces pronto, sualled in the remote
            // host, with destination address set to the address of the UE
            if (direction == "DL")
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(addressUtils::ConvertToSocketAddress(ueAddress, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(remoteHost));
            }
            else
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(
                        addressUtils::ConvertToSocketAddress(remoteHostIpv4Address, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(ue));
            }
            // Activate a dedicated bearer for the traffic type
            if (lteHelper != nullptr)
            {
                lteHelper->ActivateDedicatedEpsBearer(ueDevice, lowLatBearer, lowLatTft);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->ActivateDedicatedEpsBearer(ueDevice, nrLowLatBearer, nrLowLatTft);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
        }

        for (uint32_t i = 0; i < ueSector3Container.GetN(); ++i)
        {
            dlClientLowLat.SetAttribute(
                "Interval",
                TimeValue(Seconds(1.0 / lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                                          gridScenario.GetNumSectorsPerSite() +
                                                      2])));
            std::cout << "ue (sector3): " << i << " index: "
                      << (i % gridScenario.GetNumSites()) * gridScenario.GetNumSectorsPerSite() + 2
                      << " lambda: "
                      << lambdaPerCell[(i % gridScenario.GetNumSites()) *
                                           gridScenario.GetNumSectorsPerSite() +
                                       2]
                      << std::endl;
            Ptr<Node> ue = ueSector3Container.Get(i);
            Ptr<NetDevice> ueDevice = ueSector3NetDev.Get(i);
            Address ueAddress = ueSector3IpIface.GetAddress(i);

            // The client, who is transmitting, is installed in the remote host,
            // with destination address set to the address of the UE
            if (direction == "DL")
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(addressUtils::ConvertToSocketAddress(ueAddress, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(remoteHost));
            }
            else
            {
                dlClientLowLat.SetAttribute(
                    "Remote",
                    AddressValue(
                        addressUtils::ConvertToSocketAddress(remoteHostIpv4Address, dlPortLowLat)));
                clientApps.Add(dlClientLowLat.Install(ue));
            }
            // Activate a dedicated bearer for the traffic type
            if (lteHelper != nullptr)
            {
                lteHelper->ActivateDedicatedEpsBearer(ueDevice, lowLatBearer, lowLatTft);
            }
            else if (nrHelper != nullptr)
            {
                nrHelper->ActivateDedicatedEpsBearer(ueDevice, nrLowLatBearer, nrLowLatTft);
            }
            else
            {
                NS_ABORT_MSG("Programming error");
            }
        }
    }

    // start UDP server and client apps
    serverApps.Start(MilliSeconds(udpAppStartTimeMs));
    clientApps.Start(MilliSeconds(udpAppStartTimeMs));
    serverApps.Stop(MilliSeconds(simTimeMs));
    clientApps.Stop(MilliSeconds(simTimeMs));

    // enable the traces provided by the nr module
    if (traces)
    {
        if (lteHelper != nullptr)
        {
            lteHelper->EnableTraces();
        }
        else if (nrHelper != nullptr)
        {
            nrHelper->EnableTraces();
        }
    }

    FlowMonitorHelper flowmonHelper;
    NodeContainer endpointNodes;
    endpointNodes.Add(remoteHost);
    endpointNodes.Add(gridScenario.GetUserTerminals());

    Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install(endpointNodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(20));

    Simulator::Stop(MilliSeconds(simTimeMs));
    Simulator::Run();

    /*
     * To check what was installed in the memory, i.e., BWPs of gNB Device, and its configuration.
     * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
    GtkConfigStore config;
    config.ConfigureAttributes ();
    */

    // Print per-flow statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();

    double averageFlowThroughput = 0.0;
    double averageFlowDelay = 0.0;
    double averageUpt = 0.0; // average user perceived throughput per file transfer

    std::ofstream outFile;
    std::string filename = outputDir + "/" + simTag;
    std::vector<double> delayValues(stats.size());
    uint64_t cont = 0;

    outFile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    if (!outFile.is_open())
    {
        std::cerr << "Can't open file " << filename << std::endl;
        return 1;
    }

    outFile.setf(std::ios_base::fixed);

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();
         i != stats.end();
         ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
        std::stringstream protoStream;
        protoStream << (uint16_t)t.protocol;
        if (t.protocol == 6)
        {
            protoStream.str("TCP");
        }
        if (t.protocol == 17)
        {
            protoStream.str("UDP");
        }
        outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " ->"
                << t.destinationAddress << ":" << t.destinationPort << ") proto "
                << protoStream.str() << "\n";
        outFile << "  Tx Packets: " << i->second.txPackets << "\n";
        outFile << "  Tx Bytes : " << i->second.txBytes << "\n ";
        outFile << "  TxOffered: "
                << (i->second.txBytes * 8.0 /
                    (i->second.timeLastTxPacket.GetSeconds() -
                     i->second.timeFirstTxPacket.GetSeconds())) /
                       1e6
                << " Mbps\n";
        outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        if (i->second.rxPackets > 0)
        {
            // Measure the duration of the flow from receiver's perspective
            double rxDuration =
                i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds();
            auto binsCount = i->second.flowInterruptionsHistogram.GetNBins();
            double rxInterruptions = 0;
            for (uint32_t bi = 0; bi < binsCount; bi++)
            {
                // interruptions threshold to count time between file transferes of the same flow
                if ((i->second.flowInterruptionsHistogram.GetBinStart(bi)) >= 0.150)
                {
                    rxInterruptions += i->second.flowInterruptionsHistogram.GetBinEnd(bi) *
                                       i->second.flowInterruptionsHistogram.GetBinCount(bi);
                }
            }
            double upt =
                ((i->second.rxBytes * 8.0) /
                 (((i->second.timeLastRxPacket - i->second.timeFirstRxPacket).GetSeconds()) -
                  rxInterruptions)) /
                1e6;
            averageUpt += upt;

            averageFlowThroughput += i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
            averageFlowDelay += 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
            delayValues[cont] = 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
            cont++;

            outFile << "  Throughput: " << (i->second.rxBytes * 8.0 / rxDuration) / 1e6
                    << " Mbps\n";
            outFile << "  Mean delay: "
                    << 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets << " ms\n";
            outFile << "  Mean jitter: "
                    << 1000 * i->second.jitterSum.GetSeconds() / i->second.rxPackets << " ms\n";
            outFile << "  Upt: " << upt << " Mbps \n";
        }
        else
        {
            outFile << "  Throughput:  0 Mbps\n";
            outFile << "  Mean delay:  0 ms\n";
            outFile << "  Mean jitter: 0 ms\n";
        }
        outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }
    std::stable_sort(delayValues.begin(), delayValues.end());
    // for (uint32_t i = 0; i < stats.size(); i++)
    //   {
    //     std::cout << delayValues[i] << " ";
    //   }
    // double FiftyTileFlowDelay = (delayValues[stats.size()/2] + delayValues[stats.size()/2 -1])/2;
    double FiftyTileFlowDelay = delayValues[stats.size() / 2];

    outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size() << "\n";
    outFile << "  Mean flow delay: " << averageFlowDelay / stats.size() << "\n";
    outFile << "  Median flow delay: " << FiftyTileFlowDelay << "\n";
    outFile << "  Mean UPT: " << averageUpt / stats.size() << " Mbps\n";
    outFile << "  Mean delay: " << averageFlowDelay / stats.size() << "\n";
    outFile << "  Median delay: " << FiftyTileFlowDelay << "\n";

    outFile.close();

    std::ifstream f(filename.c_str());

    if (f.is_open())
    {
        std::cout << f.rdbuf();
    }

    Simulator::Destroy();
    return 0;
}
