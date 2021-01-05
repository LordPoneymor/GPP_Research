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

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Mesh);
	SpringArm->TargetArmLength = 2500.f; 
	//SpringArm->bUsePawnControlRotation = true; //rotate arm based on controller
	SpringArm->SetRelativeRotation(FRotator(0, 0, -45));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	//attach the camera to the end of the boom and let the boom adjust the controller orientation
	Camera->bUsePawnControlRotation = false;

	SpeedMultiplier = 2.f;
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
}

void ACameraPawn::MoveForward(float value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Here"));
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