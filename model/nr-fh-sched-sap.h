/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_FH_SCHED_SAP_H
#define NR_FH_SCHED_SAP_H

#include "nr-mac-scheduler-ns3.h"

// #include "nr-fh-control.h"

namespace ns3
{

/**
 * \brief Service Access Point (SAP) offered by the FhControl instance
 *        to the MAC Scheduler instance.
 *
 * This is the *NrFhSchedSapProvider*, i.e., the part of the SAP that
 * contains the FhControl methods called by the MAC Scheduler instance.
 */

class NrFhSchedSapProvider
{
  public:
    virtual ~NrFhSchedSapProvider();

    virtual void DoesAllocationFit() = 0;
};

/**
 * \brief Service Access Point (SAP) offered by the MAC Scheduler instance
 * to the FhControl instance.
 *
 * This is the *NrFhSchedSapUser*, i.e., the part of the SAP that contains the
 * gnb PHY methods called by the FhControl instance.
 */

class NrFhSchedSapUser
{
  public:
    virtual ~NrFhSchedSapUser();
};

/**
 * \brief Template for the implementation of the NrFhSchedSapProvider as a member of
 *        an owner class of type C to which all methods are forwarded.
 */

template <class C>
class MemberNrFhSchedSapProvider : public NrFhSchedSapProvider
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrFhSchedSapProvider(C* owner);

    virtual void DoesAllocationFit();

  private:
    MemberNrFhSchedSapProvider();
    C* m_owner; ///< the owner class

}; // end of class MemberNrFhSchedSapProvider

template <class C>
MemberNrFhSchedSapProvider<C>::MemberNrFhSchedSapProvider(C* owner)
    : m_owner(owner)
{
}

template <class C>
void
MemberNrFhSchedSapProvider<C>::DoesAllocationFit()
{
    return m_owner->DoGetDoesAllocationFit();
}

/**
 * \brief Template for the implementation of the NrFhSchedSapUser as a member of an
 *        owner class of type C to which all methods are forwarded.
 */

template <class C>
class MemberNrFhSchedSapUser : public NrFhSchedSapUser
{
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberNrFhSchedSapUser(C* owner);

  private:
    MemberNrFhSchedSapUser();
    C* m_owner; ///< the owner class

}; // end of class MemberNrFhSchedSapUser

template <class C>
MemberNrFhSchedSapUser<C>::MemberNrFhSchedSapUser(C* owner)
    : m_owner(owner)
{
}

} // namespace ns3
#endif // NR_FH_SCHED_SAP_H
