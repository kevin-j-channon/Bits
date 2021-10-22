// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Register.h"

#include <iostream>
#include <bitset>

///////////////////////////////////////////////////////////////////////////////

using MyRange = RegisterBaseAddressRange<uint64_t, 0x00000000, 0x00001000>;

using MyRegister = RegisterAddress<MyRange::Value_t, MyRange, 0x00000005>;

using Status = bitmask::SingleBit<MyRegister, 0>;
using TestRange1 = bitmask::Bitrange<MyRegister, 1, 6>;
using TestRange2 = bitmask::Bitrange<MyRegister, 7, 31>;
using TestRange3 = bitmask::SingleBit<MyRegister, 33>;

///////////////////////////////////////////////////////////////////////////////

int main()
{
    auto r1 = RegisterValue<MyRegister>(0u);

    r1.set<Status>(true);
    r1.set<TestRange1>(29);
    r1.set<TestRange2>(66458843794);
    r1.set<TestRange3>(1);

    auto x = std::bitset<bitmask::WORD_SIZE * sizeof(MyRegister::Value_t)>(r1.raw());

    std::cout << "Raw value = " << x << std::endl;

    std::cout << "Status = " << std::boolalpha << r1.get<Status>() << std::endl;
    std::cout << "Range 1 = " << r1.get<TestRange1>() << std::endl;
    std::cout << "Range 2 = " << r1.get<TestRange2>() << std::endl;
    std::cout << "Range 3 = " << std::boolalpha << r1.get<TestRange3>() << std::endl;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
