#ifndef LUA_UTILITIES_H
#define LUA_UTILITIES_H

#include <lua.hpp>

#include <QString>
#include <QMap>
#include <QVariant>

typedef QMap<QString,lua_CFunction>		LuaMap;

extern void *luaL_testudata( lua_State *L, int ud, const char *tname );

extern int luaL_pushvariant( lua_State *L, const QVariant &pV );

class lua_util
{
public:
};

#endif // LUA_UTILITIES_H
