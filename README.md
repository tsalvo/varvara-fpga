# uxn-fpga
Very early work in progress implementation of [Varvara / UXN](https://100r.co/site/uxn.html) by hundredrabbits in FPGA using [PipelineC](https://github.com/JulianKemmerer/PipelineC). Currently hard-coded to run a mandelbrot set drawing ROM. Intended for importing into the [openfpga-uxn](https://github.com/tsalvo/openfpga-uxn) core for Analogue Pocket.

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
ghdl -i --std=08 --work=work [sequence of vhd files] # import
ghdl -m --std=08 --work=work top_test
ghdl -r --std=08 --work=work top_test --ieee-asserts=disable --stop-time=1ms
```
