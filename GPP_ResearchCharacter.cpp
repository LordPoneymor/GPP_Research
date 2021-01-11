// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPP_ResearchCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "AIController.h"

AGPP_ResearchCharacter::AGPP_ResearchCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a decal in the world to show the cursor's location
	SelectedDecal = CreateDefaultSubobject<UDecalComponent>("SelectedDecal");
	SelectedDecal->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		SelectedDecal->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	SelectedDecal->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	SelectedDecal->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	SelectedDecal->SetRelativeLocation(FVector(0,0,-90));
	SelectedDecal->SetHiddenInGame(true);

	TargetDecal = CreateDefaultSubobject<UDecalComponent>("TargetDecal");
	TargetDecal->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> SecondDecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/MI_Decal'"));
	if (SecondDecalMaterialAsset.Succeeded())
	{
		TargetDecal->SetDecalMaterial(SecondDecalMaterialAsset.Object);
	}
	TargetDecal->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	TargetDecal->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
	TargetDecal->SetRelativeLocation(FVector(0, 0, -90));
	TargetDecal->SetHiddenInGame(true);

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bIsSelected = false;
}

void AGPP_ResearchCharacter::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());
	//GetCharacterMovement()->SetAvoidanceEnabled(true);
}

void AGPP_ResearchCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (bIsSelected)
	{
		SelectedDecal->SetHiddenInGame(false);
	}
	else
	{
		SelectedDecal->SetHiddenInGame(true);
	}

	if (!GetMovementComponent()->Velocity.IsNearlyZero())
	{
		if (FollowSlot == nullptr)
		{
			TargetDecal->SetWorldLocation(Destination);
		}
		else
		{
			TargetDecal->SetWorldLocation(FollowSlot->Destination);
		}
		TargetDecal->SetHiddenInGame(false);
	}
	else
	{
		TargetDecal->SetHiddenInGame(true);
	}

	if (bIsInFormation)
	{
		if (FollowSlot)
		{
			SetActorRotation(FollowSlot->GetActorRotation());
			MoveTo(FollowSlot->GetActorLocation());
		}

	}
}

void AGPP_ResearchCharacter::MoveToDestination()
{
	MoveTo(Destination);
}

void AGPP_ResearchCharacter::MoveTo(const FVector& destination)
{
	FAIMoveRequest moveRequest;
	moveRequest.SetGoalLocation(destination);
	moveRequest.SetAcceptanceRadius(5.f);

	FNavPathSharedPtr navPath;
	AIController->MoveTo(moveRequest, &navPath);
}
