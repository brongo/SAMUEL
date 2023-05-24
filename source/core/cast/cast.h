#pragma once

#include <map>
#include <utility>
#include <vector>
#include <string>
#include <memory>

namespace Cast {

    struct Vector2 {
        float x{}, y{};

        Vector2() = default;

        Vector2(float x, float y);
    };

    struct Vector3 {
        float x{}, y{}, z{};

        Vector3() = default;

        Vector3(float x, float y, float z);
    };

    struct Vector4 {
        float x{}, y{}, z{}, w{};

        Vector4() = default;

        Vector4(float x, float y, float z, float w);
    };

    struct CastHeader {
        uint32_t Magic;            // char[4] cast	(0x74736163)
        uint32_t Version;        // 0x1
        uint32_t RootNodes;        // Number of root nodes, which contain various sub nodes if necessary
        uint32_t Flags;            // Reserved for flags, or padding, whichever is needed
    };


    enum class CastPropertyId : uint16_t {
        Byte = 'b',            // <uint8_t>
        Short = 'h',        // <uint16_t>
        Integer32 = 'i',    // <uint32_t>
        Integer64 = 'l',    // <uint64_t>

        Float = 'f',        // <float>
        Double = 'd',        // <double>

        String = 's',        // Null terminated UTF-8 string

        Vector2 = 'v2',        // Float precision vector XY
        Vector3 = 'v3',        // Float precision vector XYZ
        Vector4 = 'v4'        // Float precision vector XYZW
    };

    struct CastPropertyHeader {
        CastPropertyId Identifier;    // The element type of this property
        uint16_t NameSize;            // The size of the name of this property
        uint32_t ArrayLength;        // The number of elements this property contains (1 for single)

        // Following is UTF-8 string lowercase, size of namesize, NOT null terminated
        // cast_property[ArrayLength] array of data
    };

    class CastProperty {
    public:
        virtual void serialize(std::ofstream &stream) const;

        virtual size_t calcSize() const;

        virtual size_t arraySize() const;

    protected:
        explicit CastProperty(std::string name, CastPropertyId mId) : m_name(std::move(name)), m_id(mId) {}

        CastPropertyId m_id;
        std::string m_name;
    };

#define DEFINE_CAST_PROP(NAME, TYPE, PROP_ID)                                                              \
class NAME##CastProperty : public CastProperty {                                                         \
    public:                                                                                             \
    explicit  NAME##CastProperty(std::string name) : CastProperty(std::move(name),PROP_ID), m_data{} {};\
    explicit  NAME##CastProperty(std::string name,TYPE value) : CastProperty(std::move(name),PROP_ID), m_data{value} {}\
    explicit  NAME##CastProperty(std::string name,const std::vector<TYPE> &&value) : CastProperty(std::move(name),PROP_ID), m_data(std::move(value)) {}\
    void append(TYPE value) { m_data.emplace_back(value); }                                                \
    void resize(size_t newSize){m_data.resize(newSize);}                                                   \
    [[nodiscard]] TYPE* data(){return m_data.data();}                                                      \
    TYPE& operator[](size_t index){return m_data[index];}                                                                                                       \
    std::vector<TYPE>& array(){return m_data;}                                                                                                       \
    void serialize(std::ofstream &stream) const override;                                                  \
    [[nodiscard]] size_t calcSize() const override;                                                                        \
    [[nodiscard]] size_t arraySize() const override{return m_data.size();};                                                                        \
private:\
std::vector<TYPE> m_data;\
};


    DEFINE_CAST_PROP(Uint8, uint8_t, CastPropertyId::Byte)

    DEFINE_CAST_PROP(Uint16, uint16_t, CastPropertyId::Short)

    DEFINE_CAST_PROP(Uint32, uint32_t, CastPropertyId::Integer32)

    DEFINE_CAST_PROP(Uint64, uint64_t, CastPropertyId::Integer64)

    DEFINE_CAST_PROP(Float, float, CastPropertyId::Float)

    DEFINE_CAST_PROP(Double, double, CastPropertyId::Double)

    DEFINE_CAST_PROP(Vector2, Vector2, CastPropertyId::Vector2)

    DEFINE_CAST_PROP(Vector3, Vector3, CastPropertyId::Vector3)

    DEFINE_CAST_PROP(Vector4, Vector4, CastPropertyId::Vector4)

#undef DEFINE_CAST_PROP

    class StringCastProperty : public CastProperty {
    public:
        explicit StringCastProperty(std::string name) :
                CastProperty(std::move(name), CastPropertyId::String),
                m_data{} {};

        explicit StringCastProperty(std::string name, const std::string &value) :
                CastProperty(std::move(name), CastPropertyId::String) { m_data.emplace_back(value); }

        explicit StringCastProperty(std::string name, const std::vector<std::string> &&value) :
                CastProperty(std::move(name), CastPropertyId::String),
                m_data(value) {}

        void append(const std::string &value) { m_data.emplace_back(value); }

        void resize(size_t newSize) { m_data.resize(newSize); }

        std::string *data() { return m_data.data(); }

        std::vector<std::string> &array() { return m_data; }

        std::string &operator[](size_t index) { return m_data[index]; }

        void serialize(std::ofstream &stream) const override;

        [[nodiscard]] size_t calcSize() const override;

        [[nodiscard]] size_t arraySize() const override { return m_data.size(); };

    private:
        std::vector<std::string> m_data;
    };


    enum class CastId : uint32_t {
        Root = 0x746F6F72,
        Model = 0x6C646F6D,
        Mesh = 0x6873656D,
        BlendShape = 0x68736C62,
        Skeleton = 0x6C656B73,
        Bone = 0x656E6F62,
        Animation = 0x6D696E61,
        Curve = 0x76727563,
        NotificationTrack = 0x6669746E,
        Material = 0x6C74616D,
        File = 0x656C6966,
    };

    struct CastNodeHeader {
        CastId Identifier;        // Used to signify which class this node uses
        uint32_t NodeSize;        // Size of all data and sub data following the node
        uint64_t NodeHash;        // Unique hash, like an id, used to link nodes together
        uint32_t PropertyCount;   // The count of properties
        uint32_t ChildCount;      // The count of direct children nodes
    };

    class CastNode {
    public:
        explicit CastNode(CastId id) : m_id(id) {};

        void addProperty(std::unique_ptr<CastProperty> prop) {
            m_properties.emplace_back(std::move(prop));
        }

        void addChildren(CastNode &&node) {
            m_children.emplace_back(std::move(node));
        }

        void serialize(std::ofstream &stream) const;

        [[nodiscard]] size_t calcSize() const;

    private:
        CastId m_id;
        std::vector<std::unique_ptr<CastProperty>> m_properties{};
        std::vector<CastNode> m_children{};
    };


}
