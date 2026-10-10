#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseProjectGameMode.generated.h"

UCLASS()
class BASEPROJECT_API ABaseProjectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseProjectGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
};
