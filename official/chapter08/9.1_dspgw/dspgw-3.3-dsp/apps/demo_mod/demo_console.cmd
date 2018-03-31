MEMORY {
	EXRAM_DEMO: origin = 0x30000, len = 0x1000
}

SECTIONS {
	.text: {} > EXRAM_DEMO
}
