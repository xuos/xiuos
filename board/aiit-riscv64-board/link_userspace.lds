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

OUTPUT_ARCH("riscv")

MEMORY
{
    usram (rw)  : ORIGIN = 0x80101000, LENGTH =  ( 5 * 1024 * 1024 - 4 * 1024 )  /* 5M -4k usram */
}

SECTIONS
{
    .userspace : 
    {
        . = ALIGN(8);
        KEEP(*(.userspace))
    } > usram

    .text : 
    {
        _ustext = ABSOLUTE(.);
        *(.text .text.*)
        *(.rodata .rodata*)
        *(.glue_7)
        *(.glue_7t)
        PROVIDE(_uetext = ABSOLUTE(.));
    } > usram
    
    .init_section : {
        _sinit = ABSOLUTE(.);
        KEEP(*(.init_array .init_array.*))
        _einit = ABSOLUTE(.);
    } > usram

    __uexidx_start = ABSOLUTE(.);

    .ARM.exidx :
    {
        PROVIDE(__uexidx_start = ABSOLUTE(.));
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        PROVIDE(__uexidx_end = ABSOLUTE(.));
    } > usram

    _ueronly = ABSOLUTE(.);

    .data : AT(_ueronly)
    {
        _usdata = ABSOLUTE(.);
        *(.data .data.*)
        . = ALIGN(8);
        _uedata = ABSOLUTE(.);
    } > usram 

    .bss : {
        . = ALIGN(8);
        _usbss = ABSOLUTE(.);
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        . = ALIGN(8);
        _uebss = ABSOLUTE(.);
    } > usram
}