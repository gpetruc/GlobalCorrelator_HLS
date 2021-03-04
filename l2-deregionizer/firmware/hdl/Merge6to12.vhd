library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

-- Append 2 arrays of length 6 to one array of length 12
-- The 'valid' elements of the input arrays are contiguous in the output
entity Merge6to12 is
port(
    clk : in std_logic := '0';
    a : in Vector(0 to 5) := NullVector(6);
    b : in Vector(0 to 5) := NullVector(6);
    q : out Vector(0 to 11) := NullVector(12)
);
end Merge6to12;

architecture rtl of Merge6to12 is

    -- Internally we map the length-6 arrays to length-8 (with null data)
    -- and the length-12 output is initially length-16, then truncated
    -- Just use one layer of 'Unique Router' since 8 inputs is small enough
    constant RouterLatency : integer := 4;
    signal a8        : Vector(0 to 7) := NullVector(8);
    signal b8        : Vector(0 to 7) := NullVector(8);
    signal b8_reg    : Vector(0 to 7) := NullVector(8);
    signal aPipe : VectorPipe(0 to RouterLatency - 1)(0 to 7) := NulLVectorPipe(RouterLatency, 8);

    -- Layer input arrays
    -- First index is group, second is within-group
    signal X0 : Vector(0 to 7) := NullVector(8);

    -- Layer output arrays
    signal Y0 : Vector(0 to 7) := NullVector(8);

    -- Global Address arrays
    signal XA0 : Int.ArrayTypes.Vector(0 to 7) := Int.ArrayTypes.NullVector(8);
    signal YA0 : Int.ArrayTypes.Vector(0 to 7) := Int.ArrayTypes.NullVector(8);

    -- Local Address arrays
    signal XLA0 : Int.ArrayTypes.Vector(0 to 7) := Int.ArrayTypes.NullVector(8);

    -- Final route arrays
    signal bRouted  : Vector(0 to 15) := NullVector(16);
    signal Y : Vector(0 to 15) := NullVector(16);

    -- N is the current base address to route to
    -- M is the max
    signal N : integer range 0 to 8 := 0;

begin

    a8(0 to 5) <= a;
    a8(6 to 7) <= (others => ((others => '0'), false, a(5).FrameValid));
    b8(0 to 5) <= b;
    b8(6 to 7) <= (others => ((others => '0'), false, b(5).FrameValid));
    b8_reg <= b8 when rising_edge(clk);

    aPipeEnt:
    entity work.DataPipe
    port map(clk, a8, aPipe);
    
    NMProc:
    process(clk)
    begin
        if rising_edge(clk) then
            -- Find the first invalid input in a to route the b array
            -- to that position
            for i in 0 to 5 loop
                if not a8(i).DataValid then
                    N <= i;
                    exit;
                end if;
            end loop;
            -- edge condition (all of a was valid)
            if a8(5).DataValid then
                N <= 6;
            end if;
        end if;
    end process;
    
    -- Compute an address for every input
    -- Also clock the input into the next array to keep sync with addr
    ILoop:
    for i in 0 to 7 generate
        signal k0 : Int.DataType.tData := (0, True, True);
        signal k1 : Int.DataType.tData := (0, True, True);
    begin
        k0.x <= N + i;
        k1.x <= to_integer(to_unsigned(k0.x, 6)(2 downto 0));
        X0(i) <= b8_reg(i);
        XA0(i) <= k0; 
        XLA0(i) <= k1;
    end generate;

    -- First route layer
    Route0:
    entity work.UniqueRouter
    port map(
        clk             => clk,
        DataIn          => X0,
        DataInGlobAddr  => XA0,
        Addr            => XLA0,
        DataOut         => Y0,
        DataOutGlobAddr => YA0
    );

    -- Fan out the 8 to 16
    bRoute:
    for i in 0 to 15 generate
        bRouteProc:
        process(clk)
        begin
            if rising_edge(clk) then
                if YA0(i mod 8).x = i and YA0(i mod 8).DataValid then
                    bRouted(i) <= Y0(i mod 8);
                else
                    bRouted(i) <= cNull;
                    if Y0(i mod 8).FrameValid then
                        bRouted(i).FrameValid <= True;
                    end if;
                end if;
            end if;
        end process;
    end generate;

    FinalRoute:
    for i in 0 to 15 generate
        FRLowerHalf:
        if i < 8 generate
            FinalRouteProcA:
            process(clk)
            begin
                if rising_edge(clk) then
                    if aPipe(RouterLatency-1)(i).DataValid then
                        Y(i) <= aPipe(RouterLatency-1)(i);
                    elsif bRouted(i).DataValid then
                        Y(i) <=  bRouted(i);
                    else
                        Y(i) <= cNull;
                        if aPipe(RouterLatency-1)(i).FrameValid then
                            Y(i).FrameValid <= True;
                        end if;
                    end if;
                end if;
            end process;
        end generate;
        FRUpperHalf:
        if i >= 8 generate
            FinalRouteProcB:
            process(clk)
            begin
                if rising_edge(clk) then
                    if bRouted(i).DataValid then
                        Y(i) <= bRouted(i);
                    else
                        Y(i) <= cNull;
                        if bRouted(i).FrameValid then
                            Y(i).FrameValid <= True;
                        end if;
                    end if;
                end if;
            end process;
        end generate;
    end generate;

    ProcOut:
    process(clk)
    begin
        if rising_edge(clk) then
            q <= Y(0 to 11);
        end if;
    end process;

end rtl;

