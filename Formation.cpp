// Fill out your copyright notice in the Description page of Project Settings.
#include "Formation.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GPP_ResearchCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
Formation::Formation()
{
}

Formation::~Formation()
{
	Leader = nullptr;

	for (AFormationSlot* slot : Slots)
	{
		delete slot;
	}
	Slots.Empty();
}

void Formation::SetDestination(const FVector& dest)
{
	Leader->Destination = dest;
	FRotator targetOrientation = UKismetMathLibrary::FindLookAtRotation(Leader->GetActorLocation(), dest);

	FVector forwardAtDest = targetOrientation.RotateVector(Leader->GetActorForwardVector());
	FVector rightAtDest = FRotator{ 0.f,90.f,0.f }.RotateVector(forwardAtDest);

	for (int i = 1; i < MaxNbrSlots; i++)
	{
		Slots[i]->Destination = dest + rightAtDest * Slots[i]->RelativePosition;
	}
}

void Formation::UpdateSlots(float deltaTime)
{
	if (bIsMoving)
	{
		for (size_t i = 0; i <= MaxNbrSlots; i++)
		{
			Slots[i]->SetActorLocation(Leader->GetActorLocation() + Leader->GetActorRightVector() * Slots[i]->RelativePosition);
			Slots[i]->SetActorRotation(Leader->GetActorRotation());
		}
	}
}

void Formation::MoveToDestination(const FVector& dest)
{
	SetDestination(dest);

	Leader->MoveToDestination();
}

Line::Line()
{
	Offset = FVector{ 150,0,0 };
}



void Line::AssignSlots(TArray<AActor*> actors)
{
	if (Leader == nullptr)
	{
		UWorld* world = actors[0]->GetWorld();
		const FActorSpawnParameters spawnParams;

		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[0]);
		Leader = character;
		Leader->FollowSlot = nullptr;
		for (int i = 1; i <= MaxNbrSlots; i++)
		{
			FVector location;
			if (i % 2 == 0)
			{
				location = Leader->GetActorLocation() + Offset * i * Leader->GetActorRightVector();
			}
			else
			{
				location = Leader->GetActorLocation() - Offset * i * Leader->GetActorRightVector();
			}
			Slots[i] = world->SpawnActor<AFormationSlot>
				(AFormationSlot::StaticClass(), location, actors[i]->GetActorRotation(), spawnParams);
			Slots[i]->RelativePosition = location;
			character = Cast<AGPP_ResearchCharacter>(actors[i]);
			character->FollowSlot = Slots[i];
			character->bIsInFormation = true;
			Slots[i]->MovementSpeed = character->GetMovementComponent()->GetMaxSpeed();
		}
	}
	else
	{
		Leader->FollowSlot = nullptr;
		Leader->bIsInFormation = true;

		for (int i = 1; i < MaxNbrSlots; i++)
		{
			AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[i]);
			character->FollowSlot = Slots[i];
			character->bIsInFormation = true;
			Slots[i]->MovementSpeed = character->GetMovementComponent()->GetMaxSpeed();
		}
	}

	bAreSlotAssigned = true;
}


