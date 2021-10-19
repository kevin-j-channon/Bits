// BitmaskWrapper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Register.h"

#include <iostream>
#include <bitset>

///////////////////////////////////////////////////////////////////////////////

using MyRange = RegisterBaseAddressRange<uint32_t, 0x00000000, 0x00001000>;

using MyRegister = RegisterAddress<MyRange::Value_t, MyRange, 0x00000005>;

using Status = bitmask::SingleBit<0>;
using TestRange1 = bitmask::BitRange<1, 6>;
using TestRange2 = bitmask::BitRange<7, 31>;

///////////////////////////////////////////////////////////////////////////////

int main()
{
    auto r1 = RegisterValue(0u);

    r1.set<Status>(true);
    r1.set<TestRange1>(29);
    r1.set<TestRange2>(6645);

    auto x = std::bitset<32>(r1.raw());

    std::cout << "Raw value = " <<  x << std::endl;

    std::cout << "Status = " << std::boolalpha << r1.get<Status>() << std::endl;
    std::cout << "Range 1 = " << r1.get<TestRange1>() << std::endl;
    std::cout << "Range 2 = " << r1.get<TestRange2>() << std::endl;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
