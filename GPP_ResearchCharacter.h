// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GPP_ResearchCharacter.generated.h"

UCLASS(Blueprintable)
class AGPP_ResearchCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGPP_ResearchCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns SelectedDecal subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return SelectedDecal; }

	bool bIsSelected;
private:

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* SelectedDecal;
};

