-- Library functions for Lua mk* scripts

-- Utility function for parsing tbl_*.h
function X(...)
  xarg = {...}
end

-- Utility function for parsing doc-stringed definitions
docstring = {}
function doc(t)
  local name, s
  for i, v in pairs (t) do
    if type(i) == "string" then
      name = i
      _G[name] = v
    elseif type(i) == "number" and i == 1 then
      s = v
    end
  end
  if name then
    docstring[name] = s
  end
end
