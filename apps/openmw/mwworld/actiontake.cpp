#include "actiontake.hpp"

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/mechanicsmanager.hpp"

#include "../mwgui/inventorywindow.hpp"

#include "class.hpp"
#include "containerstore.hpp"

namespace MWWorld
{
    ActionTake::ActionTake (const MWWorld::Ptr& object) : Action (true, object) {}

    void ActionTake::executeImp (const Ptr& actor)
    {
        // When in GUI mode, we should use drag and drop
        if (actor == MWBase::Environment::get().getWorld()->getPlayerPtr())
        {
            MWGui::GuiMode mode = MWBase::Environment::get().getWindowManager()->getMode();
            if (mode == MWGui::GM_Inventory || mode == MWGui::GM_Container)
            {
                MWBase::Environment::get().getWindowManager()->getInventoryWindow()->pickUpObject(getTarget());
                return;
            }
        }

        int count = std::abs(getTarget().getRefData().getCount());
        MWBase::Environment::get().getMechanicsManager()->itemTaken(
                    actor, getTarget(), MWWorld::Ptr(), count);
        MWWorld::Ptr newitem = *actor.getClass().getStoreManager (actor).getMutable().add (getTarget(), count, actor);
        MWBase::Environment::get().getWorld()->deleteObject (getTarget());
        setTarget(newitem);
    }
}
