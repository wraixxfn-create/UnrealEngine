#include "BaseProjectDoor.h"

#include "StoneDoorLayout.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float BasicShapeSize = 100.0f;
	using BaseProjectStoneDoor::FrameCourseCount;
	using BaseProjectStoneDoor::LintelBlockCount;
	using BaseProjectStoneDoor::PanelStoneColumns;
	using BaseProjectStoneDoor::PanelStoneRows;
	using BaseProjectStoneDoor::LeafCount;
	using BaseProjectStoneDoor::FaceCount;
}

ABaseProjectDoor::ABaseProjectDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BaseMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	DefaultMaterial = BaseMaterial.Object;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Root);

	auto MakePart = [this](FName Name, USceneComponent* Parent, bool bBlocking = true) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Part->SetupAttachment(Parent);
		Part->SetMobility(EComponentMobility::Movable);
		Part->SetCollisionProfileName(bBlocking ? TEXT("BlockAllDynamic") : TEXT("NoCollision"));
		Part->SetGenerateOverlapEvents(false);
		if (!bBlocking)
		{
			Part->SetCanEverAffectNavigation(false);
		}
		if (CubeMesh.Succeeded())
		{
			Part->SetStaticMesh(CubeMesh.Object);
		}
		if (DefaultMaterial)
		{
			Part->SetMaterial(0, DefaultMaterial);
		}
		return Part;
	};

	LeftPost = MakePart(TEXT("LeftPost"), Root);
	RightPost = MakePart(TEXT("RightPost"), Root);
	Lintel = MakePart(TEXT("Lintel"), Root);
	for (int32 Course = 0; Course < FrameCourseCount; ++Course)
	{
		for (int32 Side = 0; Side < LeafCount; ++Side)
		{
			FrameStones.Add(MakePart(FName(*FString::Printf(TEXT("PostStone_%d_%d"), Course, Side)), Root));
		}
	}
	for (int32 Block = 0; Block < LintelBlockCount; ++Block)
	{
		if (Block != LintelBlockCount / 2)
		{
			FrameStones.Add(MakePart(FName(*FString::Printf(TEXT("LintelStone_%d"), Block)), Root));
		}
	}
	for (int32 Side = 0; Side < LeafCount; ++Side)
	{
		FrameBases.Add(MakePart(FName(*FString::Printf(TEXT("FrameBase_%d"), Side)), Root));
		FrameCapitals.Add(MakePart(FName(*FString::Printf(TEXT("FrameCapital_%d"), Side)), Root));
	}
	for (int32 Step = 0; Step < 2; ++Step)
	{
		CorniceSteps.Add(MakePart(FName(*FString::Printf(TEXT("Cornice_%d"), Step)), Root));
	}
	Keystone = MakePart(TEXT("Keystone"), Root);
	for (int32 Face = 0; Face < FaceCount; ++Face)
	{
		KeystoneEmblems.Add(MakePart(FName(*FString::Printf(TEXT("KeystoneEmblem_%d"), Face)), Root, false));
	}
	Threshold = MakePart(TEXT("Threshold"), Root);

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetupAttachment(Root);
	Hinge->SetMobility(EComponentMobility::Movable);
	RightHinge = CreateDefaultSubobject<USceneComponent>(TEXT("RightHinge"));
	RightHinge->SetupAttachment(Root);
	RightHinge->SetMobility(EComponentMobility::Movable);

	Panel = MakePart(TEXT("Panel"), Hinge);
	RightPanel = MakePart(TEXT("RightPanel"), RightHinge);
	Panel->SetCanEverAffectNavigation(false);
	RightPanel->SetCanEverAffectNavigation(false);
	for (int32 Leaf = 0; Leaf < LeafCount; ++Leaf)
	{
		USceneComponent* Parent = Leaf == 0 ? Hinge.Get() : RightHinge.Get();
		for (int32 Face = 0; Face < FaceCount; ++Face)
		{
			for (int32 Row = 0; Row < PanelStoneRows; ++Row)
			{
				for (int32 Column = 0; Column < PanelStoneColumns; ++Column)
				{
					PanelStones.Add(MakePart(FName(*FString::Printf(TEXT("PanelStone_%d_%d_%d_%d"), Leaf, Face, Row, Column)), Parent, false));
				}
			}
			for (int32 Border = 0; Border < 4; ++Border)
			{
				PanelBorders.Add(MakePart(FName(*FString::Printf(TEXT("PanelBorder_%d_%d_%d"), Leaf, Face, Border)), Parent, false));
			}
			PanelEmblems.Add(MakePart(FName(*FString::Printf(TEXT("PanelEmblem_%d_%d"), Leaf, Face)), Parent, false));
		}
	}

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCanEverAffectNavigation(false);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectDoor::OnTriggerBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ABaseProjectDoor::OnTriggerEndOverlap);
}

void ABaseProjectDoor::ApplyCubeTransform(UStaticMeshComponent* Comp, const FVector& Center, const FVector& Size) const
{
	if (Comp)
	{
		Comp->SetRelativeLocation(Center);
		Comp->SetRelativeScale3D(Size / BasicShapeSize);
	}
}

void ABaseProjectDoor::ApplyColor(UStaticMeshComponent* Comp, const FLinearColor& Color) const
{
	UMaterialInterface* Material = StoneMaterial ? StoneMaterial.Get() : DefaultMaterial.Get();
	if (!Comp || !Material)
	{
		return;
	}

	// Riutilizza le istanze quando l'Editor riesegue la costruzione.
	UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Comp->GetMaterial(0));
	if (!MID || MID->Parent != Material)
	{
		MID = Comp->CreateDynamicMaterialInstance(0, Material);
	}
	if (MID)
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetScalarParameterValue(TEXT("Roughness"), FMath::Clamp(StoneRoughness, 0.0f, 1.0f));
	}
}

void ABaseProjectDoor::SetStoneMaterial(UMaterialInterface* NewMaterial)
{
	StoneMaterial = NewMaterial;
	RefreshMaterials();
}

void ABaseProjectDoor::RefreshMaterials()
{
	ApplyColor(LeftPost, JointColor);
	ApplyColor(RightPost, JointColor);
	ApplyColor(Lintel, JointColor);
	ApplyColor(Panel, PanelColor);
	ApplyColor(RightPanel, PanelColor);

	FRandomStream Variation(StoneVariationSeed);
	auto VariedColor = [&Variation](const FLinearColor& Color) -> FLinearColor
	{
		FLinearColor Result = Color * Variation.FRandRange(0.86f, 1.08f);
		Result.A = 1.0f;
		return Result;
	};
	for (const TObjectPtr<UStaticMeshComponent>& Stone : FrameStones)
	{
		ApplyColor(Stone.Get(), VariedColor(FrameColor));
	}
	for (const TObjectPtr<UStaticMeshComponent>& Stone : PanelStones)
	{
		ApplyColor(Stone.Get(), VariedColor(StoneDetailColor * 0.80f));
	}
	for (const TObjectPtr<UStaticMeshComponent>& Base : FrameBases)
	{
		ApplyColor(Base.Get(), FrameColor);
	}
	for (const TObjectPtr<UStaticMeshComponent>& Capital : FrameCapitals)
	{
		ApplyColor(Capital.Get(), StoneDetailColor);
	}
	for (const TObjectPtr<UStaticMeshComponent>& Step : CorniceSteps)
	{
		ApplyColor(Step.Get(), FrameColor);
	}
	for (const TObjectPtr<UStaticMeshComponent>& Border : PanelBorders)
	{
		ApplyColor(Border.Get(), StoneDetailColor);
	}
	for (const TObjectPtr<UStaticMeshComponent>& Emblem : PanelEmblems)
	{
		ApplyColor(Emblem.Get(), StoneDetailColor);
	}
	for (const TObjectPtr<UStaticMeshComponent>& Emblem : KeystoneEmblems)
	{
		ApplyColor(Emblem.Get(), StoneDetailColor);
	}
	ApplyColor(Keystone, StoneDetailColor);
	ApplyColor(Threshold, FrameColor);
}

void ABaseProjectDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// I limiti valgono anche per valori impostati da codice, non solo dai dettagli.
	const BaseProjectStoneDoor::FLayout Layout = BaseProjectStoneDoor::MakeLayout(
		DoorWidth, DoorHeight, FrameThickness, FrameDepth, PanelThickness, ThresholdHeight, ActivationDistance);
	const float W = Layout.Width;
	const float H = Layout.Height;
	const float HalfW = Layout.HalfWidth;
	const float FT = Layout.FrameThickness;
	const float FD = Layout.FrameDepth;
	const float PT = Layout.PanelThickness;
	const float Gap = Layout.JointGap;
	const float CourseHeight = Layout.CourseHeight;
	const float LintelWidth = Layout.LintelWidth;

	ApplyCubeTransform(LeftPost, FVector(0.0f, -HalfW - FT * 0.5f, H * 0.5f), FVector(FD * 0.92f, FT * 0.92f, H));
	ApplyCubeTransform(RightPost, FVector(0.0f, HalfW + FT * 0.5f, H * 0.5f), FVector(FD * 0.92f, FT * 0.92f, H));
	ApplyCubeTransform(Lintel, FVector(0.0f, 0.0f, H + FT * 0.5f), FVector(FD * 0.92f, LintelWidth, FT * 0.92f));

	int32 StoneIndex = 0;
	for (int32 Course = 0; Course < FrameCourseCount; ++Course)
	{
		for (int32 Side = 0; Side < LeafCount; ++Side)
		{
			const float Sign = Side == 0 ? -1.0f : 1.0f;
			ApplyCubeTransform(FrameStones[StoneIndex++].Get(),
				FVector(0.0f, Sign * (HalfW + FT * 0.5f), (Course + 0.5f) * CourseHeight),
				FVector(FD, FT, CourseHeight - Gap));
		}
	}
	const float LintelStoneWidth = LintelWidth / LintelBlockCount;
	for (int32 Block = 0; Block < LintelBlockCount; ++Block)
	{
		if (Block != LintelBlockCount / 2)
		{
			ApplyCubeTransform(FrameStones[StoneIndex++].Get(),
				FVector(0.0f, -LintelWidth * 0.5f + (Block + 0.5f) * LintelStoneWidth, H + FT * 0.5f),
				FVector(FD, LintelStoneWidth - Gap, FT - Gap));
		}
	}
	ApplyCubeTransform(Keystone, FVector(0.0f, 0.0f, H + FT * 0.5f), FVector(FD * 1.10f, LintelStoneWidth - Gap, FT - Gap));
	for (int32 Face = 0; Face < FaceCount; ++Face)
	{
		const float Sign = Face == 0 ? -1.0f : 1.0f;
		ApplyCubeTransform(KeystoneEmblems[Face].Get(), FVector(Sign * FD * 0.56f, 0.0f, H + FT * 0.5f),
			FVector(FD * 0.035f, FT * 0.42f, FT * 0.42f));
		KeystoneEmblems[Face]->SetRelativeRotation(FRotator(0.0f, 0.0f, 45.0f));
	}

	// Basi e capitelli si allargano solo verso l'esterno, lasciando libero il vano.
	for (int32 Side = 0; Side < LeafCount; ++Side)
	{
		const float Sign = Side == 0 ? -1.0f : 1.0f;
		const float BaseWidth = FT * 1.30f;
		const float BaseHeight = FT * 0.40f;
		ApplyCubeTransform(FrameBases[Side].Get(), FVector(0.0f, Sign * (HalfW + BaseWidth * 0.5f), BaseHeight * 0.5f),
			FVector(FD * 1.30f, BaseWidth, BaseHeight));
		const float CapitalWidth = FT * 1.18f;
		ApplyCubeTransform(FrameCapitals[Side].Get(), FVector(0.0f, Sign * (HalfW + CapitalWidth * 0.5f), H - FT * 0.12f),
			FVector(FD * 1.20f, CapitalWidth, FT * 0.24f));
	}
	const float CorniceHeight = FT * 0.25f;
	for (int32 Step = 0; Step < CorniceSteps.Num(); ++Step)
	{
		const float Overhang = FT * (Step == 0 ? 0.20f : 0.35f);
		ApplyCubeTransform(CorniceSteps[Step].Get(), FVector(0.0f, 0.0f, H + FT + (Step + 0.5f) * CorniceHeight),
			FVector(FD + Overhang, LintelWidth + Overhang * 2.0f, CorniceHeight - Gap * 0.5f));
	}

	const float SillHeight = Layout.SillHeight;
	Threshold->SetVisibility(SillHeight > 0.0f);
	Threshold->SetCollisionEnabled(SillHeight > 0.0f ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	ApplyCubeTransform(Threshold, FVector(0.0f, 0.0f, SillHeight * 0.5f), FVector(FD * 0.85f, W, FMath::Max(0.1f, SillHeight)));

	const float EdgeGap = Layout.EdgeGap;
	const float Bottom = Layout.Bottom;
	const float LeafHeight = Layout.LeafHeight;
	const float LeafWidth = Layout.LeafWidth;
	const float ReliefDepth = Layout.ReliefDepth;
	const float BorderDepth = Layout.BorderDepth;
	const float BorderWidth = Layout.BorderWidth;
	const float HingeDepth = Layout.HingeDepth;
	// La fuga centrale e i perni arretrati tengono conto dello spessore delle ante.
	Hinge->SetRelativeLocation(FVector(HingeDepth, -HalfW + EdgeGap, 0.0f));
	RightHinge->SetRelativeLocation(FVector(HingeDepth, HalfW - EdgeGap, 0.0f));

	const float StoneGap = Layout.StoneGap;
	const float StoneWidth = Layout.StoneWidth;
	const float StoneHeight = Layout.StoneHeight;
	int32 PanelStoneIndex = 0;
	int32 BorderIndex = 0;
	int32 EmblemIndex = 0;
	for (int32 Leaf = 0; Leaf < LeafCount; ++Leaf)
	{
		const float LeafSign = Leaf == 0 ? 1.0f : -1.0f;
		UStaticMeshComponent* LeafPanel = Leaf == 0 ? Panel.Get() : RightPanel.Get();
		ApplyCubeTransform(LeafPanel, FVector(0.0f, LeafSign * LeafWidth * 0.5f, Bottom + LeafHeight * 0.5f),
			FVector(PT, LeafWidth, LeafHeight));
		for (int32 Face = 0; Face < FaceCount; ++Face)
		{
			const float FaceSign = Face == 0 ? -1.0f : 1.0f;
			const float SurfaceX = FaceSign * (PT * 0.5f + ReliefDepth * 0.40f);
			for (int32 Row = 0; Row < PanelStoneRows; ++Row)
			{
				for (int32 Column = 0; Column < PanelStoneColumns; ++Column)
				{
					const float Y = BorderWidth + StoneGap + StoneWidth * 0.5f + Column * (StoneWidth + StoneGap);
					const float Z = Bottom + BorderWidth + StoneGap + StoneHeight * 0.5f + Row * (StoneHeight + StoneGap);
					ApplyCubeTransform(PanelStones[PanelStoneIndex++].Get(), FVector(SurfaceX, LeafSign * Y, Z),
						FVector(ReliefDepth, StoneWidth, StoneHeight));
				}
			}

			const float BorderX = FaceSign * (PT * 0.5f + BorderDepth * 0.40f);
			for (int32 Border = 0; Border < 4; ++Border)
			{
				const bool bVertical = Border < 2;
				const float Y = bVertical ? (Border == 0 ? BorderWidth * 0.5f : LeafWidth - BorderWidth * 0.5f) : LeafWidth * 0.5f;
				const float Z = bVertical ? Bottom + LeafHeight * 0.5f : Bottom + (Border == 2 ? BorderWidth * 0.5f : LeafHeight - BorderWidth * 0.5f);
				ApplyCubeTransform(PanelBorders[BorderIndex++].Get(), FVector(BorderX, LeafSign * Y, Z),
					FVector(BorderDepth, bVertical ? BorderWidth : LeafWidth, bVertical ? LeafHeight - BorderWidth * 2.0f : BorderWidth));
			}
			const float EmblemSize = FMath::Min(LeafWidth * 0.24f, LeafHeight * 0.16f);
			ApplyCubeTransform(PanelEmblems[EmblemIndex].Get(),
				FVector(FaceSign * (PT * 0.5f + BorderDepth * 1.25f), LeafSign * LeafWidth * 0.5f, Bottom + LeafHeight * 0.55f),
				FVector(BorderDepth, EmblemSize, EmblemSize));
			PanelEmblems[EmblemIndex++]->SetRelativeRotation(FRotator(0.0f, 0.0f, 45.0f));
		}
	}

	// Comprende l'intero arco delle ante: non richiude addosso a un pawn nel vano.
	const float TriggerDepth = Layout.TriggerDepth;
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, H * 0.5f));
	TriggerBox->SetBoxExtent(FVector(TriggerDepth, Layout.TriggerHalfWidth, H * 0.5f));

	RefreshMaterials();
	CurrentOpenAngle = FMath::Clamp(CurrentOpenAngle, 0.0f, FMath::Clamp(OpenAngle, 0.0f, 90.0f));
	ApplyOpeningAngle();
	SetTargetOpenState(bIsOpen);
}

void ABaseProjectDoor::BeginPlay()
{
	Super::BeginPlay();
	// Include anche pawn gia' presenti nel trigger all'avvio del livello.
	TriggerBox->UpdateOverlaps();
	RefreshOpenState();
}

void ABaseProjectDoor::ApplyOpeningAngle()
{
	Hinge->SetRelativeRotation(FRotator(0.0f, -CurrentOpenAngle, 0.0f));
	RightHinge->SetRelativeRotation(FRotator(0.0f, CurrentOpenAngle, 0.0f));
}

void ABaseProjectDoor::SetTargetOpenState(bool bOpen)
{
	bIsOpen = bOpen;
	const float Target = bIsOpen ? FMath::Clamp(OpenAngle, 0.0f, 90.0f) : 0.0f;
	SetActorTickEnabled(!FMath::IsNearlyEqual(CurrentOpenAngle, Target, 0.01f));
}

void ABaseProjectDoor::RefreshOpenState()
{
	// Conteggia attori, non componenti: un pawn con piu' collider non causa una
	// chiusura anticipata. Anche un secondo pawn mantiene aperta la porta.
	TArray<AActor*> Pawns;
	TriggerBox->GetOverlappingActors(Pawns, APawn::StaticClass());
	bool bHasPawn = false;
	for (AActor* Pawn : Pawns)
	{
		if (IsValid(Pawn))
		{
			bHasPawn = true;
			break;
		}
	}
	SetTargetOpenState(bHasPawn);
}

void ABaseProjectDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float Target = bIsOpen ? FMath::Clamp(OpenAngle, 0.0f, 90.0f) : 0.0f;
	CurrentOpenAngle = FMath::FInterpConstantTo(CurrentOpenAngle, Target, DeltaSeconds, FMath::Max(1.0f, OpenSpeed));
	ApplyOpeningAngle();
	if (FMath::IsNearlyEqual(CurrentOpenAngle, Target, 0.01f))
	{
		CurrentOpenAngle = Target;
		ApplyOpeningAngle();
		SetActorTickEnabled(false);
	}
}

void ABaseProjectDoor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsValid(OtherActor) && Cast<APawn>(OtherActor))
	{
		SetTargetOpenState(true);
	}
}

void ABaseProjectDoor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && Cast<APawn>(OtherActor))
	{
		RefreshOpenState();
	}
}
