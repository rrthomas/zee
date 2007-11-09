/* Bitwise operations library */
/* (c) Reuben Thomas 2000-2007 */
/* See README for license */

#include <inttypes.h>
#include <lauxlib.h>
#include <lua.h>

typedef uintmax_t Integer;
typedef intmax_t UInteger;

#define checkUInteger(L, n) ((UInteger)luaL_checknumber((L), (n)))

#define TDYADIC(name, op, type1, type2) \
  static int bit_ ## name(lua_State* L) { \
    lua_pushnumber(L, (type1)checkUInteger(L, 1) op (type2)checkUInteger(L, 2)); \
    return 1; \
  }

#define MONADIC(name, op, type) \
  static int bit_ ## name(lua_State* L) { \
    lua_pushnumber(L, op (type)checkUInteger(L, 1)); \
    return 1; \
  }

#define VARIADIC(name, op, type) \
  static int bit_ ## name(lua_State *L) { \
    int n = lua_gettop(L), i; \
    Integer w = (type)checkUInteger(L, 1); \
    for (i = 2; i <= n; i++) \
      w op (type)checkUInteger(L, i);      \
    lua_pushnumber(L, w); \
    return 1; \
  }

MONADIC(bnot,    ~,  Integer)
VARIADIC(band,   &=, Integer)
VARIADIC(bor,    |=, Integer)
VARIADIC(bxor,   ^=, Integer)
TDYADIC(lshift,  <<, Integer, UInteger)
TDYADIC(rshift,  >>, UInteger, UInteger)
TDYADIC(arshift, >>, Integer, UInteger)
TDYADIC(mod,     %,  Integer, Integer)

static const struct luaL_reg bitlib[] = {
  {"bnot",    bit_bnot},
  {"band",    bit_band},
  {"bor",     bit_bor},
  {"bxor",    bit_bxor},
  {"lshift",  bit_lshift},
  {"rshift",  bit_rshift},
  {"arshift", bit_arshift},
  {"mod",     bit_mod},
  {NULL, NULL}
};

LUALIB_API int luaopen_bit (lua_State *L) {
  luaL_openlib(L, "bit", bitlib, 0);
  return 1;
}
