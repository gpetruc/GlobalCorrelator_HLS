-- null_algo
--
-- Do-nothing top level algo for testing
--
-- Dave Newbold, July 2013

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

library IO;
use IO.DataType;
use IO.ArrayTypes;

use work.ipbus.all;
use work.emp_data_types.all;
use work.emp_project_decl.all;

use work.emp_device_decl.all;
use work.emp_ttc_decl.all;

use work.PkgConstants.all;

entity emp_payload is
	port(
		clk: in std_logic; -- ipbus signals
		rst: in std_logic;
		ipb_in: in ipb_wbus;
		ipb_out: out ipb_rbus;
		clk_payload: in std_logic_vector(2 downto 0);
		rst_payload: in std_logic_vector(2 downto 0);
		clk_p: in std_logic; -- data clock
		rst_loc: in std_logic_vector(N_REGION - 1 downto 0);
		clken_loc: in std_logic_vector(N_REGION - 1 downto 0);
		ctrs: in ttc_stuff_array;
		bc0: out std_logic;
		d: in ldata(4 * N_REGION - 1 downto 0); -- data in
		q: out ldata(4 * N_REGION - 1 downto 0); -- data out
		gpio: out std_logic_vector(29 downto 0); -- IO to mezzanine connector
		gpio_en: out std_logic_vector(29 downto 0) -- IO to mezzanine connector (three-state enables)
	);
		
end emp_payload;

architecture rtl of emp_payload is
	
    signal dIO   : IO.ArrayTypes.Matrix(0 to 5)(0 to 5)      := IO.ArrayTypes.NullMatrix(6, 6);
    signal qIO   : IO.ArrayTypes.Vector(0 to 127)             := IO.ArrayTypes.NullVector(128);
    signal qIOP  : IO.ArrayTypes.VectorPipe(0 to 7)(0 to 127) := IO.ArrayTypes.NullVectorPipe(8, 128);
    signal DebugLayer1  : IO.ArrayTypes.Vector(0 to 15) := IO.ArrayTypes.NullVector(16);
    signal DebugLayer1P : IO.ArrayTypes.VectorPipe(0 to 3)(0 to 15) := IO.ArrayTypes.NullVectorPipe(4,16);
    signal DebugLayer2  : IO.ArrayTypes.Vector(0 to 31) := IO.ArrayTypes.NullVector(32);
    signal DebugLayer2P : IO.ArrayTypes.VectorPipe(0 to 7)(0 to 31) := IO.ArrayTypes.NullVectorPipe(8, 32);
    signal HLSStart : std_logic := '0';

begin

    LinkMap : entity work.link_map
    generic map(True)
    port map(clk_p, d, dIO);

    Merge : entity IO.MergeAccumulateInputRegions
    generic map(NFramesPerEvent)
    port map(clk_p, dIO, qIO, HLSStart, DebugLayer1, DebugLayer2);

    MergeOutPipe : entity IO.DataPipe
    port map(clk_p, qIO, qIOP);

    DebugPipe1 : entity IO.DataPipe
    port map(clk_p, DebugLayer1, DebugLayer1P);

    DebugPipe2 : entity IO.DataPipe
    port map(clk_p, DebugLayer2, DebugLayer2P);

    -- Transmit the 128 particles on 16 links over 8 frames
    OLinkGen:
    for i in 0 to 15 generate
        process(clk_p) is
            variable any_valid : boolean := false;
        begin
            any_valid := false;
            if rising_edge(clk_p) then
                for j in 0 to 7 loop
                    if qIOP(j)(i+16*j).DataValid then
                        q(76 + i).data   <= qIOP(j)(i+16*j).data;
                        q(76 + i).valid  <= '1';
                        q(76 + i).strobe <= '1';
                        q(76 + i).start  <= '0';
                        any_valid := true;
                    end if;
                end loop;
                if not any_valid then
                    q(76 + i).data   <= (others => '0');
                    q(76 + i).valid  <= '0';
                    q(76 + i).strobe <= '1';
                    q(76 + i).start <= '0';
                end if;
            end if;
        end process;
    end generate;

    -- Send the DebugLayer1 (16 particle merge of Regions 0 & 1) on some links
    DebugLinkGen1:
    for i in 0 to 3 generate
        process(clk_p) is
            variable any_valid : boolean := false;
        begin
            any_valid := false;
            if rising_edge(clk_p) then
                for j in 0 to 3 loop
                    if debugLayer1P(j)(2*j + i).DataValid then
                        q(92+i).data  <= debugLayer1P(j)(2*j + i).data;
                        q(92+i).valid <= '1';
                        q(92+i).strobe <= '1';
                        q(92+i).start <= '0';
                        any_valid := true;
                    end if;
                end loop;
                if not any_valid then
                    q(92+i).data  <= (others => '0');
                    q(92+i).valid <= '0';
                    q(92+i).strobe <= '1';
                    q(92+i).start <= '0';
                end if;
            end if;
        end process;
    end generate;

    -- Send the DebugLayer2 (64 particle merge of Regions 0, 1, 2, & 3) on some links
    DebugLinkGen2:
    for i in 0 to 3 generate
        process(clk_p) is
            variable any_valid : boolean := false;
        begin
            any_valid := false;
            if rising_edge(clk_p) then
                for j in 0 to 7 loop
                    if debugLayer2P(j)(2*j + i).DataValid then
                        q(96+i).data  <= debugLayer2P(j)(2*j+i).data;
                        q(96+i).valid <= '1';
                        q(96+i).strobe <= '1';
                        q(96+i).start <= '0';
                        any_valid := true;
                    end if;
                end loop;
                if not any_valid then
                    q(96+i).data  <= (others => '0');
                    q(96+i).valid <= '0';
                    q(96+i).strobe <= '1';
                    q(96+i).start <= '0';
                end if;
            end if;
        end process;
    end generate;


	ipb_out <= IPB_RBUS_NULL;
	bc0 <= '0';
	
	gpio <= (others => '0');
	gpio_en <= (others => '0');

end rtl;
