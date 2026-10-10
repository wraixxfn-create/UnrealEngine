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

	/** Giunti e corsi di pietra che rendono leggibili i blocchi dello stipite. */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> FrameJoints;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Keystone;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Panel;

	/** Rilievi modulari sulla lastra, attaccati all'anta per ruotare con essa. */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> PanelStones;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Larghezza del vano monumentale (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "50.0", Units = "cm"))
	float DoorWidth = 900.0f;

	/** Altezza del vano monumentale (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "100.0", Units = "cm"))
	float DoorHeight = 1200.0f;

	/** Spessore dei massicci stipiti in pietra (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "5.0", Units = "cm"))
	float FrameThickness = 180.0f;

	/** Profondita' del portale lungo X (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "5.0", Units = "cm"))
	float FrameDepth = 220.0f;

	/** Spessore della pesante anta in pietra (cm). */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "2.0", Units = "cm"))
	float PanelThickness = 70.0f;

	/** Angolo di apertura dell'anta (gradi). */
	UPROPERTY(EditAnywhere, Category = "Door")
	float OpenAngle = 90.0f;

	/** Velocita' d'interpolazione per apertura e chiusura dell'anta. */
	UPROPERTY(EditAnywhere, Category = "Door", meta = (ClampMin = "0.1"))
	float OpenSpeed = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor FrameColor = FLinearColor(0.36f, 0.37f, 0.37f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor PanelColor = FLinearColor(0.28f, 0.30f, 0.31f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor JointColor = FLinearColor(0.13f, 0.14f, 0.15f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor StoneDetailColor = FLinearColor(0.39f, 0.40f, 0.39f);

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
