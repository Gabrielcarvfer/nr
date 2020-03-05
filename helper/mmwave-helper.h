/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#ifndef MMWAVE_HELPER_H
#define MMWAVE_HELPER_H

#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/eps-bearer.h>
#include <ns3/object-factory.h>
#include <ns3/mmwave-bearer-stats-connector.h>
#include <ns3/mmwave-control-messages.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/mmwave-spectrum-phy.h>
#include "ideal-beamforming-helper.h"

namespace ns3 {

class MmWaveUePhy;
class MmWaveEnbPhy;
class SpectrumChannel;
class MmWaveSpectrumValueHelper;
class MmWaveEnbMac;
class EpcHelper;
class EpcTft;
class MmWaveBearerStatsCalculator;
class MmwaveMacRxTrace;
class MmWavePhyRxTrace;
class MmWavePhyMacCommon;
class ComponentCarrierEnb;
class ComponentCarrier;
class MmWaveMacScheduler;
class MmWaveEnbNetDevice;
class MmWaveUeMac;

enum OperationMode
{
  TDD,
  FDD
};


enum ComponentCarrierState
{
  PRIMARY = 1,
  SECONDARY = 2
};

/**
 * Bandwidth part configuration information
 */
//struct BandwidthPartInfo : public SimpleRefCount<BandwidthPartInfo>
struct BandwidthPartInfo : public Object
{
//  virtual ~BandwidthPartInfo() = default;  //<! Virtual destructor needed to enable polymorphism

  uint8_t m_bwpId {0};             //<! BWP id
  uint8_t m_numerology {0};        //<! BWP numerology: 0,1,2,3,4
  double m_centralFrequency {0};   //<! BWP central frequency
  double m_lowerFrequency {0};     //<! BWP lower frequency
  double m_higherFrequency {0};    //<! BWP higher frequency
  uint32_t m_bandwidth {0};        //<! BWP bandwidth
  ComponentCarrierState m_cc {SECONDARY}; //<! Tells if the associated CC is primary or secondary
};


/**
 * TDD BWP configuration information
 */
struct BandwidthPartInfoTdd : public BandwidthPartInfo
{
//  ~BandwidthPartInfoTdd ();

  std::vector<LteNrTddSlotType> m_tddPattern;
};


/**
 * FDD BWP configuration information
 */
struct BandwidthPartInfoFdd : public BandwidthPartInfo
{
//  ~BandwidthPartInfoFdd ();
  /**
   * For FDD patterns, the BWP should be configured for either DL or UL
   */
  enum flowDirection : uint8_t
  {
    DL = 0,
    UL = 1
  };

  enum flowDirection m_direction;   //<! Flow direction of the BWP
  std::vector<uint8_t> m_linkedBwps;  //<! Vector of BWPs associated to this BWP configuration but in the reverse transmission direction
};



/**
 * Component carrier configuration element
 */
struct ComponentCarrierInfo
{
  uint8_t m_ccId {0};          //<! CC id
  uint8_t m_numBwps {0};       //<! Number of BWP in the carrier
  uint8_t m_activeBwp;         //<! Active BWP index
  OperationMode m_mode {OperationMode::TDD};    //<! Sets the carrier operation mode (TDD or FDD)
  double m_centralFrequency;   //<! BWP central frequency
  double m_lowerFrequency;     //<! BWP lower frequency
  double m_higherFrequency;    //<! BWP higher frequency
  uint32_t m_bandwidth;        //<! BWP bandwidth
  ComponentCarrierState m_primaryCc {ComponentCarrierState::PRIMARY};  //<! Primary or secondary CC
  std::map<uint8_t, Ptr<BandwidthPartInfo>> m_bwp;  //<! Space for BWP

  /**
   * \brief Adds a BWP to the carrier
   * \param bwp Description of the BWP
   */
  void AddBwp (Ptr<BandwidthPartInfo> bwp);
  void AddBwp (Ptr<BandwidthPartInfoTdd> bwp);
  void AddBwp (Ptr<BandwidthPartInfoFdd> bwp);
};


/**
 * Carrier aggregation contiguous allocation mode within an operation band
 */
enum ContiguousMode
{
  CONTIGUOUS,
  NON_CONTIGUOUS
};


/*
 * Upper limits of the number of component carriers used for
 * Carrier Aggregation depends on the CC contiguousness.
 * Eventually, the number of CCs may depend on the operation frequency
 */
const uint8_t MAX_CC_INTRA_BAND = 8;  //<! Up to 8 CCs can be aggregated in the same operation band
const uint8_t MAX_CC_INTER_BAND = 16; //<! The maximum number of aggregated CCs is 16 in NR Rel. 16 (in more than one operation band)

/**
 * \brief The operation band information structure
 */
struct OperationBandInfo
{
  uint8_t m_bandId {0};       //<! Operation band id
  double m_centralFrequency;  //<! Operation band central frequency
  double m_lowerFrequency;    //<! Operation band lower frequency
  double m_higherFrequency;   //<! Operation band higher frequency
  uint32_t m_bandwidth;       //<! Operation band bandwidth
  uint8_t m_numCarriers {0};  //<! Number of configured carriers in the operation band
  ContiguousMode m_contiguousCc {ContiguousMode::CONTIGUOUS};  //<! CA intra-band contiguousness
  std::map<uint8_t, ComponentCarrierInfo> m_cc;

  void AddCc (const ComponentCarrierInfo &cc);
  void AddCc (uint8_t ccId, const ComponentCarrierInfo &cc);
};



/**
 * \brief Manages the correct creation of operation bands, component carriers and bandwidth parts
 */
class ComponentCarrierBandwidthPartCreator
{
public:
  ComponentCarrierBandwidthPartCreator ();
  ComponentCarrierBandwidthPartCreator (uint8_t maxNumBands);
  virtual ~ComponentCarrierBandwidthPartCreator ();

  ComponentCarrierBandwidthPartCreator& operator= (const ns3::ComponentCarrierBandwidthPartCreator&);


  /**
   * \brief Creates an operation band by splitting the available bandwidth into equally-large contiguous carriers. Carriers will have common parameters like numerology
   * \param centralFrequency Central operation frequency in Hz
   * \param operationBandwidth Operation band bandwidth
   * \param numCCs Number of contiguous CC
   * \param mode The carriers' operation mode can be TDD or FDD. All the same.
   */
  void CreateOperationBandContiguousCc (double centralFrequency,
                                        uint32_t operationBandwidth,
                                        uint8_t numCCs,
                                        uint8_t numerology,
                                        OperationMode mode);

  /**
   * \brief Creates an operation band with the desired central frequency and bandwidth with no CC information
   * \param centralFrequency The central frequency of the operation band
   * \param operationBandwidth The operation band bandwidth
   */
  OperationBandInfo CreateEmptyOperationBand (double centralFrequency, uint32_t operationBandwidth);

  /**
   * \brief Creates an operation band with the desired central frequency and bandwidth, with a single CC occupying the whole operation band
   * \param centralFrequency The central frequency of the operation band
   * \param operationBandwidth The operation band bandwidth
   */
  OperationBandInfo CreateOperationBand (double centralFrequency, uint32_t operationBandwidth);

  /**
   * \brief Adds the operation band to the class
   * \param band Description of the operation band
   */
  void AddOperationBand (const OperationBandInfo &bandInfo);

  /**
   * \brief Performs some validation checks on the provided operation band configuration and its child CC and BWP structures
   * \param band Operation band parameters
   */
  void ValidateOperationBand (OperationBandInfo &band);

  /**
   * \brief Checks the consistency of BWP within the carrier
   * \note Simulation will stop if a bad configuration is found
   * \param cc Component Carrier definition
   */
  void CheckBwpsInCc (ComponentCarrierInfo &cc);

  /**
   * \brief Validates the CA/BWP configuration
   */
  void ValidateCaBwpConfiguration ();

  /**
   * \brief Determines whether the CCs in the band are contiguous or not based on a intra-band CC frequency offset
   * \param band Operation band information structure
   * \param freqSeparation Maximum separation between contiguous CCs in Hz
   * \returns Flag indicating contiguous or non-contiguous CCs for the given band
   */
  ContiguousMode GetCcContiguousnessState (OperationBandInfo &band, uint32_t freqSeparation);

  /**
   * \brief Gets the BandwidthPartInfo struct of the active BWP of the primary CC
   * \return The active BWP information object
   */
  Ptr<BandwidthPartInfo> GetActiveBwpInfo ();

  /**
   * \brief Gets the BandwidthPartInfo struct of the active BWP of the provided band and carrier index
   * \param bandIndex Operation band id
   * \param ccIndex Component carrier id
   * \return The active BWP information object
   */
  Ptr<BandwidthPartInfo> GetActiveBwpInfo (uint8_t bandIndex, uint8_t ccIndex);

  /**
   * \brief Gets the ComponentCarrierInfo struct associated to the provided operation band and carrier indices
   * \param bandIndex Operation band id
   * \param ccIndex Component carrier id
   * \return The desired CC information object
   */
  ComponentCarrierInfo GetComponentCarrier (uint8_t bandId, uint8_t ccId);

  /**
   * \brief Returns the aggregated bandwidth in the CA configuration, considering the active BWP
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetAggregatedBandwidth ();

  /**
   * \brief Iterates along the operation bands and returns the active BWP of the given CC id
   * \param ccId CC id (position in the operation band's internal vector of CCs)
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetCarrierBandwidth (uint8_t ccId);


  /**
   * \brief Returns the bandwidth of the BWP identified by its unique id
   * \params bwpId The BWP id
   */
  uint32_t GetBwpBandwidth (uint8_t bwpId);

  /**
   * \brief Returns the active BWP of the given CC in the given operation band
   * \param bandId Operation band id (position in the class internal vector of operation bands)
   * \param ccId CC id (position in the operation band's internal vector of CCs)
   * \return Aggregated bandwidth in Hz
   */
  uint32_t GetCarrierBandwidth (uint8_t bandId, uint8_t ccId);

  /**
   * \brief Change the active BWP of a given UE operating in the given ccId
   * \param bandId Operation band id containing the CC/BWP to set to active
   * \param ccId Component carrier id which BWP is to set to active
   * \param activeBwpId BWP id to set to active
   */
  void ChangeActiveBwp (uint8_t bandId, uint8_t ccId, uint8_t activeBwpId);

  /**
   * \brief Get a vector with all the created BWP
   * \param Vector of pointers the function will populate on
   */
  void GetConfiguredBwp (std::vector<Ptr<BandwidthPartInfo>> &bwpList);

  /**
   * \brief Plots the CA/BWP configuration using GNUPLOT. There must be a valid
   * configuration
   */
  void PlotNrCaBwpConfiguration (const std::string &filename);


  /**
   * \brief Plots the CA/BWP configuration using GNUPLOT. There must be a valid
   * configuration
   */
  void PlotLteCaConfiguration (const std::string &filename);

private:
  uint32_t m_id {0};       //!< UE/flow/bearer id
  uint8_t m_maxBands {1};  //!< Limit the number of operation bands
  uint8_t m_numBands {0};  //!< Number of current operation bands. It must be smaller or equal than m_maxBands
  uint8_t m_numBwps {0};   //!< Number of BWP created. Consider removing
  uint8_t m_numCcs {0};    //!< Number of Component Carriers created
  std::vector<OperationBandInfo> m_bands;  //!< Vector to the operation band information elements

  /**
   * \brief Plots a 2D rectangle defined by the input points and places a label
   */
  void PlotFrequencyBand (std::ofstream &outFile,
                      uint16_t index,
                      double xmin,
                      double xmax,
                      double ymin,
                      double ymax,
                      const std::string &label);
};


class BandwidthPartRepresentation
{
public:
  BandwidthPartRepresentation (uint32_t id, const Ptr<MmWavePhyMacCommon> &phyMacCommon,
                               const Ptr<SpectrumChannel> &channel,
                               const Ptr<ThreeGppPropagationLossModel> &propagation,
                               const Ptr<ThreeGppSpectrumPropagationLossModel> & spectrumPropagation);
  BandwidthPartRepresentation (const BandwidthPartRepresentation & o);
  ~BandwidthPartRepresentation ();

  BandwidthPartRepresentation& operator= (const ns3::BandwidthPartRepresentation&);

  uint32_t m_id {0};
  Ptr<MmWavePhyMacCommon> m_phyMacCommon;
  Ptr<SpectrumChannel> m_channel;
  Ptr<ThreeGppPropagationLossModel> m_propagation;
  Ptr<ThreeGppSpectrumPropagationLossModel> m_3gppChannel;
  std::string m_gnbChannelAccessManagerType {"ns3::NrAlwaysOnAccessManager"}; //!< Channel access manager type for GNB
  std::string m_ueChannelAccessManagerType {"ns3::NrAlwaysOnAccessManager"}; //!< Channel access manager type for UE
  std::vector<LteNrTddSlotType> m_pattern {LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
                                           LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F,
                                           LteNrTddSlotType::F, LteNrTddSlotType::F, LteNrTddSlotType::F};
};

class MmWaveHelper : public Object
{

public:
  MmWaveHelper (void);
  virtual ~MmWaveHelper (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);

  NetDeviceContainer InstallUeDevice (NodeContainer c);
  NetDeviceContainer InstallEnbDevice (NodeContainer c);

  void ConfigureCarriers (std::map<uint8_t, Ptr<ComponentCarrierEnb> > ccPhyConf);

  void SetChannelModelType (std::string type);

  /**
   * \brief Get the number of configured BWP for a specific GNB NetDevice
   * \param gnbDevice The GNB NetDevice
   * \return the number of BWP installed, or 0 if there are errors
   */
  static uint32_t GetNumberBwp (const Ptr<const NetDevice> &gnbDevice);
  /**
   * \brief Get a pointer to the PHY of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the PHY layer of the GNB, or nullptr if there are errors
   */
  static Ptr<MmWaveEnbPhy> GetEnbPhy (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);
  /**
   * \brief Get a pointer to the MAC of the GNB at the specified BWP
   * \param gnbDevice The GNB NetDevice
   * \param bwpIndex The index of the BWP required
   * \return A pointer to the MAC layer of the GNB, or nullptr if there are errors
   */
  static Ptr<MmWaveEnbMac> GetEnbMac (const Ptr<NetDevice> &gnbDevice, uint32_t bwpIndex);

  /**
   * This method is used to send the ComponentCarrier map created with CcHelper
   * to the helper, the structure will be used within InstallSingleEnbDevice
   *
   * \param ccmap the component carrier map
   */
  void SetCcPhyParams (const std::map<uint8_t, ComponentCarrier> &ccmap);

  void AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices);
  void AttachToEnb (const Ptr<NetDevice> &ueDevice, const Ptr<NetDevice> &gnbDevice);

  void EnableTraces ();

  void ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer);
  void ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer);
  void SetEpcHelper (Ptr<EpcHelper> epcHelper);
  void SetIdealBeamformingHelper (Ptr<IdealBeamformingHelper> idealBeamformingHelper);

  void SetHarqEnabled (bool harqEnabled);
  bool GetHarqEnabled ();
  void SetSnrTest (bool snrTest);
  bool GetSnrTest ();
  void AddBandwidthPart (uint32_t id, const BandwidthPartRepresentation &bwpRepr);

  /**
   * Activate a dedicated EPS bearer on a given set of UE devices.
   *
   * \param ueDevices the set of UE devices
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   * Activate a dedicated EPS bearer on a given UE device.
   *
   * \param ueDevice the UE device
   * \param bearer the characteristics of the bearer to be activated
   * \param tft the Traffic Flow Template that identifies the traffic to go on this bearer.
   * \returns bearer ID
   */
  uint8_t ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<EpcTft> tft);

  /**
   *  \brief Manually trigger dedicated bearer de-activation at specific simulation time
   *  \param ueDevice the UE on which dedicated bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  \warning Requires the use of EPC mode. See SetEpcHelper() method.
   */

  void DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbMacAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeSpectrumAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUeChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbChannelAccessManagerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetSchedulerAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetUePhyAttribute (const std::string &n, const AttributeValue &v);

  /**
   * Set an attribute for the <> to be created.
   *
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetGnbPhyAttribute (const std::string &n, const AttributeValue &v);

protected:
  /**
   * \brief Initialize things inside the helper.
   *
   * The most important thing is the channel and the propagation loss model
   * for each bandwidth part. If they are not specified by the user through
   * AddBandwidthPart, one will created by default. If the user specifies
   * the channel and the propagation model as bwp configuration, they will be
   * not touched. Otherwise, the models will be created and connected for each
   * bwp.
   */
  virtual void DoInitialize ();

private:
  /**
   *  \brief The actual function to trigger a manual bearer de-activation
   *  \param ueDevice the UE on which bearer to be de-activated must be of the type LteUeNetDevice
   *  \param enbDevice eNB, must be of the type LteEnbNetDevice
   *  \param bearerId Bearer Identity which is to be de-activated
   *
   *  This method is normally scheduled by DeActivateDedicatedEpsBearer() to run at a specific
   *  time when a manual bearer de-activation is desired by the simulation user.
   */
  void DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId);

  Ptr<MmWaveEnbPhy> CreateGnbPhy (const Ptr<Node> &n, const BandwidthPartRepresentation& conf,
                                   const Ptr<MmWaveEnbNetDevice> &dev, uint16_t cellId,
                                  const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyEndCtrlCallback);
  Ptr<MmWaveMacScheduler> CreateGnbSched (const BandwidthPartRepresentation& conf);
  Ptr<MmWaveEnbMac> CreateGnbMac (const BandwidthPartRepresentation& conf);

  Ptr<MmWaveUeMac> CreateUeMac () const;
  Ptr<MmWaveUePhy> CreateUePhy (const Ptr<Node> &n, const BandwidthPartRepresentation &conf,
                                const MmWaveSpectrumPhy::MmWavePhyDlHarqFeedbackCallback &dlHarqCallback,
                                const MmWaveSpectrumPhy::MmWavePhyRxCtrlEndOkCallback &phyRxCtrlCallback);

  Ptr<NetDevice> InstallSingleUeDevice (Ptr<Node> n);
  Ptr<NetDevice> InstallSingleEnbDevice (Ptr<Node> n);
  void AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices);
  void EnableDlPhyTrace ();
  void EnableUlPhyTrace ();
  void EnableEnbPacketCountTrace ();
  void EnableUePacketCountTrace ();
  void EnableTransportBlockTrace ();
  void EnableRlcTraces (void);
  void EnableEnbPhyCtrlMsgsTraces (void);
  void EnableUePhyCtrlMsgsTraces (void);
  void EnableEnbMacCtrlMsgsTraces (void);
  void EnableUeMacCtrlMsgsTraces (void);
  Ptr<MmWaveBearerStatsCalculator> GetRlcStats (void);
  void EnablePdcpTraces (void);
  Ptr<MmWaveBearerStatsCalculator> GetPdcpStats (void);

  std::map<uint8_t, ComponentCarrier> GetBandwidthPartMap ();

  std::string m_channelModelType;

  ObjectFactory m_enbNetDeviceFactory;  //!< NetDevice factory for gnb
  ObjectFactory m_ueNetDeviceFactory;   //!< NetDevice factory for ue
  ObjectFactory m_channelFactory;       //!< Channel factory
  ObjectFactory m_phyMacCommonFactory;  //!< PhyMacCommon factory
  ObjectFactory m_ueMacFactory;         //!< UE MAC factory
  ObjectFactory m_gnbMacFactory;        //!< GNB MAC factory
  ObjectFactory m_ueSpectrumFactory;    //!< UE Spectrum factory
  ObjectFactory m_gnbSpectrumFactory;   //!< GNB spectrum factory
  ObjectFactory m_uePhyFactory;         //!< UE PHY factory
  ObjectFactory m_gnbPhyFactory;        //!< GNB PHY factory
  ObjectFactory m_ueChannelAccessManagerFactory; //!< UE Channel access manager factory
  ObjectFactory m_gnbChannelAccessManagerFactory; //!< GNB Channel access manager factory
  ObjectFactory m_schedFactory;         //!< Scheduler factory


  uint64_t m_imsiCounter;
  uint16_t m_cellIdCounter;

  Ptr<MmWavePhyRxTrace> m_phyStats;
  Ptr<MmwaveMacRxTrace> m_macStats;

  Ptr<EpcHelper> m_epcHelper;
  Ptr<IdealBeamformingHelper> m_idealBeamformingHelper {nullptr};

  bool m_harqEnabled;
  bool m_snrTest;

  Ptr<MmWaveBearerStatsCalculator> m_rlcStats;
  Ptr<MmWaveBearerStatsCalculator> m_pdcpStats;
  MmWaveBearerStatsConnector m_radioBearerStatsConnector;

  bool m_initialized {false}; //!< Is helper initialized correctly?

  /**
   * This contains all the information about each component carrier
   */
  std::map<uint8_t, ComponentCarrier> m_componentCarrierPhyParams;

  std::unordered_map<uint32_t, BandwidthPartRepresentation> m_bwpConfiguration;
  std::string m_scenario;  //!< Important parameter that specifies the type of propagation loss and condition model types
};

}

#endif /* MMWAVE_HELPER_H */

