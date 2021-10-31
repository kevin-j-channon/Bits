#pragma once

///////////////////////////////////////////////////////////////////////////////

#include "BitmaskWrapper.h"

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Allows easy access to the individual bit values and ranges of bits.
/// </summary>
/// <typeparam name="Register_T">The type of the register to be accessed</typeparam>
template<typename Register_T>
class RegisterValue
{
public:
    using Value_t = typename Register_T::Value_t;

    RegisterValue()
        : m_value(0)
    {
        static_assert(std::is_integral<Value_t>::value && std::is_unsigned<Value_t>::value, "A register value must be an integral type");
    }

    explicit RegisterValue(Value_t val)
        : m_value(val)
    {
        static_assert(std::is_integral<Value_t>::value && std::is_unsigned<Value_t>::value, "A register value must be an integral type");
    }

    const Value_t& raw() const { return m_value; }
    Value_t& raw() { return m_value; }

#if (__cplusplus >= 201703L)
    template<typename BitRange_T, typename Result_T = Value_t>
    auto get() const
    {
        if constexpr (BitRange_T::lowest_bit != BitRange_T::highest_bit)
        {
            static_assert(std::is_integral<Result_T>::value, "Result of a resister::get must be an integral type");

            return static_cast<Result_T>(bitmask::GetValue<BitRange_T, Value_t>(m_value));
        }
        else {
            return bitmask::GetValue<BitRange_T, Result_T>(m_value) != 0;
        }
    }
#else
    template<typename BitRange_T>
    std::enable_if_t<BitRange_T::lowest_bit != BitRange_T::highest_bit, Value_t> get() const
    {
        return get<BitRange_T, Value_t>();
    }

    template<typename BitRange_T, typename Result_T>
    std::enable_if_t<BitRange_T::lowest_bit != BitRange_T::highest_bit, Result_T> get() const
    {
        static_assert(std::is_integral<Result_T>::value, "Result of a resister::get must be an integral type");

        return static_cast<Result_T>(bitmask::GetValue<BitRange_T, Value_t>(m_value));
    }

    /// Get the value of the bits defined by BitRange_T.  This version is called when the Range is exactly one bit wide.
    template<typename BitRange_T>
    std::enable_if_t<BitRange_T::lowest_bit == BitRange_T::highest_bit, bool> get() const
    {
        return bitmask::GetValue<BitRange_T, Value_t>(m_value) != 0;
    }
#endif

#if (__cplusplus >= 201703L)
    template<typename BitRange_T>
    void set(auto value_to_set) {
        if constexpr (BitRange_T::lowest_bit != BitRange_T::highest_bit)
        {
            bitmask::SetValue<BitRange_T, Value_t>(m_value, value_to_set);
        }
        else
        {
            bitmask::SetValue<BitRange_T, Value_t>(m_value, value_to_set ? 1 : 0);
        }
    }
#else
    /// Set the value of the bits defined by BitRange_T.  This version is called when the Range is more than one bit wide.
    template<typename BitRange_T>
    void set(std::enable_if_t<BitRange_T::lowest_bit != BitRange_T::highest_bit, Value_t> value_to_set)
    {
        bitmask::SetValue<BitRange_T, Value_t>(m_value, value_to_set);
    }

    /// Set the value of the bits defined by BitRange_T.  This version is called when the Range is exactly one bit wide.
    template<typename BitRange_T>
    void set(std::enable_if_t<BitRange_T::lowest_bit == BitRange_T::highest_bit, bool> bit_value)
    {
        bitmask::SetValue<BitRange_T, Value_t>(m_value, bit_value ? 1 : 0);
    }
#endif

private:
    Value_t m_value;
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// 
/// </summary>
/// <typeparam name="Value_T"></typeparam>
template<typename Value_T, Value_T BEGIN, Value_T END>
class RegisterBaseAddressRange
{
public:
    using Value_t = Value_T;

    static constexpr Value_t begin = BEGIN;
    static constexpr Value_t end   = END;

    static constexpr Value_t size = end - begin;

    static_assert(begin < end, "Address range is malformed");
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T, typename BaseRange_T, typename BaseRange_T::Value_t OFFSET>
class RegisterAddress
{
public:
    using Value_t = Value_T;
    using Offset_t = typename BaseRange_T::Value_t;

    static constexpr Offset_t base    = BaseRange_T::begin;
    static constexpr Offset_t offset  = OFFSET;
    static constexpr Offset_t address = BaseRange_T::begin + offset;

    static constexpr size_t size = sizeof(Value_t);

    static_assert(static_cast<uint64_t>(address) >= static_cast<uint64_t>(BaseRange_T::begin), "Register address is outside of it base range");
    static_assert(static_cast<uint64_t>(address) < static_cast<uint64_t>(BaseRange_T::end), "Register address is outside fof it base range");
};

///////////////////////////////////////////////////////////////////////////////
