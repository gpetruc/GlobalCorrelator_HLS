library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.regionizer_data.all;

entity tk_regionizer_mux_stream is
    port(
            ap_clk : IN STD_LOGIC;
            ap_rst : IN STD_LOGIC;
            ap_start : IN STD_LOGIC;
            ap_done : OUT STD_LOGIC;
            ap_idle : OUT STD_LOGIC;
            ap_ready : OUT STD_LOGIC;
            newevent : IN STD_LOGIC;
            tracks_in_0_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_0_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_1_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_1_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_2_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_2_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_3_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_3_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_4_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_4_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_5_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_5_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_6_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_6_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_7_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_7_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_8_0_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_in_8_1_V : IN STD_LOGIC_VECTOR (71 downto 0);
            tracks_out       : OUT w72s(NTKSTREAM-1 downto 0);
            newevent_out     : OUT STD_LOGIC
    );
end tk_regionizer_mux_stream;

architecture Behavioral of tk_regionizer_mux_stream is

    signal regionized:        w72s(NPFREGIONS-1 downto 0);
    signal regionized_valid:  std_logic_vector(NPFREGIONS-1 downto 0) := (others => '0');
    signal regionized_roll:   std_logic := '0';

begin

    tk_regionizer : entity work.tk_regionizer 
                port map(ap_clk => ap_clk, ap_rst => ap_rst,
                             ap_start => ap_start,
                             newevent => newevent,
                             tracks_in_0_0_V => tracks_in_0_0_V,
                             tracks_in_0_1_V => tracks_in_0_1_V,
                             tracks_in_1_0_V => tracks_in_1_0_V,
                             tracks_in_1_1_V => tracks_in_1_1_V,
                             tracks_in_2_0_V => tracks_in_2_0_V,
                             tracks_in_2_1_V => tracks_in_2_1_V,
                             tracks_in_3_0_V => tracks_in_3_0_V,
                             tracks_in_3_1_V => tracks_in_3_1_V,
                             tracks_in_4_0_V => tracks_in_4_0_V,
                             tracks_in_4_1_V => tracks_in_4_1_V,
                             tracks_in_5_0_V => tracks_in_5_0_V,
                             tracks_in_5_1_V => tracks_in_5_1_V,
                             tracks_in_6_0_V => tracks_in_6_0_V,
                             tracks_in_6_1_V => tracks_in_6_1_V,
                             tracks_in_7_0_V => tracks_in_7_0_V,
                             tracks_in_7_1_V => tracks_in_7_1_V,
                             tracks_in_8_0_V => tracks_in_8_0_V,
                             tracks_in_8_1_V => tracks_in_8_1_V,
                             tracks_out_0_V => regionized(0),
                             tracks_out_1_V => regionized(1),
                             tracks_out_2_V => regionized(2),
                             tracks_out_3_V => regionized(3),
                             tracks_out_4_V => regionized(4),
                             tracks_out_5_V => regionized(5),
                             tracks_out_6_V => regionized(6),
                             tracks_out_7_V => regionized(7),
                             tracks_out_8_V => regionized(8),
                             tracks_out_valid_0 => regionized_valid(0),
                             tracks_out_valid_1 => regionized_valid(1),
                             tracks_out_valid_2 => regionized_valid(2),
                             tracks_out_valid_3 => regionized_valid(3),
                             tracks_out_valid_4 => regionized_valid(4),
                             tracks_out_valid_5 => regionized_valid(5),
                             tracks_out_valid_6 => regionized_valid(6),
                             tracks_out_valid_7 => regionized_valid(7),
                             tracks_out_valid_8 => regionized_valid(8),
                             newevent_out => regionized_roll);

    tk_delay_sort_mux_stream : entity work.delay_sort_mux_stream
                generic map(NREGIONS => NPFREGIONS, 
                            NSORTED  => NTKSORTED,
                            NSTREAM  => NTKSTREAM,
                            OUTII    => PFII240,
                            DELAY    => TKDELAY)
                port map(ap_clk => ap_clk,
                         d_in => regionized,
                         valid_in => regionized_valid,
                         roll => regionized_roll,
                         d_out => tracks_out,
                         roll_out => newevent_out);

end Behavioral;
