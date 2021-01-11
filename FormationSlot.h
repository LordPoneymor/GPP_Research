// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FormationSlot.generated.h"

UCLASS()
class GPP_RESEARCH_API AFormationSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFormationSlot();

	float MovementSpeed;
	float Offset;
	FVector Destination;
	FVector RelativePosition;
	FRotator Orientation;

	void InvertOffset() { Offset = -Offset; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
