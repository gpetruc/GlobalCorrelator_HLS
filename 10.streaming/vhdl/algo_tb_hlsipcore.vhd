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
    signal start, ready, idle, reading : std_logic;
    signal threshold_in  : std_logic_vector(15 downto 0);                              
    signal value_in  : std_logic_vector(15 downto 0);                              
    signal sum_out  : std_logic_vector(15 downto 0);                              
    signal out_valid : std_logic;                              
    file Fi : text open read_mode is "input.txt";
    file Fo : text open write_mode is "output_hlsipcore.txt";

    component algo_main is
        port (
                 ap_clk : in std_logic;
                 ap_rst : in std_logic;
                 ap_start : in std_logic;
                 ap_done : out std_logic;
                 ap_idle : out std_logic;
                 ap_ready : out std_logic;
                 threshold_v : in std_logic_vector (15 downto 0);
                 input_v_v_dout : in std_logic_vector (15 downto 0);
                 input_v_v_empty_n : in std_logic;
                 input_v_v_read : out std_logic;
                 ap_return : out std_logic_VECTOR (15 downto 0) );
    end component;
begin
    clk  <= not clk after 1.25 ns;
    
    uut : algo_main
        port map(ap_clk => clk, 
                 ap_rst => rst, 
                 ap_start => start,
                 ap_ready => ready,
                 ap_idle =>  idle,
                 threshold_V => threshold_in, 
                 input_V_V_dout => value_in,
                 input_V_V_empty_n => '1',
                 input_V_V_read => reading,
                 ap_return => sum_out, 
                 ap_done => out_valid);
   
    runit : process 
        variable remainingEvents : integer := 1;
        variable frame : integer := 0;
        variable Li, Lo : line;
        variable itest, iobj, thresh, val : integer;
    begin
        rst <= '1';
        wait for 50 ns;
        rst <= '0';
        threshold_in <= std_logic_vector(to_signed(0, 16));
        value_in <= std_logic_vector(to_signed(0, 16));
        start <= '0';
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
            threshold_in  <= std_logic_vector(to_signed(thresh, 16));
            value_in      <= std_logic_vector(to_signed(val,    16));
            start <= '1';
            -- ready to dispatch ---
            wait until rising_edge(clk);
            -- write out the output --
            frame := frame + 1;
            write(Lo, frame, field=>5);  
            write(Lo, string'(" ")); 
            write(Lo, out_valid); 
            write(Lo, string'(" ")); 
            if out_valid = '1' then
                write(Lo, to_integer(signed(sum_out)), field => 5); 
            else
                write(Lo, 0, field => 5); 
            end if;
            write(Lo, string'(" |  ready ")); 
            write(Lo, ready); 
            write(Lo, string'("   idle ")); 
            write(Lo, idle); 
            write(Lo, string'("  reading ")); 
            write(Lo, reading); 
            writeline(Fo, Lo);
        end loop;
        wait for 50 ns;
        finish(0);
    end process;

    
end Behavioral;
