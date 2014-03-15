version = "git-1"
dependencies = {
  "lua == 5.2",
  "stdlib >= 37",
  "luaposix >= 5.1.23",
  "lrexlib-gnu >= 2.7.1",
  "alien >= 0.7.0",
}
package = "zee"
source = {
  url = "git://github.com/rrthomas/zee.git",
}
description = {
  summary = "Experimental lightweight editor",
  homepage = "http://github.com/rrthomas/zee/",
  detailed = "Zee is a lightweight editor. Zee is aimed at small footprint\
systems and quick editing sessions (it starts up and shuts down\
instantly).",
  license = "GPLv3+",
}
build = {
  copy_directories = {
  },
  build_command = "LUA=$(LUA) autoreconf -i && ./configure --prefix=$(PREFIX) --libdir=$(LIBDIR) --datadir=$(LUADIR) && make clean && make",
  type = "command",
  install_command = "make install",
}
