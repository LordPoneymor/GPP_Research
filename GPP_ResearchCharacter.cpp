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
		TargetDecal->SetWorldLocation(Destination);
		TargetDecal->SetHiddenInGame(false);
	}
	else
	{
		TargetDecal->SetHiddenInGame(true);
	}

	if (bIsInFormation)
	{
		SetActorRotation(FollowSlot->Orientation);
		if (bIsMoving)
		{
			MoveTo(FollowSlot->GetActorLocation());
		}
	}

	if ((GetActorLocation() - FollowSlot->GetActorLocation()).Size() <= 5)
	{
		bIsMoving = false;
	}



	//if (SelectedDecal != nullptr)
	//{
	//	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	//	{
	//		if (UWorld* World = GetWorld())
	//		{
	//			FHitResult HitResult;
	//			FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
	//			FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
	//			FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
	//			Params.AddIgnoredActor(this);
	//			World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
	//			FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
	//			SelectedDecal->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
	//		}
	//	}
	//	else if (APlayerController* PC = Cast<APlayerController>(GetController()))
	//	{
	//		FHitResult TraceHitResult;
	//		PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
	//		FVector CursorFV = TraceHitResult.ImpactNormal;
	//		FRotator CursorR = CursorFV.Rotation();
	//		SelectedDecal->SetWorldLocation(TraceHitResult.Location);
	//		SelectedDecal->SetWorldRotation(CursorR);
	//	}
	//}
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
