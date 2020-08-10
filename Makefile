obj-m += echo.o

all:
	make -C /lib/modules/5.7.12-arch1-1/build M=$(PWD) V=1 modules

clean:
	make -C /lib/modules/5.7.12-arch1-1/build M=$(PWD) V=1 clean
