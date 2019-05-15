#include "resources/script_resource.h"
#include "lua.h"

namespace engine
{
bool initializeLua();
void shutdownLua();
struct lua_State* getLuaState();

}