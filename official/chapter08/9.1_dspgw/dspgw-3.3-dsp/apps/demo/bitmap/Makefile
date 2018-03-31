all: \
	red.bitmap16 \
	green.bitmap16 \
	blue.bitmap16 \
	yellow.bitmap16 \
	cyan.bitmap16 \
	magenta.bitmap16 \
	white.bitmap16 \
	black.bitmap16

clean:
	-rm *.bitmap16

.SUFFIXES: .bitmap .bitmap16

.bitmap.bitmap16:
	mkbitmap16 $*.bitmap > $@
