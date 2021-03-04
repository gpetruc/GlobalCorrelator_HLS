library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use std.textio.all;

package DataType is

  type tData is record
    x : integer range 0 to 128;
    DataValid : boolean;
    FrameValid : boolean;
  end record;

  constant cNull : tData := (0, false, false);

  function "+" (a, b : tData) return tData;

  function WriteHeader return string;
  function WriteData(d : tData) return string;

end DataType;

package body DataType is

  function "+" (a, b : tData) return tData is
    variable z : tData := cNull;
  begin
    z.x := a.x + b.x;
    z.DataValid := a.DataValid or b.DataValid;
    z.FrameValid := a.FrameValid or b.FrameValid;
    return z;
  end function;

  function WriteHeader return string is
    variable x : line;
  begin
    write(x, string'("x"), right, 15);
    write(x, string'("FrameValid"), right, 15);
    write(x, string'("DataValid"), right, 15);
    return x.all;
  end WriteHeader;

  function WriteData(d : tData) return string is 
    variable x : line;
  begin
    write(x, d.x, right, 15);
    write(x, d.FrameValid, right, 15);
    write(x, d.DataValid, right, 15);
    return x.all;
  end WriteData;

end DataType;
