ENTRY(_start)

SECTIONS
{

  . = 0x4000;

	low_kern_start = .;

	.boot : {
		*(.mbhdr)
		_loadStart = .;

		*(.boot)
		. = ALIGN(4096);

	}

	. = ALIGN(0x1000);
	idt_block = .;
	. += 0x1000;


	boot_stack_start = .;
	. += 0x4000;  /* we start out with a 4 pages of stack */
	boot_stack_end = .;

	.text : {
		*(.text*)
		*(.gnu.linkonce.t*)
	}

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	/* Adjust the address for the data segment to the next page */
	. = ALIGN(0x1000);
	/* The data segment */
	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.eh_frame .note.GNU-stack)
	}
}
