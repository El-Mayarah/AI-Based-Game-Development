// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HostileNPC.generated.h"


UENUM(BlueprintType)
enum class EHostileMovementStatus : uint8 {
	EHMS_Idle UMETA(DisplayName = "Idle"),
	EHMS_NavToTarget UMETA(DisplayName = "NavToTarget"),
	EHMS_Attacking UMETA(DisplayName = "Attacking"),
	EHMS_Dead UMETA(DisplayName = "Dead"),

	EHMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class KAERMORHEN_API AHostileNPC : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHostileNPC();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	EHostileMovementStatus HostileMovementStatus;

	FORCEINLINE void SetHostileMovementStatus(EHostileMovementStatus Status) {
		HostileMovementStatus = Status;
	}

	FORCEINLINE EHostileMovementStatus GetHostileMovementStatus() {
		return HostileMovementStatus;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	class USphereComponent* AgroSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	class AAIController* AIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI)
	AMain* CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	USoundCue* SwingSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	class UBoxComponent* CombatCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat)
	class UAnimMontage* CombatMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bAttacking;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float AttackMinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float AttackMaxTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	TSubclassOf<UDamageType> DamageTypeClass;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float DeathDelay;

	bool bHasValidTarget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void NavToTarget(class AMain* Target);

	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	bool Alive();

	void Disappear();
};
