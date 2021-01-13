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

void Formation::MoveToDestination(const FVector& dest)
{
	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(Leader);
	if (character)
	{
		if ((Leader->GetActorForwardVector() | dest - Leader->GetActorLocation()) < 0.f)
		{
			for (AFormationSlot* slot : Slots)
			{
				slot->InvertOffset();
			}
		}
		SetDestination(dest);
		character->MoveToDestination();
	}
	else
	{
		SetDestination(dest);
	}
	
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
		character->FollowSlot = nullptr;
		Leader = character;
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

void Line::SetDestination(const FVector& dest)
{
	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(Leader);
	if (character)
	{
		character->Destination = dest;
	}
	FRotator targetOrientation = UKismetMathLibrary::FindLookAtRotation(Leader->GetActorLocation(), dest);
	FVector destZFixed = dest;

	destZFixed.Z = Leader->GetActorLocation().Z;
	FVector forwardAtDest = destZFixed - Leader->GetActorLocation();
	
	forwardAtDest.Normalize();
	FVector rightAtDest = FRotator{ 0.f,90.f,0.f }.RotateVector(forwardAtDest);

	for (int i = 0; i < Slots.Num(); i++)
	{
		Slots[i]->Destination = destZFixed + rightAtDest * Slots[i]->Offset;
		Slots[i]->Destination.Z = dest.Z;
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
		character->FollowSlot = nullptr;
		Leader = character;
		
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
	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(Leader);
	if (character)
	{
		character->Destination = dest;
		for (AFormationSlot* slot : Slots)
		{
			slot->Destination = dest + slot->RelativePosition;
		}
	}
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
		if (character)
		{
			character->FollowSlot = nullptr;
			Leader = character;

			float angle = 360 / actors.Num();

			FVector center = GetAverageActorPos(actors);
			FVector centerToPos{ (Leader->GetActorLocation() - center).Normalize() * Radius };
			CenterRelativeToLeader = -centerToPos;
			CenterRelativeToLeader.Z = 0;
			FVector leaderPosOnCircle{ center + centerToPos };
			character->MoveTo(leaderPosOnCircle);

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
	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(Leader);
	if (character)
	{
		character->Destination = dest + CenterRelativeToLeader;
		for (AFormationSlot* slot : Slots)
		{
			slot->Destination = dest + slot->RelativePosition + CenterRelativeToLeader;
		}
	}
}

Splited::Splited(UClass* bpSlotRef, EFormation form, const TArray<AActor*>& actors)
	:Formation(bpSlotRef),
	SubGroupFormation1{ nullptr },
	SubGroupFormation2{ nullptr },
	Form{form}
{
	AssignSlots(actors);
}

Splited::~Splited()
{
	delete SubGroupFormation1;
	delete SubGroupFormation2;
	Leader->Destroy();
}

void Splited::AssignSlots(const TArray<AActor*>& actors)
{
	UWorld* world = actors[0]->GetWorld();
	const FActorSpawnParameters spawnParams;

	int groupSize{ actors.Num() / 2 };
	int group2Size{ };

	TArray<AActor*> subGroup1;
	TArray<AActor*> subGroup2;


	if (actors.Num() % 2 != 0)
	{
		group2Size = groupSize + 1;
		for (size_t i = 0; i < groupSize; i++)
		{
			subGroup1.Add(actors[i]);
		}

		for (size_t i = group2Size; i < actors.Num(); i++)
		{
			subGroup2.Add(actors[i]);
		}
	}
	else
	{
		for (size_t i = 0; i < groupSize; i++)
		{
			subGroup1.Add(actors[i]);
		}

		for (size_t i = groupSize; i < actors.Num(); i++)
		{
			subGroup2.Add(actors[i]);
		}
	}

	switch (Form)
	{
	case EFormation::EF_Line:
		SubGroupFormation1 = new Line(SlotBP);
		SubGroupFormation2 = new Line(SlotBP);
		break;
	case EFormation::EF_ProtectionCircle:
		SubGroupFormation1 = new Circle(SlotBP);
		SubGroupFormation2 = new Circle(SlotBP);
		break;
	case EFormation::EF_Circle:
		SubGroupFormation1 = new ProtectionCircle(SlotBP);
		SubGroupFormation2 = new ProtectionCircle(SlotBP);
		break;
	}
	SubGroupFormation1->AssignSlots(subGroup1);
	SubGroupFormation2->AssignSlots(subGroup2);

	FVector shifting{ SubGroupFormation1->Leader->GetActorLocation() - SubGroupFormation2->Leader->GetActorLocation() };
	float length{ shifting.Size() };
	shifting.Normalize();
	shifting *= length / 2;
	FVector location = SubGroupFormation1->Leader->GetActorLocation() + location;

	AFormationSlot* leaderSlot = world->SpawnActor<AFormationSlot>
		(SlotBP, location, SubGroupFormation1->Leader->GetActorRotation(), spawnParams);
	Leader = leaderSlot;
	if (length < DistanceBetweenGroups)
	{
		float movemntNeeded{ (DistanceBetweenGroups - length)/2 };
		shifting.Normalize();
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(SubGroupFormation1->Leader);
		if(character)
		{
			FVector dest{ character->GetActorLocation() + shifting * movemntNeeded };
			character->MoveTo(dest);
		}
		character = nullptr;
		character = Cast<AGPP_ResearchCharacter>(SubGroupFormation2->Leader);
		if (character)
		{
			FVector dest{ character->GetActorLocation() + (-shifting) * movemntNeeded };
			character->MoveTo(dest);
		}
	}
	else
	{
		float movemntNeeded{ (length - DistanceBetweenGroups) / 2 };
		shifting.Normalize();
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(SubGroupFormation1->Leader);
		if (character)
		{
			FVector dest{ character->GetActorLocation() + (-shifting) * movemntNeeded };
			character->MoveTo(dest);
		}
		character = nullptr;
		character = Cast<AGPP_ResearchCharacter>(SubGroupFormation2->Leader);
		if (character)
		{
			FVector dest{ character->GetActorLocation() + shifting * movemntNeeded };
			character->MoveTo(dest);
		}
	}

	AFormationSlot* slot = Cast<AFormationSlot>(Leader);
	if (slot)
	{
		AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(SubGroupFormation1->Leader);
		if (character)
		{
			character->FollowSlot = slot;
		}
		character = nullptr;
		character = Cast<AGPP_ResearchCharacter>(SubGroupFormation2->Leader);
		if (character)
		{
			character->FollowSlot = slot;
		}
	}
}

void Splited::UpdateSlots(float deltaTime)
{
	SubGroupFormation1->UpdateSlots(deltaTime);
	SubGroupFormation2->UpdateSlots(deltaTime);
}

void Splited::SetDestination(const FVector& dest)
{
	FVector forwardAtDest = dest - Leader->GetActorLocation();

	forwardAtDest.Normalize();
	FVector rightAtDest = FRotator{ 0.f,90.f,0.f }.RotateVector(forwardAtDest);

	AFormationSlot* slot = Cast<AFormationSlot>(Leader);
	if (slot)
	{
		slot->Orientation = UKismetMathLibrary::FindLookAtRotation(slot->GetActorLocation(), dest);
		slot->SetActorRotation(slot->Orientation);
		slot->Destination = dest;
		slot->SetActorLocation(dest);
	}

	AGPP_ResearchCharacter* character = Cast<AGPP_ResearchCharacter>(SubGroupFormation1->Leader);
	if (character)
	{
		character->Destination = Leader->GetActorLocation() + rightAtDest * DistanceBetweenGroups / 2;
		character->MoveToDestination();
	}
	character = nullptr;
	character = Cast<AGPP_ResearchCharacter>(SubGroupFormation2->Leader);
	if (character)
	{
		character->Destination = Leader->GetActorLocation() + (-rightAtDest) * DistanceBetweenGroups / 2;
		character->MoveToDestination();
	}
}