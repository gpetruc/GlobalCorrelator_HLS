library std;
use std.textio.all;
use std.env.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;

entity testbench is
--  Port ( );
end testbench;

architecture Behavioral of testbench is
    constant NITEMS : natural := 6;
    signal clk : std_logic := '0';
    signal rst : std_logic := '0';
    signal event_in      : std_logic;
    signal threshold_in  : signed(15 downto 0);                              
    signal value_in  : signed(15 downto 0);                              
    signal sum_out  : signed(15 downto 0);                              
    signal out_valid : std_logic;                              
    file Fi : text open read_mode is "input.txt";
    file Fo : text open write_mode is "output.txt";
begin
    clk  <= not clk after 2.0833333 ns;
    
    uut : entity work.algo
        port map(clk => clk, rst => rst, event_in => event_in, threshold_in => threshold_in, value_in => value_in, sum_out => sum_out, out_valid => out_valid);
   
    runit : process 
        variable remainingEvents : integer := 1;
        variable frame : integer := 0;
        variable Li, Lo : line;
        -- variable sep : string(1 to 2);
        variable itest, iobj, thresh, val : integer;
    begin
        rst <= '1';
        wait for 50 ns;
        rst <= '0';
        event_in <= '0';
        threshold_in <= to_signed(0, 16);
        value_in <= to_signed(0, 16);
        wait until rising_edge(clk);
        while remainingEvents > 0 loop
            if not endfile(Fi) then
                readline(Fi, Li);
                read(Li, itest);
                read(Li, iobj); 
                read(Li, thresh);
                read(Li, val); 
                --report "read itest = " & integer'image(itest) & "   iobj = " & integer'image(iobj) & "  thresh = " & integer'image(thresh) & "   val = " & integer'image(val);
             else
                remainingEvents := remainingEvents - 1;
                iobj    := NITEMS+1;
                thresh := 0;
                val    := 0;
            end if;
            -- prepare stuff
            if iobj = 0 then
                event_in <= '1';
            else
                event_in <= '0';
            end if;
            threshold_in  <= to_signed(thresh, 16);
            value_in      <= to_signed(val,    16);
            -- ready to dispatch ---
            wait until rising_edge(clk);
            -- write out the output --
            frame := frame + 1;
            write(Lo, frame, field=>5);  
            write(Lo, string'(" ")); 
            write(Lo, out_valid); 
            write(Lo, string'(" ")); 
            if out_valid = '1' then
                write(Lo, to_integer(sum_out), field => 5); 
            else
                write(Lo, 0, field => 5); 
            end if;
            writeline(Fo, Lo);
        end loop;
        wait for 50 ns;
        finish(0);
    end process;

    
end Behavioral;
