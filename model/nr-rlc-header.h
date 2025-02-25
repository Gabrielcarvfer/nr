// Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#ifndef NR_RLC_HEADER_H
#define NR_RLC_HEADER_H

#include "nr-rlc-sequence-number.h"

#include "ns3/header.h"

#include <list>

namespace ns3
{

/**
 * @ingroup nr
 * @brief The packet header for the Radio Link Control (RLC) protocol packets
 *
 * This class has fields corresponding to those in an RLC header as well as
 * methods for serialization to and deserialization from a byte buffer.
 * It follows 3GPP TS 36.322 Radio Link Control (RLC) protocol specification.
 */
class NrRlcHeader : public Header
{
  public:
    /**
     * @brief Constructor
     *
     * Creates a null header
     */
    NrRlcHeader();
    ~NrRlcHeader() override;

    /**
     * Set framing info
     *
     * @param framingInfo framing info
     */
    void SetFramingInfo(uint8_t framingInfo);
    /**
     * Set sequence number
     *
     * @param sequenceNumber sequence number
     */
    void SetSequenceNumber(nr::SequenceNumber10 sequenceNumber);

    /**
     * Get framing info
     *
     * @returns framing info
     */
    uint8_t GetFramingInfo() const;
    /**
     * Get sequence number
     *
     * @returns sequence number
     */
    nr::SequenceNumber10 GetSequenceNumber() const;

    /**
     * Push extension bit
     *
     * @param extensionBit the extension bit
     */
    void PushExtensionBit(uint8_t extensionBit);
    /**
     * Push length indicator
     *
     * @param lengthIndicator the length indicator
     */
    void PushLengthIndicator(uint16_t lengthIndicator);

    /**
     * Pop extension bit
     *
     * @returns the extension bit
     */
    uint8_t PopExtensionBit();
    /**
     * Pop length indicator
     *
     * @returns the length indicator
     */
    uint16_t PopLengthIndicator();

    /// ExtensionBit_t typedef
    enum ExtensionBit_t
    {
        DATA_FIELD_FOLLOWS = 0,
        E_LI_FIELDS_FOLLOWS = 1
    };

    /// FramingInfoByte_t typedef
    enum FramingInfoByte_t
    {
        FIRST_BYTE = 0x00,
        NO_FIRST_BYTE = 0x02,
        LAST_BYTE = 0x00,
        NO_LAST_BYTE = 0x01
    };

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

  private:
    uint16_t m_headerLength;               ///< header length
    uint8_t m_framingInfo;                 ///<  2 bits
    nr::SequenceNumber10 m_sequenceNumber; ///< sequence number

    std::list<uint8_t> m_extensionBits;     ///< Includes extensionBit of the fixed part
    std::list<uint16_t> m_lengthIndicators; ///< length indicators
};

}; // namespace ns3

#endif // NR_RLC_HEADER_H
