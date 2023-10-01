# uxn-fpga
Very early work in progress implementation of Varvara / UXN in FPGA using PipelineC. Currently hard-coded to run a mandelbrot set drawing ROM. Intended for importing into openfpga-uxn core for Analogue Pocket.

simulation:
ghdl -i --std=08 --work=work [sequence of vhd files] # import
ghdl -m --std=08 --work=work top_test
ghdl -r --std=08 --work=work top_test --ieee-asserts=disable --stop-time=1ms
