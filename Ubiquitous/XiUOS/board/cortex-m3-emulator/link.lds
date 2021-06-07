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

MEMORY
{
    flash (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    sram (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
OUTPUT_ARCH(arm)

__SYSTEM_STACKSIZE__ = 0x1000;

ENTRY(Reset_Handler)
SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))            /* Startup code */
        . = ALIGN(4);
        *(.text .text.*)               
        *(.rodata .rodata*)             /* read-only data (constants) */
        *(.glue_7)
        *(.glue_7t)

        /* section information for  shell */
        . = ALIGN(4);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(4);
      
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

        _etext = .;
    } > flash = 0

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        _sidata = .;
    } > flash
    __exidx_end = .;

    .data : AT (_sidata)
    {
        . = ALIGN(4);
        _sdata = . ;

        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = . ;
    } >sram

    __bss_start = .;
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
    } > sram
    __bss_end = .;
    _end = .;

   .stack ORIGIN(sram) + LENGTH(sram) - __SYSTEM_STACKSIZE__ :
  {
    PROVIDE( _heap_end = . );
    . = __SYSTEM_STACKSIZE__;
    PROVIDE( _sp = . );
  } >sram 
   
}
