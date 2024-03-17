library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
  
entity top_test is
end entity;
  
architecture sim of top_test is
	  signal test_clk    : std_logic := '0';
	  signal test_controller_button : unsigned(0 downto 0) := to_unsigned(0, 1);
	  signal test_is_visible_pixel : unsigned(0 downto 0) := to_unsigned(1, 1);
	  signal test_is_double_buffer_enabled: unsigned(0 downto 0) := to_unsigned(1, 1);
	  signal test_rom_load_valid_byte : unsigned(0 downto 0) := to_unsigned(0, 1);
	  signal test_rom_load_address : unsigned(15 downto 0) := x"0000";
	  signal test_rom_load_value : unsigned(7 downto 0) := x"00";
	constant clk_period : time := 65 ns;
begin
	-- The Device Under Test (DUT)
	i_top : entity work.top
	port map(
		clk_None => test_clk,
		uxn_top_controller0_up => test_controller_button,
		uxn_top_controller0_down => test_controller_button,
		uxn_top_controller0_left => test_controller_button,
		uxn_top_controller0_right => test_controller_button,
		uxn_top_controller0_a => test_controller_button,
		uxn_top_controller0_b => test_controller_button,
		uxn_top_controller0_x => test_controller_button,
		uxn_top_controller0_y => test_controller_button,
		uxn_top_controller0_l => test_controller_button,
		uxn_top_controller0_r => test_controller_button,
		uxn_top_controller0_select => test_controller_button,
		uxn_top_controller0_start => test_controller_button,
		uxn_top_is_visible_pixel => test_is_visible_pixel,
		uxn_top_is_double_buffer_enabled => test_is_double_buffer_enabled,
		uxn_top_rom_load_valid_byte => test_rom_load_valid_byte,
		uxn_top_rom_load_address => test_rom_load_address,
		uxn_top_rom_load_value => test_rom_load_value
	);
	test_clk <= not test_clk after clk_period / 2;
end architecture;