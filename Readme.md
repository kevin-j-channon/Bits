# Bits: Simplify bit twiddling and hardware register access

Bits is a small set of, header-only classes that simplifies bit packing and unpacking bits into ints.  This is something that you mmmight do if you're packing bits into registers on some piece of hardware, or something like that.

## Usage

### Include
There are only two header files and you probably only need one of them: `Bits/Register.hpp`.  The other defines a the basic bit manipulation operations used in Register.hpp, you you wouldn't generally use that directly.  So, to use Bits, just put the two includes somewhere in your code and then you should be able to just do `#include <Bits/Register.hpp>` to get going.

### Register Ranges
Bits allows you to express the fact that your hardware address space is likely to be divided up into different areas for different general things. So, you might have some range of addresses that it's acceptable to use for GPIO, or another range that is used by the GPU, or something like that. To define a range then use the `RegisterBaseAddressRange` template.

Bits uses the C++ type system to statically check that the values that you're passing into it are correct. THis means that "defining" an address range is actually accomplished by defining a *type*, not a variable:

`using SystemControls = RegisterBaseAddressRange<uint32_t, 0x10000, 0x100000>;`

This range is used to validate (at compile-time) any register addresses that you want to specify inside the range. Compile-time checks ensure that:

- The range is well-formed (i.e. the end address is greater than the start address)
- Any register declared to be in the range is actually in the range

The `uint32_t` template type-parameter is the type of the addresses in the range. Typically, this might be `uint32_t`, as shown here, but maybe your system has only 16-bits of address space, or something. In this case, you'd put `uint16_t` in here and you're max accessible address would be `0xFFFF`.  The other two template parameters are the start and end addresses in the range. If you try to create an address for this address range, but outside these address values, then compilation will fail with a message telling you that you've specified an invalid address.

### Register Address
Once you have defined an address range, then you can define specific addresses within that range.  So, the `SystemControls` might contain addresses for things like "Main Fan info", "Left Arm Servo", "Position Sensor", and things like that.  You can define a Register Address like this:

`using MainFanInfo = RegisterAddress<uint32_t, SystemControls, 0x50>;`

This defines a 32-bit register in the `SystemControls` range with an offset of `0x50`. Note that the addresses of registers are expressed as *offsets* into their base range, not the absolute address.  So, in this case, the `MainFanInfo` address is actually `0x10050`, since the base range starts at `0x10000` and the offset for the address is `0x50`. The first template parameter determines how many bits are in the register. This is almost always the same as the type of the base address range that the address is part of, but it doesn't have to be.  So, you could have a 16-bit range that contained 64-bit registers, for example.

If your registers are not grouped into ranges in any meaningful way, then convevience classes `Any32BitAddress` and `Any64BitAddress` are provided for 32- and 64-bit ranges, respectively.  These simply define a range that spans the entire range of values from 0 to the max value for the type. For example:

```
using MainFanInfo = RegisterAddress<uint32_t, Any32BitAddress, 0x50>
```

### Bitmasks
In many cases, a single register will contain a number of pieces of data.  In these cases, the positions of the bits in the register have different meanings. So, bit 0 might be an "error bit", or bits 1-6 would be a 5-bit position indicator, or something.  you can express these meanings with a `bitmask::Bitrange` type:

`using FanTachoSpeed = bitmask::Bitrange<MainFanInfo, 0, 5>;`

In some cases, like the "error bit" example mentioned above, you might want to address only one bit. You can do this using `BitRange` and setting the start and end bits to the same index, but there is a dedicated type for it too:

`using FanError = bitmask::SingleBit<MainFanInfo, 0>;`

### Reading register values

Once the various aspects of your hardware are encoded into types in the manner previously described, then you can use them to get at the data in a reasonably readabable way.  To do this, you use an instance of `RegisterValue` to wrap the `int` that is inevitably returned by the underlying API that you're using to actually access the hardware.  That might look something like this:

```
// Here "GetValue()" is just meant to represent some API function that
// you call in your code to get a raw register value from some device, or other.
const auto fan_register = RegisterValue<MainFanInfo>{GetValue()};

// Get the value that we care about.
const auto fan_speed = fan_register.get<FanTachoSpeed>();
```

So, in this way, you can easily, safely and readably get at the value you're after.  If you're keen-eyed, you might have spotted that the fan tacho is only 5 bits, but the result is returned with the value-type of the underlying register.  If you would like to force the return value to a particular type (like a `uint8_t` in ths case, for example) then that can be supplied as a second template parameter:

```
// fan_speed has type uint8_t.
const auto fan_speed = fan_register.get<FanTachoSpeed, uint8_t>();
```

If the register is only a single bit, then there is some compile-time branching that results in the return value being a `bool`, instead of using the register type:

```
// is_in_error_state is a bool here.
const auto is_in_error_state = fan_register.get<FanError>();
```

### Writing register values

Writing works in a similar way to reading.  So, say there were 5 bits in the `MainFanInfo` register for some kind of speed set-point, or something. And maybe a "turbo" setting in the 31st bit, or something like that.  To write those, then you'd end up with something like:

```
using FanSpeedSetpoint = bitmask::Bitrange<MainFanInfo, 6, 10>;
using TurboActive = bitmask::Singlebit<MainFanInfo, 31>;

void warp_speed_mr_sulu() {
    auto fan_info = RegisterValue<MainFanInfo>{GetValue()};

    fan_info.set<FanSpeedSetpoint>(31);
    fan_info.set<TurboActive>(true);
}
```

### Direct register access

Bits also provides a wrapper class to contain accesssor functions that read and write values to your hardware too: `Register`. So, say you have some kind of `HardwareAccess` object in your code that does the reading and writing to the actual registers, or whatever. It might look something like this:

```
const uint32_t FAN_INFO_REGISTER = 0x10050;
const uint32_t FAN_ERROR_MASK = 0x00000001;
const uint32_t FAN_ERROR_SHIFT = 0;

class HardwareAccess
{
public:
    void ReadRegister(uint32_t address, uint32_t& value);
    void WriteRegister(uint32_t address, uint32_t value);
};
```

You might have some existing code that looks like:

```
bool SystemControls::GetFanStatus()
{
    const auto reg_val = uint32_t{0};
    m_hw_access.ReadRegister(FAN_INFO_REGISTER, reg_val);
    
    return 1 != ((reg_val & FAN_ERROR_MASK) >> FAN_ERROR_SHIFT)
}
```

Bits can make this more readable if you set up some registers in the constructor of `SystemControls`, so that we can refactor the `GetFanStatus` function to:

```
bool SystemControls::GetFanStatus()
{
    m_fan_info.read();
    return !m_fan_info.get<FanError>();
}
```

which is simpler to understand and less error prone when making changes in the area.  In order to do this, we have to set up some things in the constructor of the `SystemControls` object.  Brace yourself, because this is a bit of an eye-full if you're not ready for it:

```
SystemControls::SystemControls(HardwareAccess& hw_access)
    : m_fan_info{
        Register<MainFanInfo>{
            Register<MainFanInfo>::Reader{[&hw_access]() -> uint32_t {
                    uint32_t val;
                    hw_access.ReadRegister(MainFanInfo::address, val);
                    return val;
                }},
            Register<MainFanInfo>::Writer{[&hw_access](uint32_t){
                    hw_access.WriteRegister(MainFanInfo::address, val);
                }}
            }
         }
{
}
```

Now, the `SystemControls` class holds a `Register` object and that object manages all the interactions with the underlying hardware, *via* the `HardwareAccess` object that's injected in through the getter and setter functions.

## Example

```
#include <Bits/Register.hpp>

#include <iostream>
#include <string>
#include <utility>

using SystemControls = RegisterBaseAddressRange<uint32_t, 0x10000, 0x100000>;

namespace system_control_registers
{
using MainFan = RegisterAddress<uint32_t, SystemControls, 0x50>;
namespace main_fan
{
    using Error = bitmask::SingleBit<MainFan, 0>;
    using TachoSpeed = bitmask::Bitrange<MainFan, 1, 5>;
    using SpeedSetpoint = bitmask::Bitrange<MainFan, 6, 10>;
    using TurboActive = bitmask::SingleBit<MainFan, 31>;
}

using LeftArm = RegisterAddress<uint32_t, SystemControls, 0x51>;
namespace left_arm
{
    using Error = bitmask::SingleBit<LeftArm, 0>;
    using CurrentPosition = bitmask::BitRange<LeftArm, 1, 15>;
    using TargetPosition = bitmask::BitRange<LeftArm, 16, 30>;
    using Seeking = bitmask::SingleBit<LeftArm, 31>;
}

using RightArm = RegisterAddress<uint32_t, SystemControls, 0x56>;
namespace right_arm
{
    using Error = left_arm::Error;
    using CurrentPosition = left_arm::CurrentPosition;
    using TargetPosition = left_arm::TargetPosition;
    using Seeking = left_arm::Seeking;
}

int  main() {
    while (true)
    {
        std::cout << "enter new arm position" << std::endl;

        std::string arm_pos;
        std::cin >> arm_pos;

        const auto [which_arm, new_pos] = ParseInput(arm_pos);
        switch (which_arm)
        {
            case LEFT_ARM:
            {
                auto arm = Register<LeftArm>{GetLeftArm, SetLeftArm};
                if (arm.read().get<left_arm::CurrentPosition>() != new_pos)
                {
                    arm.set<left_arm::TargetPosition>(new_pos);
                    arm.write();

                    while (arm.read().get<left_arm::Seeking>())
                    {
                        ::Sleep(100);
                    }
                }

                break;
            }
            case RIGHT_ARM:
            {
                auto arm = Register<RightArm>{GetRightArm, SetRightArm};
                if (arm.read().get<right_arm::CurrentPosition>() != new_pos)
                {
                    arm.set<right_arm::TargetPosition>(new_pos);
                    arm.write();

                    while (arm.read().get<right_arm::Seeking>())
                    {
                        ::Sleep(100);
                    }
                }

                break;
            }
        }
    }

    return 0;
}
```
