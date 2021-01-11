// Fill out your copyright notice in the Description page of Project Settings.
#include "FormationSlot.h"
#include "GameFramework/NavMovementComponent.h"

// Sets default values
AFormationSlot::AFormationSlot()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFormationSlot::BeginPlay()
{
	Super::BeginPlay();
}

void AFormationSlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}