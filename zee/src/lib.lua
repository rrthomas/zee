-- Zee-specific library functions

-- Parse re-usable C headers
function X(...)
  xarg = {...}
end

-- Extract the docstrings from tbl_funcs.lua
docstring = {}
function def(t)
  local name, s
  for i, v in pairs(t) do
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
