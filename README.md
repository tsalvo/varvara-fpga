# uxn-fpga
Very early work in progress implementation of [Varvara / UXN](https://100r.co/site/uxn.html) by [hundredrabbits](https://100r.co/site/home.html) in FPGA using [PipelineC](https://github.com/JulianKemmerer/PipelineC). Currently hard-coded to run one of the test ROMs listed below. Intended for importing into the [openfpga-uxn](https://github.com/tsalvo/openfpga-uxn) core for Analogue Pocket.

### Change test ROM:
Currently two ROMs are available as C arrays within `.h` files in the `roms/` directory:
- `fill_test.h` (draws a series of rectangles using the fill command)
- `mandelbrot_test.h` (draws a mandelbrot set image, one pixel at a time)

To use a different ROM, just change the import statement in `uxn.c` to import the correct ROM.

### build into VHDL files (for later importing into openfpga-uxn project):
```
pipelinec uxn.c
```

### build for simulation (requires additional plugins)
```
pipelinec uxn.c --sim --comb --ghdl
```

### run simulation:
```
ghdl -i --std=08 --work=work [sequence of vhd files appended by top_test.vhd]
ghdl -m --std=08 --work=work top_test
ghdl -r --std=08 --work=work top_test --ieee-asserts=disable --stop-time=1ms
```
