-l tokliBIOScfg.cmd
-l tokliBIOS.cmd

SECTIONS {
        ipbuf: align = 0x8000 {} > SARAM
	dbgbuf: align = 0x4 {} > SARAM
}
