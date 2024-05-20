# STM32F767zi-ILI9341-FMC-16bit-Parallel-Interface
For my library, I imitate, modify and simplify the two following ILI9341 libraries using FSMC for STM32F4

https://github.com/taburyak/STM32-ILI9341-320x240-FSMC-Library (this one is awesome, complete and useful)

[iwatake's digital camera OV7](https://www.youtube.com/watch?v=FAS0qRHHPxc&list=LL&index=3&t=1217s)

Most ILI9341 libraries, using FMC/FSMC, are for STM32F4. The problem when porting those libaries to F7 is that, in F7 microcontrollers,
the memory system is more complex, involving cache as well. This makes the use of FMC to interface with memory-like devices (i.e TFT LCD) 
not as simple as configging FMC through CubeMX and done. The problem was completely solved and the solution can be found
on STM community forums. Thus, I simply reorganize and add the MPU config and document it, so I can have something
to reference later without having to re-search solutions online again. That is the purpose for this repo.

IMPORTANT NOTES:
  TFT LCD works like SRAM, that is why using FMC to interface is possible. The 16bit parallel interface used is also called Intel's 8080 Parallel Interface
  FMC takes care of signal pin timings and it offers two SRAM memory addresses to write/read command and data(internal to microcontroller) ==>  greatly simplify interface procedures
  With ILI9341, PIN BL (backlight) and PIN RST (reset) must be held HIGH continously in order for LCD to work properly (took me a few days to learn this)
  ILI9341 has its own RAM, called GRAM aka Graphic RAM

"Driving an LCD module is driving the LCD driver IC"
