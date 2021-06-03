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

__SYSTEM_STACKSIZE__ = 0x400;

OUTPUT_ARCH(arm)

MEMORY
{
    flash (rx) : ORIGIN = 0x08000000, LENGTH = 256k /* 256KB flash */
    sram (rw) :  ORIGIN = 0x20001000, LENGTH =  124k  /* 124k sram */
}

ENTRY(Reset_Handler)
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        PROVIDE(_stext = ABSOLUTE(.));
        KEEP(*(.isr_vector))                   /* Startup code */


        . = ALIGN(4);
        *(.text .text.*)                        /* Normal code */
        *(.rodata .rodata*)                     /* read-only data (constants) */

        *(.glue_7)
        *(.glue_7t)

        /* section information for  shell */
        . = ALIGN(4);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4);

        /* the work i did */
        PROVIDE(__ctors_start__ = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE(__ctors_end__ = .);

        . = ALIGN(4);
        /* the work i did */
        
        __isrtbl_idx_start = .;
        KEEP(*(.isrtbl.idx))
        __isrtbl_start = .;
        KEEP(*(.isrtbl))
        __isrtbl_end = .;
        . = ALIGN(4);

        PROVIDE(g_service_table_start = ABSOLUTE(.));
        KEEP(*(.g_service_table))
        PROVIDE(g_service_table_end = ABSOLUTE(.));

        PROVIDE(_etext =  ABSOLUTE(.));
    } > flash

    __exidx_start = .;
    .ARM.exidx :
    {
        PROVIDE(__exidx_start = ABSOLUTE(.));
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        _sidata = .;
        PROVIDE(__exidx_end = ABSOLUTE(.));
    } > flash

    .data : AT(__exidx_end)
    {
        . = ALIGN(4);
        PROVIDE(_sdata = ABSOLUTE(.));
        *(.data .data.*)
        PROVIDE(_edata = ABSOLUTE(.));
    } > sram

    .bss :
    {
        . = ALIGN(4);
        __bss_start = ABSOLUTE(.);

        *(.bss .bss.*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end = ABSOLUTE(.);
    } > sram

    .stack : 
    {
        . = ALIGN(4);
        PROVIDE(__stack_start__ = ABSOLUTE(.));
        /* cpu stack */
        . = . + __SYSTEM_STACKSIZE__;
        __stack_tp = .;
        PROVIDE(__stack_end__ = ABSOLUTE(.));
    } > sram

    _end = ABSOLUTE(.);
}
