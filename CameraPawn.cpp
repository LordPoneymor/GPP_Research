// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GPP_ResearchPlayerController.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	this->Mesh->SetStaticMesh(MeshAsset.Object);
	Mesh->SetCollisionProfileName("NoCollision");
	Mesh->SetHiddenInGame(true);
	Mesh->SetRelativeLocation(FVector(0, 0, 200));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Mesh);
	SpringArm->TargetArmLength = 1500.f; 
	SpringArm->SetRelativeRotation(FRotator(-45, 0, 0));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	SpeedMultiplier = 2.f;
	MaxArmLength = 2500.f;
	ZoomSensitivity = 100.f;
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();

	CameraController = Cast<AGPP_ResearchPlayerController>(GetController());

	FVector location = GetActorLocation();
	SetActorLocation(FVector(location.X , location.Y, 200));
	
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACameraPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACameraPawn::MoveSideways);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &ACameraPawn::ZoomIn);
	PlayerInputComponent->BindAction("ZoomOut", IE_Pressed, this, &ACameraPawn::ZoomOut);
}

void ACameraPawn::MoveForward(float value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Here"));
	//Movement X Math
	float sensitivity = 5;// MovementSpeedCalculation();
	float x = SpeedMultiplier * (value * sensitivity);
	FVector direction = FVector(x, 0, 0);

	FTransform transform = GetActorTransform();
	FVector velocity = UKismetMathLibrary::TransformDirection(transform, direction);

	FTransform newTransform;
	newTransform.SetLocation(velocity + transform.GetLocation());

	SetActorTransform(newTransform);
}

void ACameraPawn::MoveSideways(float value)
{
	//Movement Y Math
	float sensitivity = 5;//MovementSpeedCalculation();
	float y = SpeedMultiplier * (value * sensitivity);
	FVector direction = FVector(0, y, 0);

	FTransform transform = GetActorTransform();
	FVector velocity = UKismetMathLibrary::TransformDirection(transform, direction);

	FTransform newTransform;
	newTransform.SetLocation(velocity + transform.GetLocation());

	SetActorTransform(newTransform);
}

float ACameraPawn::MovementSpeedCalculation()
{
	float sensitivity = SpringArm->TargetArmLength / 100;
	FMath::Clamp(sensitivity, 5.f, 20.f);
	return sensitivity;
}

void ACameraPawn::ZoomIn()
{
	float armLength = SpringArm->TargetArmLength;

	SpringArm->TargetArmLength = FMath::Clamp(armLength - ZoomSensitivity, 500.f, MaxArmLength);
}

void ACameraPawn::ZoomOut()
{
	float armLength = SpringArm->TargetArmLength;

	SpringArm->TargetArmLength = FMath::Clamp(armLength + ZoomSensitivity, 500.f, MaxArmLength);
}