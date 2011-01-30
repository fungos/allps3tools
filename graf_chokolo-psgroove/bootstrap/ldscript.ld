
ENTRY(_start)

SECTIONS
{
	.text 0x0 :
	{
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.t.*)
	}

	.data :
	{
		*(.data)
		*(.data.*)
	}

	.got :
	{
		*(.got)
		*(.got.*)
		__toc_start = .;
		*(.toc)
		*(.toc.*)
		__toc_end = .;
	}

	.bss :
	{
		*(.bss)
		*(.bss.*)
	}

	.opd :
	{
		*(.opd)
	}
} 
