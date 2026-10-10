#include "BaseProjectGameMode.h"

#include "BaseProjectCharacter.h"
#include "BaseProjectMapBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

ABaseProjectGameMode::ABaseProjectGameMode()
{
	DefaultPawnClass = ABaseProjectCharacter::StaticClass();
}

void ABaseProjectGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Il repository non contiene ancora un .umap binario: crea la mappa base
	// al volo, senza duplicarla se e' gia' stata aggiunta manualmente al livello.
	bool bHasMapBase = false;
	for (TActorIterator<ABaseProjectMapBase> It(World); It; ++It)
	{
		bHasMapBase = true;
		break;
	}
	if (!bHasMapBase)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ABaseProjectMapBase>(ABaseProjectMapBase::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	}

	// Fornisce un punto di partenza rivolto verso il portale se il livello e' vuoto.
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}
	if (!bHasPlayerStart)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
			FVector(-4000.0f, 0.0f, 96.0f), FRotator::ZeroRotator, SpawnParameters);
	}
}
