#include "BaseProjectDoor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// Le primitive di Engine sono cubi da 100 unita' centrati nell'origine.
	constexpr float BasicShapeSize = 100.0f;
}

ABaseProjectDoor::ABaseProjectDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	auto MakePart = [this, &CubeMesh, &BaseMaterial](const TCHAR* Name, USceneComponent* Parent) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Parent);
		if (CubeMesh.Succeeded())
		{
			Part->SetStaticMesh(CubeMesh.Object);
		}
		if (BaseMaterial.Succeeded())
		{
			Part->SetMaterial(0, BaseMaterial.Object);
		}
		return Part;
	};

	LeftPost = MakePart(TEXT("LeftPost"), Root);
	RightPost = MakePart(TEXT("RightPost"), Root);
	Lintel = MakePart(TEXT("Lintel"), Root);

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(Root);

	Panel = MakePart(TEXT("Panel"), Hinge);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectDoor::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ABaseProjectDoor::OnTriggerEndOverlap);
}

void ABaseProjectDoor::ApplyCubeTransform(UStaticMeshComponent* Comp, const FVector& Center, const FVector& Size) const
{
	if (!Comp)
	{
		return;
	}
	Comp->SetRelativeLocation(Center);
	Comp->SetRelativeScale3D(Size / BasicShapeSize);
}

void ABaseProjectDoor::ApplyColor(UStaticMeshComponent* Comp, const FLinearColor& Color) const
{
	if (!Comp)
	{
		return;
	}
	if (UMaterialInstanceDynamic* MID = Comp->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

void ABaseProjectDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const float HalfW = DoorWidth * 0.5f;
	const float H = DoorHeight;
	const float FT = FrameThickness;
	const float FD = FrameDepth;

	// Stipiti: ai lati del vano, dal suolo fino all'architrave.
	ApplyCubeTransform(LeftPost, FVector(0.0f, -HalfW - FT * 0.5f, H * 0.5f), FVector(FD, FT, H));
	ApplyCubeTransform(RightPost, FVector(0.0f, HalfW + FT * 0.5f, H * 0.5f), FVector(FD, FT, H));
	// Architrave sopra il vano.
	ApplyCubeTransform(Lintel, FVector(0.0f, 0.0f, H + FT * 0.5f), FVector(FD, DoorWidth + 2.0f * FT, FT));

	// Cerniera sul bordo sinistro del vano.
	Hinge->SetRelativeLocation(FVector(0.0f, -HalfW, 0.0f));

	// Anta: la posizione e' riferita alla cerniera.
	ApplyCubeTransform(Panel, FVector(0.0f, HalfW, H * 0.5f), FVector(PanelThickness, DoorWidth, H));

	// Area di attivazione davanti e dietro il vano.
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, H * 0.5f));
	TriggerBox->SetBoxExtent(FVector(FD + 150.0f, HalfW + 50.0f, H * 0.5f));

	ApplyColor(LeftPost, FrameColor);
	ApplyColor(RightPost, FrameColor);
	ApplyColor(Lintel, FrameColor);
	ApplyColor(Panel, PanelColor);

	// Applica subito lo stato corrente senza animazione.
	Hinge->SetRelativeRotation(FRotator(0.0f, bIsOpen ? -OpenAngle : 0.0f, 0.0f));
}

void ABaseProjectDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float TargetYaw = bIsOpen ? -OpenAngle : 0.0f;
	const FRotator Current = Hinge->GetRelativeRotation();
	if (!FMath::IsNearlyEqual(Current.Yaw, TargetYaw, 0.1f))
	{
		const float NewYaw = FMath::FInterpTo(Current.Yaw, TargetYaw, DeltaSeconds, OpenSpeed);
		Hinge->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));
	}
}

void ABaseProjectDoor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && Cast<APawn>(OtherActor))
	{
		++OverlappingPawns;
		bIsOpen = true;
	}
}

void ABaseProjectDoor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && Cast<APawn>(OtherActor))
	{
		OverlappingPawns = FMath::Max(0, OverlappingPawns - 1);
		if (OverlappingPawns == 0)
		{
			bIsOpen = false;
		}
	}
}
