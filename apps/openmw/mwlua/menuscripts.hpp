#ifndef MWLUA_MENUSCRIPTS_H
#define MWLUA_MENUSCRIPTS_H

#include <SDL_events.h>

#include <components/lua/luastate.hpp>
#include <components/lua/scriptscontainer.hpp>
#include <components/sdlutil/events.hpp>

#include "../mwbase/luamanager.hpp"

#include "context.hpp"
#include "inputprocessor.hpp"

namespace MWLua
{

    sol::table initMenuPackage(const Context& context);

    class MenuScripts : public LuaUtil::ScriptsContainer
    {
    public:
        MenuScripts(LuaUtil::LuaState* lua)
            : LuaUtil::ScriptsContainer(lua, "Menu")
            , mInputProcessor(this)
        {
            registerEngineHandlers({ &mStateChanged, &mConsoleCommandHandlers, &mUiModeChanged });
        }

        void processInputEvent(const MWBase::LuaManager::InputEvent& event)
        {
            mInputProcessor.processInputEvent(event);
        }

        void stateChanged() { callEngineHandlers(mStateChanged); }

        bool consoleCommand(const std::string& consoleMode, const std::string& command)
        {
            callEngineHandlers(mConsoleCommandHandlers, consoleMode, command);
            return !mConsoleCommandHandlers.mList.empty();
        }

        void uiModeChanged() { callEngineHandlers(mUiModeChanged); }

    private:
        MWLua::InputProcessor mInputProcessor;
        EngineHandlerList mStateChanged{ "onStateChanged" };
        EngineHandlerList mConsoleCommandHandlers{ "onConsoleCommand" };
        EngineHandlerList mUiModeChanged{ "_onUiModeChanged" };
    };

}

#endif // MWLUA_GLOBALSCRIPTS_H
