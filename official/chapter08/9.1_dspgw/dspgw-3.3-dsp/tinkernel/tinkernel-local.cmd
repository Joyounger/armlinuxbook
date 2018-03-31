-l tokliBIOScfg.cmd
-l tokliBIOS.cmd

MEMORY {
	EXRAM_KERNEL: origin = 0x28000, len = 0x2000
}
SECTIONS {
	ipbuf: align = 0x4 {} > EXRAM_KERNEL
	dbgbuf: align = 0x4 {} > EXRAM_KERNEL
}
