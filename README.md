# f303k8_nucleo
Test to try Clang vs GCC

## What's this all about?
In a recent lengthy thread over on a mail list I'm on there emerged an assertion that GCC is crap for embedded ARM programming and anyone who's serious about good performance should be using Clang. And not just any Clang, but the versions from Apple/ARM/etc which apparently have some sort of sekrit sawce that the free version doesn't. Now I've been using the Gnu ARM Embedded version of GCC for all my Cortex Mx development for the last decade or so (keeping up with the quarterly releases) and I've had no complaints, but I'm willing to entertain that there's something better so I tried to get some simple STM32F3xx stuff compiling w/ Clang.

I found  a description here:

https://interrupt.memfault.com/blog/arm-cortexm-with-llvm-clang

This purports to show how to adapt a GCC makefile for Clang. There are some errors and omissions but after a few hours I got it compiling.

## Benchmarking
Having gotten the open-source Clang working, I began doing some benchmarks between it and several versions of GCC using a toy DSP application that's typical of the sort of thing I do in my day-to-day work. The results were interesting and not quite aligned with what I'd been told about the relative performance of the two toolchains so I wanted to try a proprietary build of Clang as well. I signed up for a 30-day trial of the ARM Development Studio which provides version 6.18 of their Clang-based toolchain and got that working as well. This yielded the following results:

<img src="doc/compiler_comparison_chart.png" width="640" />
<img src="doc/load_chart.png" width="640" />
<img src="doc/size_chart.png" width="640" />

### Notes
* This example is a simple audio DSP application running on an STM32F303K8 MCU with three channels of noise processed through a hardware floating point 4th-order filter model and streamed out to on-chip DACs via DMA. Sample rate is roughly 48ksps and the CPU is running at 64MHz clock rate.
* CPU load is computed by using the Cortex M4 cycle counter to measure the duration of the DMA buffer ISR. Time between IRQs is the period and duty cycle of the DSP is computed by sampling the start and end cycle counts. There are no significant I/O operations taking place during the measured time, only operatons to on-board SRAM. 100% load means that the DSP ISR is using all avaliable CPU cycles, so lower numbers indicate more efficient computations.
* Size is just the raw binary size of the full embedded application, including setup code.
* GCC does not have an -Oz setting so those values are left blank.
* I had posted some earlier results that were run at a slightly lower sample rate and thus had lower overall CPU load percentages. If you notice any differences between these results and the earlier ones it's caused by that change.
* An earlier version of this benchmark neglected to enable Link Time Optimization on the ARM Clang runs. Adding that made little difference in the load but reduced the size somewhat.

## Summary
Based on this one example it appears that recent builds of GCC are not grossly out of line with the performance of Clang in both the free and proprietary flavors. For all levels of optimization greater than -O0 GCC performs roughly as well if not better than either version of Clang. This test also shows very little difference in CPU load between the free and proprietary versions of Clang, although LTO did reduce the binary size of the ARM Clang output.