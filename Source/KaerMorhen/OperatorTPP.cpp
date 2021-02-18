// Fill out your copyright notice in the Description page of Project Settings.


#include "OperatorTPP.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "OperatorMovementComponent.h"


// Sets default values
AOperatorTPP::AOperatorTPP()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	//SphereComponent->SetupAttachment(GetRootComponent());
	SetRootComponent(SphereComponent);
	SphereComponent->InitSphereRadius(40.f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));
	if (MeshComponentAsset.Succeeded()) {
		MeshComponent->SetStaticMesh(MeshComponentAsset.Object);
		MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		MeshComponent->SetWorldScale3D(FVector(0.8f));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	OurMovementComponent = CreateDefaultSubobject<UOperatorMovementComponent>(TEXT("OurMovementComponent"));
	OurMovementComponent->UpdatedComponent = RootComponent;

	CameraInput = FVector2D(0.f);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void AOperatorTPP::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOperatorTPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += CameraInput.X;
	SetActorRotation(NewRotation);

	FRotator NewSpringArmRotation = SpringArm->GetComponentRotation();
	NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch += CameraInput.Y, -80.f, -15.f);
	SpringArm->SetWorldRotation(NewSpringArmRotation);
}

// Called to bind functionality to input
void AOperatorTPP::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveFB"), this, &AOperatorTPP::MoveFB);
	PlayerInputComponent->BindAxis(TEXT("MoveLR"), this, &AOperatorTPP::MoveLR);

	PlayerInputComponent->BindAxis(TEXT("CameraPitch"), this, &AOperatorTPP::PitchCamera);
	PlayerInputComponent->BindAxis(TEXT("CameraYaw"), this, &AOperatorTPP::YawCamera);

}


void AOperatorTPP::MoveFB(float input) {
	FVector Forward = GetActorForwardVector();
	//AddMovementInput(input * Forward);
	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(Forward * input);
	}
}


void AOperatorTPP::MoveLR(float input) {
	FVector Right = GetActorRightVector();
	//AddMovementInput(input * Right);
	if (OurMovementComponent) {
		OurMovementComponent->AddInputVector(Right * input);
	}
}

UPawnMovementComponent* AOperatorTPP::GetMovementComponent() const {
	return OurMovementComponent;
}

void AOperatorTPP::PitchCamera(float val) {
	CameraInput.Y = val;
}

void AOperatorTPP::YawCamera(float val) {
	CameraInput.X = val;
}