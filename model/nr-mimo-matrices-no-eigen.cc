/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mimo-matrices.h"

#include <ns3/fatal-error.h>

namespace ns3
{

NrIntfNormChanMat
NrCovMat::CalcIntfNormChannelMimo([[maybe_unused]] const ComplexMatrixArray& chanMat) const
{
    NS_FATAL_ERROR("MIMO channel normalization requires Eigen matrix library.");
}

ComplexMatrixArray
NrIntfNormChanMat::ComputeMseMimo([[maybe_unused]] const ComplexMatrixArray& precMats) const
{
    NS_FATAL_ERROR("MIMO MSE computation requires Eigen matrix library.");
}

uint8_t
NrIntfNormChanMat::GetEigenWidebandRank([[maybe_unused]] double thr) const
{
    NS_FATAL_ERROR("GetEigenWidebandRank requires Eigen matrix library.");
}

std::vector<uint8_t>
NrIntfNormChanMat::GetEigenSubbandRanks([[maybe_unused]] double thr) const
{
    NS_FATAL_ERROR("GetEigenSubbandRanks requires Eigen matrix library.");
}

ComplexMatrixArray
NrIntfNormChanMat::ExtractOptimalPrecodingMatrices(uint8_t rank) const
{
    NS_FATAL_ERROR("GetEigenRank requires Eigen matrix library.");
}

} // namespace ns3
