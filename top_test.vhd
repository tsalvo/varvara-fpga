library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
  
entity top_test is
end entity;
  
architecture sim of top_test is
	  signal test_clk    : std_logic := '0';
	  signal test_input : unsigned(15 downto 0) := x"0000";
	constant clk_period : time := 56 ns;
begin
	-- The Device Under Test (DUT)
	i_top : entity work.top
	port map(
		clk_18p0 => test_clk,
		uxn_eval_input => test_input
	);
	test_clk <= not test_clk after clk_period / 2;
end architecture;