ENTRY(_start)

SECTIONS
{
  . = 0x1000;
  .text : { *(.text) }
  .data : { *(.data) }

	/* allocate 4k of stack */
	. += 0x1000;
	stack_start = .;

  .bss : { *(.bss) }

}
