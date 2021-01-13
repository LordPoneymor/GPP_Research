// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "FormationSlot.h"
#include "CoreMinimal.h"

enum class EFormation : uint8
{
	EF_Line UMETA(DisplayName = "Line"),
	EF_ProtectionCircle UMETA(DisplayName = "Protection Circle"),
	EF_Circle UMETA(DisplayName = "Circle"),
	EF_Splited UMETA(DisplayName = "Splited"),
	EF_None UMETA(DisplayName = "None"),

	EF_MAX UMETA(DisplayName = "Default Max")
};

class GPP_RESEARCH_API Formation
{
public:
	Formation(UClass* bpSlotRef);
	virtual ~Formation();

	virtual void UpdateSlots(float deltaTime) = 0;
	virtual void AssignSlots(const TArray<AActor*>& actors) = 0;
	
	void MoveToDestination(const FVector& dest);

	bool bIsMoving;
	class AActor* Leader;

protected:
	

	TSubclassOf<class AFormationSlot> SlotBP;
	TArray<AFormationSlot*> Slots;

	int NbrSlotCreated;
	bool bAreSlotCreated;

	virtual void SetDestination(const FVector& dest) = 0;
	
};

class GPP_RESEARCH_API Line final : public Formation
{
public:
	Line(UClass* bpSlotRef);
	~Line() = default;

	virtual void AssignSlots(const TArray<AActor*>& actors) override;
	virtual void UpdateSlots(float deltaTime) override;

protected:
	virtual void SetDestination(const FVector& dest) override;

private:
	float Offset;

	void CreateSlot(AActor* actor, int& positionIndex, int index, UWorld* world, const FActorSpawnParameters& params);
};

class GPP_RESEARCH_API Circle : public Formation
{
public:
	Circle(UClass* bpSlotRef);
	~Circle() = default;

	virtual void AssignSlots(const TArray<AActor*>& actors) override;
	virtual void UpdateSlots(float deltaTime) override;

protected:
	float Radius;
	const float MinRadius{ 150 };
	const float RadiusStep{ 25 };
	virtual void SetDestination(const FVector& dest) override;
private:
	FVector CenterRelativeToLeader;
	FVector GetAverageActorPos(const TArray<AActor*>& actors);
};

class GPP_RESEARCH_API ProtectionCircle final : public Circle
{
public:
	ProtectionCircle(UClass * bpSlotRef);
	~ProtectionCircle() = default;

	virtual void AssignSlots(const TArray<AActor*> & actors) override;

protected:
	virtual void SetDestination(const FVector& dest) override;
};

class GPP_RESEARCH_API Splited final : public Formation
{
public:
	Splited(UClass * bpSlotRef, EFormation form, const TArray<AActor*>& actors);
	~Splited();

	virtual void AssignSlots(const TArray<AActor*> & actors) override;
	virtual void UpdateSlots(float deltaTime) override;
	EFormation GetFormation()const { return Form; }
protected:
	virtual void SetDestination(const FVector & dest) override;
private:
	const float DistanceBetweenGroups{ 2000 };
	const float LeaderAheadPos{ 200 };

	Formation* SubGroupFormation1;
	Formation* SubGroupFormation2;
	EFormation Form;
};