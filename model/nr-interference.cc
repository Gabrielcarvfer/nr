/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-interference.h"

#include <ns3/log.h>
#include <ns3/lte-chunk-processor.h>
#include <ns3/simulator.h>

#include <algorithm>
#include <stdio.h>

NS_LOG_COMPONENT_DEFINE("NrInterference");

namespace ns3
{

NrInterference::NrInterference()
    : LteInterference(),
      m_firstPower(0.0)
{
    NS_LOG_FUNCTION(this);
}

NrInterference::~NrInterference()
{
    NS_LOG_FUNCTION(this);
}

void
NrInterference::DoDispose()
{
    NS_LOG_FUNCTION(this);
    LteInterference::DoDispose();
}

TypeId
NrInterference::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NrInterference")
            .SetParent<Object>()
            .AddTraceSource("SnrPerProcessedChunk",
                            "Snr per processed chunk.",
                            MakeTraceSourceAccessor(&NrInterference::m_snrPerProcessedChunk),
                            "ns3::SnrPerProcessedChunk::TracedCallback")
            .AddTraceSource("RssiPerProcessedChunk",
                            "Rssi per processed chunk.",
                            MakeTraceSourceAccessor(&NrInterference::m_rssiPerProcessedChunk),
                            "ns3::RssiPerProcessedChunk::TracedCallback");
    return tid;
}

void
NrInterference::AddSignal(Ptr<const SpectrumValue> spd, Time duration)
{
    NS_LOG_FUNCTION(this << *spd << duration);

    // Integrate over our receive bandwidth.
    // Note that differently from wifi, we do not need to pass the
    // signal through the filter. This is because
    // before receiving the signal already passed through the
    // spectrum converter, thus we will consider only the power over the
    // spectrum that corresponds to the spectrum of the receiver.
    // Also, differently from wifi we do not account here for the antenna gain,
    // since this is already taken into account by the spectrum channel.
    double rxPowerW = Integral(*spd);
    // We are creating two events, one that adds the rxPowerW, and
    // another that subtracts the rxPowerW at the endTime.
    // These events will be used to determine if the channel is busy and
    // for how long.
    AppendEvent(Simulator::Now(), Simulator::Now() + duration, rxPowerW);

    LteInterference::AddSignal(spd, duration);
}

void
NrInterference::EndRx()
{
    NS_LOG_FUNCTION(this);
    if (!m_receiving)
    {
        NS_LOG_INFO("EndRx was already evaluated or RX was aborted");
    }
    else
    {
        SpectrumValue snr = (*m_rxSignal) / (*m_noise);
        double avgSnr = Sum(snr) / (snr.GetSpectrumModel()->GetNumBands());
        m_snrPerProcessedChunk(avgSnr);

        NrInterference::ConditionallyEvaluateChunk();

        m_receiving = false;
        for (std::list<Ptr<LteChunkProcessor>>::const_iterator it =
                 m_rsPowerChunkProcessorList.begin();
             it != m_rsPowerChunkProcessorList.end();
             ++it)
        {
            (*it)->End();
        }
        for (std::list<Ptr<LteChunkProcessor>>::const_iterator it =
                 m_interfChunkProcessorList.begin();
             it != m_interfChunkProcessorList.end();
             ++it)
        {
            (*it)->End();
        }
        for (std::list<Ptr<LteChunkProcessor>>::const_iterator it =
                 m_sinrChunkProcessorList.begin();
             it != m_sinrChunkProcessorList.end();
             ++it)
        {
            (*it)->End();
        }
    }
}

void
NrInterference::ConditionallyEvaluateChunk()
{
    NS_LOG_FUNCTION(this);
    if (m_receiving)
    {
        NS_LOG_DEBUG(this << " Receiving");
    }
    NS_LOG_DEBUG(this << " now " << Now() << " last " << m_lastChangeTime);
    if (m_receiving && (Now() > m_lastChangeTime))
    {
        NS_LOG_LOGIC(this << " signal = " << *m_rxSignal << " allSignals = " << *m_allSignals
                          << " noise = " << *m_noise);
        SpectrumValue interf = (*m_allSignals) - (*m_rxSignal) + (*m_noise);
        SpectrumValue sinr = (*m_rxSignal) / interf;
        double rbWidth = (*m_rxSignal).GetSpectrumModel()->Begin()->fh -
                         (*m_rxSignal).GetSpectrumModel()->Begin()->fl;
        double rssidBm = 10 * log10(Sum((*m_noise + *m_allSignals) * rbWidth) * 1000);
        m_rssiPerProcessedChunk(rssidBm);

        NS_LOG_DEBUG("All signals: " << (*m_allSignals)[0] << ", rxSignal:" << (*m_rxSignal)[0]
                                     << " , noise:" << (*m_noise)[0]);

        Time duration = Now() - m_lastChangeTime;
        for (std::list<Ptr<LteChunkProcessor>>::const_iterator it =
                 m_rsPowerChunkProcessorList.begin();
             it != m_rsPowerChunkProcessorList.end();
             ++it)
        {
            (*it)->EvaluateChunk(*m_rxSignal, duration);
        }
        for (std::list<Ptr<LteChunkProcessor>>::const_iterator it =
                 m_sinrChunkProcessorList.begin();
             it != m_sinrChunkProcessorList.end();
             ++it)
        {
            (*it)->EvaluateChunk(sinr, duration);
        }
        m_lastChangeTime = Now();
    }
}

/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

NrInterference::NiChange::NiChange(Time time, double delta)
    : m_time(time),
      m_delta(delta)
{
}

Time
NrInterference::NiChange::GetTime() const
{
    return m_time;
}

double
NrInterference::NiChange::GetDelta() const
{
    return m_delta;
}

bool
NrInterference::NiChange::operator<(const NrInterference::NiChange& o) const
{
    return (m_time < o.m_time);
}

bool
NrInterference::IsChannelBusyNow(double energyW)
{
    double detectedPowerW = Integral(*m_allSignals);
    double powerDbm = 10 * log10(detectedPowerW * 1000);

    NS_LOG_INFO("IsChannelBusyNow detected power is: "
                << powerDbm << "  detectedPowerW: " << detectedPowerW << " length spectrum: "
                << (*m_allSignals).GetValuesN() << " thresholdW:" << energyW);

    if (detectedPowerW > energyW)
    {
        NS_LOG_INFO("Channel is BUSY.");
        return true;
    }
    else
    {
        NS_LOG_INFO("Channel is IDLE.");
        return false;
    }
}

Time
NrInterference::GetEnergyDuration(double energyW)
{
    if (!IsChannelBusyNow(energyW))
    {
        return Seconds(0);
    }

    Time now = Simulator::Now();
    double noiseInterferenceW = 0.0;
    Time end = now;
    noiseInterferenceW = m_firstPower;

    NS_LOG_INFO("First power: " << m_firstPower);

    for (NiChanges::const_iterator i = m_niChanges.begin(); i != m_niChanges.end(); i++)
    {
        noiseInterferenceW += i->GetDelta();
        end = i->GetTime();
        NS_LOG_INFO("Delta: " << i->GetDelta() << "time: " << i->GetTime());
        if (end < now)
        {
            continue;
        }
        if (noiseInterferenceW < energyW)
        {
            break;
        }
    }

    NS_LOG_INFO("Future power dBm:" << 10 * log10(noiseInterferenceW * 1000)
                                    << " W:" << noiseInterferenceW
                                    << " and energy threshold in W is: " << energyW);

    if (end > now)
    {
        NS_LOG_INFO("Channel BUSY until." << end);
    }
    else
    {
        NS_LOG_INFO("Channel IDLE.");
    }

    return end > now ? end - now : MicroSeconds(0);
}

void
NrInterference::EraseEvents()
{
    m_niChanges.clear();
    m_firstPower = 0.0;
}

NrInterference::NiChanges::iterator
NrInterference::GetPosition(Time moment)
{
    return std::upper_bound(m_niChanges.begin(), m_niChanges.end(), NiChange(moment, 0));
}

void
NrInterference::AddNiChangeEvent(NiChange change)
{
    m_niChanges.insert(GetPosition(change.GetTime()), change);
}

void
NrInterference::AppendEvent(Time startTime, Time endTime, double rxPowerW)
{
    Time now = Simulator::Now();

    if (!m_receiving)
    {
        NiChanges::iterator nowIterator = GetPosition(now);
        // We empty the list until the current moment. To do so we
        // first we sum all the energies until the current moment
        // and save it in m_firstPower.
        for (NiChanges::iterator i = m_niChanges.begin(); i != nowIterator; i++)
        {
            m_firstPower += i->GetDelta();
        }
        // then we remove all the events up to the current moment
        m_niChanges.erase(m_niChanges.begin(), nowIterator);
        // we create an event that represents the new energy
        m_niChanges.insert(m_niChanges.begin(), NiChange(startTime, rxPowerW));
    }
    else
    {
        // for the startTime create the event that adds the energy
        AddNiChangeEvent(NiChange(startTime, rxPowerW));
    }

    // for the endTime create event that will subtract energy
    AddNiChangeEvent(NiChange(endTime, -rxPowerW));
}

} // namespace ns3
