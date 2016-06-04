build = {
  build_command = "LUA=$(LUA) autoreconf -i && ./configure --prefix=$(PREFIX) --libdir=$(LIBDIR) --datadir=$(LUADIR) && make clean && make",
  copy_directories = {
  },
  install_command = "make install",
  type = "command",
}
dependencies = {
  "lua == 5.3",
  "stdlib >= 37",
  "luaposix >= 5.1.23",
  "lrexlib-gnu >= 2.7.1",
  "alien >= 0.7.0",
}
description = {
  detailed = "Zee is a lightweight editor. Zee is aimed at small footprint\
systems and quick editing sessions (it starts up and shuts down\
instantly).",
  homepage = "http://github.com/rrthomas/zee/",
  license = "GPLv3+",
  summary = "Experimental lightweight editor",
}
package = "zee"
source = {
  url = "git://github.com/rrthomas/zee.git",
}
version = "git-1"
