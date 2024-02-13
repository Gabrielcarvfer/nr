/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

// Copyright (c) 2024 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#include "nr-mimo-matrices.h"

#include <Eigen/Dense>

namespace ns3
{

template <class T>
using EigenMatrix = Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>;
template <class T>
using ConstEigenMatrix = Eigen::Map<const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>;

NrIntfNormChanMat
NrCovMat::CalcIntfNormChannelMimo(const ComplexMatrixArray& chanMat) const
{
    auto res = NrIntfNormChanMat{
        ComplexMatrixArray{chanMat.GetNumRows(), chanMat.GetNumCols(), chanMat.GetNumPages()}};
    for (size_t iRb = 0; iRb < chanMat.GetNumPages(); iRb++)
    {
        ConstEigenMatrix<std::complex<double>> covMatEigen(GetPagePtr(iRb),
                                                           GetNumRows(),
                                                           GetNumCols());
        ConstEigenMatrix<std::complex<double>> chanMatEigen(chanMat.GetPagePtr(iRb),
                                                            chanMat.GetNumRows(),
                                                            chanMat.GetNumCols());
        EigenMatrix<std::complex<double>> resEigen(res.GetPagePtr(iRb),
                                                   res.GetNumRows(),
                                                   res.GetNumCols());
        auto covMat = covMatEigen.selfadjointView<Eigen::Upper>();
        resEigen = covMat.llt().matrixL().solve(chanMatEigen);
    }
    return res;
}

auto res = ComplexMatrixArray {1,1,1};
auto chanPrec = ComplexMatrixArray {1,1,1};
auto chanPrecTrans = ComplexMatrixArray {1,1,1};
auto chanCov = ComplexMatrixArray {1,1,1};

ComplexMatrixArray
NrIntfNormChanMat::ComputeMseMimo(const ComplexMatrixArray& precMats) const
{
    auto nDims = precMats.GetNumCols();
    auto identity = Eigen::MatrixXcd::Identity(nDims, nDims);
    res.SetDimensions(nDims, nDims, precMats.GetNumPages());
    // Temporary tables only need a single page, since we do the calculations per page anyways
    chanPrec.SetDimensions(this->GetNumRows(), precMats.GetNumCols(), 1);
    chanPrecTrans.SetDimensions(chanPrec.GetNumCols(), chanPrec.GetNumRows(), 1);
    chanCov.SetDimensions(chanPrecTrans.GetNumRows(), chanPrec.GetNumCols(), 1);

    EigenMatrix<std::complex<double>> chanPrecEigen(chanPrec.GetPagePtr(0),
                                                    chanPrec.GetNumRows(),
                                                    chanPrec.GetNumCols());
    EigenMatrix<std::complex<double>> chanPrecTransEigen(chanPrecTrans.GetPagePtr(0),
                                                         chanPrecTrans.GetNumRows(),
                                                         chanPrecTrans.GetNumCols());
    EigenMatrix<std::complex<double>> chanCovEigen(chanCov.GetPagePtr(0),
                                                   chanCov.GetNumRows(),
                                                   chanCov.GetNumCols());
    Eigen::MatrixXcd temp(chanCov.GetNumRows(), chanCov.GetNumCols());
    for (size_t iRb = 0; iRb < res.GetNumPages(); iRb++)
    {
        ConstEigenMatrix<std::complex<double>> thisEigen(this->GetPagePtr(iRb),
                                                         this->GetNumRows(),
                                                         this->GetNumCols());
        ConstEigenMatrix<std::complex<double>> precMatsEigen(precMats.GetPagePtr(iRb),
                                                             precMats.GetNumRows(),
                                                             precMats.GetNumCols());
        // Compute the following product without intermediary allocations
        //chanPrec = (*this) * precMats;
        chanPrecEigen.noalias() = thisEigen * precMatsEigen;

        // Compute the transposed matrix without intermediary allocations
        //chanPrecTrans = chanPrec.HermitianTranspose();

        chanPrecTransEigen.noalias() = chanPrecEigen.transpose();
        // Compute the conjugates of the transposed matrix to make it Hermitian
        // chanPrecTransposed = chanPrec.HermitianTranspose();
        auto& chanPrecTransValues = chanPrecTrans.GetValuesRef();
        for (size_t value = 0; value < chanPrecTrans.GetSize(); value++)
        {
            chanPrecTransValues[value] = std::conj(chanPrecTransValues[value]);
        }
        // Compute the product
        //chanCov = chanPrecTrans * chanPrec;
        chanCovEigen.noalias() = chanPrecTransEigen * chanPrecEigen;

        // Calculate whatever
        EigenMatrix<std::complex<double>> resEigen(res.GetPagePtr(iRb),
                                                   res.GetNumRows(),
                                                   res.GetNumCols());
        temp.noalias() = chanCovEigen + identity;
    }
    return res;
}

} // namespace ns3
