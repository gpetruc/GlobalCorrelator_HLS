library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

entity UniqueRouter is
port(
    clk             : in std_logic := '0';
    DataIn          : in Vector;
    DataInGlobAddr  : in Int.ArrayTypes.Vector;
    Addr            : in Int.ArrayTypes.Vector;
    DataOut         : out Vector;
    DataOutGlobAddr : out Int.ArrayTypes.Vector
);
end UniqueRouter;

-------------------------------------------------
-- Route the incoming data
-- No buffering is used so assumes each input is
-- sent to a uniqe output
-------------------------------------------------

architecture rtl of UniqueRouter is
begin

    Gen:
    for i in DataIn'range generate
    begin
        Proc:
        process(clk)
        begin
            if rising_edge(clk) then
                DataOut(i) <= cNull;
                DataOutGlobAddr(i) <= Int.DataType.cNull;
                for j in DataIn'range loop
                    if i = Addr(j).x then
                        DataOut(i) <= DataIn(j);
                        DataOutGlobAddr(i) <= DataInGlobAddr(j);
                        exit;
                    end if;
                end loop;
            end if;
        end process;
    end generate;

end rtl;
