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
	constexpr int32 FrameCourseCount = 5;
	constexpr int32 LintelBlockCount = 5;
	constexpr int32 PanelStoneColumns = 3;
	constexpr int32 PanelStoneRows = 4;
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

	// Sottili fasce scure incidono i corsi dei due piedritti e i blocchi
	// dell'architrave, simulando le fughe tra le grandi pietre.
	for (int32 Course = 1; Course < FrameCourseCount; ++Course)
	{
		for (int32 Side = 0; Side < 2; ++Side)
		{
			const FString Name = FString::Printf(TEXT("FrameJoint_%d_%d"), Course, Side);
			FrameJoints.Add(MakePart(*Name, Root));
		}
	}
	for (int32 Block = 1; Block < LintelBlockCount; ++Block)
	{
		const FString Name = FString::Printf(TEXT("LintelJoint_%d"), Block);
		FrameJoints.Add(MakePart(*Name, Root));
	}
	Keystone = MakePart(TEXT("Keystone"), Root);

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(Root);

	Panel = MakePart(TEXT("Panel"), Hinge);
	for (int32 Side = 0; Side < 2; ++Side)
	{
		for (int32 Row = 0; Row < PanelStoneRows; ++Row)
		{
			for (int32 Column = 0; Column < PanelStoneColumns; ++Column)
			{
				const FString Name = FString::Printf(TEXT("PanelStone_%d_%d_%d"), Side, Row, Column);
				PanelStones.Add(MakePart(*Name, Hinge));
			}
		}
	}

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

	// Stipiti: massicci piedritti dal suolo fino all'architrave.
	ApplyCubeTransform(LeftPost, FVector(0.0f, -HalfW - FT * 0.5f, H * 0.5f), FVector(FD, FT, H));
	ApplyCubeTransform(RightPost, FVector(0.0f, HalfW + FT * 0.5f, H * 0.5f), FVector(FD, FT, H));
	// Architrave sopra il vano.
	const float LintelWidth = DoorWidth + 2.0f * FT;
	ApplyCubeTransform(Lintel, FVector(0.0f, 0.0f, H + FT * 0.5f), FVector(FD, LintelWidth, FT));

	// Le fasce scure segnano i singoli blocchi degli stipiti e dell'architrave.
	int32 JointIndex = 0;
	const float CourseHeight = H / static_cast<float>(FrameCourseCount);
	const float JointBand = FMath::Min(14.0f, FMath::Min(FT * 0.08f, CourseHeight * 0.08f));
	for (int32 Course = 1; Course < FrameCourseCount; ++Course)
	{
		const float JointZ = CourseHeight * Course;
		for (int32 Side = 0; Side < 2; ++Side)
		{
			const float Sign = (Side == 0) ? -1.0f : 1.0f;
			const float PostY = Sign * (HalfW + FT * 0.5f);
			ApplyCubeTransform(FrameJoints[JointIndex++].Get(), FVector(0.0f, PostY, JointZ),
				FVector(FD + 8.0f, FT + 8.0f, JointBand));
		}
	}
	const float LintelJointWidth = FMath::Min(14.0f, LintelWidth / (LintelBlockCount * 2.0f));
	const float LintelOuterHalfWidth = LintelWidth * 0.5f;
	for (int32 Block = 1; Block < LintelBlockCount; ++Block)
	{
		const float JointY = -LintelOuterHalfWidth + LintelWidth * Block / LintelBlockCount;
		ApplyCubeTransform(FrameJoints[JointIndex++].Get(), FVector(0.0f, JointY, H + FT * 0.5f),
			FVector(FD + 8.0f, LintelJointWidth, FT + 8.0f));
	}

	// Una chiave di volta leggermente sporgente chiude il portale monumentale.
	const float KeystoneWidth = FMath::Min(FT * 1.2f, DoorWidth * 0.45f);
	ApplyCubeTransform(Keystone, FVector(0.0f, 0.0f, H + FT * 0.5f),
		FVector(FD + 16.0f, KeystoneWidth, FT * 1.2f));

	// Cerniera sul bordo sinistro del vano.
	Hinge->SetRelativeLocation(FVector(0.0f, -HalfW, 0.0f));

	// Anta massiccia: la posizione e' riferita alla cerniera.
	ApplyCubeTransform(Panel, FVector(0.0f, HalfW, H * 0.5f), FVector(PanelThickness, DoorWidth, H));

	// Blocchi in rilievo su entrambi i lati della lastra; ruotano con l'anta.
	const float StoneGap = FMath::Min(24.0f,
		FMath::Min(DoorWidth / (PanelStoneColumns * 2.0f), H / (PanelStoneRows * 2.0f)));
	const float StoneWidth = (DoorWidth - StoneGap * (PanelStoneColumns + 1)) / PanelStoneColumns;
	const float StoneHeight = (H - StoneGap * (PanelStoneRows + 1)) / PanelStoneRows;
	const float ReliefDepth = FMath::Clamp(PanelThickness * 0.12f, 6.0f, 14.0f);
	const float ReliefOffset = PanelThickness * 0.5f + ReliefDepth * 0.15f;
	int32 StoneIndex = 0;
	for (int32 Side = 0; Side < 2; ++Side)
	{
		const float Sign = (Side == 0) ? -1.0f : 1.0f;
		for (int32 Row = 0; Row < PanelStoneRows; ++Row)
		{
			const float StoneZ = StoneGap + StoneHeight * 0.5f + Row * (StoneHeight + StoneGap);
			for (int32 Column = 0; Column < PanelStoneColumns; ++Column)
			{
				const float StoneY = StoneGap + StoneWidth * 0.5f + Column * (StoneWidth + StoneGap);
				ApplyCubeTransform(PanelStones[StoneIndex].Get(), FVector(Sign * ReliefOffset, StoneY, StoneZ),
					FVector(ReliefDepth, StoneWidth, StoneHeight));

				const float Tone = ((Row + Column) % 2 == 0) ? 1.0f : 0.84f;
				FLinearColor StoneColor = StoneDetailColor * Tone;
				StoneColor.A = 1.0f;
				ApplyColor(PanelStones[StoneIndex].Get(), StoneColor);
				++StoneIndex;
			}
		}
	}

	// Area di attivazione davanti e dietro il vano.
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, H * 0.5f));
	TriggerBox->SetBoxExtent(FVector(FD + 150.0f, HalfW + 50.0f, H * 0.5f));

	ApplyColor(LeftPost, FrameColor);
	ApplyColor(RightPost, FrameColor);
	ApplyColor(Lintel, FrameColor);
	for (const TObjectPtr<UStaticMeshComponent>& Joint : FrameJoints)
	{
		ApplyColor(Joint.Get(), JointColor);
	}
	ApplyColor(Keystone, StoneDetailColor);
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
