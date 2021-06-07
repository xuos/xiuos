/*
* Copyright (c) 2020 AIIT XUOS Lab
* XiUOS is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*        http://license.coscl.org.cn/MulanPSL2
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
* See the Mulan PSL v2 for more details.
*/

OUTPUT_ARCH(arm)

MEMORY
{
    uflash (rx) : ORIGIN = 0x08040000, LENGTH = 256k /* 256KB uflash */
    usram (rw)  : ORIGIN = 0x10000000, LENGTH =  64k  /* 64K usram */
}

SECTIONS
{
    .userspace : 
    {
        . = ALIGN(4);
        KEEP(*(.userspace))
    } > uflash

    .text : 
    {
        _ustext = ABSOLUTE(.);
        *(.text .text.*)
        *(.rodata .rodata*)
        *(.glue_7)
        *(.glue_7t)
        PROVIDE(_uetext = ABSOLUTE(.));
    } > uflash
    
    .init_section : {
        _sinit = ABSOLUTE(.);
        KEEP(*(.init_array .init_array.*))
        _einit = ABSOLUTE(.);
    } > uflash

    __uexidx_start = ABSOLUTE(.);

    .ARM.exidx :
    {
        PROVIDE(__uexidx_start = ABSOLUTE(.));
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        PROVIDE(__uexidx_end = ABSOLUTE(.));
    } > uflash

    _ueronly = ABSOLUTE(.);

    .data : AT(_ueronly)
    {
        _usdata = ABSOLUTE(.);
        *(.data .data.*)
        . = ALIGN(4);
        _uedata = ABSOLUTE(.);
    } > usram 

    .bss : {
        . = ALIGN(4);
        _usbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        . = ALIGN(4);
        _uebss = ABSOLUTE(.);
    } > usram
}