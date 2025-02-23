#include <ShmitCore/Data/Packet.hpp>

#include <cxxabi.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <typeinfo>

using namespace shmit;
using namespace shmit::data;

template<typename... FieldT>
std::string PrintPacketFields(Packet<FieldT...> const& packet);

template<typename T>
std::string DataString(T const& data)
{
    int               status;
    std::stringstream ss;
    ss << data << " (" << abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status) << ")";
    return ss.str();
}

template<>
std::string DataString(unsigned char const& data)
{
    int               status;
    std::stringstream ss;
    ss << static_cast<int>(data) << " (unsigned char)";
    return ss.str();
}

template<typename... FieldT>
std::string DataString(Packet<FieldT...> const& data)
{
    std::stringstream ss;
    ss << "\n{\n" << PrintPacketFields(data) << "}";
    return ss.str();
}

template<typename T>
struct print_field_info;

template<typename T>
struct print_field_info<Field<T>>
{
    static std::string Do()
    {
        std::stringstream ss;
        ss << "Field -> " << Field<T>::kSizeBits << " bits";
        return ss.str();
    }
};

template<typename... FieldT>
struct print_field_info<Packet<FieldT...>>
{
    static std::string Do()
    {
        int               status;
        std::stringstream ss;

        ss << "Nested Packet: ";
        ss << abi::__cxa_demangle(typeid(Packet<FieldT...>).name(), nullptr, nullptr, &status) << " -> "
           << Field<Packet<FieldT...>>::kSizeBits << " bits";
        return ss.str();
    }
};

template<size_t N>
struct print_field_info<BitField<N>>
{
    static std::string Do()
    {
        std::stringstream ss;
        ss << "BitField -> ";
        ss << N << " bits";
        return ss.str();
    }
};

template<size_t N>
struct print_field_info<ConstBitField<N>>
{
    static std::string Do()
    {
        std::stringstream ss;
        ss << "Const Bitfield -> ";
        ss << N << " bits";
        return ss.str();
    }
};

template<size_t IndexV, size_t EndV, typename... FieldT>
struct PrintPacketField
{
    using FieldPack = typename Packet<FieldT...>::Fields;
    using Field     = typename std::tuple_element<IndexV, FieldPack>::type;
    using Data      = typename Field::value_type;

    static_assert(IndexV < EndV, "Out of bounds");

    static std::string Do(Packet<FieldT...> const& packet)
    {
        Data const& value {packet_field_value<IndexV>(packet)};

        std::stringstream ss;
        ss << IndexV << ": " << print_field_info<Field>::Do();
        ss << " = " << DataString(value);
        ss << std::endl << PrintPacketField<IndexV + 1, EndV, FieldT...>::Do(packet);

        return ss.str();
    }
};

template<size_t IndexV, typename... FieldT>
struct PrintPacketField<IndexV, IndexV, FieldT...>
{
    static std::string Do(Packet<FieldT...> const& packet)
    {
        // Do nothing
        return "";
    }
};

template<typename... FieldT>
std::string PrintPacketFields(Packet<FieldT...> const& packet)
{
    using FieldPack = typename Packet<FieldT...>::Fields;
    return PrintPacketField<0U, std::tuple_size<FieldPack>::value, FieldT...>::Do(packet);
}

template<typename... FieldT>
void PrintPacket(Packet<FieldT...> const& packet)
{
    int status;
    std::cout << "Packet:\n"
              << abi::__cxa_demangle(typeid(packet).name(), nullptr, nullptr, &status) << "\r\n\nFields:\n"
              << abi::__cxa_demangle(typeid(typename Packet<FieldT...>::Fields).name(), nullptr, nullptr, &status)
              << "\r\n\n"
              << "Breakdown:\n"
              << PrintPacketFields(packet) << "\nTotal size: " << Packet<FieldT...>::kSizeBits << " Bits ("
              << Packet<FieldT...>::kSizeBytes << " Bytes)" << std::endl;
}

struct TestField
{
    int value {0};
};

std::ostream& operator<<(std::ostream& os, TestField const& field)
{
    os << field.value;
    return os;
}

using NestedPacket = packet_t<bool, int>;

using TestPacket = packet_t<TestField, NestedPacket, ConstBitField<4>, bool, Field<long>, ConstBitField<10>, Bit,
                            ConstBit, BitField<5>, BitField<11>, BitField<24>>;

int main()
{
    TestPacket packet {TestField {-96}, NestedPacket {true, 42}, 7, true, 420, 1023, 1, 0, 20, 40, 69};

    int status;
    std::cout << "Is packet: " << abi::__cxa_demangle(typeid(is_packet<TestPacket>).name(), nullptr, nullptr, &status)
              << std::endl
              << std::endl;

    PrintPacket(packet);

    return 0;
}