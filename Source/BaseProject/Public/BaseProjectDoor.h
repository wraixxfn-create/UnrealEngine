#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectDoor.generated.h"

class APawn;
class UBoxComponent;
class UMaterialInterface;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Portale monumentale in pietra a due ante. La geometria e' modificabile nei
 * dettagli; le ante si aprono lentamente quando un pawn si avvicina.
 * X e' la profondita', Y la larghezza e Z l'altezza, in centimetri.
 */
UCLASS()
class BASEPROJECT_API ABaseProjectDoor : public AActor
{
	GENERATED_BODY()

public:
	ABaseProjectDoor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Assegna il materiale anche a una porta gia' costruita. */
	UFUNCTION(BlueprintCallable, Category = "Door|Appearance")
	void SetStoneMaterial(UMaterialInterface* NewMaterial);

	/** True durante l'apertura e finche' ci sono pawn nell'area di attivazione. */
	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsDoorOpen() const { return bIsOpen; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> Root;

	/** Nuclei arretrati: collisione continua e pietra scura visibile nelle fughe. */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> LeftPost;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> RightPost;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Lintel;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> FrameStones;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> FrameBases;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> FrameCapitals;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> CorniceSteps;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Keystone;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> KeystoneEmblems;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Threshold;

	/** I perni sono sul bordo posteriore delle ante: nessun incastro nello stipite. */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<USceneComponent> RightHinge;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> Panel;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UStaticMeshComponent> RightPanel;

	/** Conci, bordi e incisioni su entrambe le facce, solidali alle ante. */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> PanelStones;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> PanelBorders;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TArray<TObjectPtr<UStaticMeshComponent>> PanelEmblems;

	UPROPERTY(VisibleAnywhere, Category = "Door")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Vano largo 12 metri per default. */
	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "200.0", Units = "cm"))
	float DoorWidth = 1200.0f;

	/** Vano alto 18 metri per default; cornice e architrave si aggiungono sopra. */
	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "300.0", Units = "cm"))
	float DoorHeight = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "25.0", Units = "cm"))
	float FrameThickness = 260.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "50.0", Units = "cm"))
	float FrameDepth = 360.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "5.0", Units = "cm"))
	float PanelThickness = 100.0f;

	/** La soglia e' bassa e attraversabile dal personaggio. */
	UPROPERTY(EditAnywhere, Category = "Door|Dimensions", meta = (ClampMin = "0.0", ClampMax = "30.0", Units = "cm"))
	float ThresholdHeight = 16.0f;

	UPROPERTY(EditAnywhere, Category = "Door|Animation", meta = (ClampMin = "0.0", ClampMax = "90.0", Units = "deg"))
	float OpenAngle = 90.0f;

	/** Velocita' costante in gradi al secondo: apertura completa in circa 3 s. */
	UPROPERTY(EditAnywhere, Category = "Door|Animation", meta = (ClampMin = "1.0", Units = "deg/s"))
	float OpenSpeed = 30.0f;

	/** Distanza di attivazione oltre la faccia del portale, su entrambi i lati. */
	UPROPERTY(EditAnywhere, Category = "Door|Animation", meta = (ClampMin = "100.0", Units = "cm"))
	float ActivationDistance = 900.0f;

	/** Materiale procedurale creato da Content/Python/CreateStoneMaterial.py. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Appearance")
	TObjectPtr<UMaterialInterface> StoneMaterial;

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor FrameColor = FLinearColor(0.34f, 0.33f, 0.30f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor PanelColor = FLinearColor(0.23f, 0.24f, 0.23f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor JointColor = FLinearColor(0.075f, 0.08f, 0.075f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	FLinearColor StoneDetailColor = FLinearColor(0.41f, 0.40f, 0.36f);

	UPROPERTY(EditAnywhere, Category = "Door|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StoneRoughness = 0.94f;

	/** Variazioni deterministiche fra i conci; non cambiano a ogni ricostruzione. */
	UPROPERTY(EditAnywhere, Category = "Door|Appearance")
	int32 StoneVariationSeed = 37;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInterface> DefaultMaterial;

	bool bIsOpen = false;
	float CurrentOpenAngle = 0.0f;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void RefreshOpenState();
	void SetTargetOpenState(bool bOpen);
	void ApplyOpeningAngle();
	void RefreshMaterials();
	void ApplyCubeTransform(UStaticMeshComponent* Comp, const FVector& Center, const FVector& Size) const;
	void ApplyColor(UStaticMeshComponent* Comp, const FLinearColor& Color) const;
};
