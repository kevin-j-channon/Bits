# Bits: Simplify bit twiddling and hardware register access

Bits is a small set of, header-only classes that simplifies bit packing and unpacking bits into ints.  This is something that you mmmight do if you're packing bits into registers on some piece of hardware, or something like that.

## Usage

### Include
There are only two header files and you probably only need one of them: `Bits/Register.hpp`.  The other defines a the basic bit manipulation operations used in Register.hpp, you you wouldn't generally use that directly.  So, to use Bits, just put the two includes somewhere in your code and then you should be able to just do `#include <Bits/Register.hpp>` to get going.

### Register Ranges
Bits allows you to express the fact that your hardware address space is likely to be divided up into different areas for different general things. So, you might have some range of addresses that it's acceptable to use for GPIO, or another range that is used by the GPU, or something like that. To define a range then use the `RegisterBaseAddressRange` template.

Bits uses the C++ type system to statically check that the values that you're passing into it are correct. THis means that "defining" an address range is actually accomplished by defining a *type*, not a variable:

`using SystemControls = RegisterBaseAddressRange<uint32_t, 0x10000, 0x100000>;`

This range can then be used to validate any register addresses that you want to specify inside the range.  The `uint32_t` parameter is the type of the addresses in the range. Typically, this might be `uint32_t`, as shown here, but maybe your system has only 16-bits of address space, or something. In this case, you'd put `uint16_t` in here and you're max accessible address would be `0xFFFF`.  The other two template parameters are the start and end addresses in the range. If you try to create an address for this address range, but outside these address values, then compilation will fail with a message telling you that you've specified an invalid address.

### Register Address
Once you have defined an address range, then you can define specific addresses within that range.  So, the `SystemControls` might contain addresses for things like "Main Fan info", "Left Arm Servo", "Position Sensor", and things like that.  You can define a Register Address like this:

`using MainFanInfo = RegisterAddress<uint32_t, SystemControls, 0x50>;`

This defines a 32-bit register in the `SystemControls` range with an offset of `0x50`. The addresses of registers are actually *offets* into theier base range, no the absolute address.  So, in this case, the `MainFanInfo` address is actually `0x10050`, since the base range starts at `0x10000` and the offset for the address is `0x50`. The first template parameter determines how many bits are in the register. This is almost always the same as the type of the base address range that the address is part of, but it doesn't have to be.  So, you could have a 16-bit range that contained 64-bit registers, for example.

### Bitmasks
In many cases, a single register will contain a number of pieces of data.

## Example

```
#include <Bits/Register.hpp>



int  main() {



}
```
