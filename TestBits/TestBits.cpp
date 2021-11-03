#include "CppUnitTest.h"

#include <Bits/Bitmask.hpp>
#include <Bits/Register.hpp>

#include <bitset>
#include <string>
#include <algorithm>

using namespace std::string_literals;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace test_bits
{


template<typename Value_T>
class TestRegisterFake
{
public:
    using Value_t = Value_T;

    explicit TestRegisterFake(std::string bit_str)
        : bit_string(std::move(bit_str))
    {
    }

    template<uint8_t FIRST_BIT, uint8_t LAST_BIT>
    Value_t bits_value() const
    {
        constexpr auto size = LAST_BIT - FIRST_BIT + 1;
        const auto temp     = bit_string.substr(bit_string.length() - FIRST_BIT - size, size);
        return static_cast<Value_t>(std::bitset<size>(temp).to_ullong());
    }

    template<uint8_t BIT>
    bool bit_value() const
    {
        return *std::next(bit_string.crbegin(), BIT) == '1';
    }

    Value_t value() const { return bits_value<0, 8*sizeof(Value_t) - 1>(); }

private:
    const std::string bit_string;
};

using TestRegisterFake_32 = TestRegisterFake<uint32_t>;
using TestRegisterFake_64 = TestRegisterFake<uint64_t>;

#define TEST_BIT_RANGE_GET_VALUE(size, first_bit, last_bit)            \
    Assert::AreEqual(uint##size##_t(test_reg.bits_value<first_bit, last_bit>()), \
                     bitmask::GetValue<bitmask::Bitrange<TestRegisterFake_##size, first_bit, last_bit>, uint##size##_t>(test_reg.value()))

#define TEST_SINGLE_BIT_GET_VALUE(size, bit)          \
    Assert::AreEqual(test_reg.bit_value<bit>(), \
                     static_cast<bool>(bitmask::GetValue<bitmask::Bitrange<TestRegisterFake_##size, bit, bit>>(test_reg.value())))

using MyRange = RegisterBaseAddressRange<uint64_t, 0x00000000, 0x00001000>;

using MyRegister = RegisterAddress<MyRange::Value_t, MyRange, 0x00000005>;

using Status     = bitmask::SingleBit<MyRegister, 0>;
using TestRange1 = bitmask::Bitrange<MyRegister, 1, 6>;
using TestRange2 = bitmask::Bitrange<MyRegister, 7, 31>;
using TestRange3 = bitmask::SingleBit<MyRegister, 33>;

/*
auto r1 = RegisterValue<MyRegister>(0u);

r1.set<Status>(true);
r1.set<TestRange1>(29);
r1.set<TestRange2, 33554431>();
// Won't build because the value won't fit into the register.
// r1.set<TestRange2, 66458843794>();
r1.set<TestRange3>(true);
*/

TEST_CLASS (Bitrange)
{
public:
    TEST_METHOD(MaskIsCorrect_1) { Assert::AreEqual(std::bitset<1>("1"s).to_ulong(), bitmask::Mask<unsigned long, 1>::value); }
    TEST_METHOD(MaskIsCorrect_2) { Assert::AreEqual(std::bitset<2>("11"s).to_ulong(), bitmask::Mask<unsigned long, 2>::value); }
    TEST_METHOD(MaskIsCorrect_3) { Assert::AreEqual(std::bitset<3>("111"s).to_ulong(), bitmask::Mask<unsigned long, 3>::value); }
    TEST_METHOD(MaskIsCorrect_4) { Assert::AreEqual(std::bitset<4>("1111"s).to_ulong(), bitmask::Mask<unsigned long, 4>::value); }
    TEST_METHOD(MaskIsCorrect_20) { Assert::AreEqual(std::bitset<20>("11111111111111111111"s).to_ulong(), bitmask::Mask<unsigned long, 20>::value); }
    TEST_METHOD(MaskIsCorrect_64) { Assert::AreEqual(std::bitset<64>(0xFFFFFFFFFFFFFFFF).to_ullong(), bitmask::Mask<unsigned long long, 64>::value); }

    TEST_METHOD(ShiftIsCorrectForZeroShift) { Assert::AreEqual(uint32_t(0x1), bitmask::Shift<uint32_t, 0x1, 0>::value); }
    TEST_METHOD(ShiftIsCorrectForShift_1) { Assert::AreEqual(uint32_t(0x1) << 1, bitmask::Shift<uint32_t, 0x1, 1>::value); }
    TEST_METHOD(ShiftIsCorrectForShift_10) { Assert::AreEqual(uint32_t(0x1) << 10, bitmask::Shift<uint32_t, 0x1, 10>::value); }
    TEST_METHOD(ShiftIsCorrectForShift64Bit_33) { Assert::AreEqual(uint64_t(0x1) << 33, bitmask::Shift<uint64_t, 0x1, 33>::value); }


    TEST_METHOD(BitRangeGetValueIsCorrect_32Bit)
    {
        //                                          3         2         1         0
        //                                         10987654321098765432109876543210 
        const auto test_reg = TestRegisterFake_32("11000000111110000101100001111001");

        TEST_BIT_RANGE_GET_VALUE(32, 0, 2);
        TEST_BIT_RANGE_GET_VALUE(32, 3, 6);
        TEST_BIT_RANGE_GET_VALUE(32, 30, 31);
        TEST_BIT_RANGE_GET_VALUE(32, 0, 31);
    }

    TEST_METHOD(SingleBitValueIsCorrect_32Bit)
    {
        //                                          3         2         1         0
        //                                         10987654321098765432109876543210
        const auto test_reg = TestRegisterFake_32("11000000111110000101100001111001");

        TEST_SINGLE_BIT_GET_VALUE(32, 0);
        TEST_SINGLE_BIT_GET_VALUE(32, 2);
        TEST_SINGLE_BIT_GET_VALUE(32, 30);
        TEST_SINGLE_BIT_GET_VALUE(32, 31);
    }

    TEST_METHOD(BitRangeGetValueIsCorrect_64Bit)
    {
        const auto test_reg = TestRegisterFake_64("1100000011111000010110000111100111000000111110000101100001111001");

        TEST_BIT_RANGE_GET_VALUE(64, 0, 2);
        TEST_BIT_RANGE_GET_VALUE(64, 3, 6);
        TEST_BIT_RANGE_GET_VALUE(64, 30, 31);
        TEST_BIT_RANGE_GET_VALUE(64, 45, 55);
        TEST_BIT_RANGE_GET_VALUE(64, 0, 63);
    }

    TEST_METHOD(SingleBitValueIsCorrect_64Bit)
    {
        const auto test_reg = TestRegisterFake_64("1100000011111000010110000111100111000000111110000101100001111001");

        TEST_SINGLE_BIT_GET_VALUE(64, 0);
        TEST_SINGLE_BIT_GET_VALUE(64, 2);
        TEST_SINGLE_BIT_GET_VALUE(64, 30);
        TEST_SINGLE_BIT_GET_VALUE(64, 45);
        TEST_SINGLE_BIT_GET_VALUE(64, 63);
    }
    
    using TestRegisterRange_32Bit = RegisterBaseAddressRange<uint32_t, 0x00000000, 0x00001000>;
    using TestRegister_32Bit = RegisterAddress<TestRegisterRange_32Bit::Value_t, TestRegisterRange_32Bit, 0x00000005>;
};
} // namespace test_bits
