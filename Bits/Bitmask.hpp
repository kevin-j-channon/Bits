#pragma once

///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cassert>
#include <cmath>

namespace bitmask
{
/// <summary>
/// The number of bits in a byte on the target system.
/// </summary>
const uint8_t WORD_SIZE = 8;

constexpr uint64_t static_power_2(auto N)
{
    return N > 0 ? 2 * static_power_2(N - 1) : 1;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A Mask has a "value" member with type Value_T with the lowest SIZE bits set.
/// </summary>
/// <typeparam name="Value_T">The type of value that this mask will be applied to. It determines the maximum permissible size of the mask.</typeparam>
template<typename Value_T, int8_t SIZE>
struct Mask
{
    static_assert(SIZE <= WORD_SIZE * sizeof(Value_T), "Mask size exceeds register size");

    static constexpr Value_T value = (Value_T(1) << (SIZE - 1)) | Mask<Value_T, SIZE - 1>::value;
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// This partial template specialization is used to terminate the recursion that sets all the bits in the mask.
/// </summary>
/// <typeparam name="Value_T">
/// The type of the value that this mask will be applied to. Not directly used in this struct, but is required to match the
/// type required to terminate the recursion in Mask&lt;typename Value_T, int8_t SIZE&gt;
/// </typeparam>
template<typename Value_T>
struct Mask<Value_T, 0>
{
    static constexpr Value_T value = Value_T(0);
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A  shift contains a "value" member that is equal to VALUE bitshifted to the left by SHIFT bits. It is intended to shift a Mask<>::value into the
/// correct place to mask a target value from a register.
/// </summary>
/// <typeparam name="Value_T">
/// The type of the value that the final, shifted mask will be aplied to. Not directly used in this class, but used to check
/// whether the specified shift is sensible, given the type of the target values.
/// </typeparam>
template<typename Value_T, auto VALUE, int8_t SHIFT>
struct Shift
{
    static_assert(SHIFT < WORD_SIZE * sizeof(Value_T), "Shift exceeds value size");

    static constexpr Value_T value = VALUE << SHIFT;
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A Bitmask contains a "value" member that will mask a value of type Value_T from LOWEST_BIT to HIGHEST_BIT.
/// </summary>
/// <typeparam name="Value_T">The type of the target values to be masked.</typeparam>
template<typename Register_T, uint8_t LOWEST_BIT, uint8_t HIGHEST_BIT>
struct Bitrange
{
    using Value_t = typename Register_T::Value_t;

    static_assert(LOWEST_BIT < WORD_SIZE * sizeof(Value_t), "First bit of bitmask is outside value range");
    static_assert(HIGHEST_BIT < WORD_SIZE * sizeof(Value_t), "Last bit of bitmask is outside value range");
    static_assert(LOWEST_BIT <= HIGHEST_BIT, "Bit mask is out of order");

    static constexpr uint8_t lowest_bit  = LOWEST_BIT;
    static constexpr uint8_t highest_bit = HIGHEST_BIT;
    static constexpr uint8_t size        = 1 + highest_bit - lowest_bit;
    
    static constexpr auto max() { return static_power_2(size) - 1; }

    static constexpr Value_t mask = Shift<Value_t, Mask<Value_t, 1 + highest_bit - lowest_bit>::value, lowest_bit>::value;
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A specialization of BitRange that is only a single bit wide.
/// </summary>
/// <typeparam name="Register_T">The type of the target value to be masked</typeparam>
template<typename Register_T, uint8_t BIT>
struct SingleBit : public Bitrange<Register_T, BIT, BIT>
{
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Extract the value in the specified bit range from the provided register value.
/// </summary>
/// <typeparam name="Range_T">The range type that will be used to mask register_val to get the result.</typeparam>
/// <typeparam name="Value_T">The target value type.</typeparam>
/// <param name="register_val">The value that contains the bits from which to extract the result.</param>
/// <returns>The value stored in the specified bits of register_val.</returns>
template<typename Range_T, typename Value_T>
Value_T GetValue(Value_T register_val)
{
    return (register_val & Range_T::mask) >> Range_T::lowest_bit;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Set the value into the specified bits of the provided register value.
/// </summary>
/// <typeparam name="Range_T">The range type that will be used to determine which bits in register_val are set.</typeparam>
/// <typeparam name="Value_T">The target value type.</typeparam>
/// <param name="register_val">The value that will contain the final bit values.</param>
/// <param name="val">The value to set into the specified bits of register_val</param>
template<typename Range_T, typename Value_T>
void SetValue(Value_T& register_val, Value_T val)
{
    assert(val <= std::powl(2, Range_T::size));

    register_val &= ~Range_T::mask;
    register_val |= (val << Range_T::lowest_bit) & Range_T::mask;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace bitmask

///////////////////////////////////////////////////////////////////////////////
