// Fill out your copyright notice in the Description page of Project Settings.
#include "Formation.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GPP_ResearchCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include <math.h>

Formation::Formation(UClass* bpSlotRef)
{
	SlotBP = bpSlotRef;
	bAreSlotCreated = false;

}

Formation::~Formation()
{
	Leader = nullptr;

	for (int i = 0; i < Slots.Num(); i++)
	{
		Slots[i]->Destroy();
	}
	Slots.Empty();
}

void Formation::SetDestination(const FVector& dest)
{
	Leader->Destination = dest;
	FRotator targetOrientation = UKismetMathLibrary::FindLookAtRotation(Leader->GetActorLocation(), dest);

	FVector forwardAtDest = targetOrientation.RotateVector(Leader->GetActorForwardVector());
	FVector rightAtDest = FRotator{ 0.f,90.f,0.f }.RotateVector(forwardAtDest);

	for (int i = 1; i < Slots.Num(); i++)
	{
		Slots[i]->Destination = dest + rightAtDest * Slots[i]->RelativePosition;
	}
}

void Formation::MoveToDestination(const FVector& dest)
{
	if ((Leader->GetActorForwardVector() | dest - Leader->GetActorLocation()) < 0.f)
	{
		for (AFormationSlot* slot : Slots)
		{
			slot->InvertOffset();
		}
	}
	SetDestination(dest);

	Leader->MoveToDestination();
}

void Line::CreateSlot(AActor* actor, int& positionIndex, int index, UWorld* world, const FActorSpawnParameters& params)
{
	FVector location;
	FVector relativeLocation = Offset * positionIndex * Leader->GetActorRightVector();
	float slotOffset = Offset * positionIndex;
	if (index % 2 == 0)
	{
		location = Leader->GetActorLocation() + relativeLocation;
		positionIndex++;
	}
	else
	{
		location = Leader->GetActorLocation() - relativeLocation;
		relativeLocation = -relativeLocation;
		slotOffset = -slotOffset;
	}
	AFormationSlot* slot = world->SpawnActor<AFormationSlot>(SlotBP, location, Leader->GetActorRotation(), params);

	slot->RelativePosition = relativeLocation;
	slot->Offset = slotOffset;

	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actor);

	if (character)
	{
		slot->MovementSpeed = character->GetMovementComponent()->GetMaxSpeed();
		character->FollowSlot = slot;
		character->bIsInFormation = true;
	}
	Slots.Add(slot);
	NbrSlotCreated++;
}

Line::Line(UClass* bpSlotRef)
	:Formation{bpSlotRef}
{
	Offset = 150;
}

void Line::AssignSlots(const TArray<AActor*>& actors)
{
	Leader = nullptr;
	if (!bAreSlotCreated)
	{
		UWorld* world = actors[0]->GetWorld();
		const FActorSpawnParameters spawnParams;

		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[0]);
		Leader = character;
		Leader->FollowSlot = nullptr;
		int positionIndex = 1;
		for (int i = 1; i < actors.Num(); i++)
		{
			CreateSlot(actors[i], positionIndex, i, world, spawnParams);
		}
		bAreSlotCreated = true;
	}
	else
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[0]);
		if (character)
		{
			Leader = character;
		}

		if (NbrSlotCreated < actors.Num() - 1)
		{
			for (int i = 0; i < NbrSlotCreated; i++)
			{
				character = Cast<AGPP_ResearchCharacter>(actors[i+1]);
				if (character)
				{
					character->FollowSlot = Slots[i];
					character->bIsInFormation = true;
				}
			}

			UWorld* world = actors[0]->GetWorld();
			const FActorSpawnParameters spawnParams;
			int positionIndex = NbrSlotCreated / 2;

			for (int i = NbrSlotCreated; i < actors.Num() - NbrSlotCreated; i++)
			{
				CreateSlot(actors[i], positionIndex, i, world, spawnParams);
			}
		}
		else
		{
			for (int i = 0; i < actors.Num(); i++)
			{
				character = Cast<AGPP_ResearchCharacter>(actors[i + 1]);
				if (character)
				{
					character->FollowSlot = Slots[i];
					character->bIsInFormation = true;
				}
			}
		}
	}
}

void Line::UpdateSlots(float deltaTime)
{
	if (bAreSlotCreated)
	{
		for (size_t i = 0; i < Slots.Num(); i++)
		{
			Slots[i]->SetActorLocation(Leader->GetActorLocation() + Leader->GetActorRightVector() * Slots[i]->Offset);
			Slots[i]->SetActorRotation(Leader->GetActorRotation());
		}
	}
}

ProtectionCircle::ProtectionCircle(UClass* bpSlotRef)
	:Circle{bpSlotRef}
{
}

void ProtectionCircle::AssignSlots(const TArray<AActor*>& actors)
{
	if (actors.Num() == 0)
	{
		return;
	}

	if (actors.Num() <= 8)
	{
		Radius = MinRadius;
	}
	else
	{
		Radius = MinRadius + (actors.Num() - 8) * RadiusStep;
	}

	if (!bAreSlotCreated)
	{
		float angle = 2 * PI / (actors.Num() - 1);
		float currentAngle =0;

		UWorld* world = actors[0]->GetWorld();
		const FActorSpawnParameters spawnParams;

		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[0]);
		Leader = character;
		Leader->FollowSlot = nullptr;

		for (int i = 1; i < actors.Num(); i++)
		{
			FVector location(FMath::Cos(currentAngle)* Radius, FMath::Sin(currentAngle) * Radius, 0);
			AFormationSlot* slot = world->SpawnActor<AFormationSlot>
				(SlotBP, Leader->GetActorLocation() + location, Leader->GetActorRotation(), spawnParams);
			currentAngle += angle;
			slot->RelativePosition = location;

			character = Cast<AGPP_ResearchCharacter>(actors[i]);
			if (character)
			{
				slot->MovementSpeed = character->GetMovementComponent()->GetMaxSpeed();
				character->FollowSlot = slot;
				character->bIsInFormation = true;
			}

			Slots.Add(slot);
		}
		bAreSlotCreated = true;
	}
}

void ProtectionCircle::SetDestination(const FVector& dest)
{
	Formation::SetDestination(dest);
}

Circle::Circle(UClass* bpSlotRef)
	:Formation{bpSlotRef}
{
}

void Circle::AssignSlots(const TArray<AActor*>& actors)
{
	if (actors.Num() == 0)
	{
		return;
	}

	if (actors.Num() <= 8)
	{
		Radius = MinRadius;
	}
	else
	{
		Radius = MinRadius + (actors.Num() - 8) * RadiusStep;
	}

	if (!bAreSlotCreated)
	{
		UWorld* world = actors[0]->GetWorld();
		const FActorSpawnParameters spawnParams;

		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(actors[0]);
		Leader = character;
		Leader->FollowSlot = nullptr;

		float angle = 360 / actors.Num();

		FVector center = GetAverageActorPos(actors);
		FVector centerToPos{ (Leader->GetActorLocation() - center).Normalize() * Radius };
		CenterRelativeToLeader = -centerToPos;
		CenterRelativeToLeader.Z = 0;
		FVector leaderPosOnCircle{ center + centerToPos };
		Leader->MoveTo(leaderPosOnCircle);

		FRotator rot{ 0, angle, 0 };
		for (int i = 1; i < actors.Num(); i++)
		{
			centerToPos = rot.RotateVector(centerToPos);
			FVector location(center + centerToPos);
			AFormationSlot* slot = world->SpawnActor<AFormationSlot>
				(SlotBP, location, Leader->GetActorRotation(), spawnParams);

			slot->RelativePosition = leaderPosOnCircle - location;

			character = Cast<AGPP_ResearchCharacter>(actors[i]);
			if (character)
			{
				slot->MovementSpeed = character->GetMovementComponent()->GetMaxSpeed();
				character->FollowSlot = slot;
				character->bIsInFormation = true;
			}
			Slots.Add(slot);
		}
		bAreSlotCreated = true;
	}
}

void Circle::UpdateSlots(float deltaTime)
{
	if (bAreSlotCreated)
	{
		for (size_t i = 0; i < Slots.Num(); i++)
		{
			Slots[i]->SetActorLocation(Leader->GetActorLocation() + Slots[i]->RelativePosition);
			Slots[i]->SetActorRotation(Leader->GetActorRotation());
		}
	}
}

FVector Circle::GetAverageActorPos(const TArray<AActor*>& actors)
{
	FVector accPos{};

	for (AActor* actor : actors)
	{
		accPos += actor->GetActorLocation();
	}

	return accPos / (float)actors.Num();
}

void Circle::SetDestination(const FVector& dest)
{
	FVector centerToLeader = -CenterRelativeToLeader;

	Leader->Destination = dest + centerToLeader;
}

