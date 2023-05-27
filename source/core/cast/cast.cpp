#include "cast.h"

#include <fstream>

using namespace Cast;

void CastProperty::serialize(std::ofstream &stream) const {
    CastPropertyHeader header{m_id, static_cast<uint16_t>(m_name.size()), static_cast<uint32_t>(arraySize())};
    stream.write((char *) &header, sizeof(CastPropertyHeader));
    stream.write(m_name.data(), m_name.size());
}

void StringCastProperty::serialize(std::ofstream &stream) const {
    CastProperty::serialize(stream);
    for (const auto &item: m_data) {
        stream.write(item.data(), item.size());
        stream.write("\x00", 1);
    }
}

#define DEFINE_CAST_PROP_SERIALIZE(NAME)                                            \
void NAME::serialize(std::ofstream &stream) const {                   \
    CastProperty::serialize(stream);                                                \
    stream.write((char *) m_data.data(), m_data.size() * sizeof(m_data.front()));   \
}

DEFINE_CAST_PROP_SERIALIZE(Uint8CastProperty)

DEFINE_CAST_PROP_SERIALIZE(Uint16CastProperty)

DEFINE_CAST_PROP_SERIALIZE(Uint32CastProperty)

DEFINE_CAST_PROP_SERIALIZE(Uint64CastProperty)

DEFINE_CAST_PROP_SERIALIZE(FloatCastProperty)

DEFINE_CAST_PROP_SERIALIZE(DoubleCastProperty)

DEFINE_CAST_PROP_SERIALIZE(Vector2CastProperty)

DEFINE_CAST_PROP_SERIALIZE(Vector3CastProperty)

DEFINE_CAST_PROP_SERIALIZE(Vector4CastProperty)


size_t CastProperty::arraySize() const {
    return 0;
}

size_t CastProperty::calcSize() const {
    return 0;
}

size_t Uint8CastProperty::calcSize() const {
    return m_data.size() + sizeof(CastPropertyHeader) + m_name.size();
}

size_t Uint16CastProperty::calcSize() const {
    return m_data.size() * 2 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t Uint32CastProperty::calcSize() const {
    return m_data.size() * 4 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t Uint64CastProperty::calcSize() const {
    return m_data.size() * 8 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t FloatCastProperty::calcSize() const {
    return m_data.size() * 4 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t DoubleCastProperty::calcSize() const {
    return m_data.size() * 8 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t StringCastProperty::calcSize() const {
    size_t size = sizeof(CastPropertyHeader) + m_name.size();
    for (const auto &item: m_data) {
        size += item.size() + 1;
    }
    return size;
}

size_t Vector2CastProperty::calcSize() const {
    return m_data.size() * 8 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t Vector3CastProperty::calcSize() const {
    return m_data.size() * 12 + sizeof(CastPropertyHeader) + m_name.size();
}

size_t Vector4CastProperty::calcSize() const {
    return m_data.size() * 16 + sizeof(CastPropertyHeader) + m_name.size();
}

void CastNode::serialize(std::ofstream &stream) const {
    CastNodeHeader header{m_id, static_cast<uint32_t>(calcSize()),
                          m_hash,
                          static_cast<uint32_t>(m_properties.size()),
                          static_cast<uint32_t>(m_children.size())};
    stream.write((char *) &header, sizeof(CastNodeHeader));

    for (const auto &item: m_properties) {
        item->serialize(stream);
    }
    for (const auto &item: m_children) {
        item.serialize(stream);
    }
}

size_t CastNode::calcSize() const {
    size_t size = sizeof(CastNodeHeader);
    for (const auto &item: m_children) {
        size += item.calcSize();
    }
    for (const auto &item: m_properties) {
        size += item->calcSize();
    }
    return size;
}

Vector2::Vector2(float x, float y) : x(x), y(y) {}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
