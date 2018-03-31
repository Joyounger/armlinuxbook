-l tokliBIOS.cmd
-l tokliBIOScfg.cmd

MEMORY {
	SDRAM:  origin = 0x2f000, len = 0x1000
}

SECTIONS {
	shared_buf: align=0x1000 {} > SDRAM
}

