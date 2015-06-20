-- Rockspec data

-- Variables to be interpolated:
--
-- package_name
-- version

local default = {
  package = package_name,
  version = version.."-1",
  source = {
    url = "git://github.com/rrthomas/"..package_name..".git",
  },
  description = {
    summary = "Experimental lightweight editor",
    detailed = [[
Zee is a lightweight editor. Zee is aimed at small footprint
systems and quick editing sessions (it starts up and shuts down
instantly).]],
    homepage = "http://github.com/rrthomas/"..package_name.."/",
    license = "GPLv3+",
  },
  dependencies = {
    "lua == 5.3",
    "stdlib >= 37",
    "luaposix >= 5.1.23",
    "lrexlib-gnu >= 2.7.1",
    "alien >= 0.7.0",
  },
  build = {
    type = "command",
    build_command = "LUA=$(LUA) autoreconf -i && ./configure --prefix=$(PREFIX) --libdir=$(LIBDIR) --datadir=$(LUADIR) && make clean && make",
    install_command = "make install",
    copy_directories = {},
  },
}

if version ~= "git" then
  default.source.branch = "v"..version
end

return {default=default, [""]={}}
