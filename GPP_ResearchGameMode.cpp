// Copyright Epic Games, Inc. All Rights Reserved.

#include "GPP_ResearchGameMode.h"
#include "GPP_ResearchPlayerController.h"
#include "GPP_ResearchCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGPP_ResearchGameMode::AGPP_ResearchGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AGPP_ResearchPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}