// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "nr-mac-scheduler-ue-info-rr.h"

namespace ns3
{

/**
 * @ingroup scheduler
 * @brief UE representation for a proportional fair scheduler
 *
 * The representation stores the current throughput, the average throughput,
 * and the last average throughput, as well as providing comparison functions
 * to sort the UEs in case of a PF scheduler.
 *
 * @see CompareUeWeightsDl
 * @see CompareUeWeightsUl
 */
class NrMacSchedulerUeInfoPF : public NrMacSchedulerUeInfo
{
  public:
    /**
     * @brief NrMacSchedulerUeInfoPF constructor
     * @param rnti RNTI of the UE
     * @param beamId Beam ID of the UE
     * @param fn A function that tells how many RB per RBG
     */
    NrMacSchedulerUeInfoPF(float alpha, uint16_t rnti, BeamId beamId, const GetRbPerRbgFn& fn)
        : NrMacSchedulerUeInfo(rnti, beamId, fn),
          m_alpha(alpha)
    {
    }

    /**
     * @brief Reset DL PF scheduler info
     *
     * Set the last average throughput to the current average throughput,
     * and zeroes the average throughput as well as the current throughput.
     *
     * It calls also NrMacSchedulerUeInfo::ResetDlSchedInfo.
     */
    void ResetDlSchedInfo() override
    {
        m_lastAvgTputDl = m_avgTputDl;
        m_currTputDl = 0.0;
        m_potentialTputDl = 0.0;
        NrMacSchedulerUeInfo::ResetDlSchedInfo();
    }

    /**
     * @brief Reset UL PF scheduler info
     *
     * Set the last average throughput to the current average throughput,
     * and zeroes the average throughput as well as the current throughput.
     *
     * It also calls NrMacSchedulerUeInfo::ResetUlSchedInfo.
     */
    void ResetUlSchedInfo() override
    {
        m_lastAvgTputUl = m_avgTputUl;
        m_currTputUl = 0.0;
        m_potentialTputUl = 0.0;
        NrMacSchedulerUeInfo::ResetUlSchedInfo();
    }

    /**
     * @brief Reset the DL avg Th to the last value
     */
    void ResetDlMetric() override
    {
        NrMacSchedulerUeInfo::ResetDlMetric();
        m_avgTputDl = m_lastAvgTputDl;
    }

    /**
     * @brief Reset the UL avg Th to the last value
     */
    void ResetUlMetric() override
    {
        NrMacSchedulerUeInfo::ResetUlMetric();
        m_avgTputUl = m_lastAvgTputUl;
    }

    /**
     * @brief Update the PF metric for downlink
     * @param totAssigned the resources assigned
     * @param timeWindow the time window
     *
     * Updates m_currTputDl and m_avgTputDl by keeping in consideration
     * the assigned resources (in form of TBS) and the time window.
     * It gets the tbSize by calling NrMacSchedulerUeInfo::UpdateDlMetric.
     */
    void UpdateDlPFMetric(const NrMacSchedulerNs3::FTResources& totAssigned, double timeWindow);

    /**
     * @brief Update the PF metric for uplink
     * @param totAssigned the resources assigned
     * @param timeWindow the time window
     *
     * Updates m_currTputUl and m_avgTputUl by keeping in consideration
     * the assigned resources (in form of TBS) and the time window.
     * It gets the tbSize by calling NrMacSchedulerUeInfo::UpdateUlMetric.
     */
    void UpdateUlPFMetric(const NrMacSchedulerNs3::FTResources& totAssigned, double timeWindow);

    /**
     * @brief Calculate the Potential throughput for downlink
     * @param assignableInIteration resources assignable
     */
    void CalculatePotentialTPutDl(const NrMacSchedulerNs3::FTResources& assignableInIteration);

    /**
     * @brief Calculate the Potential throughput for uplink
     * @param assignableInIteration resources assignable
     */
    void CalculatePotentialTPutUl(const NrMacSchedulerNs3::FTResources& assignableInIteration);

    /**
     * @brief comparison function object (i.e. an object that satisfies the
     * requirements of Compare) which returns true if the first argument is less
     * than (i.e. is ordered before) the second.
     * @param lue Left UE
     * @param rue Right UE
     * @return true if the PF metric of the left UE is higher than the right UE
     *
     * The PF metric is calculated as following:
     *
     * \f$ pfMetric_{i} = std::pow(potentialTPut_{i}, alpha) / std::max (1E-9, m_avgTput_{i}) \f$
     *
     * Alpha is a fairness metric. Please note that the throughput is calculated
     * in bit/symbol.
     */
    static bool CompareUeWeightsDl(const NrMacSchedulerNs3::UePtrAndBufferReq& lue,
                                   const NrMacSchedulerNs3::UePtrAndBufferReq& rue)
    {
        auto luePtr = dynamic_cast<NrMacSchedulerUeInfoPF*>(lue.first.get());
        auto ruePtr = dynamic_cast<NrMacSchedulerUeInfoPF*>(rue.first.get());

        double lPfMetric = std::pow(luePtr->m_potentialTputDl, luePtr->m_alpha) /
                           std::max(1E-9, luePtr->m_avgTputDl);
        double rPfMetric = std::pow(ruePtr->m_potentialTputDl, ruePtr->m_alpha) /
                           std::max(1E-9, ruePtr->m_avgTputDl);

        return (lPfMetric > rPfMetric);
    }

    /**
     * @brief comparison function object (i.e. an object that satisfies the
     * requirements of Compare) which returns true if the first argument is less
     * than (i.e. is ordered before) the second.
     * @param lue Left UE
     * @param rue Right UE
     * @return true if the PF metric of the left UE is higher than the right UE
     *
     * The PF metric is calculated as following:
     *
     * \f$ pfMetric_{i} = std::pow(potentialTPut_{i}, alpha) / std::max (1E-9, m_avgTput_{i}) \f$
     *
     * Alpha is a fairness metric. Please note that the throughput is calculated
     * in bit/symbol.
     */
    static bool CompareUeWeightsUl(const NrMacSchedulerNs3::UePtrAndBufferReq& lue,
                                   const NrMacSchedulerNs3::UePtrAndBufferReq& rue)
    {
        auto luePtr = dynamic_cast<NrMacSchedulerUeInfoPF*>(lue.first.get());
        auto ruePtr = dynamic_cast<NrMacSchedulerUeInfoPF*>(rue.first.get());

        double lPfMetric = std::pow(luePtr->m_potentialTputUl, luePtr->m_alpha) /
                           std::max(1E-9, luePtr->m_avgTputUl);
        double rPfMetric = std::pow(ruePtr->m_potentialTputUl, ruePtr->m_alpha) /
                           std::max(1E-9, ruePtr->m_avgTputUl);

        return (lPfMetric > rPfMetric);
    }

    double m_currTputDl{0.0};      //!< Current slot throughput in downlink
    double m_avgTputDl{0.0};       //!< Average throughput in downlink during all the slots
    double m_lastAvgTputDl{0.0};   //!< Last average throughput in downlink
    double m_potentialTputDl{0.0}; //!< Potential throughput in downlink in one assignable resource
                                   //!< (can be a symbol or a RBG)
    float m_alpha{0.0};            //!< PF fairness metric

    double m_currTputUl{0.0};      //!< Current slot throughput in uplink
    double m_avgTputUl{0.0};       //!< Average throughput in uplink during all the slots
    double m_lastAvgTputUl{0.0};   //!< Last average throughput in uplink
    double m_potentialTputUl{0.0}; //!< Potential throughput in uplink in one assignable resource
                                   //!< (can be a symbol or a RBG)
};

} // namespace ns3
