library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
library unisim;
use unisim.vcomponents.all;

use work.regionizer_data.all;

entity pfregion_loop is
    generic(
        NREGIONS : natural := NPFREGIONS;
        ETA_CENTER : signed(11 downto 0);
        PHI_START : signed(10 downto 0) := to_signed(0, 11);
        PHI_STEP : signed(10 downto 0) := to_signed(PHI_SHIFT_INT, 11);
        PHI_HALFWIDTH : signed(9 downto 0) := PHI_HALFWIDTH_FID;
        ETA_HALFWIDTH : signed(9 downto 0) := ETA_HALFWIDTH_FID;
        PHI_EXTRA : signed(9 downto 0)     := PHI_BORDER;
        ETA_EXTRA :  signed(9 downto 0)    := PHI_BORDER; -- same border by default
        OUTII : natural
    );
    port(
        ap_clk  : in std_logic;
        roll    : in  std_logic;
        reg_out : out pfregion;
        vld_out : out std_logic
    );
end pfregion_loop;

architecture Behavioral of pfregion_loop is
    signal region : pfregion := (etaCenter => ETA_CENTER, 
                                 phiCenter => PHI_START,
                                 etaHalfWidth => ETA_HALFWIDTH,
                                 phiHalfWidth => PHI_HALFWIDTH,
                                 phiExtra => PHI_EXTRA,
                                 etaExtra => ETA_EXTRA);
    signal count  : integer range 0 to OUTII-1 := 0;
    signal rcount : integer range 0 to NREGIONS := 0;
    constant PHI_POS_MAX : signed(10 downto 0) := resize(PHI_PI - PHI_SHIFT, 11);
    constant PHI_ROLL    : signed(11 downto 0) := PHI_M2PI + PHI_SHIFT;
begin

     reg_out <= region;

     logic: process(ap_clk) 
           variable phi_new_pos, phi_new_neg : signed(11 downto 0);
        begin
            if rising_edge(ap_clk) then
                if roll = '1' then
                    region.phiCenter <= PHI_START;
                    count <= 0;
                    rcount <= 0;
                    vld_out <= '1';
                else
                    if count < OUTII-1 then
                        count <= count + 1;
                    else
                        if region.phiCenter > PHI_POS_MAX then
                            region.phiCenter <= resize(region.phiCenter + PHI_ROLL, 11);
                        else
                            region.phiCenter <= region.phiCenter + PHI_SHIFT;
                        end if;
                        count <= 0;
                        if rcount < NREGIONS then
                            rcount <= rcount + 1;
                        end if;
                        if rcount = NREGIONS - 1 then
                            vld_out <= '0';
                        end if;
                    end if;
                end if;
            end if;
        end process logic;


end Behavioral;
