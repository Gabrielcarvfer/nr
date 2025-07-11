// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-ns3.h"
#include "nr-mac-scheduler-ue-info.h"
#include "nr-phy-mac-common.h"

#include <unordered_set>

namespace ns3
{

/**
 * @ingroup scheduler
 * @brief Schedule the HARQ retransmission
 *
 * The class manages, in a round-robin fashion, the retransmission to be
 * performed. It implements ScheduleDlHarq and ScheduleUlHarq that
 * has the same signature of the methods in NrMacSchedulerNs3. For the
 * details about the HARQ scheduling, please refer to the method documentation.
 */
class NrMacSchedulerHarqRr : public Object
{
  public:
    using Ns3Sched = NrMacSchedulerNs3;

    static TypeId GetTypeId();
    /**
     * @brief NrMacSchedulerHarqRr constructor
     */
    NrMacSchedulerHarqRr();

    /**
     * @brief Default deconstructor
     */
    virtual ~NrMacSchedulerHarqRr() = default;

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

    /**
     * @brief Install a function to retrieve the bandwidth in RBG
     * @param fn
     */
    void InstallGetBwInRBG(const std::function<uint16_t()>& fn);

    /**
     * @brief Install a function to retrieve the FH Control
     *        Method (when enabled)
     * @param fn
     */
    void InstallGetFhControlMethodFn(const std::function<uint8_t()>& fn);

    /**
     * @brief Install a function to retrieve whether the allocation
     *        fits when FH Control is enabled
     * @param fn the function
     */
    void InstallDoesFhAllocationFitFn(
        const std::function<bool(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank)>&
            fn);
    /**
     * @brief Install a function to retrieve whether the allocation
     *        fits when FH Control is enabled
     * @param fn the function
     */
    void InstallReshapeAllocation(const std::function<const std::vector<DciInfoElementTdma>(
                                      const std::vector<DciInfoElementTdma>& dcis,
                                      uint8_t& startingSymbol,
                                      uint8_t& numSymbols,
                                      std::vector<bool>& bitmask,
                                      const bool isDl)>& fn);

    /**
     * @brief Install a function to the downlink bitmask from the scheduler
     * @param fn the function
     */
    void InstallGetDlBitmask(const std::function<std::vector<bool>()>& fn);
    /**
     * @brief Install a function to the uplink bitmask from the scheduler
     * @param fn the function
     */
    void InstallGetUlBitmask(const std::function<std::vector<bool>()>& fn);

    virtual uint8_t ScheduleDlHarq(
        NrMacSchedulerNs3::PointInFTPlane* startingPoint,
        uint8_t symAvail,
        const NrMacSchedulerNs3::ActiveHarqMap& activeDlHarq,
        const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
        std::vector<DlHarqInfo>* dlHarqToRetransmit,
        const std::vector<DlHarqInfo>& dlHarqFeedback,
        SlotAllocInfo* slotAlloc) const;
    virtual uint8_t ScheduleUlHarq(
        NrMacSchedulerNs3::PointInFTPlane* startingPoint,
        uint8_t symAvail,
        const std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>& ueMap,
        std::vector<UlHarqInfo>* ulHarqToRetransmit,
        const std::vector<UlHarqInfo>& ulHarqFeedback,
        SlotAllocInfo* slotAlloc) const;
    virtual void SortDlHarq(NrMacSchedulerNs3::ActiveHarqMap* activeDlHarq) const;
    virtual void SortUlHarq(NrMacSchedulerNs3::ActiveHarqMap* activeUlHarq) const;

  protected:
    void BufferHARQFeedback(const std::vector<DlHarqInfo>& dlHarqFeedback,
                            std::vector<DlHarqInfo>* dlHarqToRetransmit,
                            uint16_t rnti,
                            uint8_t harqProcess) const;
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
     * @brief Get the bandwidth in RBG
     * @return the BW in RBG
     */
    uint16_t GetBandwidthInRbg() const;

    /**
     * @brief Get the FH Control method.
     * @return the FH Control Method (uint8_t)
     */
    uint8_t GetFromSchedFhControlMethod() const;

    /**
     * @brief Get from sched if the allocation fits when FH Control is enabled
     * @return whether the allocation fits
     */
    bool GetDoesFhAllocationFit(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank) const;

  private:
    /**
     * @brief Orders active beams of associated HARQ processes following round-robin order
     * @return vector of BeamIds in round-robin order
     */
    std::vector<BeamId> GetBeamOrderRR(NrMacSchedulerNs3::ActiveHarqMap activeHarqMap) const;

    std::function<uint16_t()> m_getBwpId;          //!< Function to retrieve bwp id
    std::function<uint16_t()> m_getCellId;         //!< Function to retrieve cell id
    std::function<uint16_t()> m_getBwInRbg;        //!< Function to retrieve bw in rbg
    std::function<uint8_t()> m_getFhControlMethod; //!< Function to retrieve the FH Control Method
    std::function<bool(uint16_t bwpId, uint32_t mcs, uint32_t nRegs, uint8_t dlRank)>
        m_getDoesAllocationFit; //!< Function to retrieve if allocation fits
    std::function<std::vector<DciInfoElementTdma>(const std::vector<DciInfoElementTdma>& dcis,
                                                  uint8_t& startingSymbol,
                                                  uint8_t& numSymbols,
                                                  std::vector<bool>& bitmask,
                                                  const bool isDl)>
        m_getReshapeAllocation; //!< Function to reshape allocation to maximize MCS and reduce
                                //!< number of symbols
    std::function<std::vector<bool>()> m_getDlBitmask; //!< Retrieve DL bitmask from scheduler
    std::function<std::vector<bool>()> m_getUlBitmask; //!< Retrieve UL bitmask from scheduler

    mutable std::deque<BeamId> m_rrBeams; //!< Queue of order of beams to transmit
    mutable std::unordered_set<BeamId, BeamIdHash> m_rrBeamsSet; //!< Set of known beams
    bool m_consolidateHarqRetx; //!< Flag configured by attribute ConsolidateHarqRetx
};

} // namespace ns3
