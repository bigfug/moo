#ifndef LUA_UTILITIES_H
#define LUA_UTILITIES_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <QString>
#include <QMap>
#include <QVariant>
#include <QXmlDefaultHandler>
#include <QXmlAttributes>
#include <QVector>

typedef QMap<QString,lua_CFunction>		LuaMap;

#if LUA_VERSION_NUM < 502
extern void *luaL_testudata( lua_State *L, int ud, const char *tname );
#endif

extern int luaL_pushvariant( lua_State *L, const QVariant &pV );

class lua_util
{
public:
};

#endif // LUA_UTILITIES_H
