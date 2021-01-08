// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FormationSlot.h"
#include "CoreMinimal.h"

class GPP_RESEARCH_API Formation
{
public:
	Formation();
	virtual ~Formation();

	void UpdateSlots(float deltaTime);
	virtual void AssignSlots(TArray<AActor*> actors) = 0;
	
	void MoveToDestination(const FVector& dest);

	class AGPP_ResearchCharacter* Leader;
	TArray<class AFormationSlot*> Slots;
	const int MaxNbrSlots{ 6 };
	bool bAreSlotAssigned;
	bool bIsMoving;

protected:
	void SetDestination(const FVector& dest);
};

class GPP_RESEARCH_API Line : public Formation
{
public:
	Line();
	~Line() = default;

	virtual void AssignSlots(TArray<AActor*> actors) override;

private:
	FVector Offset;

	

};
