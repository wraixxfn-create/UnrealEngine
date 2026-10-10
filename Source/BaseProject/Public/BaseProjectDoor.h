#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Porta con stipite e anta incernierata. Si apre automaticamente quando un pawn
 * entra nell'area di attivazione e si richiude quando esce.
 */
UCLASS()
class BASEPROJECT_API ABaseProjectDoor : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectDoor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> LeftPost;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> RightPost;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Lintel;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Panel;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Larghezza del vano porta (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "50.0"))
	float DoorWidth = 120.0f;

	/** Altezza del vano porta (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "100.0"))
	float DoorHeight = 240.0f;

	/** Spessore dello stipite (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "5.0"))
	float FrameThickness = 20.0f;

	/** Profondita' del muro/stipite lungo X (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "5.0"))
	float FrameDepth = 30.0f;

	/** Spessore dell'anta (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "2.0"))
	float PanelThickness = 6.0f;

	/** Angolo di apertura dell'anta (gradi). */
	UPROPERTY(EditAnywhere, Category = "Door")
	float OpenAngle = 100.0f;

	/** Velocita' di apertura/chiusura. */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.1"))
	float OpenSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor FrameColor = FLinearColor(0.35f, 0.25f, 0.15f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor PanelColor = FLinearColor(0.45f, 0.30f, 0.18f);

private:
	bool bIsOpen = false;
	int32 OverlappingPawns = 0;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyCubeTransform(UStaticMeshComponent* Comp, const FVector& Center, const FVector& Size) const;
	void ApplyColor(UStaticMeshComponent* Comp, const FLinearColor& Color) const;
};
