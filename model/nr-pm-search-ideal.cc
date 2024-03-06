/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-pm-search-ideal.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrPmSearchIdeal");
NS_OBJECT_ENSURE_REGISTERED(NrPmSearchIdeal);

TypeId
NrPmSearchIdeal::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrPmSearchIdeal")
                            .SetParent<NrPmSearchFull>()
                            .AddConstructor<NrPmSearchIdeal>();
    return tid;
}

PmCqiInfo
NrPmSearchIdeal::CreateCqiFeedbackMimo(const NrMimoSignal& rxSignalRb, PmiUpdate pmiUpdate)
{
    NS_LOG_FUNCTION(this);

    // Extract parameters from received signal
    auto nRows = rxSignalRb.m_chanMat.GetNumRows();
    auto nCols = rxSignalRb.m_chanMat.GetNumCols();
    NS_ASSERT_MSG(nRows == m_nRxPorts, "Channel mat has {} rows but UE has {} ports");
    NS_ASSERT_MSG(nCols == m_nGnbPorts, "Channel mat has {} cols but gNB has {} ports");

    // Compute the interference-normalized channel matrix
    auto rbNormChanMat = rxSignalRb.m_covMat.CalcIntfNormChannel(rxSignalRb.m_chanMat);

    NS_ASSERT(m_subbandSize == 1); // TODO: enable compression from RB to subband and remove
    // auto sbNormChanMat = rbNormChanMat; // TODO: enable compression from RB to subband

    // Update precoding matrices
    if (pmiUpdate.updateWb || pmiUpdate.updateSb)
    {
        // Compute the channel correlation for each band (C = H^h * H)
        auto C = NrIntfNormChanMat(rbNormChanMat.HermitianTranspose() * rbNormChanMat);

        // Compute the channel average over bands
        auto Cavg = C.GetWidebandChannel();

        // Select the maximum rank
        uint8_t maxRank = SelectRank(Cavg);

        auto res = Create<NrPmSearchFull::PrecMatParams>();
        res->wbPmi = 0;                    // todo: put a meaningful placeholder
        res->sbPmis.resize(m_subbandSize); // todo: put a meaningful placeholder
        res->sbPrecMat = rbNormChanMat.ExtractOptimalPrecodingMatrices(maxRank);

        // Compute wideband capacity of optimal precoders
        auto sinr = rbNormChanMat.ComputeSinrForPrecoding(res->sbPrecMat);
        for (auto iSb = size_t{0}; iSb < m_subbandSize; iSb++)
        {
            double currCap = 0;
            for (size_t iLayer = 0; iLayer < sinr.GetNumRows(); iLayer++)
            {
                currCap += log2(1.0 + sinr(iLayer, iSb));
            }
            res->perfMetric += currCap;
        }
        m_rankParams[maxRank].precParams = res;
        m_periodMaxRank = maxRank;
    }

    // Return corresponding CQI/PMI to optimal rank
    return CreateCqiForRank(m_periodMaxRank, rbNormChanMat);
}

} // namespace ns3
