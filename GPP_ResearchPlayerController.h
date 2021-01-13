// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Formation.h"
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

	TArray<AActor*> AllActors;

	
	class AGPP_Research_HUD* Hud;

	EFormation CurrentFormation;
	TSubclassOf<class AFormationSlot> SlotBP;
	Formation* GroupFormation;

	bool bIsLMBDown;
	bool bIsGroupInFormation;
	bool bIsGroupMoving;
protected:
	virtual void BeginPlay() override;

	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	float UnitOffset;

	void LMBDown();
	void LMBUp();
	void MoveTo();
	void GetInLine();
	void FormProtectionCircle();
	void FormCircle();
	void SplitFormation();
	void BreakFormation();
};


