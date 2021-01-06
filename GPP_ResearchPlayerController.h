// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GPP_ResearchPlayerController.generated.h"

UCLASS()
class AGPP_ResearchPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGPP_ResearchPlayerController();

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> SelectedActors;

	class AGPP_Research_HUD* Hud;

	bool bIsLMBDown;

protected:
	virtual void BeginPlay() override;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Navigate player to the current mouse cursor location. */
	void MoveToMouseCursor();

	/** Navigate player to the current touch location. */
	void MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Navigate player to the given world location. */
	void SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void OnSetDestinationPressed();
	void OnSetDestinationReleased();

private:

	void LMBDown();
	void LMBUp();
};


