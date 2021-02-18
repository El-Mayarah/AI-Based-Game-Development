// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Operator.generated.h"

UCLASS()
class KAERMORHEN_API AOperator : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AOperator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	USkeletalMeshComponent* MeshComponent;
	//UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "PawnMovement")
	float MaxSpeed;

private:
	
	void MoveFB(float value);
	void MoveLR(float value);

	FVector CurrentVelocity;

};
