// Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Nicola Baldo <nbaldo@cttc.es>,
//         Marco Miozzo <mmiozzo@cttc.es>

#ifndef NR_ENB_CPHY_SAP_H
#define NR_ENB_CPHY_SAP_H

#include "nr-rrc-sap.h"

#include <ns3/ptr.h>

#include <stdint.h>

namespace ns3
{

class NrGnbNetDevice;

/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the PHY methods called by the MAC
 */
class NrEnbCphySapProvider
{
  public:
    /**
     * Destructor
     */
    virtual ~NrEnbCphySapProvider();

    /**
     * Set cell ID
     *
     * \param cellId the Cell Identifier
     */
    virtual void SetCellId(uint16_t cellId) = 0;

    /**
     * Set bandwidth
     *
     * \param ulBandwidth the UL bandwidth in PRBs
     * \param dlBandwidth the DL bandwidth in PRBs
     */
    virtual void SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth) = 0;

    /**
     * Set EARFCN
     *
     * \param ulEarfcn the UL EARFCN
     * \param dlEarfcn the DL EARFCN
     */
    virtual void SetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn) = 0;

    /**
     * Add a new UE to the cell
     *
     * \param rnti the UE id relative to this cell
     */
    virtual void AddUe(uint16_t rnti) = 0;

    /**
     * Remove an UE from the cell
     *
     * \param rnti the UE id relative to this cell
     */
    virtual void RemoveUe(uint16_t rnti) = 0;

    /**
     * Set the UE transmission power offset P_A
     *
     * \param rnti the UE id relative to this cell
     * \param pa transmission power offset
     */
    virtual void SetPa(uint16_t rnti, double pa) = 0;

    /**
     * Set transmission mode
     *
     * \param rnti the RNTI of the user
     * \param txMode the transmissionMode of the user
     */
    virtual void SetTransmissionMode(uint16_t rnti, uint8_t txMode) = 0;

    /**
     * Set SRS configuration index
     *
     * \param rnti the RNTI of the user
     * \param srsCi the SRS Configuration Index of the user
     */
    virtual void SetSrsConfigurationIndex(uint16_t rnti, uint16_t srsCi) = 0;

    /**
     * Set master information block
     *
     * \param mib the Master Information Block to be sent on the BCH
     */
    virtual void SetMasterInformationBlock(NrRrcSap::MasterInformationBlock mib) = 0;

    /**
     * Set system information block type 1
     *
     * \param sib1 the System Information Block Type 1 to be sent on the BCH
     */
    virtual void SetSystemInformationBlockType1(NrRrcSap::SystemInformationBlockType1 sib1) = 0;

    /**
     * Get reference signal power
     *
     * \return Reference Signal Power for SIB2
     */
    virtual int8_t GetReferenceSignalPower() = 0;
};

/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the CPHY SAP User, i.e., the part of the SAP that contains the RRC
 * methods called by the PHY
 */
class NrEnbCphySapUser
{
  public:
    /**
     * Destructor
     */
    virtual ~NrEnbCphySapUser();
};

/**
 * Template for the implementation of the NrEnbCphySapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class MemberNrEnbCphySapProvider : public NrEnbCphySapProvider
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrEnbCphySapProvider(C* owner);

    // Delete default constructor to avoid misuse
    MemberNrEnbCphySapProvider() = delete;

    // inherited from NrEnbCphySapProvider
    void SetCellId(uint16_t cellId) override;
    void SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth) override;
    void SetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn) override;
    void AddUe(uint16_t rnti) override;
    void RemoveUe(uint16_t rnti) override;
    void SetPa(uint16_t rnti, double pa) override;
    void SetTransmissionMode(uint16_t rnti, uint8_t txMode) override;
    void SetSrsConfigurationIndex(uint16_t rnti, uint16_t srsCi) override;
    void SetMasterInformationBlock(NrRrcSap::MasterInformationBlock mib) override;
    void SetSystemInformationBlockType1(NrRrcSap::SystemInformationBlockType1 sib1) override;
    int8_t GetReferenceSignalPower() override;

  private:
    C* m_owner; ///< the owner class
};

template <class C>
MemberNrEnbCphySapProvider<C>::MemberNrEnbCphySapProvider(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetCellId(uint16_t cellId)
{
    m_owner->DoSetCellId(cellId);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetBandwidth(uint16_t ulBandwidth, uint16_t dlBandwidth)
{
    m_owner->DoSetBandwidth(ulBandwidth, dlBandwidth);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetEarfcn(uint32_t ulEarfcn, uint32_t dlEarfcn)
{
    m_owner->DoSetEarfcn(ulEarfcn, dlEarfcn);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::AddUe(uint16_t rnti)
{
    m_owner->DoAddUe(rnti);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::RemoveUe(uint16_t rnti)
{
    m_owner->DoRemoveUe(rnti);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetPa(uint16_t rnti, double pa)
{
    m_owner->DoSetPa(rnti, pa);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetTransmissionMode(uint16_t rnti, uint8_t txMode)
{
    m_owner->DoSetTransmissionMode(rnti, txMode);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetSrsConfigurationIndex(uint16_t rnti, uint16_t srsCi)
{
    m_owner->DoSetSrsConfigurationIndex(rnti, srsCi);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetMasterInformationBlock(NrRrcSap::MasterInformationBlock mib)
{
    m_owner->DoSetMasterInformationBlock(mib);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetSystemInformationBlockType1(
    NrRrcSap::SystemInformationBlockType1 sib1)
{
    m_owner->DoSetSystemInformationBlockType1(sib1);
}

template <class C>
int8_t
MemberNrEnbCphySapProvider<C>::GetReferenceSignalPower()
{
    return m_owner->DoGetReferenceSignalPower();
}

/**
 * Template for the implementation of the NrEnbCphySapUser as a member
 * of an owner class of type C to which all methods are forwarded
 */
template <class C>
class MemberNrEnbCphySapUser : public NrEnbCphySapUser
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrEnbCphySapUser(C* owner);

    // Delete default constructor to avoid misuse
    MemberNrEnbCphySapUser() = delete;

    // methods inherited from NrEnbCphySapUser go here

  private:
    C* m_owner; ///< the owner class
};

template <class C>
MemberNrEnbCphySapUser<C>::MemberNrEnbCphySapUser(C* owner)
    : m_owner(owner)
{
}

} // namespace ns3

#endif // NR_ENB_CPHY_SAP_H
