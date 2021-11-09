#include "CppUnitTest.h"

#include <Bits/Bitmask.hpp>
#include <Bits/Register.hpp>

#include <bitset>
#include <string>
#include <algorithm>
#include <random>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////

using namespace std::string_literals;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

///////////////////////////////////////////////////////////////////////////////

namespace test_bits
{
///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A simple, but not efficient way of getting the values of bits in an int in a more readable way in a test.
/// </summary>
/// <typeparam name="Value_T">The type of the fake register (i.e. uint32_t, or uint64_t)</typeparam>
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

    Value_t value() const { return bits_value<0, 8 * sizeof(Value_t) - 1>(); }

private:
    const std::string bit_string;
};

///////////////////////////////////////////////////////////////////////////////

using TestRegisterFake_32 = TestRegisterFake<uint32_t>;
using TestRegisterFake_64 = TestRegisterFake<uint64_t>;

///////////////////////////////////////////////////////////////////////////////

#define TEST_BIT_RANGE_GET_VALUE(size, first_bit, last_bit)                      \
    Assert::AreEqual(uint##size##_t(test_reg.bits_value<first_bit, last_bit>()), \
                     bitmask::GetValue<bitmask::Bitrange<TestRegisterFake_##size, first_bit, last_bit>, uint##size##_t>(test_reg.value()))

#define TEST_SINGLE_BIT_GET_VALUE(size, bit)    \
    Assert::AreEqual(test_reg.bit_value<bit>(), \
                     static_cast<bool>(bitmask::GetValue<bitmask::Bitrange<TestRegisterFake_##size, bit, bit>>(test_reg.value())))

///////////////////////////////////////////////////////////////////////////////

TEST_CLASS (BitMask)
{
public:
    TEST_METHOD(MaskIsCorrect_1) { Assert::AreEqual(std::bitset<1>("1"s).to_ulong(), bitmask::Mask<unsigned long, 1>::value); }
    TEST_METHOD(MaskIsCorrect_2) { Assert::AreEqual(std::bitset<2>("11"s).to_ulong(), bitmask::Mask<unsigned long, 2>::value); }
    TEST_METHOD(MaskIsCorrect_3) { Assert::AreEqual(std::bitset<3>("111"s).to_ulong(), bitmask::Mask<unsigned long, 3>::value); }
    TEST_METHOD(MaskIsCorrect_4) { Assert::AreEqual(std::bitset<4>("1111"s).to_ulong(), bitmask::Mask<unsigned long, 4>::value); }
    TEST_METHOD(MaskIsCorrect_20) { Assert::AreEqual(std::bitset<20>("11111111111111111111"s).to_ulong(), bitmask::Mask<unsigned long, 20>::value); }
    TEST_METHOD(MaskIsCorrect_64) { Assert::AreEqual(std::bitset<64>(0xFFFFFFFFFFFFFFFF).to_ullong(), bitmask::Mask<unsigned long long, 64>::value); }
};

TEST_CLASS (BitShift)
{
public:
    TEST_METHOD(ShiftIsCorrectForZeroShift) { Assert::AreEqual(uint32_t(0x1), bitmask::Shift<uint32_t, 0x1, 0>::value); }
    TEST_METHOD(ShiftIsCorrectForShift_1) { Assert::AreEqual(uint32_t(0x1) << 1, bitmask::Shift<uint32_t, 0x1, 1>::value); }
    TEST_METHOD(ShiftIsCorrectForShift_10) { Assert::AreEqual(uint32_t(0x1) << 10, bitmask::Shift<uint32_t, 0x1, 10>::value); }
    TEST_METHOD(ShiftIsCorrectForShift64Bit_33) { Assert::AreEqual(uint64_t(0x1) << 33, bitmask::Shift<uint64_t, 0x1, 33>::value); }
};

///////////////////////////////////////////////////////////////////////////////

TEST_CLASS (BitRange)
{
public:
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

    TEST_METHOD(SetBitRangeValue_32)
    {
        // GIVEN
        //                                          3         2         1         0
        //                                         10987654321098765432109876543210
        const auto test_reg       = TestRegisterFake_32("11000000111110000101100001111001");
        auto reg_value            = test_reg.value();
        const auto new_bit_values = uint32_t{0xA};

        using TestBitrange = bitmask::Bitrange<TestRegisterFake_32, 7, 10>;

        // WHEN
        bitmask::SetValue<TestBitrange>(reg_value, new_bit_values);

        // THEN
        Assert::AreEqual(new_bit_values, bitmask::GetValue<TestBitrange, uint32_t>(reg_value));
    }

    TEST_METHOD(SetSingleBitValue_32)
    {
        // GIVEN
        //                                                s3         2         1         0
        //                                               10987654321098765432109876543210
        const auto test_reg = TestRegisterFake_32("11000000111110000101100001111001");
        auto reg_value      = test_reg.value();

        using TestBitrange = bitmask::SingleBit<TestRegisterFake_32, 28>;

        // WHEN
        bitmask::SetValue<TestBitrange>(reg_value, uint32_t{1});

        // THEN
        Assert::AreEqual(uint32_t{1}, bitmask::GetValue<TestBitrange, uint32_t>(reg_value));
    }
    TEST_METHOD(SetBitRangeValue_64)
    {
        // GIVEN
        //                                                  6         5         4         3         2         1         0
        //                                               3210987654321098765432109876543210987654321098765432109876543210
        const auto test_reg       = TestRegisterFake_64("0000000000000000000000000000000000000000000000000000000000000000");
        auto reg_value            = test_reg.value();
        const auto new_bit_values = uint64_t{0xF};

        using ZeroBits_lower = bitmask::Bitrange<TestRegisterFake_64, 0, 6>;
        using TestBitrange   = bitmask::Bitrange<TestRegisterFake_64, 7, 10>;
        using ZeroBits_upper = bitmask::Bitrange<TestRegisterFake_64, 11, 63>;

        // WHEN
        bitmask::SetValue<TestBitrange>(reg_value, new_bit_values);

        // THEN
        Assert::AreEqual(uint64_t{0}, bitmask::GetValue<ZeroBits_lower, uint64_t>(reg_value));
        Assert::AreEqual(new_bit_values, bitmask::GetValue<TestBitrange, uint64_t>(reg_value));
        Assert::AreEqual(uint64_t{0}, bitmask::GetValue<ZeroBits_upper, uint64_t>(reg_value));
    }

    TEST_METHOD(SetSingleBitValue_64)
    {
        // GIVEN
        //                                            6         5         4         3         2         1         0
        //                                         3210987654321098765432109876543210987654321098765432109876543210
        const auto test_reg = TestRegisterFake_64("0000000000000000000000000000000000000000000000000000000000000000");
        auto reg_value      = test_reg.value();

        using ZeroBits_lower = bitmask::Bitrange<TestRegisterFake_64, 0, 27>;
        using TestBitrange_1 = bitmask::SingleBit<TestRegisterFake_64, 28>;
        using ZeroBits_mid   = bitmask::Bitrange<TestRegisterFake_64, 29, 59>;
        using TestBitrange_2 = bitmask::SingleBit<TestRegisterFake_64, 60>;
        using ZeroBits_upper = bitmask::Bitrange<TestRegisterFake_64, 61, 63>;

        // WHEN
        bitmask::SetValue<TestBitrange_1>(reg_value, uint64_t{1});
        bitmask::SetValue<TestBitrange_2>(reg_value, uint64_t{1});

        // THEN
        Assert::AreEqual(uint64_t{0}, bitmask::GetValue<ZeroBits_lower, uint64_t>(reg_value));
        Assert::AreEqual(uint64_t{1}, bitmask::GetValue<TestBitrange_1, uint64_t>(reg_value));
        Assert::AreEqual(uint64_t{0}, bitmask::GetValue<ZeroBits_mid, uint64_t>(reg_value));
        Assert::AreEqual(uint64_t{1}, bitmask::GetValue<TestBitrange_2, uint64_t>(reg_value));
        Assert::AreEqual(uint64_t{0}, bitmask::GetValue<ZeroBits_upper, uint64_t>(reg_value));
    }
};

///////////////////////////////////////////////////////////////////////////////

TEST_CLASS (TestRegisterValue)
{
public:
    using TestRegRange_32 = RegisterBaseAddressRange<uint32_t, 0x00000000, 0x00001000>;
    using TestRegister_32 = RegisterAddress<TestRegRange_32::Value_t, TestRegRange_32, 0x20>;

    using TestRegRange_64 = RegisterBaseAddressRange<uint64_t, 0x0000000000000000, 0x0000100000000000>;
    using TestRegister_64 = RegisterAddress<TestRegRange_64::Value_t, TestRegRange_64, 0x50000>;

    TEST_METHOD(RawRegisterValue_32)
    {
        std::default_random_engine rng(23432); // Arbitrary seed.
        std::uniform_int_distribution<uint32_t> uniform_dist{};
        const auto val = uniform_dist(rng);

        const auto reg_value = RegisterValue<TestRegister_32>{val};
        Assert::AreEqual(val, reg_value.raw());
    }

    TEST_METHOD(RawRegisterValue_64)
    {
        std::default_random_engine rng(34234); // Arbitrary seed.
        std::uniform_int_distribution<uint64_t> uniform_dist{};
        const auto val = uniform_dist(rng);

        const auto reg_value = RegisterValue<TestRegister_64>{val};
        Assert::AreEqual(val, reg_value.raw());
    }

    TEST_METHOD(GetValue_32)
    {
        std::default_random_engine rng(993924); // Arbitrary seed.
        std::uniform_int_distribution<uint32_t> uniform_dist{};
        const auto val      = uniform_dist(rng);
        const auto test_reg = TestRegisterFake_32{std::bitset<32>{val}.to_string()};

        const auto reg_val = RegisterValue<TestRegister_32>{val};

        using field_1 = bitmask::Bitrange<TestRegister_32, 2, 10>;
        using field_2 = bitmask::Bitrange<TestRegister_32, 12, 15>;

        Assert::AreEqual(test_reg.bits_value<field_1::lowest_bit, field_1::highest_bit>(), reg_val.get<field_1>());
        Assert::AreEqual(test_reg.bits_value<field_2::lowest_bit, field_2::highest_bit>(), reg_val.get<field_2>());
    }

    TEST_METHOD(GetValue_64)
    {
        std::default_random_engine rng(111); // Arbitrary seed.
        std::uniform_int_distribution<uint64_t> uniform_dist{};
        const auto val      = uniform_dist(rng);
        const auto test_reg = TestRegisterFake_64{std::bitset<64>{val}.to_string()};

        const auto reg_val = RegisterValue<TestRegister_64>{val};

        using field_1 = bitmask::Bitrange<TestRegister_64, 2, 10>;
        using field_2 = bitmask::Bitrange<TestRegister_64, 12, 15>;
        using field_3 = bitmask::Bitrange<TestRegister_64, 33, 35>;
        using field_4 = bitmask::Bitrange<TestRegister_64, 55, 63>;

        Assert::AreEqual(test_reg.bits_value<field_1::lowest_bit, field_1::highest_bit>(), reg_val.get<field_1>());
        Assert::AreEqual(test_reg.bits_value<field_2::lowest_bit, field_2::highest_bit>(), reg_val.get<field_2>());
        Assert::AreEqual(test_reg.bits_value<field_3::lowest_bit, field_3::highest_bit>(), reg_val.get<field_3>());
        Assert::AreEqual(test_reg.bits_value<field_4::lowest_bit, field_4::highest_bit>(), reg_val.get<field_4>());
    }

    TEST_METHOD(SetValue_32)
    {
        auto reg_val = RegisterValue<TestRegisterFake_32>{0};

        using zeros_lower = bitmask::Bitrange<TestRegister_32, 0, 1>;
        using field_1     = bitmask::Bitrange<TestRegister_32, 2, 5>;
        using zeros_mid   = bitmask::Bitrange<TestRegister_32, 6, 20>;
        using field_2     = bitmask::SingleBit<TestRegister_32, 21>;
        using zeros_upper = bitmask::Bitrange<TestRegister_32, 22, 31>;

        reg_val.set<field_1>(15);
        reg_val.set<field_2>(true);

        Assert::AreEqual(uint32_t{0}, reg_val.get<zeros_lower>());
        Assert::AreEqual(uint32_t{15}, reg_val.get<field_1>());
        Assert::AreEqual(uint32_t{0}, reg_val.get<zeros_mid>());
        Assert::AreEqual(true, reg_val.get<field_2>());
        Assert::AreEqual(uint32_t{0}, reg_val.get<zeros_upper>());
    }

    TEST_METHOD(SetValue_64)
    {
        auto reg_val = RegisterValue<TestRegisterFake_64>{0};

        using zeros_lower = bitmask::Bitrange<TestRegister_64, 0, 1>;
        using field_1     = bitmask::Bitrange<TestRegister_64, 2, 5>;
        using zeros_mid   = bitmask::Bitrange<TestRegister_64, 6, 20>;
        using field_2     = bitmask::SingleBit<TestRegister_64, 21>;
        using zeros_upper = bitmask::Bitrange<TestRegister_64, 22, 52>;
        using field_3     = bitmask::Bitrange<TestRegisterFake_64, 53, 63>;

        reg_val.set<field_1>(15);
        reg_val.set<field_2>(true);
        reg_val.set<field_3>(bitmask::Mask<uint64_t, 10>::value);

        Assert::AreEqual(uint64_t{0}, reg_val.get<zeros_lower>());
        Assert::AreEqual(uint64_t{15}, reg_val.get<field_1>());
        Assert::AreEqual(uint64_t{0}, reg_val.get<zeros_mid>());
        Assert::AreEqual(true, reg_val.get<field_2>());
        Assert::AreEqual(uint64_t{0}, reg_val.get<zeros_upper>());
        Assert::AreEqual(bitmask::Mask<uint64_t, 10>::value, reg_val.get<field_3>());
    }
};

///////////////////////////////////////////////////////////////////////////////

TEST_CLASS (TestRegister)
{
public:
    TEST_METHOD(ConstructRegister)
    {
        std::function<uint32_t()> reader = []() { return uint32_t{12345678}; };

        auto write_val = uint32_t{};
        std::function<void(uint32_t)> writer = [&write_val](uint32_t v) { write_val = v; };

        auto reg = Register<TestRegisterFake_32>{reader, writer};

        reg.read();
        reg.write();

        Assert::AreEqual(reader(), write_val);
    }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace test_bits

///////////////////////////////////////////////////////////////////////////////
