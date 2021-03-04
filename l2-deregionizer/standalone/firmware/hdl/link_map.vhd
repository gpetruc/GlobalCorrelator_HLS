library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

library IO;
use IO.DataType;
use IO.ArrayTypes;

use work.emp_data_types.all;
use work.emp_project_decl.all;

use work.emp_device_decl.all;

-- Map links to regions such that each group originates from the same SLR
-- Using the map of https://gitlab.cern.ch/p2-xware/firmware/emp-fwk/blob/task/board-datapath-region-diagrams/boards/vcu118/regions.md
entity link_map is
    generic(
        opt : boolean := false
    );
	port(
		clk: in std_logic; -- ipbus signals
		d: in ldata(4 * N_REGION - 1 downto 0); -- data in
        q: out IO.ArrayTypes.Matrix(0 to 5)(0 to 5) := IO.ArrayTypes.NullMatrix(6, 6)
	);
end link_map;

architecture rtl of link_map is
	
    signal dVIO : IO.ArrayTypes.Vector(0 to 4 * N_REGION - 1) := IO.ArrayTypes.NullVector(4 * N_REGION);
    signal dPIO : IO.ArrayTypes.VectorPipe(0 to 4)(0 to 4 * N_REGION - 1) := IO.ArrayTypes.NullVectorPipe(5, 4 * N_REGION);
    signal qInt : IO.ArrayTypes.Matrix(0 to 5)(0 to 5) := IO.ArrayTypes.NullMatrix(6, 6);

begin

    InputCast:
    for i in 0 to 4 * N_REGION - 1 generate
        dVIO(i).data       <= d(i).data;
        dVIO(i).DataValid  <= true when d(i).data(63) = '1' else false; -- Use the top bit
        dVIO(i).FrameValid <= true when d(i).valid = '1' else false;
    end generate;

    Pipe : entity IO.DataPipe
    port map(clk, dVIO, dPIO);

    GMap:
    if opt generate

        -- Take 3 big-regions from SLR1 RHS
        Q0to2:
        for i in 0 to 2 generate
            qInt(i) <= dPIO(4)(16 + 6*i to 16 + 6*(i+1)-1);
        end generate;
        -- Take 3 big-regions from SLR1 LHS
        Q3to5:
        for i in 0 to 2 generate
            qInt(3+i) <= dPIO(4)(76 + 6*i to 76 + 6*(i+1)-1);
        end generate;

    else generate
        Gi:
        for i in 0 to 5 generate
            Gj:
            for j in 0 to 5 generate
            begin
                qInt(i)(j) <= dPIO(4)(6*i + j);
            end generate;
        end generate;
    end generate;


    -- Map to the Merge inputs such that the first layer of merging uses
    -- inputs in the same SLR
    q(0) <= qInt(0);
    q(1) <= qInt(1);
    q(2) <= qInt(2);
    q(3) <= qInt(3);
    q(4) <= qInt(4);
    q(5) <= qInt(5);

    Debug : entity IO.Debug
    generic map("Regionizer-Inputs", "./")
    port map(clk, dVIO);
end rtl;
