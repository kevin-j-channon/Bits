#include "CppUnitTest.h"

#include <Bits/Bitmask.hpp>
#include <Bits/Register.hpp>

#include <bitset>
#include <string>

using namespace std::string_literals;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace test_bits
{
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
};
} // namespace test_bits
