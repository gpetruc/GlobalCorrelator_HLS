library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity bit_delay is
    generic(
        DELAY : natural;
        SHREG : string := "no"
    );
    port(
        clk    : in  std_logic;
        enable : in  std_logic; -- set to 1 to have it running
        d      : in  std_logic; -- 
        q      : out std_logic  -- 
    );
end bit_delay;

architecture Behavioral of bit_delay is
    signal bits: std_logic_vector(DELAY-1 downto 0) := (others => '0');

     attribute keep : string;
     attribute keep of bits : signal is "true";

     attribute shreg_extract : string;
     attribute shreg_extract of bits : signal is SHREG;
    
begin
    timer: process(clk)
    begin
        if rising_edge(clk) then
            if enable = '1' then
                bits(0) <= d;
                bits(DELAY-1 downto 1) <= bits(DELAY-2 downto 0);
            end if;
        end if;
    end process;

    q <= bits(DELAY-1);
end Behavioral;
