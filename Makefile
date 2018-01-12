NAME = wedding
BOARD = nes-nrom-128
EMULATOR = fceux

build:
	cc65 -Oi -Iinclude/ -Idata/ -Ilib/ src/*.c --add-source
	ls src/*.s | xargs -n 1 ca65
	ca65 -Ilib/ -Idata/ -Idata/music asm/reset.s
	ld65 -C $(BOARD).cfg -o $(NAME).nes asm/*.o src/*.o nes.lib

clean:
	rm -f src/*.s src/*.o asm/*.o lib/*.o $(NAME).nes

play: $(NAME).nes
	$(EMULATOR) $(NAME).nes

