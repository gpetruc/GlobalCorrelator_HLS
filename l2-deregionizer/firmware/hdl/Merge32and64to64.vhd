library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

-- Merge 1x64 & 1x32 inputs to 64 outputs
-- Same as Merge32to64 apart from final route
-- Valid data from a and b is 'left aligned' to q
-- Uses UniqueRouter to perform array rotation
-- In config: [8x(4:4)] : [4x(8:8)]
entity Merge32and64to64 is
port(
    clk : in std_logic := '0';
    a : in Vector(0 to 63) := NullVector(64);
    b : in Vector(0 to 31) := NullVector(32);
    q : out Vector(0 to 63) := NullVector(64)
);
end Merge32and64to64;

architecture rtl of Merge32and64to64 is

    constant RouterLatency : integer := 7; -- a guess for now
    signal aPipe : VectorPipe(0 to RouterLatency - 1)(0 to 64 - 1) := NulLVectorPipe(RouterLatency, 64);

    -- Layer input arrays
    -- First index is group, second is within-group
    signal X0 : Matrix(0 to 7)(0 to 3) := NullMatrix(8,4);
    signal X1 : Matrix(0 to 3)(0 to 7) := NullMatrix(4,8);
    signal X2 : Matrix(0 to 3)(0 to 7) := NullMatrix(4,8);

    -- Layer output arrays
    signal Y0 : Matrix(0 to 7)(0 to 3) := NullMatrix(8,4);
    signal Y1 : Matrix(0 to 3)(0 to 7) := NullMatrix(4,8);
    signal Y2 : Matrix(0 to 3)(0 to 7) := NullMatrix(4,8);

    -- Global Address arrays
    signal XA0 : Int.ArrayTypes.Matrix(0 to 7)(0 to 3) := Int.ArrayTypes.NullMatrix(8,4);
    signal XA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);
    signal XA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);

    signal YA0 : Int.ArrayTypes.Matrix(0 to 7)(0 to 3) := Int.ArrayTypes.NullMatrix(8,4);
    signal YA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);
    signal YA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);

    -- Local Address arrays
    signal XLA0 : Int.ArrayTypes.Matrix(0 to 7)(0 to 3) := Int.ArrayTypes.NullMatrix(8,4);
    signal XLA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);
    signal XLA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 7) := Int.ArrayTypes.NullMatrix(4,8);

    -- Final route arrays
    signal bFlat   : Vector(0 to 31) := NullVector(32);
    signal bAFlat  : Int.ArrayTypes.Vector(0 to 31) := Int.ArrayTypes.NullVector(32);
    signal bRouted : Vector(0 to 63) := NullVector(64);
    signal Y       : Vector(0 to 63) := NullVector(64);

    -- N is the current base address to route to
    -- M is the max
    --signal N : Int.DataType.tData := Int.DataType.cNull;
    --signal M : Int.DataType.tData := Int.DataType.cNull;
    signal N : integer range 0 to 63 := 0;

begin

    aPipeEnt:
    entity work.DataPipe
    port map(clk, a, aPipe);
    
    NMProc:
    process(clk)
    begin
        --if rising_edge(clk) then
        -- Find the first invalid input in a to route the b array
        -- to that position
        for i in 0 to 31 loop
            if not a(i).DataValid then
                N <= i;
                exit;
            end if;
        end loop;
        -- edge condition (all of a was valid)
        if a(31).DataValid then
            N <= 32;
        end if;
    end process;
    
    Layer0OLoop:
    for i in 0 to 7 generate
        Layer1ILoop:
        for j in 0 to 3 generate
            signal k0 : Int.DataType.tData := (0, True, True);
            signal k1 : Int.DataType.tData := (0, True, True);
        begin
            -- Connect inputs of group 0 to every 8th input
            -- That way it's guaranteed that they all want different outputs
            -- of Layer0 (route to correct 8th)
            -- Use bits -XX--- aka
            k0.x <= N + 8 * j + i;
            k1.x <= to_integer(to_unsigned(k0.x, 6)(4 downto 3));
            AddrProc:
            process(clk)
            begin
                if rising_edge(clk) then
                    X0(i)(j) <= b(8*j + i);
                    XA0(i)(j) <= k0; 
                    XLA0(i)(j) <= k1;
                end if;
            end process;
        end generate;
        -- First route layer
        Route0:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X0(i),
            DataInGlobAddr  => XA0(i),
            Addr            => XLA0(i),
            DataOut         => Y0(i),
            DataOutGlobAddr => YA0(i)
        );
    end generate;

    Layer1OLoop:
    for i in 0 to 3 generate
        Layer1ILoop:
        for j in 0 to 7 generate
            signal k0 : Int.DataType.tData := (0, True, True);
        begin
            k0.x <= to_integer(to_unsigned(YA0(j)(i).x, 6)(2 downto 0));
            LayerClocks:
            process(clk)
            begin
                if rising_edge(clk) then
                    XLA1(i)(j) <= k0; 
                    -- Inter layer connections
                    X1(i)(j) <= Y0(j)(i);
                    XA1(i)(j) <= YA0(j)(i);
                    X2(i)(j) <= Y1(i)(j);
                    XA2(i)(j) <= YA1(i)(j);
                end if;
            end process;
        end generate;

        -- Second route layer
        Route1:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X1(i),
            DataInGlobAddr  => XA1(i),
            Addr            => XLA1(i),
            DataOut         => Y1(i),
            DataOutGlobAddr => YA1(i)
        );
    end generate;

    -- Fan out the 32 to 64
    bFlatGen:
    for i in 0 to 31 generate
    begin
    	bFlat(i) <= X2(i / 8)(i mod 8);
	    bAFlat(i) <= XA2(i / 8)(i mod 8);
    end generate;

    bRoute:
    for i in 0 to 63 generate
        bRouteProc:
        process(clk)
        begin
            if rising_edge(clk) then
                if bAFlat(i mod 32).x = i and bFlat(i mod 32).DataValid then
                    bRouted(i) <= bFlat(i mod 32);
                else
                    bRouted(i) <= cNull;
                    if bFlat(i mod 32).FrameValid then
                        bRouted(i).FrameValid <= True;
                    end if;
                end if;
            end if;
        end process;
    end generate;

    FinalRoute:
    for i in 0 to 63 generate
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

    q <= Y;

end rtl;

