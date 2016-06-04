version = "git-1"
package = "zee"
build = {
  build_command = "LUA=$(LUA) autoreconf -i && ./configure --prefix=$(PREFIX) --libdir=$(LIBDIR) --datadir=$(LUADIR) && make clean && make",
  type = "command",
  copy_directories = {
  },
  install_command = "make install",
}
dependencies = {
  "lua == 5.3",
  "stdlib >= 37",
  "luaposix >= 5.1.23",
  "lrexlib-gnu >= 2.7.1",
  "alien >= 0.7.0",
}
description = {
  license = "GPLv3+",
  summary = "Experimental lightweight editor",
  detailed = "Zee is a lightweight editor. Zee is aimed at small footprint\
systems and quick editing sessions (it starts up and shuts down\
instantly).",
  homepage = "http://github.com/rrthomas/zee/",
}
source = {
  url = "git://github.com/rrthomas/zee.git",
}
