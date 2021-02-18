library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use std.textio.all;

--library utilities;
--use utilities.utilities.all;

package DataType is

  type tData is record
    data       : std_logic_vector(63 downto 0);  
    DataValid  : boolean;
    FrameValid : boolean;
  end record;
 
  attribute size : natural;
  attribute size of tData : type is 66;  

  constant cNull : tData := ((others => '0'), false, false);

  function ToStdLogicVector(x : tData) return std_logic_vector;
  function ToDataType(x : std_logic_vector) return tData;

  function WriteHeader return string;
  function WriteData(d : tData) return string;

  function ">" (x, y : tData) return boolean;

end DataType;

package body DataType is

  function ToStdLogicVector(x : tData) return std_logic_vector is
    variable h : integer := x.data'high;
    variable l : integer := 0;
    variable y : std_logic_vector(tData'size - 1 downto 0) := (others => '0');
  begin
    y(h downto l) := std_logic_vector(x.data);
    return y;
  end function;

  function ToDataType(x : std_logic_vector) return tData is
    variable y : tData := cNull;
    variable h : integer := y.data'high;
    variable l : integer := 0;
  begin
    y.data := std_logic_vector(x(h downto l));
    return y;
  end function;

  function WriteHeader return string is
    variable x : line;
  begin
    write(x, string'("pt"), right, 15);
    write(x, string'("eta"), right, 15);
    write(x, string'("phi"), right, 15);
    write(x, string'("FrameValid"), right, 15);
    write(x, string'("DataValid"), right, 15);
    return x.all;
  end WriteHeader;

  function WriteData(d : tData) return string is 
    variable x : line;
  begin
    write(x, to_integer(unsigned(d.data(15 downto 0))), right, 15);
    write(x, to_integer(signed(d.data(25 downto 16))), right, 15);
    write(x, to_integer(signed(d.data(35 downto 26))), right, 15);
    write(x, d.FrameValid, right, 15);
    write(x, d.DataValid, right, 15);
    return x.all;
  end WriteData;

  function ">" (x, y : tData) return boolean is
    variable ret : boolean := false;
  begin
    if x.DataValid and not y.DataValid then
      ret := true;
    elsif y.DataValid and not x.DataValid then
      ret := true;
    elsif to_integer(unsigned(x.data)) > to_integer(unsigned(y.data)) then
      ret := true;
    else
      ret := false;
    end if;
    return ret;
  end ">";

end DataType;
