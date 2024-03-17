// Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>

#ifndef NR_HEX_GRID_ENB_TOPOLOGY_HELPER_H
#define NR_HEX_GRID_ENB_TOPOLOGY_HELPER_H

#include "nr-helper.h"

namespace ns3
{

/**
 * \ingroup nr
 *
 * This helper class allows to easily create a topology with eNBs
 * grouped in three-sector sites laid out on an hexagonal grid. The
 * layout is done row-wise.
 *
 */
class NrHexGridEnbTopologyHelper : public Object
{
  public:
    NrHexGridEnbTopologyHelper();
    ~NrHexGridEnbTopologyHelper() override;

    /**
     * Register this type.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();
    void DoDispose() override;

    /**
     * Set the NrHelper to be used to actually create the EnbNetDevices
     *
     * \note if no NrEpcHelper is ever set, then NrHexGridEnbTopologyHelper will default
     * to creating an LTE-only simulation with no EPC, using NrRlcSm as
     * the RLC model, and without supporting any IP networking. In other
     * words, it will be a radio-level simulation involving only LTE PHY
     * and MAC and the FF Scheduler, with a saturation traffic model for
     * the RLC.
     *
     * \param h a pointer to the NrEpcHelper to be used
     */
    void SetNrHelper(Ptr<NrHelper> h);

    /**
     * Position the nodes on a hex grid and install the corresponding
     * EnbNetDevices with antenna boresight configured properly
     *
     * \param c the node container where the devices are to be installed
     *
     * \return the NetDeviceContainer with the newly created devices
     */
    NetDeviceContainer SetPositionAndInstallEnbDevice(NodeContainer c);

  private:
    /**
     * Pointer to NrHelper object
     */
    Ptr<NrHelper> m_lteHelper;

    /**
     * The offset [m] in the position for the node of each sector with
     * respect to the center of the three-sector site
     */
    double m_offset;

    /**
     * The distance [m] between nearby sites
     */
    double m_d;

    /**
     * The x coordinate where the hex grid starts
     */
    double m_xMin;

    /**
     * The y coordinate where the hex grid starts
     */
    double m_yMin;

    /**
     * The number of sites in even rows (odd rows will have
     * one additional site)
     */
    uint32_t m_gridWidth;

    /**
     * The height [m] of each site
     */
    uint32_t m_siteHeight;
};

} // namespace ns3

#endif // NR_HEX_GRID_ENB_TOPOLOGY_HELPER_H
