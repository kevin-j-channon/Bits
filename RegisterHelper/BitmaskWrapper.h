#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

namespace bitmask
{

const uint8_t WORD_SIZE = 8;

///////////////////////////////////////////////////////////////////////////////

template< typename RegValue_T, int8_t SIZE>
struct Mask
{
    static_assert(SIZE < WORD_SIZE * sizeof(RegValue_T), "Mask size exceeds register size");

    static constexpr RegValue_T value = (1 << (SIZE - 1)) | Mask<RegValue_T, SIZE - 1>::value;
};

///////////////////////////////////////////////////////////////////////////////

template<typename RegValue_T>
struct Mask<RegValue_T, 0>
{
    static constexpr RegValue_T value = 0;
};

///////////////////////////////////////////////////////////////////////////////

template<typename RegValue_T, RegValue_T VALUE, int8_t SHIFT>
struct Shift
{
    static_assert(SHIFT < WORD_SIZE * sizeof(RegValue_T), "Shift exceeds register size");

    static constexpr RegValue_T value = VALUE << SHIFT;
};

///////////////////////////////////////////////////////////////////////////////

template<typename RegValue_T, uint8_t FIRST_BIT, uint8_t LAST_BIT>
struct BitMask
{
    static_assert(FIRST_BIT < WORD_SIZE * sizeof(RegValue_T), "First bit of bitmask is outside register range");
    static_assert(LAST_BIT < WORD_SIZE * sizeof(RegValue_T), "Last bit of bitmask is outside register range");
    static_assert(FIRST_BIT <= LAST_BIT, "Bit mask is out of order");

    static constexpr RegValue_T value = Shift<RegValue_T, Mask<RegValue_T, 1 + LAST_BIT - FIRST_BIT>::value, FIRST_BIT>::value;
};

///////////////////////////////////////////////////////////////////////////////

template<uint8_t FIRST_BIT, uint8_t LAST_BIT>
struct BitRange
{
    static constexpr uint8_t first_bit = FIRST_BIT;
    static constexpr uint8_t last_bit  = LAST_BIT;
};

///////////////////////////////////////////////////////////////////////////////

template<uint8_t BIT>
struct SingleBit : public BitRange<BIT, BIT>
{
};

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T, typename Value_T>
Value_T GetValue(Value_T register_val)
{
    static_assert(Range_T::first_bit < WORD_SIZE * sizeof(Value_T), "First bit in bit mask is out-of-range");
    static_assert(Range_T::last_bit < WORD_SIZE * sizeof(Value_T), "Last bit in bit mask is out-of-range");
    static_assert(Range_T::first_bit <= Range_T::last_bit, "First and last bits in bit mask are out of order");

    return (register_val & BitMask<Range_T::first_bit, Range_T::last_bit>::value) >> Range_T::first_bit;
}

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T, typename Value_T>
void SetValue(Value_T& register_val, Value_T val)
{
    register_val &= ~BitMask<Range_T::first_bit, Range_T::last_bit>::value;
    auto x1 = val << Range_T::first_bit;
    auto x2 = Range_T::first_bit;
    auto x3 = BitMask<Range_T::first_bit, Range_T::last_bit>::value;
    x1 &= BitMask<Range_T::first_bit, Range_T::last_bit>::value;
    register_val |= (val << Range_T::first_bit) & BitMask<Range_T::first_bit, Range_T::last_bit>::value;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace bitmask

///////////////////////////////////////////////////////////////////////////////
