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

OUTPUT_ARCH( "riscv" )
__STACKSIZE__ = 1024;
ENTRY( _begin )

MEMORY
{
  flash (rxai!w) : ORIGIN = 0x20400000, LENGTH = 4096K
  sram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 16K
}


SECTIONS
{
    .start :
    {
        *(.init);
    } > flash
  .text           :
  {
    *(.text .text.*)
    *(.rodata .rodata*)  
     /* section information for  shell */
        . = ALIGN(8);
        _shell_command_start = .;
        KEEP (*(shellCommand))
        _shell_command_end = .;
        . = ALIGN(8);

        /* the work i did */
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

  } > flash

  PROVIDE (_etext = .);
  . = ALIGN(4);

  .lalign         :
  {
    . = ALIGN(4);
    PROVIDE( _data_lma = . );
  } >flash AT>flash

  .data          :
  {
    . = ALIGN(4);
    PROVIDE( _data = . );
    *(.data .data.*)
    . = ALIGN(4);
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata .sdata.*)
  } >sram AT>flash

  . = ALIGN(4);
  PROVIDE( _edata = . );

  PROVIDE( __bss_start = . );
  .bss            :
  {
    *(.sbss*)
    *(.bss .bss.*)
    *(.scommon)
    . = ALIGN(4);
  } > sram 

  . = ALIGN(8);
  PROVIDE( _end = . );

  .stack :
    {
        . = ALIGN(4);
        /* cpu0 stack top site */
        . = . + __STACKSIZE__;
        _sp = . ;
        PROVIDE(_sp = ABSOLUTE(.));
        PROVIDE( __stack_end__ = ABSOLUTE(.) );
    } > sram 

}
