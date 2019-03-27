FAUST = faust
FAUSTFLAGS =

DSP = chorus mutator octaver wahwah

all: $(patsubst %,build/%.dsp.cpp,$(DSP))

clean:
	rm -rf build

build/%.dsp.cpp: %.dsp architecture.cpp
	@install -d build
	$(FAUST) $(FAUSTFLAGS) -a architecture.cpp -o $@ $<
