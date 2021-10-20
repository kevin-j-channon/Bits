#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <cstdint>

namespace bitmask
{

const uint8_t WORD_SIZE = 8;

///////////////////////////////////////////////////////////////////////////////

template< typename Value_T, int8_t SIZE>
struct Mask
{
    static_assert(SIZE < WORD_SIZE * sizeof(Value_T), "Mask size exceeds register size");

    static constexpr Value_T value = (1 << (SIZE - 1)) | Mask<Value_T, SIZE - 1>::value;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T>
struct Mask<Value_T, 0>
{
    static constexpr Value_T value = 0;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T, Value_T VALUE, int8_t SHIFT>
struct Shift
{
    static_assert(SHIFT < WORD_SIZE * sizeof(Value_T), "Shift exceeds value size");

    static constexpr Value_T value = VALUE << SHIFT;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T, uint8_t FIRST_BIT, uint8_t LAST_BIT>
struct BitMask
{
    static_assert(FIRST_BIT < WORD_SIZE * sizeof(Value_T), "First bit of bitmask is outside value range");
    static_assert(LAST_BIT < WORD_SIZE * sizeof(Value_T), "Last bit of bitmask is outside value range");
    static_assert(FIRST_BIT <= LAST_BIT, "Bit mask is out of order");

    static constexpr Value_T value = Shift<Value_T, Mask<Value_T, 1 + LAST_BIT - FIRST_BIT>::value, FIRST_BIT>::value;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T, uint8_t FIRST_BIT, uint8_t LAST_BIT>
struct BitRange
{
    using Range_t = Range_T;

    static_assert(FIRST_BIT < (WORD_SIZE * sizeof(Range_T)), "First bit of bit range is outside range");
    static_assert(LAST_BIT < (WORD_SIZE * sizeof(Range_T)), "Last bit of bit range is outside range");
    static_assert(FIRST_BIT <= LAST_BIT, "Bit range is out of order");

    static constexpr uint8_t first_bit = FIRST_BIT;
    static constexpr uint8_t last_bit  = LAST_BIT;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T, uint8_t BIT>
struct SingleBit : public BitRange<Value_T, BIT, BIT>
{
};

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T, typename Value_T>
Value_T GetValue(Value_T register_val)
{
    return (register_val & BitMask<typename Range_T::Range_t, Range_T::first_bit, Range_T::last_bit>::value) >> Range_T::first_bit;
}

///////////////////////////////////////////////////////////////////////////////

template<typename Range_T, typename Value_T>
void SetValue(Value_T& register_val, Value_T val)
{
    register_val &= ~BitMask<typename Range_T::Range_t, Range_T::first_bit, Range_T::last_bit>::value;
    register_val |= (val << Range_T::first_bit) & BitMask<typename Range_T::Range_t, Range_T::first_bit, Range_T::last_bit>::value;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace bitmask

///////////////////////////////////////////////////////////////////////////////
