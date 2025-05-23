// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-ue-info.h"
#include "nr-phy-mac-common.h"

#include <memory>

namespace ns3
{

class NrAmc;

/**
 * @ingroup scheduler
 * @brief CQI management for schedulers.
 *
 * The scheduler will call either DlCqiReported to calculate
 * a new DL MCS. For UL, only the method UlSBCQIReported is implemented,
 * and it is a bit more complicated. For any detail, check the respective
 * documentation.
 *
 * @see UlSBCQIReported
 * @see DlCqiReported
 */
class NrMacSchedulerCQIManagement
{
  public:
    /**
     * @brief NrMacSchedulerCQIManagement default constructor
     */
    NrMacSchedulerCQIManagement() = default;

    /**
     * @brief NrMacSchedulerCQIManagement copy constructor (deleted)
     * @param o other instance
     */
    NrMacSchedulerCQIManagement(const NrMacSchedulerCQIManagement& o) = delete;
    /**
     * @brief Deconstructor
     */
    ~NrMacSchedulerCQIManagement() = default;

    /**
     * @brief Install a function to retrieve the bwp id
     * @param fn the function
     */
    void InstallGetBwpIdFn(const std::function<uint16_t()>& fn);

    /**
     * @brief Install a function to retrieve the cell id
     * @param fn the function
     */
    void InstallGetCellIdFn(const std::function<uint16_t()>& fn);

    void InstallGetStartMcsDlFn(const std::function<uint8_t()>& fn);

    void InstallGetStartMcsUlFn(const std::function<uint8_t()>& fn);

    void InstallGetNrAmcDlFn(const std::function<Ptr<const NrAmc>()>& fn);

    void InstallGetNrAmcUlFn(const std::function<Ptr<const NrAmc>()>& fn);

    /**
     * @brief A wideband CQI has been reported for the specified UE
     * @param info WB CQI
     * @param ueInfo UE
     * @param expirationTime expiration time of the CQI in number of slot
     * @param maxDlMcs maximum DL MCS index
     * @param bandwidthInRbgs number of RBGs in the channel
     *
     * Store the CQI information inside the m_dlCqi value of the UE, and then
     * calculate the corresponding MCS through NrAmc. The information is
     * contained in the structure CqiInfo, so no need to make calculation
     * here.
     */
    void DlCqiReported(const DlCqiInfo& info,
                       const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
                       uint32_t expirationTime,
                       int8_t maxDlMcs,
                       uint16_t bandwidthInRbgs) const;

    /**
     * @brief An UL SB CQI has been reported for the specified UE
     * @param expirationTime expiration time (in slot) of the CQI value
     * @param tbs TBS of the allocation
     * @param params parameters of the received CQI
     * @param ueInfo UE info
     * @param rbgMask RBG mask
     * @param numRbPerRbg How many RB do we have per RBG
     * @param model SpectrumModel to calculate the CQI
     *
     * To calculate the UL MCS, is necessary to remember the allocation done to
     * be able to retrieve the number of symbols and the TBS assigned. This is
     * done inside the class NrMacSchedulerNs3, and here we assume correct
     * parameters as input.
     *
     * From a vector of SINR (along the entire band) a SpectrumValue is calculated
     * and then passed as input to NrAmc::CreateCqiFeedbackSiso. From this
     * function, we have as a result an updated value of CQI, as well as an updated
     * version of MCS for the UL.
     */
    void UlSBCQIReported(uint32_t expirationTime,
                         uint32_t tbs,
                         const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params,
                         const std::shared_ptr<NrMacSchedulerUeInfo>& ueInfo,
                         const std::vector<bool>& rbgMask,
                         uint32_t numRbPerRbg,
                         const Ptr<const SpectrumModel>& model) const;

    /**
     * @brief Refresh the DL CQI for all the UE
     *
     * This method should be called every slot.
     * Decrement the validity counter DL CQI, and if a CQI expires, reset its
     * value to the default (MCS 0)
     *
     * @param m_ueMap UE map
     */
    void RefreshDlCqiMaps(
        const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& m_ueMap) const;

    /**
     * @brief Refresh the UL CQI for all the UE
     *
     * This method should be called every slot.
     * Decrement the validity counter UL CQI, and if a CQI expires, reset its
     * value to the default (MCS 0)
     *
     * @param m_ueMap UE map
     */
    void RefreshUlCqiMaps(
        const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& m_ueMap) const;

  private:
    /**
     * @brief Get the bwp id of this MAC
     * @return the bwp id
     */
    uint16_t GetBwpId() const;

    /**
     * @brief Get the cell id of this MAC
     * @return the cell id
     */
    uint16_t GetCellId() const;

    /**
     * @return the starting MCS for DL
     */
    uint8_t GetStartMcsDl() const;

    /**
     * @return the starting MCS for UL
     */
    uint8_t GetStartMcsUl() const;

    /**
     * @return the AMC for DL
     */
    Ptr<const NrAmc> GetAmcDl() const;

    /**
     * @return the AMC for UL
     */
    Ptr<const NrAmc> GetAmcUl() const;

    std::function<uint16_t()> m_getBwpId;         //!< Function to retrieve bwp id
    std::function<uint16_t()> m_getCellId;        //!< Function to retrieve cell id
    std::function<uint8_t()> m_getStartMcsDl;     //!< Function to retrieve the starting MCS for DL
    std::function<uint8_t()> m_getStartMcsUl;     //!< Function to retrieve the starting MCS for UL
    std::function<Ptr<const NrAmc>()> m_getAmcDl; //!< Function to retrieve the AMC for DL
    std::function<Ptr<const NrAmc>()> m_getAmcUl; //!< Function to retrieve the AMC for UL
};

} // namespace ns3
