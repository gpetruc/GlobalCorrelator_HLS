library ieee;
use ieee.std_logic_1164.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

-- One region is received on 6 links over 3 clocks
-- And demuxed to 16 particles (eventually 18)
entity DemuxMergeAccumulateInputRegions is
generic(
    NFRAMESPEREVENT : integer := 54
);
port(
    clk : in std_logic := '0';
    RegionStreams : in Matrix(0 to 5)(0 to 5) := NullMatrix(6, 6);
    EventParticles : out Vector(0 to 127) := NullVector(128);
    HLSstart : out std_logic := '0';
    -- Just for sending debug data out for standalone deregionizer testing
    -- Leave unconnected for deregionizer + algorithm designs
    DebugLayer1    : out Vector(0 to 31) := NullVector(32);
    DebugLayer2    : out Vector(0 to 63) := NullVector(64);
    DebugMerged    : out Vector(0 to 63) := NullVector(64)
);
end DemuxMergeAccumulateInputRegions;

architecture rtl of DemuxMergeAccumulateInputRegions is

    signal RegionStreamsDemuxed          : Matrix(0 to 5)(0 to 17) := NullMatrix(6, 18);
    signal RegionStreamsDemuxedTruncated : Matrix(0 to 5)(0 to 15) := NullMatrix(6, 16);

begin

    GRegion:
    for i in 0 to 5 generate
        Demux : entity work.Demux
        generic map(6, 3, NFRAMESPEREVENT)
        port map(clk, RegionStreams(i), RegionStreamsDemuxed(i));
        -- Reduce 18 -> 16
        -- TODO remove this when deregionizer handles 18
        RegionStreamsDemuxedTruncated(i) <= RegionStreamsDemuxed(i)(0 to 15);
    end generate;

    MergeAccumulate : entity work.MergeAccumulateInputRegions
    port map(clk, RegionStreamsDemuxedTruncated, EventParticles, HLSstart, DebugLayer1, DebugLayer2, DebugMerged);

end rtl;

