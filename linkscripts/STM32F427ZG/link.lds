/*
 * linker script for STM32F427VI with GNU ld
 */

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
ROM (rx) : ORIGIN =0x08010000,LENGTH =1920k
RAM (rw) : ORIGIN =0x20030000,LENGTH =512k
TCMRAM (rw) : ORIGIN =0x10000000,LENGTH =64k
SRAM0  (rw) : ORIGIN =0x20000000,LENGTH =112k
SRAM1  (rw) : ORIGIN =0x2001C000,LENGTH =16k
SRAM2  (rw) : ORIGIN =0x20020000,LENGTH =64k
}
ENTRY(Reset_Handler)
_system_stack_size = 0x400;

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        KEEP(*(.isr_vector))            /* Startup code */
        KEEP(*thaisenBMS.o(.text .text.*))
	    KEEP(*thaisenBMSTask.o(.text .text.*))
	    KEEP(*thaisenChargFault.o(.text .text.*))
		KEEP(*thaisenChargFlow.o(.text .text.*))
		KEEP(*thaisenChargStopWay.o(.text .text.*))
		KEEP(*thaisenFan.o(.text .text.*))
	    KEEP(*thaisenGunDef.o(.text .text.*))
	    KEEP(*thaisenInsult.o(.text .text.*))
		
		KEEP(*thaisenAuxPower.o(.text .text.*))
		KEEP(*thaisenBoard.o(.text .text.*))
	    KEEP(*thaisenCCVolt.o(.text .text.*))
	    KEEP(*thaisenDoor.o(.text .text.*))
	    KEEP(*thaisenElectLock.o(.text .text.*))
		KEEP(*thaisenFan.o(.text .text.*))
		KEEP(*thaisenInsuleD.o(.text .text.*))
		KEEP(*thaisenLed.o(.text .text.*))
		KEEP(*thaisenPT1000.o(.text .text.*))
		KEEP(*thaisenRelay.o(.text .text.*))
		KEEP(*thaisenScram.o(.text .text.*))
		KEEP(*thaisenSysFault.o(.text .text.*))
		KEEP(*thaisenSysFaultCheck.o(.text .text.*))		
	    KEEP(*thaisenW25QXX.o(.text .text.*))
	    KEEP(*thaisenSPI.o(.text .text.*))
		KEEP(*thaisenADC.o(.text .text.*))
		KEEP(*thaisenCAN.o(.text .text.*))
		KEEP(*thaisenDMA.o(.text .text.*))
		KEEP(*thaisenGPIO.o(.text .text.*))
		KEEP(*thaisenIWDG.o(.text .text.*))
		KEEP(*thaisenRTC.o(.text .text.*))
		KEEP(*thaisenTim.o(.text .text.*))
		KEEP(*user_aux_power.o(.text .text.*))
		KEEP(*user_calibrat_parameter.o(.text .text.*))
		KEEP(*user_can.o(.text .text.*))
		KEEP(*user_can_spi.o(.text .text.*))
		KEEP(*user_feedback_signal.o(.text .text.*))
		KEEP(*user_get_para_task.o(.text .text.*))
		KEEP(*user_led.o(.text .text.*))
		KEEP(*user_mcp2512.o(.text .text.*))
		KEEP(*user_pwm_fan.o(.text .text.*))
		KEEP(*user_relay.o(.text .text.*))
		KEEP(*thaisenAmmeter.o(.text .text.*))
		
		KEEP(*thaisenChargModule.o(.text .text.*))
		KEEP(*thaisenGuoWang.o(.text .text.*))
		KEEP(*thaisenWinline.o(.text .text.*))
		KEEP(*thaisenYiNeng.o(.text .text.*))
		KEEP(*thaisenYingFeiYuan.o(.text .text.*))
		KEEP(*thaisenYouYou.o(.text .text.*))
		KEEP(*control_strategy.o(.text .text.*))
		KEEP(*module_control.o(.text .text.*))
		KEEP(*relay_control.o(.text .text.*))
   
        . = ALIGN(4);
        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */
        
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        /* section information for finsh shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /* section information for utest */
        . = ALIGN(4);
        __rt_utest_tc_tab_start = .;
        KEEP(*(UtestTcTab))
        __rt_utest_tc_tab_end = .;

        /* section information for at server */
        . = ALIGN(4);
        __rtatcmdtab_start = .;
        KEEP(*(RtAtCmdTab))
        __rtatcmdtab_end = .;
        . = ALIGN(4);

        /* section information for initial. */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;

        . = ALIGN(4);

        PROVIDE(__ctors_start__ = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE(__ctors_end__ = .);

        . = ALIGN(0x40000);
        *(.rodata*)
        *(.rodata)                      /* read-only data (constants) */        

        _etext = .;
    } > ROM = 0

    .TCMRAM :
    {
        . = ALIGN(0x4);
        __tcm_ram_start = .;
        *(.TCM_RAM)
        . = ALIGN(0x4);
        __tcm_ram_end = .;
    } > TCMRAM
    
    .SRAM0 :
    {
        . = ALIGN(0x4);
        __sram0_ram_start = .;
        *(.SRAM0_RAM)
        . = ALIGN(0x4);
        __sram0_ram_end = .;
    } > SRAM0 

    .SRAM1 :
    {
        . = ALIGN(0x4);
        __sram0_ram_start = .;
        *(.SRAM1_RAM)
        . = ALIGN(0x4);
        __sram1_ram_end = .;
    } > SRAM1
    
    .SRAM2 :
    {
        . = ALIGN(0x4);
        __sram2_ram_start = .;
        *(.SRAM2_RAM)
        . = ALIGN(0x4);
        __sram2_ram_end = .;
    } > SRAM2  

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)

        /* This is used by the startup in order to initialize the .data secion */
        _sidata = .;
    } > ROM
    __exidx_end = .;

    /* .data section which is used for initialized data */

    .data : AT (_sidata)
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)


        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
    } >RAM

    .stack : 
    {
        . = ALIGN(4);
        _sstack = .;
        . = . + _system_stack_size;
        . = ALIGN(4);
        _estack = .;
    } >RAM

    __bss_start = .;
    .bss :
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _ebss = . ;
        
        *(.bss.init)
    } > RAM
    __bss_end = .;

    _end = .;

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
