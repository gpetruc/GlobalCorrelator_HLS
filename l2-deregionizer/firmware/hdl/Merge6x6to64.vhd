library ieee;
use ieee.std_logic_1164.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

entity MergeArrays is
port(
    clk : in std_logic := '0';
    D : in Matrix(0 to 5)(0 to 5) := NullMatrix(6,6);
    Q : out Vector(0 to 63) := NullVector(64);
    -- Just for sending debug data out for standalone deregionizer testing
    -- Leave unconnected for deregionizer + algorithm designs
    DebugLayer1 : out Vector(0 to 15) := NullVector(16);
    DebugLayer2 : out Vector(0 to 31) := NullVector(32)
);
end MergeArrays;

architecture behavioral of MergeArrays is
    constant latency_l1 : integer := 7; -- The latency of the 2nd merge layer for pipelining

    -- Output of first merge layer
    signal d0 : Matrix(0 to 2)(0 to 15) := NullMatrix(3,16);
    -- Output of second merge layer
    signal d0_pipe : VectorPipe(0 to latency_l1 - 1)(0 to 15) := NullVectorPipe(latency_l1, 16);
    signal d1 : Matrix(0 to 1)(0 to 31) := NullMatrix(2,32);
    signal d2 : Matrix(0 to 1)(0 to 63) := NullMatrix(2,64);

begin

    MergeLayer1:
    for i in 0 to 2 generate
        merge : entity work.Merge6to12
        port map(
            clk => clk,
            a => d(2*i),
            b => d(2*i + 1),
            q => d0(i)(0 to 11)
        );
        d0(i)(12 to 15) <= (others => ((others => '0'), false, d0(i)(11).FrameValid));
    end generate;

    MergeLayer2: entity work.Merge16to32
    port map(
        clk => clk,
        a => d0(0),
        b => d0(1),
        q => d1(0)
    );

    PipeLayer2: entity work.DataPipe
    port map(clk, d0(2), d0_pipe);

    d1(1)(0 to 15) <= d0_pipe(latency_l1-1);
    DFV:
    for i in 16 to 31 generate
        d1(1)(i) <= ((others => '0'), False, d1(1)(i-16).FrameValid);
    end generate;

    MergeLayer3: entity work.Merge32to64
    port map(
        clk => clk,
        a => d1(0),
        b => d1(1),
        q => Q
    );

    DebugLayer1 <= d0(1);
    DebugLayer2 <= d1(0);

end behavioral;
