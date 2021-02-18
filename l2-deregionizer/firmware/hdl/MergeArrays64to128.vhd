library ieee;
use ieee.std_logic_1164.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

entity MergeArrays is
generic(
  constant nMerge : integer := 2;
  --constant nIn : integer := 128;
  --constant nOut : integer := 128
);
port(
    clk : in std_logic := '0';
    A : in Vector(0 to nIn - 1) := NullVector(nIn);
    B : in Vector(0 to nIn - 1) := NullVector(nIn);
    Q : out Vector(0 to nOut - 1) := NullVector(nOut);
);
end MergeArrays;

architecture rtl of MergeArrays is
    constant RouterLatency : integer := 12; -- a guess for now
    signal APipe : VectorPipe(0 to RouterLatency-1)(0 to nIn -1) := NullVectorPipe(RouterLatency, nIn);

    -- Layer input arrays
    -- First index is group, second is within-group
    signal X0 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal X1 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal X2 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal X3 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);

    -- Layer output arrays
    signal Y0 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal Y1 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal Y2 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);
    signal Y3 : Matrix(0 to 31)(0 to 3) := NullMatrix(32,4);

    -- Global Address arrays
    signal XA0 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XA1 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XA2 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XA3 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);

    signal YA0 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal YA1 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal YA2 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal YA3 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);

    -- Local Address arrays
    signal XLA0 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XLA1 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XLA2 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);
    signal XLA3 : Int.ArrayTypes.Matrix(0 to 31)(0 to 3) := Int.ArrayTypes.NullMatrix(32,4);

    -- N is the current base address to route to
    -- M is the max
    signal N : integer range 0 to nOut := 0;
begin

    NProc:
    process(clk)
    begin
        for i in 0 to 127 loop
            if not A(i).DataValid then
                N <= i;
                exit;
            end loop;
        end loop;
    end process;

    APipe:
    entity DataPipe(clk, A, APipe);

    -- Compute an address for every input
    -- Also clock the input into the next array to keep sync with addr
    OLoop:
    for i in 0 to 31 generate
        ILoop:
        for j in 0 to 3 generate
            signal k0  : integer := 0;
            signal k1  : integer := 0;
            signal k2  : integer := 0;
            signal k3  : integer := 0;
            signal k4  : integer := 0;
            signal ki0 : Int.DataType.tData := (0, True, True);
            signal ki1 : Int.DataType.tData := (0, True, True);
            signal ki2 : Int.DataType.tData := (0, True, True);
            signal ki3 : Int.DataType.tDara := (0, True, True);
            signal ki4 : Int.DataType.tDara := (0, True, True);
        begin
            --AddrInProc:
            --process(clk)
            --begin
            k0 <= N + 4 * i + j;
            --k1 <= k0 mod 4;
            --k2 <= (XA1(i)(j).x mod 16) / 4;
            -- Slice the lowest 2 bits. Aka x % 4
            k1 <= to_integer(to_unsigned(k0, 7)(1 downto 0));
            -- Slice the next 2 bits. Aka (x % 16) // 4
            k2 <= to_integer(to_unsigned(XA1(i)(j).x, 7)(3 downto 2));
            k3 <= to_integer(to_unsigned(XA2(i)(j).x, 7)(5 downto 4));
            k4 <= to_integer(to_unsigned(XA3(i)(j).x, 7)(6 downto 6));
           
            ki0.x <= k0;
            ki1.x <= k1;
            ki2.x <= k2;
            ki3.x <= k3;
            ki4.x <= k4;
            X0(i)(j) <= d(4*i + j);
            XA0(i)(j) <= ki0; 
            XLA0(i)(j) <= ki1;
            XLA1(i)(j) <= ki2; 
            XLA2(i)(j) <= ki3;
            XLA3(i)(j) <= ki4; 
            --end process;

            -- Inter layer connections
            X1(i)(j) <= Y0(j)(i);
            XA1(i)(j) <= YA0(j)(i);
            X2(i)(j) <= Y1(j)(i);
            XA2(i)(j) <= YA1(j)(i);
            X3(i)(j) <= Y2(j)(i);
            XA3(i)(j) <= YA2(j)(i);
            X4(i)(j) <= Y3(j)(i);
            XA4(i)(j) <= YA3(j)(i);
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

        -- Third route layer
        Route2:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X2(i),
            DataInGlobAddr  => XA2(i),
            Addr            => XLA2(i),
            DataOut         => Y2(i),
            DataOutGlobAddr => YA2(i)
        );

        -- Fourth route layer
        Route3:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X3(i),
            DataInGlobAddr  => XA3(i),
            Addr            => XLA3(i),
            DataOut         => Y3(i),
            DataOutGlobAddr => YA3(i)
        );
    end generate;

    FinalRoute:
    for i in 0 to 128 generate
        FinalRouteProc:
        process(clk)
            if rising_edge(clk) then
                if APipe(RouterLatency-1)(i).DataValid then
                    Y(i) <= APipe(RouterLatency)(i);
                elsif BRouted(i).DataValid then
                    Y(i) <= BRouted(i);
                else
                    Y(i) <= cNull;
                end if;
            end if;
        end process;
    end generate;

end rtl;
