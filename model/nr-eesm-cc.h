// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_EESM_CC_H
#define NR_EESM_CC_H

#include "nr-eesm-error-model.h"

namespace ns3
{

/**
 * @ingroup error-models
 * @brief The NrEesmCc class
 *
 * In HARQ-CC, every retransmission contains the same coded bits
 * (information and coding bits). Therefore, the ECR after the q-th retransmission
 * remains the same as after the first transmission. Only the effective SINR
 * varies after combining retransmissions. In this case, the SINR values of the
 * corresponding resources are summed across the retransmissions, and the combined
 * SINR values are used to get the effective SINR based on EESM.
 *
 * In HARQ-CC, the HARQ history contains the SINR per allocated RB. Given the current
 * SINR vector and RB map, and the HARQ history, the effective SINR is computed
 * according to EESM.
 *
 * Please, don't use this class directly, but one between NrEesmCcT1 or NrEesmCcT2,
 * depending on what table you want to use.
 */
class NrEesmCc : public NrEesmErrorModel
{
  public:
    /**
     * @brief Get the type id of the object
     * @return the type id of the object
     */
    static TypeId GetTypeId();
    /**
     * @brief NrEesmCc constructor
     */
    NrEesmCc();
    /**
     * @brief ~NrEesmCc
     */
    ~NrEesmCc() override;

  protected:
    /**
     * @brief Computes the effective SINR after retransmission combining with HARQ-CC.
     *
     * @param sinr the SINR vector of current transmission
     * @param map the RB map of current transmission
     * @param sizeBit the Transport block size in bits
     * @param mcs the MCS of the transmission
     * @param sinrHistory the History of the previous transmissions of the same block
     * @return The effective SINR
     */
    double ComputeSINR(const SpectrumValue& sinr,
                       const std::vector<int>& map,
                       uint8_t mcs,
                       uint32_t sizeBit,
                       const NrErrorModel::NrErrorModelHistory& sinrHistory) const override;

    /**
     * @brief Returns the MCS corresponding to the ECR after retransmissions. As the ECR
     * does not change with retransmissions with HARQ-CC, and MCS is kept fixed through
     * retransmissions, it returns current MCS.
     *
     * @param mcsTx the MCS of the transmission
     * @return The equivalent MCS after retransmissions
     */
    double GetMcsEq(uint8_t mcsTx) const override;
};

} // namespace ns3

#endif // NR_EESM_CC_H
