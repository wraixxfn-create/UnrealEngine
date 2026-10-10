#include "BaseProjectMapBase.h"

#include "BaseProjectDoor.h"
#include "Components/ChildActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float BasicShapeSize = 100.0f;
}

ABaseProjectMapBase::ABaseProjectMapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));
	Ground->SetupAttachment(Root);
	if (CubeMesh.Succeeded())
	{
		Ground->SetStaticMesh(CubeMesh.Object);
	}
	if (BaseMaterial.Succeeded())
	{
		Ground->SetMaterial(0, BaseMaterial.Object);
	}
	Ground->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Ground->SetCollisionProfileName(TEXT("BlockAll"));

	DoorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Door"));
	DoorComponent->SetupAttachment(Root);
	DoorComponent->SetChildActorClass(ABaseProjectDoor::StaticClass());
}

void ABaseProjectMapBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	const float SizeCm = MapSizeMeters * 100.0f;

	// Lastra: centrata in XY, con la superficie superiore a Z = 0.
	Ground->SetRelativeLocation(FVector(0.0f, 0.0f, -GroundThickness * 0.5f));
	Ground->SetRelativeScale3D(FVector(SizeCm, SizeCm, GroundThickness) / BasicShapeSize);

	GroundMID = Ground->CreateAndSetMaterialInstanceDynamic(0);
	if (GroundMID)
	{
		GroundMID->SetVectorParameterValue(TEXT("Color"), GroundColor);
	}

	DoorComponent->SetRelativeLocation(DoorLocation);
	DoorComponent->SetRelativeRotation(DoorRotation);
}
