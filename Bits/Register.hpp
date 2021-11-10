#pragma once

///////////////////////////////////////////////////////////////////////////////

#include "Bitmask.hpp"

#include <type_traits>
#include <functional>

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Allows easy access to the individual bit values and ranges of bits.
/// </summary>
/// <typeparam name="Register_T">The type of the register to be accessed</typeparam>
template<typename Register_T>
class RegisterValue : public bitmask::BitRangeAccessor<Register_T>
{
public:

    using Value_t = typename bitmask::BitRangeAccessor<Register_T>::Value_t;

    explicit RegisterValue(Value_t val)
        : bitmask::BitRangeAccessor<Register_T>{val}
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

template<typename Register_T>
class Register : public bitmask::BitRangeAccessor<Register_T>
{
public:
    using Value_t = typename bitmask::BitRangeAccessor<Register_T>::Value_t;

    using Reader = std::function<Value_t()>;
    using Writer = std::function<void(Value_t)>;

    Register(Reader getter, Writer setter, Value_t initial_value = Value_t{})
        : bitmask::BitRangeAccessor<Register_T>{initial_value}
        , m_reader{getter}
        , m_writer{setter}
    {
    }

    auto write(Value_t value) -> decltype(*this)&
    {
        this->raw() = value;
        return this->write();
    }

    auto write() const -> decltype(*this)&
    {
        m_writer(this->raw());

        return *this;
    }

    auto read() -> decltype(*this)&
    {
        this->raw() = m_reader();
        return *this;
    }

private:
    Reader m_reader;
    Writer m_writer;
};

///////////////////////////////////////////////////////////////////////////////

/// <summary>
/// A base address range is a range of register values assigned to a specified set of functionality, or area of usage.
/// </summary>
/// <typeparam name="Value_T">The type of the adresses in the register range</typeparam>
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

/// <summary>
/// A register address.
/// </summary>
/// <typeparam name="Value_T">The type of the register (e.g. uint32_t)</typeparam>
/// <typeparam name="BaseRange_T">The base address range for the register.</typeparam>
template<typename Value_T, typename BaseRange_T, typename BaseRange_T::Value_t OFFSET>
class RegisterAddress
{
public:
    using Value_t  = Value_T;
    using Offset_t = typename BaseRange_T::Value_t;

    static constexpr Offset_t base    = BaseRange_T::begin;
    static constexpr Offset_t offset  = OFFSET;
    static constexpr Offset_t address = BaseRange_T::begin + offset;

    static constexpr size_t size = sizeof(Value_t);

    static_assert(static_cast<uint64_t>(address) >= static_cast<uint64_t>(BaseRange_T::begin), "Register address is outside of it base range");
    static_assert(static_cast<uint64_t>(address) < static_cast<uint64_t>(BaseRange_T::end), "Register address is outside fof it base range");
};

///////////////////////////////////////////////////////////////////////////////
