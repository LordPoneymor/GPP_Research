// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GPP_Research_HUD.generated.h"

/**
 * 
 */
UCLASS()
class GPP_RESEARCH_API AGPP_Research_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	bool bIsLMBDown;

	UPROPERTY(BlueprintReadWrite)
	FVector2D ClickPosition;

	UPROPERTY(BlueprintReadWrite)
	FVector2D EndPosition;

	UPROPERTY(BlueprintReadWrite)
	class AGPP_ResearchPlayerController* Control;

	UPROPERTY(BlueprintReadWrite)
	TArray<class AGPP_ResearchCharacter*> AllCharacters;

	UFUNCTION(BlueprintImplementableEvent)
	void Select();
};
