// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_MAC_SCHEDULER_SRS_H
#define NR_MAC_SCHEDULER_SRS_H

#include "nr-mac-scheduler-ue-info.h"

namespace ns3
{

/**
 * @ingroup scheduler
 * @brief MAC scheduler SRS interface
 *
 * This class is an interface for various SRS periodicity algorithms. An
 * algorithm must assign a periodicity and an offset to a newly added UE. At
 * this moment, the constraint is that two (or more) UEs cannot send the SRS in
 * the same slot.
 *
 * @section nr_mac_srs SRS management: how it works in the standard
 *
 * The SRS periodicity and offset are set by the RRC layer, and communicated
 * to the UE. In the NR standard, complex operations as frequency-hopping SRS
 * are also defined. Luckily, there is also another opportunity, which is a
 * scheduler-based SRS. The GNB informs the UE (through a DCI format 2_3) of the
 * resources that are available to such UE to transmit its SRS.
 *
 * @section nr_mac_srs_ns3 SRS management in the NR module: how it is modeled
 *
 * Deciding the SRS offset and periodicity at RRC would involve the scheduler
 * as well, because the scheduler must not schedule any data that would be
 * on the same resources as the expected SRS (DL or UL). Hence, implementing
 * the decision at RRC would have lead to the complexity of modifying RRC
 * plus the complexity of informing the scheduler of such decision, including
 * multiple SAP interface modifications to allow intra-layer communication.
 *
 * Therefore, we went for implementing such decision inside the scheduler,
 * which will create a DCI format 2_3 to inform the UE about its scheduled
 * time for sending SRS. Note that this reuses most of the structures and code
 * used for data scheduling, and so it takes into account the various
 * L1L2 latency plus the K latencies.
 *
 * This interface will be used by the scheduler to ask the offset/periodicity
 * for a UE, and various implementation can be written to simulate different
 * algorithms.
 *
 * Note: This interface assumes that all the UEs will share the same periodicity.
 * If that's not the case, the API would have to be updated.
 *
 */
class NrMacSchedulerSrs
{
  public:
    /**
     * @brief Default
     */
    virtual ~NrMacSchedulerSrs() = default;

    /**
     * @brief Struct to indicate to the scheduler the periodicity and the offset, in slots.
     *
     * The struct must be considerated invalid if the field `m_isValid` is set
     * to false.
     */
    struct SrsPeriodicityAndOffset
    {
        bool m_isValid{false};     //!< Indicates if the values are valid.
        uint32_t m_periodicity{0}; //!< The periodicity requested (in slot).
        uint32_t m_offset{0};      //!< The offset requested (in slot).
    };

    /**
     * @brief Function called when the scheduler needs to know what is the offset and periodicy
     * of a newly added ue
     * @return a struct that contains the periodicity and the offset. If the struct
     * is not valid, an increase in periodicity is probably needed.
     *
     * @see IncreasePeriodicity
     */
    virtual SrsPeriodicityAndOffset AddUe() = 0;

    /**
     * @brief Function called when the scheduler has to release a previously owned periodicity
     * and offset.
     * @param offset The offset used by the UE
     *
     * Note: This interface assumes that all the UEs will share the same periodicity.
     * If that's not the case, the API would have to be updated.
     */
    virtual void RemoveUe(uint32_t offset) = 0;

    /**
     * @brief Increase the periodicity and assign to all UEs a different offset
     * @param ueMap the UE map
     * @return true if the operation was done
     *
     * The method increases the periodicity, and  then re-assign offsets and periodicity
     * to all the UEs to avoid conflicts.
     */
    virtual bool IncreasePeriodicity(
        std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>* ueMap) = 0;

    /**
     * @brief Decrease the periodicity and assign to all UEs a different offset
     * @param ueMap the UE map
     * @return true if the operation was done
     *
     * The method decreases the periodicity, and  then re-assign offsets and periodicity
     * to all the UEs to avoid conflicts.
     */
    virtual bool DecreasePeriodicity(
        std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo>>* ueMap) = 0;

    /**
     * @brief Check if all SRS periodicity is at the maximum allowed and all offsets have been used
     * @return true if all SRS offsets have been used
     */
    virtual bool IsMaxSrsReached() const = 0;
};
} // namespace ns3

#endif // NR_MAC_SCHEDULER_SRS_H
