// Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef NR_MAC_HEADER_FS_H
#define NR_MAC_HEADER_FS_H

#include "ns3/packet.h"

namespace ns3
{

/**
 * @ingroup ue-mac
 * @ingroup gnb-mac
 *
 * @brief Mac Fixed-Size Header
 *
 * The header consists of 8 bits, in which the first two are 0, and the last
 * six contains the Logical Channel ID. As the values for LCID are different
 * for UL and DL, please don't use this class directly. Instead, refer to
 * NrMacHeaderFsUl or NrMacHeaderFsDl, depending on the direction you need.
 *
 * The header representation is as follows:
 *
 * @verbatim
 +-----+-----+-------------------------------------------------+
 |     |     |                                                 |
 |  0  |  0  |                     m_lcId                      |   Oct 1
 |     |     |                                                 |
 +-----+-----+-------------------------------------------------+
\endverbatim
 *
 * As the first byte doesn't differ from the variable-size header, the recommended
 * de-serialization procedure is to Peek the header first, to look at the
 * lcId value, and then Remove the correct header. Some LC ID values are for
 * fixed-size headers, while others are for variable-size headers. Be sure of
 * what you're doing, or there is an assert waiting for you!
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 *
 * <b>Users, don't use this header directly: you've been warned.</b>
 *
 * @internal
 *
 * The header should be prefixed to some fixed-size data, such as short-bsr.
 */
class NrMacHeaderFs : public Header
{
  public:
    /**
     * @brief GetTypeId
     * @return the type id of the object
     */
    static TypeId GetTypeId();
    /**
     * @brief GetInstanceTypeId
     * @return the instance type id
     */
    TypeId GetInstanceTypeId() const override;

    /**
     * @brief Constructor
     */
    NrMacHeaderFs();

    /**
     * @brief Serialize on a buffer
     * @param start start position
     */
    void Serialize(Buffer::Iterator start) const override;
    /**
     * @brief Deserialize from a buffer
     * @param start start position
     * @return the number of bytes read from the buffer
     */
    uint32_t Deserialize(Buffer::Iterator start) override;
    /**
     * @brief Get the serialized size
     * @return 1
     */
    uint32_t GetSerializedSize() const override;
    /**
     * @brief Print the struct on a ostream
     * @param os ostream
     */
    void Print(std::ostream& os) const override;

    static const uint8_t PADDING = 63; //!< Padding

    /**
     * @brief Set the LC ID
     * @param lcId LC ID
     *
     * It will assert if the value is not inside the vector of allowed ones.
     */
    virtual void SetLcId(uint8_t lcId);

    /**
     * @brief Retrieve the LC ID of this header
     * @return the LC ID
     */
    uint8_t GetLcId() const;

    /**
     * @brief IsEqual
     * @param o another instance
     * @return true if this and o are equal, false otherwise
     */
    bool operator==(const NrMacHeaderFs& o) const;

  protected:
    uint8_t m_lcid{0}; //!< LC ID
};

} // namespace ns3

#endif /* NR_MAC_HEADER_FS_H */
