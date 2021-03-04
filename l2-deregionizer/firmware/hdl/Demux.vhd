library ieee;
use ieee.std_logic_1164.all;

use work.DataType.all;
use work.ArrayTypes.all;

entity Demux is
generic(
    N : integer := 1;   -- the number of inputs
    M : integer := 3;   -- the factor to demux (number of ouputs = N*M)
    NEv : integer := 54 -- the number of frames per event
);
port(
    clk : in std_logic := '0';
    D : in Vector(0 to N-1) := NullVector(N);
    Q : out Vector(0 to N*M-1) := NullVector(N*M)
);
end Demux;

architecture behavioral of Demux is
    signal counterEv    : integer range 0 to NEv-1;
    signal counterM     : integer range 0 to M-1;
    signal counterMReg  : integer range 0 to M-1;
    signal D0 : Vector(0 to N-1) := NullVector(N);
    signal X : Vector(0 to N*M-1) := NullVector(N*M);
begin
    D0 <= D when rising_edge(clk);
    counterMReg <= counterM when rising_edge(clk);
    process(clk)
    begin
        if rising_edge(clk) then
            if not D(0).FrameValid then
                counterEv <= 0;
            -- New packet
            elsif D(0).FrameValid and not D0(0).FrameValid then
                counterEv <= 0;
            elsif counterEv = NEv-1 then
                counterEv <= 0;
            else
                counterEv <= counterEv + 1;
            end if;

            if not D(0).FrameValid then
                counterM <= 0;
            --New packet
            elsif D(0).FrameValid and not D0(0).FrameValid then
                counterM <= 0;
            elsif counterM = M-1 then
                counterM <= 0;
            else 
                counterM <= counterM + 1;
            end if;
        end if;
    end process;

    XGen:
    for i in 0 to N*M-1 generate
        signal j : integer := i / N;
        signal k : integer := i mod N;
    begin
        XProc:
        process(clk)
        begin
        if rising_edge(clk) then
            if counterM = j then
                X(i) <= D0(k);
                --X(i).FrameValid <= D(j).FrameValid and counter >= M;
            --else
            --    X(i) <= cNull;
                -- the first M-1 frames each event are marked as FrameInvalid
                -- For deregionizer logic to reset between events
                --X(i).FrameValid <= D(j).FrameValid and counterEv >= M;
            end if;
        end if;
        end process;
    end generate;

    OGen:
    for i in 0 to N*M-1 generate
        OProc:
        process(clk)
        begin
            if rising_edge(clk) then
                if counterMReg = M-1 then
                    Q(i).data <= X(i).data;
                    Q(i).DataValid <= X(i).DataValid;
                else
                    Q(i).data <= (others => '0');
                    Q(i).DataValid <= False;
                end if;
                -- the first M-1 frames each event are marked as FrameInvalid
                -- For deregionizer logic to reset between events
                Q(i).FrameValid <= counterEv >= M-1;
            end if;
        end process;
    end generate;

end behavioral;
