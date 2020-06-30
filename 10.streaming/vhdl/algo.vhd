library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity algo is
    port(
        clk : in std_logic;
        rst : in std_logic;
        event_in      : in std_logic;
        threshold_in  : in signed(15 downto 0);                              
        value_in  : in signed(15 downto 0);                              
        sum_out  : out signed(15 downto 0);                              
        out_valid : out std_logic                              
    );
end algo;

architecture Behavioral of algo is
    constant NITEMS : natural := 6;
    signal masked_input  : signed(15 downto 0);
    signal running_total : signed(15 downto 0);
    signal delay         : std_logic_vector(NITEMS downto 0);
begin
    process(clk,rst)
    begin
        if rst = '1' then
            masked_input <= to_signed(0, 16);
            running_total <= to_signed(0, 16);
            delay <= (others => '0');
        elsif rising_edge(clk) then
            delay(0) <= event_in;
            delay(NITEMS downto 1) <= delay(NITEMS-1 downto 0);

            if delay(0) = '1' then
                running_total <= masked_input;
            else
                running_total <= running_total + masked_input;
            end if;

            if value_in > threshold_in then
                masked_input <= value_in;
            else
                masked_input <= to_signed(0, 16);
            end if;
        end if;
    end process;

    sum_out <= running_total;
    out_valid <= delay(NITEMS);
end Behavioral;
