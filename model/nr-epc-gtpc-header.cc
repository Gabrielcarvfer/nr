// Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Manuel Requena <manuel.requena@cttc.es>

#include "nr-epc-gtpc-header.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NrGtpcHeader");

NS_OBJECT_ENSURE_REGISTERED(NrGtpcHeader);

/// GTPv2-C protocol version number
static const uint8_t VERSION = 2;

TypeId
NrGtpcHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcHeader")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcHeader>();
    return tid;
}

NrGtpcHeader::NrGtpcHeader()
    : m_teidFlag(false),
      m_messageType(0),
      m_messageLength(4),
      m_teid(0),
      m_sequenceNumber(0)
{
}

NrGtpcHeader::~NrGtpcHeader()
{
}

TypeId
NrGtpcHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcHeader::GetSerializedSize() const
{
    return m_teidFlag ? 12 : 8;
}

void
NrGtpcHeader::Serialize(Buffer::Iterator start) const
{
    NS_FATAL_ERROR("Serialize GTP-C header is forbidden");
}

void
NrGtpcHeader::PreSerialize(Buffer::Iterator& i) const
{
    i.WriteU8((VERSION << 5) | (1 << 3));
    i.WriteU8(m_messageType);
    i.WriteHtonU16(m_messageLength);
    i.WriteHtonU32(m_teid);
    i.WriteU8((m_sequenceNumber & 0x00ff0000) >> 16);
    i.WriteU8((m_sequenceNumber & 0x0000ff00) >> 8);
    i.WriteU8(m_sequenceNumber & 0x000000ff);
    i.WriteU8(0);
}

uint32_t
NrGtpcHeader::Deserialize(Buffer::Iterator start)
{
    return PreDeserialize(start);
}

uint32_t
NrGtpcHeader::PreDeserialize(Buffer::Iterator& i)
{
    uint8_t firstByte = i.ReadU8();
    uint8_t version = (firstByte >> 5) & 0x07;
    if (version != 2)
    {
        NS_FATAL_ERROR("GTP-C version not supported");
        return 0;
    }

    m_teidFlag = ((firstByte >> 3) & 0x01) == 1;
    if (!m_teidFlag)
    {
        NS_FATAL_ERROR("TEID is missing");
        return 0;
    }

    m_messageType = i.ReadU8();
    m_messageLength = i.ReadNtohU16();
    if (m_teidFlag)
    {
        m_teid = i.ReadNtohU32();
    }
    m_sequenceNumber = i.ReadU8() << 16 | i.ReadU8() << 8 | i.ReadU8();
    i.ReadU8();

    return NrGtpcHeader::GetSerializedSize();
}

void
NrGtpcHeader::Print(std::ostream& os) const
{
    os << " messageType " << (uint32_t)m_messageType << " messageLength " << m_messageLength;
    os << " TEID " << m_teid << " sequenceNumber " << m_sequenceNumber;
}

uint32_t
NrGtpcHeader::GetMessageSize() const
{
    return 0;
}

uint8_t
NrGtpcHeader::GetMessageType() const
{
    return m_messageType;
}

uint16_t
NrGtpcHeader::GetMessageLength() const
{
    return m_messageLength;
}

uint32_t
NrGtpcHeader::GetTeid() const
{
    return m_teid;
}

uint32_t
NrGtpcHeader::GetSequenceNumber() const
{
    return m_sequenceNumber;
}

void
NrGtpcHeader::SetMessageType(uint8_t messageType)
{
    m_messageType = messageType;
}

void
NrGtpcHeader::SetMessageLength(uint16_t messageLength)
{
    m_messageLength = messageLength;
}

void
NrGtpcHeader::SetTeid(uint32_t teid)
{
    m_teidFlag = true;
    m_teid = teid;
    m_messageLength = m_teidFlag ? 8 : 4;
}

void
NrGtpcHeader::SetSequenceNumber(uint32_t sequenceNumber)
{
    m_sequenceNumber = sequenceNumber;
}

void
NrGtpcHeader::SetIesLength(uint16_t iesLength)
{
    m_messageLength = iesLength;
    m_messageLength += (m_teidFlag) ? 8 : 4;
}

void
NrGtpcHeader::ComputeMessageLength()
{
    SetIesLength(GetMessageSize());
}

/////////////////////////////////////////////////////////////////////

void
NrGtpcIes::SerializeImsi(Buffer::Iterator& i, uint64_t imsi) const
{
    i.WriteU8(1);      // IE Type = IMSI
    i.WriteHtonU16(8); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteHtonU64(imsi);
}

uint32_t
NrGtpcIes::DeserializeImsi(Buffer::Iterator& i, uint64_t& imsi) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 1, "Wrong IMSI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 8, "Wrong IMSI IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong IMSI IE instance");
    imsi = i.ReadNtohU64();

    return serializedSizeImsi;
}

void
NrGtpcIes::SerializeCause(Buffer::Iterator& i, Cause_t cause) const
{
    i.WriteU8(2);      // IE Type = Cause
    i.WriteHtonU16(2); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(cause);  // Cause value
    i.WriteU8(0);      // Spare + CS
}

uint32_t
NrGtpcIes::DeserializeCause(Buffer::Iterator& i, Cause_t& cause) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 2, "Wrong Cause IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 2, "Wrong Cause IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong Cause IE instance");
    cause = Cause_t(i.ReadU8());
    i.ReadU8();

    return serializedSizeCause;
}

void
NrGtpcIes::SerializeEbi(Buffer::Iterator& i, uint8_t epsBearerId) const
{
    i.WriteU8(73);     // IE Type = EPS Bearer ID (EBI)
    i.WriteHtonU16(1); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(epsBearerId & 0x0f);
}

uint32_t
NrGtpcIes::DeserializeEbi(Buffer::Iterator& i, uint8_t& epsBearerId) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 73, "Wrong EBI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 1, "Wrong EBI IE length");
    uint8_t instance = i.ReadU8();
    NS_ASSERT_MSG(instance == 0, "Wrong EBI IE instance");
    epsBearerId = i.ReadU8() & 0x0f;

    return serializedSizeEbi;
}

void
NrGtpcIes::WriteHtonU40(Buffer::Iterator& i, uint64_t data) const
{
    i.WriteU8((data >> 32) & 0xff);
    i.WriteU8((data >> 24) & 0xff);
    i.WriteU8((data >> 16) & 0xff);
    i.WriteU8((data >> 8) & 0xff);
    i.WriteU8((data >> 0) & 0xff);
}

uint64_t
NrGtpcIes::ReadNtohU40(Buffer::Iterator& i)
{
    uint64_t retval = 0;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    retval <<= 8;
    retval |= i.ReadU8();
    return retval;
}

void
NrGtpcIes::SerializeBearerQos(Buffer::Iterator& i, NrEpsBearer bearerQos) const
{
    i.WriteU8(80);      // IE Type = Bearer QoS
    i.WriteHtonU16(22); // Length
    i.WriteU8(0);       // Spare + Instance
    i.WriteU8(0);       // MRE TODO: bearerQos.arp
    i.WriteU8(bearerQos.qci);
    WriteHtonU40(i, bearerQos.gbrQosInfo.mbrUl);
    WriteHtonU40(i, bearerQos.gbrQosInfo.mbrDl);
    WriteHtonU40(i, bearerQos.gbrQosInfo.gbrUl);
    WriteHtonU40(i, bearerQos.gbrQosInfo.gbrDl);
}

uint32_t
NrGtpcIes::DeserializeBearerQos(Buffer::Iterator& i, NrEpsBearer& bearerQos)
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 80, "Wrong Bearer QoS IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 22, "Wrong Bearer QoS IE length");
    uint8_t instance = i.ReadU8();
    NS_ASSERT_MSG(instance == 0, "Wrong Bearer QoS IE instance");
    i.ReadU8();
    bearerQos.qci = NrEpsBearer::Qci(i.ReadU8());
    bearerQos.gbrQosInfo.mbrUl = ReadNtohU40(i);
    bearerQos.gbrQosInfo.mbrDl = ReadNtohU40(i);
    bearerQos.gbrQosInfo.gbrUl = ReadNtohU40(i);
    bearerQos.gbrQosInfo.gbrDl = ReadNtohU40(i);
    return serializedSizeBearerQos;
}

void
NrGtpcIes::SerializeBearerTft(Buffer::Iterator& i,
                              std::list<NrEpcTft::PacketFilter> packetFilters) const
{
    i.WriteU8(84); // IE Type = EPS Bearer Level Fraffic Flow Template (Bearer TFT)
    i.WriteHtonU16(1 + packetFilters.size() * serializedSizePacketFilter);
    i.WriteU8(0);                                    // Spare + Instance
    i.WriteU8(0x20 + (packetFilters.size() & 0x0f)); // Create new TFT + Number of packet filters

    for (auto& pf : packetFilters)
    {
        i.WriteU8((pf.direction << 4) & 0x30);
        i.WriteU8(pf.precedence);
        i.WriteU8(serializedSizePacketFilter - 3); // Length of Packet filter contents

        i.WriteU8(0x10); // IPv4 remote address type
        i.WriteHtonU32(pf.remoteAddress.Get());
        i.WriteHtonU32(pf.remoteMask.Get());
        i.WriteU8(0x11); // IPv4 local address type
        i.WriteHtonU32(pf.localAddress.Get());
        i.WriteHtonU32(pf.localMask.Get());
        i.WriteU8(0x41); // Local port range type
        i.WriteHtonU16(pf.localPortStart);
        i.WriteHtonU16(pf.localPortEnd);
        i.WriteU8(0x51); // Remote port range type
        i.WriteHtonU16(pf.remotePortStart);
        i.WriteHtonU16(pf.remotePortEnd);
        i.WriteU8(0x70); // Type of service
        i.WriteU8(pf.typeOfService);
        i.WriteU8(pf.typeOfServiceMask);
    }
}

uint32_t
NrGtpcIes::DeserializeBearerTft(Buffer::Iterator& i, Ptr<NrEpcTft> epcTft) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 84, "Wrong Bearer TFT IE type = " << (uint16_t)type);
    i.ReadNtohU16();
    i.ReadU8();
    uint8_t numberOfPacketFilters = i.ReadU8() & 0x0f;

    for (uint8_t pf = 0; pf < numberOfPacketFilters; ++pf)
    {
        NrEpcTft::PacketFilter packetFilter;
        packetFilter.direction = NrEpcTft::Direction((i.ReadU8() & 0x30) >> 4);
        packetFilter.precedence = i.ReadU8();
        i.ReadU8(); // Length of Packet filter contents
        i.ReadU8();
        packetFilter.remoteAddress = Ipv4Address(i.ReadNtohU32());
        packetFilter.remoteMask = Ipv4Mask(i.ReadNtohU32());
        i.ReadU8();
        packetFilter.localAddress = Ipv4Address(i.ReadNtohU32());
        packetFilter.localMask = Ipv4Mask(i.ReadNtohU32());
        i.ReadU8();
        packetFilter.localPortStart = i.ReadNtohU16();
        packetFilter.localPortEnd = i.ReadNtohU16();
        i.ReadU8();
        packetFilter.remotePortStart = i.ReadNtohU16();
        packetFilter.remotePortEnd = i.ReadNtohU16();
        i.ReadU8();
        packetFilter.typeOfService = i.ReadU8();
        packetFilter.typeOfServiceMask = i.ReadU8();
        epcTft->Add(packetFilter);
    }

    return GetSerializedSizeBearerTft(epcTft->GetPacketFilters());
}

uint32_t
NrGtpcIes::GetSerializedSizeBearerTft(std::list<NrEpcTft::PacketFilter> packetFilters) const
{
    return (5 + packetFilters.size() * serializedSizePacketFilter);
}

void
NrGtpcIes::SerializeUliEcgi(Buffer::Iterator& i, uint32_t uliEcgi) const
{
    i.WriteU8(86);     // IE Type = ULI (ECGI)
    i.WriteHtonU16(8); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(0x10);   // ECGI flag
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteU8(0);      // Dummy MCC and MNC
    i.WriteHtonU32(uliEcgi);
}

uint32_t
NrGtpcIes::DeserializeUliEcgi(Buffer::Iterator& i, uint32_t& uliEcgi) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 86, "Wrong ULI ECGI IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 8, "Wrong ULI ECGI IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong ULI ECGI IE instance");
    i.Next(4);
    uliEcgi = i.ReadNtohU32() & 0x0fffffff;

    return serializedSizeUliEcgi;
}

void
NrGtpcIes::SerializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t fteid) const
{
    i.WriteU8(87);     // IE Type = Fully Qualified TEID (F-TEID)
    i.WriteHtonU16(9); // Length
    i.WriteU8(0);      // Spare + Instance
    i.WriteU8(0x80 | ((uint8_t)fteid.interfaceType & 0x1f)); // IP version flag + Iface type
    i.WriteHtonU32(fteid.teid);                              // TEID
    i.WriteHtonU32(fteid.addr.Get());                        // IPv4 address
}

uint32_t
NrGtpcIes::DeserializeFteid(Buffer::Iterator& i, NrGtpcHeader::Fteid_t& fteid) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 87, "Wrong FTEID IE type = " << (uint16_t)type);
    uint16_t length = i.ReadNtohU16();
    NS_ASSERT_MSG(length == 9, "Wrong FTEID IE length");
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong FTEID IE instance");
    uint8_t flags = i.ReadU8(); // IP version flag + Iface type
    fteid.interfaceType = NrGtpcHeader::InterfaceType_t(flags & 0x1f);
    fteid.teid = i.ReadNtohU32();    // TEID
    fteid.addr.Set(i.ReadNtohU32()); // IPv4 address

    return serializedSizeFteid;
}

void
NrGtpcIes::SerializeBearerContextHeader(Buffer::Iterator& i, uint16_t length) const
{
    i.WriteU8(93); // IE Type = Bearer Context
    i.WriteU16(length);
    i.WriteU8(0); // Spare + Instance
}

uint32_t
NrGtpcIes::DeserializeBearerContextHeader(Buffer::Iterator& i, uint16_t& length) const
{
    uint8_t type = i.ReadU8();
    NS_ASSERT_MSG(type == 93, "Wrong Bearer Context IE type = " << (uint16_t)type);
    length = i.ReadNtohU16();
    uint8_t instance = i.ReadU8() & 0x0f;
    NS_ASSERT_MSG(instance == 0, "Wrong Bearer Context IE instance");

    return serializedSizeBearerContextHeader;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcCreateSessionRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcCreateSessionRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcCreateSessionRequestMessage>();
    return tid;
}

NrGtpcCreateSessionRequestMessage::NrGtpcCreateSessionRequestMessage()
{
    SetMessageType(NrGtpcHeader::CreateSessionRequest);
    SetSequenceNumber(0);
    m_imsi = 0;
    m_uliEcgi = 0;
    m_senderCpFteid = {};
}

NrGtpcCreateSessionRequestMessage::~NrGtpcCreateSessionRequestMessage()
{
}

TypeId
NrGtpcCreateSessionRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeImsi + serializedSizeUliEcgi + serializedSizeFteid;
    for (auto& bc : m_bearerContextsToBeCreated)
    {
        serializedSize += serializedSizeBearerContextHeader + serializedSizeEbi +
                          GetSerializedSizeBearerTft(bc.tft->GetPacketFilters()) +
                          serializedSizeFteid + serializedSizeBearerQos;
    }

    return serializedSize;
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcCreateSessionRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeImsi(i, m_imsi);
    SerializeUliEcgi(i, m_uliEcgi);
    SerializeFteid(i, m_senderCpFteid);

    for (auto& bc : m_bearerContextsToBeCreated)
    {
        std::list<NrEpcTft::PacketFilter> packetFilters = bc.tft->GetPacketFilters();

        SerializeBearerContextHeader(i,
                                     serializedSizeEbi + GetSerializedSizeBearerTft(packetFilters) +
                                         serializedSizeFteid + serializedSizeBearerQos);

        SerializeEbi(i, bc.epsBearerId);
        SerializeBearerTft(i, packetFilters);
        SerializeFteid(i, bc.sgwS5uFteid);
        SerializeBearerQos(i, bc.bearerLevelQos);
    }
}

uint32_t
NrGtpcCreateSessionRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeImsi(i, m_imsi);
    DeserializeUliEcgi(i, m_uliEcgi);
    DeserializeFteid(i, m_senderCpFteid);

    m_bearerContextsToBeCreated.clear();
    while (i.GetRemainingSize() > 0)
    {
        uint16_t length;
        DeserializeBearerContextHeader(i, length);

        BearerContextToBeCreated bearerContext;
        DeserializeEbi(i, bearerContext.epsBearerId);

        Ptr<NrEpcTft> epcTft = Create<NrEpcTft>();
        DeserializeBearerTft(i, epcTft);
        bearerContext.tft = epcTft;

        DeserializeFteid(i, bearerContext.sgwS5uFteid);
        DeserializeBearerQos(i, bearerContext.bearerLevelQos);

        m_bearerContextsToBeCreated.push_back(bearerContext);
    }

    return GetSerializedSize();
}

void
NrGtpcCreateSessionRequestMessage::Print(std::ostream& os) const
{
    os << " imsi " << m_imsi << " uliEcgi " << m_uliEcgi;
}

uint64_t
NrGtpcCreateSessionRequestMessage::GetImsi() const
{
    return m_imsi;
}

void
NrGtpcCreateSessionRequestMessage::SetImsi(uint64_t imsi)
{
    m_imsi = imsi;
}

uint32_t
NrGtpcCreateSessionRequestMessage::GetUliEcgi() const
{
    return m_uliEcgi;
}

void
NrGtpcCreateSessionRequestMessage::SetUliEcgi(uint32_t uliEcgi)
{
    m_uliEcgi = uliEcgi;
}

NrGtpcHeader::Fteid_t
NrGtpcCreateSessionRequestMessage::GetSenderCpFteid() const
{
    return m_senderCpFteid;
}

void
NrGtpcCreateSessionRequestMessage::SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid)
{
    m_senderCpFteid = fteid;
}

std::list<NrGtpcCreateSessionRequestMessage::BearerContextToBeCreated>
NrGtpcCreateSessionRequestMessage::GetBearerContextsToBeCreated() const
{
    return m_bearerContextsToBeCreated;
}

void
NrGtpcCreateSessionRequestMessage::SetBearerContextsToBeCreated(
    std::list<NrGtpcCreateSessionRequestMessage::BearerContextToBeCreated> bearerContexts)
{
    m_bearerContextsToBeCreated = bearerContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcCreateSessionResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcCreateSessionResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcCreateSessionResponseMessage>();
    return tid;
}

NrGtpcCreateSessionResponseMessage::NrGtpcCreateSessionResponseMessage()
{
    SetMessageType(NrGtpcHeader::CreateSessionResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
    m_senderCpFteid = {};
}

NrGtpcCreateSessionResponseMessage::~NrGtpcCreateSessionResponseMessage()
{
}

TypeId
NrGtpcCreateSessionResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcCreateSessionResponseMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeCause + serializedSizeFteid;
    for (auto& bc : m_bearerContextsCreated)
    {
        serializedSize += serializedSizeBearerContextHeader + serializedSizeEbi +
                          GetSerializedSizeBearerTft(bc.tft->GetPacketFilters()) +
                          serializedSizeFteid + serializedSizeBearerQos;
    }

    return serializedSize;
}

uint32_t
NrGtpcCreateSessionResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcCreateSessionResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);
    SerializeFteid(i, m_senderCpFteid);

    for (auto& bc : m_bearerContextsCreated)
    {
        std::list<NrEpcTft::PacketFilter> packetFilters = bc.tft->GetPacketFilters();

        SerializeBearerContextHeader(i,
                                     serializedSizeEbi + GetSerializedSizeBearerTft(packetFilters) +
                                         serializedSizeFteid + serializedSizeBearerQos);

        SerializeEbi(i, bc.epsBearerId);
        SerializeBearerTft(i, packetFilters);
        SerializeFteid(i, bc.fteid);
        SerializeBearerQos(i, bc.bearerLevelQos);
    }
}

uint32_t
NrGtpcCreateSessionResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);
    DeserializeFteid(i, m_senderCpFteid);

    m_bearerContextsCreated.clear();
    while (i.GetRemainingSize() > 0)
    {
        BearerContextCreated bearerContext;
        uint16_t length;

        DeserializeBearerContextHeader(i, length);
        DeserializeEbi(i, bearerContext.epsBearerId);

        Ptr<NrEpcTft> epcTft = Create<NrEpcTft>();
        DeserializeBearerTft(i, epcTft);
        bearerContext.tft = epcTft;

        DeserializeFteid(i, bearerContext.fteid);
        DeserializeBearerQos(i, bearerContext.bearerLevelQos);

        m_bearerContextsCreated.push_back(bearerContext);
    }

    return GetSerializedSize();
}

void
NrGtpcCreateSessionResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << m_cause << " FTEID " << m_senderCpFteid.addr << "," << m_senderCpFteid.teid;
}

NrGtpcCreateSessionResponseMessage::Cause_t
NrGtpcCreateSessionResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcCreateSessionResponseMessage::SetCause(NrGtpcCreateSessionResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

NrGtpcHeader::Fteid_t
NrGtpcCreateSessionResponseMessage::GetSenderCpFteid() const
{
    return m_senderCpFteid;
}

void
NrGtpcCreateSessionResponseMessage::SetSenderCpFteid(NrGtpcHeader::Fteid_t fteid)
{
    m_senderCpFteid = fteid;
}

std::list<NrGtpcCreateSessionResponseMessage::BearerContextCreated>
NrGtpcCreateSessionResponseMessage::GetBearerContextsCreated() const
{
    return m_bearerContextsCreated;
}

void
NrGtpcCreateSessionResponseMessage::SetBearerContextsCreated(
    std::list<NrGtpcCreateSessionResponseMessage::BearerContextCreated> bearerContexts)
{
    m_bearerContextsCreated = bearerContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcModifyBearerRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcModifyBearerRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcModifyBearerRequestMessage>();
    return tid;
}

NrGtpcModifyBearerRequestMessage::NrGtpcModifyBearerRequestMessage()
{
    SetMessageType(NrGtpcHeader::ModifyBearerRequest);
    SetSequenceNumber(0);
    m_imsi = 0;
    m_uliEcgi = 0;
}

NrGtpcModifyBearerRequestMessage::~NrGtpcModifyBearerRequestMessage()
{
}

TypeId
NrGtpcModifyBearerRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcModifyBearerRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize =
        serializedSizeImsi + serializedSizeUliEcgi +
        m_bearerContextsToBeModified.size() *
            (serializedSizeBearerContextHeader + serializedSizeEbi + serializedSizeFteid);
    return serializedSize;
}

uint32_t
NrGtpcModifyBearerRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcModifyBearerRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeImsi(i, m_imsi);
    SerializeUliEcgi(i, m_uliEcgi);

    for (auto& bc : m_bearerContextsToBeModified)
    {
        SerializeBearerContextHeader(i, serializedSizeEbi + serializedSizeFteid);

        SerializeEbi(i, bc.epsBearerId);
        SerializeFteid(i, bc.fteid);
    }
}

uint32_t
NrGtpcModifyBearerRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeImsi(i, m_imsi);
    DeserializeUliEcgi(i, m_uliEcgi);

    while (i.GetRemainingSize() > 0)
    {
        BearerContextToBeModified bearerContext;
        uint16_t length;

        DeserializeBearerContextHeader(i, length);

        DeserializeEbi(i, bearerContext.epsBearerId);
        DeserializeFteid(i, bearerContext.fteid);

        m_bearerContextsToBeModified.push_back(bearerContext);
    }

    return GetSerializedSize();
}

void
NrGtpcModifyBearerRequestMessage::Print(std::ostream& os) const
{
    os << " imsi " << m_imsi << " uliEcgi " << m_uliEcgi;
}

uint64_t
NrGtpcModifyBearerRequestMessage::GetImsi() const
{
    return m_imsi;
}

void
NrGtpcModifyBearerRequestMessage::SetImsi(uint64_t imsi)
{
    m_imsi = imsi;
}

uint32_t
NrGtpcModifyBearerRequestMessage::GetUliEcgi() const
{
    return m_uliEcgi;
}

void
NrGtpcModifyBearerRequestMessage::SetUliEcgi(uint32_t uliEcgi)
{
    m_uliEcgi = uliEcgi;
}

std::list<NrGtpcModifyBearerRequestMessage::BearerContextToBeModified>
NrGtpcModifyBearerRequestMessage::GetBearerContextsToBeModified() const
{
    return m_bearerContextsToBeModified;
}

void
NrGtpcModifyBearerRequestMessage::SetBearerContextsToBeModified(
    std::list<NrGtpcModifyBearerRequestMessage::BearerContextToBeModified> bearerContexts)
{
    m_bearerContextsToBeModified = bearerContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcModifyBearerResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcModifyBearerResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcModifyBearerResponseMessage>();
    return tid;
}

NrGtpcModifyBearerResponseMessage::NrGtpcModifyBearerResponseMessage()
{
    SetMessageType(NrGtpcHeader::ModifyBearerResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
}

NrGtpcModifyBearerResponseMessage::~NrGtpcModifyBearerResponseMessage()
{
}

TypeId
NrGtpcModifyBearerResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcModifyBearerResponseMessage::GetMessageSize() const
{
    return serializedSizeCause;
}

uint32_t
NrGtpcModifyBearerResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcModifyBearerResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);
}

uint32_t
NrGtpcModifyBearerResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);

    return GetSerializedSize();
}

void
NrGtpcModifyBearerResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << (uint16_t)m_cause;
}

NrGtpcModifyBearerResponseMessage::Cause_t
NrGtpcModifyBearerResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcModifyBearerResponseMessage::SetCause(NrGtpcModifyBearerResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteBearerCommandMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteBearerCommandMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteBearerCommandMessage>();
    return tid;
}

NrGtpcDeleteBearerCommandMessage::NrGtpcDeleteBearerCommandMessage()
{
    SetMessageType(NrGtpcHeader::DeleteBearerCommand);
    SetSequenceNumber(0);
}

NrGtpcDeleteBearerCommandMessage::~NrGtpcDeleteBearerCommandMessage()
{
}

TypeId
NrGtpcDeleteBearerCommandMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteBearerCommandMessage::GetMessageSize() const
{
    uint32_t serializedSize =
        m_bearerContexts.size() * (serializedSizeBearerContextHeader + serializedSizeEbi);
    return serializedSize;
}

uint32_t
NrGtpcDeleteBearerCommandMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteBearerCommandMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    for (auto& bearerContext : m_bearerContexts)
    {
        SerializeBearerContextHeader(i, serializedSizeEbi);

        SerializeEbi(i, bearerContext.m_epsBearerId);
    }
}

uint32_t
NrGtpcDeleteBearerCommandMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    while (i.GetRemainingSize() > 0)
    {
        uint16_t length;
        DeserializeBearerContextHeader(i, length);

        BearerContext bearerContext;
        DeserializeEbi(i, bearerContext.m_epsBearerId);
        m_bearerContexts.push_back(bearerContext);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteBearerCommandMessage::Print(std::ostream& os) const
{
    os << " bearerContexts [";
    for (auto& bearerContext : m_bearerContexts)
    {
        os << (uint16_t)bearerContext.m_epsBearerId << " ";
    }
    os << "]";
}

std::list<NrGtpcDeleteBearerCommandMessage::BearerContext>
NrGtpcDeleteBearerCommandMessage::GetBearerContexts() const
{
    return m_bearerContexts;
}

void
NrGtpcDeleteBearerCommandMessage::SetBearerContexts(
    std::list<NrGtpcDeleteBearerCommandMessage::BearerContext> bearerContexts)
{
    m_bearerContexts = bearerContexts;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteBearerRequestMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteBearerRequestMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteBearerRequestMessage>();
    return tid;
}

NrGtpcDeleteBearerRequestMessage::NrGtpcDeleteBearerRequestMessage()
{
    SetMessageType(NrGtpcHeader::DeleteBearerRequest);
    SetSequenceNumber(0);
}

NrGtpcDeleteBearerRequestMessage::~NrGtpcDeleteBearerRequestMessage()
{
}

TypeId
NrGtpcDeleteBearerRequestMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteBearerRequestMessage::GetMessageSize() const
{
    uint32_t serializedSize = m_epsBearerIds.size() * serializedSizeEbi;
    return serializedSize;
}

uint32_t
NrGtpcDeleteBearerRequestMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteBearerRequestMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    for (auto& epsBearerId : m_epsBearerIds)
    {
        SerializeEbi(i, epsBearerId);
    }
}

uint32_t
NrGtpcDeleteBearerRequestMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    while (i.GetRemainingSize() > 0)
    {
        uint8_t epsBearerId;
        DeserializeEbi(i, epsBearerId);
        m_epsBearerIds.push_back(epsBearerId);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteBearerRequestMessage::Print(std::ostream& os) const
{
    os << " epsBearerIds [";
    for (auto& epsBearerId : m_epsBearerIds)
    {
        os << (uint16_t)epsBearerId << " ";
    }
    os << "]";
}

std::list<uint8_t>
NrGtpcDeleteBearerRequestMessage::GetEpsBearerIds() const
{
    return m_epsBearerIds;
}

void
NrGtpcDeleteBearerRequestMessage::SetEpsBearerIds(std::list<uint8_t> epsBearerId)
{
    m_epsBearerIds = epsBearerId;
}

/////////////////////////////////////////////////////////////////////

TypeId
NrGtpcDeleteBearerResponseMessage::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NrGtpcDeleteBearerResponseMessage")
                            .SetParent<Header>()
                            .SetGroupName("Nr")
                            .AddConstructor<NrGtpcDeleteBearerResponseMessage>();
    return tid;
}

NrGtpcDeleteBearerResponseMessage::NrGtpcDeleteBearerResponseMessage()
{
    SetMessageType(NrGtpcHeader::DeleteBearerResponse);
    SetSequenceNumber(0);
    m_cause = Cause_t::RESERVED;
}

NrGtpcDeleteBearerResponseMessage::~NrGtpcDeleteBearerResponseMessage()
{
}

TypeId
NrGtpcDeleteBearerResponseMessage::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
NrGtpcDeleteBearerResponseMessage::GetMessageSize() const
{
    uint32_t serializedSize = serializedSizeCause + m_epsBearerIds.size() * serializedSizeEbi;
    return serializedSize;
}

uint32_t
NrGtpcDeleteBearerResponseMessage::GetSerializedSize() const
{
    return NrGtpcHeader::GetSerializedSize() + GetMessageSize();
}

void
NrGtpcDeleteBearerResponseMessage::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;

    NrGtpcHeader::PreSerialize(i);
    SerializeCause(i, m_cause);

    for (auto& epsBearerId : m_epsBearerIds)
    {
        SerializeEbi(i, epsBearerId);
    }
}

uint32_t
NrGtpcDeleteBearerResponseMessage::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    NrGtpcHeader::PreDeserialize(i);

    DeserializeCause(i, m_cause);

    while (i.GetRemainingSize() > 0)
    {
        uint8_t epsBearerId;
        DeserializeEbi(i, epsBearerId);
        m_epsBearerIds.push_back(epsBearerId);
    }

    return GetSerializedSize();
}

void
NrGtpcDeleteBearerResponseMessage::Print(std::ostream& os) const
{
    os << " cause " << (uint16_t)m_cause << " epsBearerIds [";
    for (auto& epsBearerId : m_epsBearerIds)
    {
        os << (uint16_t)epsBearerId << " ";
    }
    os << "]";
}

NrGtpcDeleteBearerResponseMessage::Cause_t
NrGtpcDeleteBearerResponseMessage::GetCause() const
{
    return m_cause;
}

void
NrGtpcDeleteBearerResponseMessage::SetCause(NrGtpcDeleteBearerResponseMessage::Cause_t cause)
{
    m_cause = cause;
}

std::list<uint8_t>
NrGtpcDeleteBearerResponseMessage::GetEpsBearerIds() const
{
    return m_epsBearerIds;
}

void
NrGtpcDeleteBearerResponseMessage::SetEpsBearerIds(std::list<uint8_t> epsBearerId)
{
    m_epsBearerIds = epsBearerId;
}

} // namespace ns3
