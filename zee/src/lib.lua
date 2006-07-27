-- Library functions for Lua scripts

function die(s)
  error(NAME .. ": " .. s)
end

-- Utility function for parsing tbl_*.h
function X(...)
  xarg = {...}
end
