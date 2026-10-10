#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectMapBase.generated.h"

class UChildActorComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Base di mappa enorme: una lastra di terreno quadrata (con collisione) la cui
 * dimensione e' configurabile, con una porta posizionata sopra la superficie.
 * Piazzala nel livello; la geometria si rigenera ad ogni modifica nell'editor.
 */
UCLASS()
class BASEPROJECT_API ABaseProjectMapBase : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectMapBase();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Map")
	TObjectPtr<USceneComponent> Root;

	/** Lastra del terreno (cubo scalato). La superficie superiore e' a Z = 0. */
	UPROPERTY(VisibleAnywhere, Category = "Map")
	TObjectPtr<UStaticMeshComponent> Ground;

	/** Porta, creata come attore figlio. */
	UPROPERTY(VisibleAnywhere, Category = "Map")
	TObjectPtr<UChildActorComponent> DoorComponent;

	/** Lato della mappa quadrata, in metri (2000 = 2 km x 2 km). */
	UPROPERTY(EditAnywhere, Category = "Map", meta = (ClampMin = "10.0", Units = "m"))
	float MapSizeMeters = 2000.0f;

	/** Spessore della lastra di terreno, in cm. */
	UPROPERTY(EditAnywhere, Category = "Map", meta = (ClampMin = "10.0"))
	float GroundThickness = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Map")
	FLinearColor GroundColor = FLinearColor(0.20f, 0.36f, 0.18f);

	/** Posizione della porta sul terreno (cm, relativa al centro della mappa). */
	UPROPERTY(EditAnywhere, Category = "Map|Door")
	FVector DoorLocation = FVector(0.0f, 0.0f, 0.0f);

	/** Rotazione della porta (gradi). */
	UPROPERTY(EditAnywhere, Category = "Map|Door")
	FRotator DoorRotation = FRotator::ZeroRotator;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GroundMID;
};
