all: proiect.hex

proiect.hex: proiect.elf
	avr-objcopy  -j .text -j .data -O ihex $^ $@
	avr-size proiect.elf

proiect.elf: proiect.c utils.c
	avr-g++ -mmcu=atmega324a -Os -Wall -o $@ $^

clean:
	rm -rf proiect.elf proiect.hex
