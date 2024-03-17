// Copyright (c) 2015 Danilo Abrignani
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Danilo Abrignani <danilo.abrignani@unibo.it>

#ifndef COMPONENT_CARRIER_ENB_H
#define COMPONENT_CARRIER_ENB_H

#include "nr-component-carrier.h"
#include "nr-gnb-phy.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/pointer.h>

namespace ns3
{

class NrGnbMac;
class NrFfMacScheduler;
class NrFfrAlgorithm;

/**
 * \ingroup nr
 *
 * Defines a single carrier for enb, and contains pointers to NrGnbPhy,
 * NrEnbMac, NrFfrAlgorithm, and NrFfMacScheduler objects.
 *
 */
class NrComponentCarrierEnb : public NrComponentCarrierBaseStation
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    NrComponentCarrierEnb();

    ~NrComponentCarrierEnb() override;
    void DoDispose() override;

    /**
     * \return a pointer to the physical layer.
     */
    Ptr<NrGnbPhy> GetPhy();

    /**
     * \return a pointer to the MAC layer.
     */
    Ptr<NrGnbMac> GetMac();

    /**
     * \return a pointer to the Ffr Algorithm.
     */
    Ptr<NrFfrAlgorithm> GetFfrAlgorithm();

    /**
     * \return a pointer to the Mac Scheduler.
     */
    Ptr<NrFfMacScheduler> GetFfMacScheduler();

    /**
     * Set the NrGnbPhy
     * \param s a pointer to the NrGnbPhy
     */
    void SetPhy(Ptr<NrGnbPhy> s);
    /**
     * Set the NrEnbMac
     * \param s a pointer to the NrEnbMac
     */
    void SetMac(Ptr<NrGnbMac> s);

    /**
     * Set the NrFfMacScheduler Algorithm
     * \param s a pointer to the NrFfMacScheduler
     */
    void SetFfMacScheduler(Ptr<NrFfMacScheduler> s);

    /**
     * Set the NrFfrAlgorithm
     * \param s a pointer to the NrFfrAlgorithm
     */
    void SetFfrAlgorithm(Ptr<NrFfrAlgorithm> s);

  protected:
    void DoInitialize() override;

  private:
    Ptr<NrGnbPhy> m_phy;               ///< the Phy instance of this eNodeB component carrier
    Ptr<NrGnbMac> m_mac;               ///< the MAC instance of this eNodeB component carrier
    Ptr<NrFfMacScheduler> m_scheduler; ///< the scheduler instance of this eNodeB component carrier
    Ptr<NrFfrAlgorithm>
        m_ffrAlgorithm; ///< the FFR algorithm instance of this eNodeB component carrier
};

} // namespace ns3

#endif /* COMPONENT_CARRIER_H */
