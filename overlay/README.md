# Overlay

This is a demonstration of using overlays to dynamically load code from flash into CCMRAM. 

## Why?

In some situations you may have separate chunks of code that need to run from on-chip SRAM but which don't need to be available at the same time. Gnu ld provides a facility for doing this via overlays which allows several different functions to be linked to run in the same memory space yet be stored in non-volatile memory at different addresses. Then at run time you merely copy those chunks of binary from nonvolatile storage into SRAM immediately prior to execution. This can use on-chip RAM more efficiently, providing the benefits of fast execution without the need for cache while allowing code that's larger than available RAM.

## Resources

Overlays are described (somewhat tersely) in the [Gnu ld](https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_node/ld_22.html#SEC22) documentation. This gives the bare minimum for functionality and makes the feature seem more limited than it needs to be. In reality it can be much more flexible by employing sections attributes rather than hard-coding filenames into the linker script and this example demonstrates this (big thanks to [Vlad](https://vpme.de) for helping me get that as well as other details figured out).

Additionally, it's possible to place the location and size information into easily accessible constants as shown in [this example from Parallax](https://forums.parallax.com/discussion/163970/overlay-code-with-gcc)

## Example

This example code is a stripped-down demonstration of how it's done. A few highlights

- The linker script has two additions
  
  - The Overlay subsection that defines two overlays that will be resident in CCMRAM.
  
  - The table definition in the .data section that provides the location and size constants needed for loading the overlays.

- The main.c source that includes 
  
  - the functions that will reside in the overlay area in CCMRAM are defined with section and used attributes.
  
  - A function pointer array that is used to invoke the overlays
  
  - An extern definition used to access the location and size info that the linker creates
  
  - A loader function that copies the functions from Flash to CCMRAM

That's all there is to it. You can make it fancier with structures that combine the function pointers with the location / size info, but this is all that's really needed.

## Going Further

This example is a simple case where the overlay code is stored in on-chip flash (or in an off-chip XIP area that's mapped into the memory space) and can be included in the binary that's programmed along with the primary code. There are many use-cases where one might want to store the overlay code in off-chip flash that's separate and this will require further fiddling with the build process to filter the overlays from the main code. An example of this can be found [in this GitHub repository](https://github.com/Apress/Beg-STM32-Devel-FreeRTOS-libopencm3-GCC/tree/master/rtos/overlay1)
