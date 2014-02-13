MEMORY
{
	rom (rx)     : ORIGIN = 0x08000000, LENGTH = 512K
	ram (rwx)    : ORIGIN = 0x20000000, LENGTH = 128K
	garbage (rw) : ORIGIN = 0x00000000, LENGTH = 1K
	config (rw)  : ORIGIN = 0x080e0000, LENGTH = 128K
}

/* Enforce emmition of the vector table. */
EXTERN (vector_table)

ENTRY(reset_handler)

EXTERN(moduleInitializeFunction0)
EXTERN(moduleInitializeFunction1)
EXTERN(moduleInitializeFunction2)
EXTERN(moduleInitializeFunction3)
EXTERN(moduleInitializeFunction4)
EXTERN(moduleInitializeFunction5)
EXTERN(moduleInitializeFunction6)
EXTERN(moduleInitializeFunction7)
EXTERN(moduleInitializeFunction8)
EXTERN(moduleInitializeFunction9)

EXTERN(moduleRPCHandles)
EXTERN(moduleRPCShortNames)
EXTERN(moduleRPCDescriptiveNames)
EXTERN(moduleRPCParameterInTypes)
EXTERN(moduleRPCParameterOutTypes)

EXTERN(genericProtocolEndpointHandles)
EXTERN(genericProtocolEndpointInternalHandles)

EXTERN(logStrings)

EXTERN(msgPumpDescriptions)
EXTERN(menuEntries)

EXTERN(applicationConfig)

/* Define sections. */
SECTIONS
{
        . = ORIGIN(rom);

        .text : {
                *(.vectors)     /* Vector table */
                *(.text*)       /* Program code */
                . = ALIGN(4);
                *(.rodata*)     /* Read-only data */
                _initFunctions_Begin = .;
                KEEP(*(.moduleInitializeFunction0*))
                KEEP(*(.moduleInitializeFunction1*))
                KEEP(*(.moduleInitializeFunction2*))
                KEEP(*(.moduleInitializeFunction3*))
                KEEP(*(.moduleInitializeFunction4*))
                KEEP(*(.moduleInitializeFunction5*))
                KEEP(*(.moduleInitializeFunction6*))
                KEEP(*(.moduleInitializeFunction7*))
                KEEP(*(.moduleInitializeFunction8*))
                KEEP(*(.moduleInitializeFunction9*))
                _initFunctions_End = .;
                
                
                . = ALIGN(4);
                _applicationConfigDescriptorsBegin = .;
                KEEP(*(.applicationConfigDescriptors*))
                _applicationConfigDescriptorsEnd   = .;
                
                
                . = ALIGN(1);
                _moduleRPCHandlesBegin = .;
                KEEP(*(.moduleRPCHandles*))
                _moduleRPCHandlesEnd = .;
                
                _moduleRPCShortNamesBegin = .;
                KEEP(*(.moduleRPCShortNames*))
                _moduleRPCShortNamesEnd = .;
                
                _moduleRPCDescriptiveNamesBegin = .;
                KEEP(*(.moduleRPCDescriptiveNames*))
                _moduleRPCDescriptiveNamesEnd = .;
                
                _moduleRPCParameterInTypesBegin = .;
                KEEP(*(.moduleRPCParameterInTypes*))
                _moduleRPCParameterInTypesEnd = .;
                
                _moduleRPCParameterOutTypesBegin = .;
                KEEP(*(.moduleRPCParameterOutTypes*))
                _moduleRPCParameterOutTypesEnd = .;
                
	            _logStringsBegin = .;
	            KEEP(*(.logStrings*))
	            _logStringsEnd = .;
	            
				KEEP(*(.genericProtocolEndpointHandles*))
				_genericProtocolEndpointInternalHandlesBegin = .;
				KEEP(*(.genericProtocolEndpointInternalHandles*))
				_genericProtocolEndpointInternalHandlesEnd = .;
				
                _msgPumpBPHandlesBegin = .;
                KEEP(*(.msgPumpDescriptions*))
                _msgPumpBPHandlesEnd = .;
                
                . = ALIGN(4);
				_displayMenuItemsBegin = .;
				KEEP(*(.menuEntries*))
				_displayMenuItemsEnd = .;
                
                _etext = .;
        } >rom

        . = ORIGIN(ram);

        .data : {
                _data = .;
                *(.data*)       /* Read-write initialized data */
                . = ALIGN(4);
                _edata = .;
        } >ram AT > rom
        PROVIDE(_data_loadaddr = LOADADDR(.data));

        .bss : {
    			. = ALIGN(4);
    			_bss = .;
                *(.bss*)        /* Read-write zero initialized data */
                *(COMMON)
                . = ALIGN(4);
                _ebss = .;
        } >ram

        .configuration (NOLOAD) : {
                . = ALIGN(4);
                _flashConfigRAMBegin = .;
                KEEP(*(.applicationConfigRAM*))   /* Read-write application configuration */
                . = ALIGN(4);
                _flashConfigRAMEnd = .;
        } >ram AT > config
		
        . = ALIGN(4);
        end = .;
        
        .garbage : AT(ORIGIN(garbage)) 
        {
        	*(.*NEVER*)
        } >garbage
}

PROVIDE(_dataAtFlash = LOADADDR(.data));
PROVIDE(_flashConfigROMBegin = ORIGIN(config));
PROVIDE(_flashConfigROMEnd = ORIGIN(config) + LENGTH(config));

PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));
PROVIDE(_ramBegin = ORIGIN(ram));
