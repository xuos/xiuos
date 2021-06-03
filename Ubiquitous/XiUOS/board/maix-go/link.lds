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

__STACKSIZE__ = 4096;

OUTPUT_ARCH( "riscv" )

MEMORY
{
   sram (wxa!ri):  ORIGIN = 0x80000000, LENGTH = ( 6 * 1024 * 1024 )
}

ENTRY(_begin)
SECTIONS
{
    .start :
    {
        *(.start);
    } > sram

    . = ALIGN(8);

    .text : 
    {
        PROVIDE( _text = ABSOLUTE(.) );

        *(.text .text.*)                        /* Normal code */
        *(.rodata .rodata*)                     /* read-only data (constants) */
  
        /* section information for  shell */
        . = ALIGN(8);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(8);

        PROVIDE(__ctors_start__ = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE(__ctors_end__ = .);

        . = ALIGN(4);
        
        __isrtbl_idx_start = .;
        KEEP(*(.isrtbl.idx))
        __isrtbl_start = .;
        KEEP(*(.isrtbl))
        __isrtbl_end = .;
        . = ALIGN(8);

        PROVIDE(g_service_table_start = ABSOLUTE(.));
        KEEP(*(.g_service_table))
        PROVIDE(g_service_table_end = ABSOLUTE(.));

        PROVIDE( _etext = ABSOLUTE(.) );
    } > sram

    . = ALIGN(8);

    .data : 
    {
        PROVIDE( _sdata = ABSOLUTE(.) );
        
        /* Writable data segment */
        *(.data .data.*)       

        . = ALIGN(8);
        PROVIDE( __global_pointer$ = ABSOLUTE(.) + 0x800 );

        /* Writable small data segment (.sdata segment) */
        *(.sdata .sdata.*)

        PROVIDE( _edata = ABSOLUTE(.) );
    } > sram

    .bss : 
    {
        __bss_start = ABSOLUTE(.);

        /* Writable uninitialized small data segment (.sbss segment)*/
        *(.sbss .sbss.*)
        *(.scommon)

        /* Uninitialized writeable data section (.bss segment)*/
        *(.bss .bss.*)
        *(COMMON)

        . = ALIGN(8);
        __bss_end = ABSOLUTE(.);
    } > sram

    .stack :
    {
        . = ALIGN(64);
        PROVIDE(__stack_start__ = ABSOLUTE(.));

        /* cpu0 stack top site */
        . = . + __STACKSIZE__;
        __stack_tp0 = . ;

        /* cpu1 stack top site */
        . = . + __STACKSIZE__;
        __stack_tp1 = . ;

        PROVIDE( __stack_end__ = ABSOLUTE(.) );
    } > sram
    _end = ABSOLUTE(.);

}
