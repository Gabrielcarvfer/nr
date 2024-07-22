// Copyright (c) 2015 Danilo Abrignani
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Danilo Abrignani <danilo.abrignani@unibo.it>

#ifndef COMPONENT_CARRIER_UE_H
#define COMPONENT_CARRIER_UE_H

#include "nr-component-carrier.h"
#include "nr-phy.h"
#include "nr-ue-phy.h"

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/packet.h>

namespace ns3
{

class NrUeMac;

/**
 * \ingroup nr
 *
 * ComponentCarrierUe Object, it defines a single Carrier for the Ue
 */
class ComponentCarrierUe : public NrComponentCarrier
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ComponentCarrierUe();

    ~ComponentCarrierUe() override;
    void DoDispose() override;

    /**
     * \return a pointer to the physical layer.
     */
    Ptr<NrUePhy> GetPhy() const;

    /**
     * \return a pointer to the MAC layer.
     */
    Ptr<NrUeMac> GetMac() const;

    /**
     * Set NrUePhy
     * \param s a pointer to the NrUePhy
     */
    void SetPhy(Ptr<NrUePhy> s);

    /**
     * Set the NrGnbMac
     * \param s a pointer to the NrGnbMac
     */
    void SetMac(Ptr<NrUeMac> s);

  protected:
    // inherited from Object
    void DoInitialize() override;

  private:
    Ptr<NrUePhy> m_phy; ///< the Phy instance of this eNodeB component carrier
    Ptr<NrUeMac> m_mac; ///< the MAC instance of this eNodeB component carrier
};

} // namespace ns3

#endif /* COMPONENT_CARRIER_UE_H */
