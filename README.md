# uxn-fpga
implementation of Varvara / UXN in FPGA

simulation:
ghdl -i --std=08 --work=work [sequence of vhd files] # import
ghdl -m --std=08 --work=work top_test
ghdl -r --std=08 --work=work top_test --ieee-asserts=disable --stop-time=1ms