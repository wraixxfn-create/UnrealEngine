#include "BaseProjectGameMode.h"

#include "BaseProjectCharacter.h"

ABaseProjectGameMode::ABaseProjectGameMode()
{
	DefaultPawnClass = ABaseProjectCharacter::StaticClass();
}
