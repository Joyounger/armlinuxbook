CC = /usr/local/arm/bin/arm-linux-gcc

CFLAGS = -I/usr/src/linux-2.6.12-omap1-dspgw3.3/include -Wall
DSP_BIN = \
	rsafile \
	rsaDecfile \
	rc4_file \
	arm_rc4 \
	arm_rsa_enc \
	arm_rsa_dec 


all: $(DSP_BIN)

clean:
	-rm $(DSP_BIN)

rsafile: rsafile.c
rsaDecfile: rsaDecfile.c
rc4_file: rc4_file.c
arm_rc4: arm_rc4.c
arm_rsa_enc: arm_rsa_enc.c
arm_rsa_dec: arm_rsa_dec.c
