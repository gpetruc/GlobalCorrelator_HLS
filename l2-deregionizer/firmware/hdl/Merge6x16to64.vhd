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
    D : in Matrix(0 to 6 - 1)(0 to 16 - 1) := NullMatrix(6, 16);
    Q : out Vector(0 to 64 - 1) := NullVector(64);
    -- Just for sending debug data out for standalone deregionizer testing
    -- Leave unconnected for deregionizer + algorithm designs
    DebugLayer1 : out Vector(0 to 31) := NullVector(32);
    DebugLayer2 : out Vector(0 to 63) := NullVector(64)
);
end MergeArrays;

architecture behavioral of MergeArrays is
    constant latency_l1 : integer := 8; -- The latency of the 2nd merge layer for pipelining

    -- Output of first merge layer
    signal d0 : Matrix(0 to 2)(0 to 31) := NullMatrix(3,32);
    -- Output of second merge layer
    signal d0_pipe : VectorPipe(0 to latency_l1 - 1)(0 to 31) := NullVectorPipe(latency_l1, 32);
    signal d1 : Matrix(0 to 1)(0 to 63) := NullMatrix(2,64);
    signal d2 : Matrix(0 to 1)(0 to 63) := NullMatrix(2,64);

begin

    MergeLayer1:
    for i in 0 to 2 generate
        merge : entity work.Merge16to32
        port map(
            clk => clk,
            a => d(2*i),
            b => d(2*i + 1),
            q => d0(i)
        );
    end generate;

    MergeLayer2: entity work.Merge32to64
    port map(
        clk => clk,
        a => d0(0),
        b => d0(1),
        q => d1(0)
    );

    PipeLayer2: entity work.DataPipe
    port map(clk, d0(2), d0_pipe);

    MergeLayer3: entity work.Merge32and64to64
    port map(
        clk => clk,
        a => d1(0),
        b => d0_pipe(latency_l1-1),
        q => Q
    );

    DebugLayer1 <= d0(0);
    DebugLayer2 <= d1(0);

end behavioral;
