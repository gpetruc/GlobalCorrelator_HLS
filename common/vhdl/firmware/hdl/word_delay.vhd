library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity word_delay is
    generic(
        DELAY  : natural;
        N_BITS : natural := 64
    );
    port(
        clk    : in  std_logic;
        enable : in  std_logic; -- set to 1 to have it running
        d      : in  std_logic_vector(N_BITS-1 downto 0);
        q      : out std_logic_vector(N_BITS-1 downto 0)
    );
end word_delay;

architecture Behavioral of word_delay is
    type  word_delay_data is array(DELAY-1 downto 0) of std_logic_vector(N_BITS-1 downto 0);
    signal data : word_delay_data;

    attribute keep : string;
    attribute keep of data : signal is "true";

begin
    tick: process(clk)
    begin
        if rising_edge(clk) then
            if enable = '1' then
                data(0) <= d;
                if DELAY > 1 then
                    data(DELAY-1 downto 1) <= data(DELAY-2 downto 0);
                end if;
            end if;
        end if;
    end process;

    q <= data(DELAY-1);
end Behavioral;
        


