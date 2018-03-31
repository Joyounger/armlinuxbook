MEMORY {
	page 0: SDRAM_MISC: origin = 0xfffd00, len = 0x100	/* misc */
	page 0: IDLEPG:     origin = 0xfffe00, len = 0x100	/* reserved for Linux */
}

SECTIONS {
	dspgw_task: {
		_task_user = .;
		*(dspgw_task)
		_task_user_end = .;
	}
	dspgw_version (COPY): {}
	SARAM_seq: {} > SARAM
	working_coeff : {} > SARAM
	working_space : {} > DARAM
	DARAM_seq: {} > DARAM
	SDRAM_seq: {} > SDRAM_MISC
	SDRAM_system_data: {} > SDRAM_MISC
	sleep_code: {} > DARAM
}
