// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Register.h"

#include <iostream>
#include <bitset>

#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"

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
    auto console    = spdlog::stdout_color_mt("console");
    auto err_logger = spdlog::stderr_color_mt("stderr");
    spdlog::get("console")->info("loggers can be retrieved from a global registry using the spdlog::get(logger_name)");

    auto r1 = RegisterValue<MyRegister>(0u);

    r1.set<Status>(true);
    r1.set<TestRange1>(29);
    r1.set<TestRange2>(423423); // 66458843794);
    r1.set<TestRange3>(true);

    auto x = std::bitset<bitmask::WORD_SIZE * sizeof(MyRegister::Value_t)>(r1.raw());

    spdlog::info("Raw value = {}", x.to_ullong());

    spdlog::info("Status = {}", r1.get<Status>());
    spdlog::info("Range 1 = {}", r1.get<TestRange1>());
    spdlog::info("Range 2 = {}", r1.get<TestRange2>());
    spdlog::info("Range 3 = {}", r1.get<TestRange3>());

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
