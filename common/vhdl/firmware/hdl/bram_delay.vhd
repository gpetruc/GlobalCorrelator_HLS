library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library unisim;
use unisim.vcomponents.all;

entity bram_delay is
    generic(
        DELAY : natural;
        N_BITS : natural := 64
    );
    port(
        clk: in std_logic;
        rst: in std_logic;
        d: in  std_logic_vector(N_BITS-1 downto 0);
        q: out std_logic_vector(N_BITS-1 downto 0)
    );
end bram_delay;

architecture rtl of bram_delay is
    constant MY_DELAY : natural := DELAY-3; -- 3 clock cycles of delay are already there because of registers & logic
    constant half_index : natural := MY_DELAY-1;  
    constant max_index : natural := 2*MY_DELAY-1; 
    signal d72: std_logic_vector(71 downto 0) := (others => '0');
    signal q72: std_logic_vector(71 downto 0) := (others => '0');
    signal raddr: std_logic_vector(14 downto 0);
    signal waddr: std_logic_vector(14 downto 0);
    signal rindex: natural range 0 to 2*MY_DELAY-1 := half_index;
    signal windex: natural range 0 to 2*MY_DELAY-1 := 0;
begin
    process(clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                rindex <= half_index;
                windex <= 0;
            else 
                if rindex = max_index then
                    rindex <= 0;
                else
                    rindex <= rindex + 1;
                end if;
                if windex = max_index then
                    windex <= 0;
                else
                    windex <= windex + 1;
               end if;
            end if;
        end if;
    end process;
    raddr(14 downto 6) <= std_logic_vector(to_unsigned(rindex,9));
    waddr(14 downto 6) <= std_logic_vector(to_unsigned(windex,9));
    raddr(5 downto 0) <= (others => '0'); 
    waddr(5 downto 0) <= (others => '0'); 

    ram: RAMB36E2
        generic map(
            CLOCK_DOMAINS => "COMMON",
            DOA_REG => 1,
            DOB_REG => 1,
            READ_WIDTH_A => 72,
            WRITE_WIDTH_B => 72,
            WRITE_MODE_A => "READ_FIRST", --"WRITE_FIRST",
            WRITE_MODE_B => "READ_FIRST" --"WRITE_FIRST"
        )
        port map(
            ADDRENA => '1',
            ADDRENB => '1',
            ADDRARDADDR => raddr,
            ADDRBWRADDR => waddr,
            CLKARDCLK => clk,
            CLKBWRCLK => clk,
            DINADIN => d72(31 downto 0),
            DINBDIN => d72(63 downto 32),
            DINPADINP => d72(67 downto 64),
            DINPBDINP => d72(71 downto 68),
            DOUTADOUT => q72(31 downto 0),
            DOUTBDOUT => q72(63 downto 32),
            DOUTPADOUTP => q72(67 downto 64),
            DOUTPBDOUTP => q72(71 downto 68),
            ENARDEN => '1', 
            ENBWREN => '1',
            REGCEAREGCE => '1',
            REGCEB => '0',
            RSTRAMARSTRAM => '0',
            RSTRAMB => '0',
            RSTREGARSTREG => '0',
            RSTREGB => '0',
            WEA => "0000",
            WEBWE => "11111111",
            CASDIMUXA => '0',
            CASDIMUXB => '0',
            CASDOMUXA => '0',
            CASDOMUXB => '0',
            CASDINA => (others => '0'),
            CASDINB => (others => '0'),
            CASDINPA => (others => '0'),
            CASDINPB => (others => '0'),
            CASDOMUXEN_A => '1',
            CASDOMUXEN_B => '1',
            CASINSBITERR => '0',
            CASINDBITERR => '0',
            CASOREGIMUXEN_A => '1',
            CASOREGIMUXEN_B => '1',
            CASOREGIMUXA => '0',
            CASOREGIMUXB => '0',
            INJECTSBITERR => '0',
            INJECTDBITERR => '0',
            ECCPIPECE => '1',
            SLEEP => '0'
        );
        q <= q72(N_BITS-1 downto 0);
        d72(N_BITS-1 downto 0) <= d;
end rtl;


