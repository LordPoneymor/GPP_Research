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

AGPP_ResearchPlayerController::AGPP_ResearchPlayerController()
{
	bShowMouseCursor = true;
	//DefaultMouseCursor = EMouseCursor::Crosshairs;
	UnitOffset = 150.f;

	bIsGroupInFormation = false;
	bIsGroupMoving = false;

	static ConstructorHelpers::FObjectFinder<UBlueprint>  BPAsset(TEXT("Blueprint'/Game/Blueprints/BP_FormationSlot'"));
	if (BPAsset.Succeeded())
	{
		SlotBP = (UClass*)BPAsset.Object->GeneratedClass;
	}
	CurrentFormation = EFormation::EF_None;
}

void AGPP_ResearchPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Hud = Cast<AGPP_Research_HUD>(GetHUD());
	Hud->Control = this;
	UGameplayStatics::GetAllActorsOfClass(this, AGPP_ResearchCharacter::StaticClass(), AllActors);
}

void AGPP_ResearchPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsSelected = true;
		}
	}

	if (GroupFormation != nullptr)
	{
		GroupFormation->UpdateSlots(DeltaTime);
	}
}

void AGPP_ResearchPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("LMB", IE_Pressed, this, &AGPP_ResearchPlayerController::LMBDown);
	InputComponent->BindAction("LMB", IE_Released, this, &AGPP_ResearchPlayerController::LMBUp);

	InputComponent->BindAction("RMB", IE_Released, this, &AGPP_ResearchPlayerController::MoveTo);
	InputComponent->BindAction("LineFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::GetInLine);
	InputComponent->BindAction("ProtectionCircleFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::FormProtectionCircle);
	InputComponent->BindAction("CircleFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::FormCircle);
	InputComponent->BindAction("BreakFormation", IE_Pressed, this, &AGPP_ResearchPlayerController::BreakFormation);
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
	if (SelectedActors.Num() == 0)
	{
		bIsGroupInFormation = false;
		for (AActor* actor : AllActors)
		{
			AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
			if (character)
			{
				character->bIsSelected = false;
				character->bIsInFormation = false;
				character->FollowSlot = nullptr;
			}
		}
	}
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

void AGPP_ResearchPlayerController::GetInLine()
{
	if (SelectedActors.Num() > 0 && CurrentFormation != EFormation::EF_Line)
	{
		if (GroupFormation != nullptr)
		{
			delete GroupFormation;		
		}
		GroupFormation = new Line{ SlotBP };
		GroupFormation->AssignSlots(SelectedActors);
		CurrentFormation = EFormation::EF_Line;
		bIsGroupInFormation = true;
	}
}

void AGPP_ResearchPlayerController::FormProtectionCircle()
{
	if (SelectedActors.Num() > 0 && CurrentFormation != EFormation::EF_ProtectionCircle)
	{
		if (GroupFormation != nullptr)
		{
			delete GroupFormation;
		}
		GroupFormation = new ProtectionCircle{ SlotBP };
		GroupFormation->AssignSlots(SelectedActors);
		CurrentFormation = EFormation::EF_ProtectionCircle;
		bIsGroupInFormation = true;
	}
}

void AGPP_ResearchPlayerController::FormCircle()
{
	if (SelectedActors.Num() > 0)
	{
		if (GroupFormation != nullptr)
		{
			delete GroupFormation;
		}
		GroupFormation = new Circle{ SlotBP };
			
		CurrentFormation = EFormation::EF_Circle;
		GroupFormation->AssignSlots(SelectedActors);
		bIsGroupInFormation = true;
	}
}

void AGPP_ResearchPlayerController::BreakFormation()
{
	bIsGroupInFormation = false;

	for (AActor* actor : SelectedActors)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);
		if (character)
		{
			character->bIsInFormation = false;
			character->FollowSlot = nullptr;
		}
	}

	delete GroupFormation;
	GroupFormation = nullptr;
}