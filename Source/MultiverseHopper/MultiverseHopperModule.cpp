#include "Modules/ModuleManager.h"
#include "Core/MultiverseGameInstance.h"
#include "Character/BrainrotCharacter.h"
#include "Weapons/StaffWeapon.h"
#include "Enemy/BaseEnemy.h"
#include "Enemy/HeavyEnemy.h"
#include "Gameplay/MultiverseGameModeBase.h"
#include "UI/MultiverseHUD.h"
#include "Gameplay/MissionSystem.h"
#include "Systems/DimensionManager.h"
#include "Systems/ProceduralUniverseGenerator.h"

class FMultiverseHopperModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
    }

    virtual void ShutdownModule() override
    {
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FMultiverseHopperModule, MultiverseHopper, "MultiverseHopper");