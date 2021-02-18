// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OperatorTPP.generated.h"

UCLASS()
class KAERMORHEN_API AOperatorTPP : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOperatorTPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UOperatorMovementComponent* OurMovementComponent;

	virtual UPawnMovementComponent* GetMovementComponent() const override;

	FORCEINLINE UStaticMeshComponent* GetMeshComponent() {
		return MeshComponent;
	}

	FORCEINLINE void SetMeshComponent(UStaticMeshComponent* Mesh) {
		MeshComponent = Mesh;
	}

	FORCEINLINE USphereComponent* GetSphereComponent() {
		return SphereComponent;
	}

	FORCEINLINE void SetSphereComponent(USphereComponent* Sphere) {
		SphereComponent = Sphere;
	}

	FORCEINLINE UCameraComponent* GetCameraComponent() {
		return Camera;
	}

	FORCEINLINE void SetCameraComponent(UCameraComponent* InCamera) {
		Camera = InCamera;
	}

	FORCEINLINE USpringArmComponent* GetSpringArmComponent() {
		return SpringArm;
	}

	FORCEINLINE void SetSpringArmComponent(USpringArmComponent* InSpringArm) {
		SpringArm = InSpringArm;
	}

private:

	void MoveFB(float input);
	void MoveLR(float input);

	void PitchCamera(float val);
	void YawCamera(float val);

	FVector2D CameraInput;

};