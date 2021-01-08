// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPP_ResearchPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GPP_ResearchCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "CameraPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GPP_Research_HUD.h"
#include "GPP_ResearchCharacter.h"
#include "AiTypes.h"
#include "Formation.h"

AGPP_ResearchPlayerController::AGPP_ResearchPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	UnitOffset = 150.f;

	bIsGroupInFormation = false;
	bIsGroupMoving = false;
	GroupFormation = new Line{};
}

void AGPP_ResearchPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Hud = Cast<AGPP_Research_HUD>(GetHUD());
	Hud->Control = this;

	UGameplayStatics::GetAllActorsOfClass(this, CharacterAsset, AllActors);
}

void AGPP_ResearchPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	for (AActor* actor : AllActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsSelected = false;
		}
	}

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsSelected = true;
		}
	}
}

void AGPP_ResearchPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("LMB", IE_Pressed, this, &AGPP_ResearchPlayerController::LMBDown);
	InputComponent->BindAction("LMB", IE_Released, this, &AGPP_ResearchPlayerController::LMBUp);

	InputComponent->BindAction("RMB", IE_Released, this, &AGPP_ResearchPlayerController::MoveTo);
	InputComponent->BindAction("GetIntoFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::GetInFormation);
	InputComponent->BindAction("BreakFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::BreakFormation);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AGPP_ResearchPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AGPP_ResearchPlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AGPP_ResearchPlayerController::OnResetVR);
}

void AGPP_ResearchPlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AGPP_ResearchPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (AGPP_ResearchCharacter* MyPawn = Cast<AGPP_ResearchCharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void AGPP_ResearchPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AGPP_ResearchPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		// We need to issue move command only if far enough in order for walk animation to play correctly
		if ((Distance > 120.0f))
		{
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
		}
	}
}

void AGPP_ResearchPlayerController::OnSetDestinationPressed()
{
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AGPP_ResearchPlayerController::OnSetDestinationReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void AGPP_ResearchPlayerController::LMBDown()
{
	float x;
	float y;
	GetMousePosition(x, y);

	Hud->ClickPosition = FVector2D(x,y);
	bIsLMBDown = true;
	Hud->bIsLMBDown = bIsLMBDown;

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsSelected = false;
		}
	}

	SelectedActors.Empty();
}

void AGPP_ResearchPlayerController::LMBUp()
{
	bIsLMBDown = false;
	Hud->bIsLMBDown = bIsLMBDown;
}

void AGPP_ResearchPlayerController::MoveTo()
{
	FHitResult hit;
	GetHitResultUnderCursor(ECC_Visibility, true, hit);

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsMoving = true;
		}
	}

	if (bIsGroupInFormation)
	{
		GroupFormation->MoveToDestination(hit.Location);
	}
	else
	{
		for (AActor* actor : SelectedActors)
		{
			AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
			if (character)
			{
				character->MoveTo(hit.Location);
			}
		}
	}

}

void AGPP_ResearchPlayerController::GetInFormation()
{
	GroupFormation->AssignSlots(SelectedActors);
	bIsGroupInFormation = true;
}

void AGPP_ResearchPlayerController::BreakFormation()
{
	bIsGroupInFormation = false;

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		character->bIsInFormation = false;
	}
}
