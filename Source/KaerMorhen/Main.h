// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8 {
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8 {
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMin UMETA(DisplayName = "BelowMin"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class KAERMORHEN_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	TArray<FVector> PickupLocations;

	UFUNCTION(BlueprintCallable)
	void PostPickupLocations();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Controller)
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Enums)
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) {
		StaminaStatus = Status;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
	float MinStaminaToSprint;

	/** Set movement status and running speed. */
	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Running)
	float SprintingSpeed;

	bool bShiftKeyDown;

	/** Press down key to enable sprinting. */
	void ShiftKeyDown();

	/** Release to stop sprinting. */
	void ShiftKeyUp();

	/** Positions the camera behind the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Camera to follow the player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	/** Base turn rates to scale turning functions for the camera. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** Weapon stats. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	FORCEINLINE AWeapon* GetEquippedWeapon() {
		return EquippedWeapon;
	}

	void SetEquippedWeapon(AWeapon* Weapon);

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) {
		ActiveOverlappingItem = Item;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	class USoundCue* HitSound;

	float InterpSpeed;

	bool bInterpToHostile;

	void SetInterpToHostile(bool Interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	class AHostileNPC* CombatTarget;

	FORCEINLINE void SetCombatTarget(AHostileNPC* Target) {
		CombatTarget = Target;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	bool bHasCombatTarget;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) {
		bHasCombatTarget = HasTarget;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat)
	FVector CombatTargetLocation;

	FRotator GetLookAtRotationYaw(FVector Target);

	/**
	/*
	/*	Player stats.
	/*
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerStats)
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerStats)
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = PlayerStats)
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerStats)
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerStats)
	int32 Coins;

	void DecrementHealth(float Amount);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	virtual void Jump() override;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	bool bMovingForward;

	bool bMovingRight;

	bool CanMove(float Value);

	/** Called for forward and backward movements. */
	void MoveForward(float val);

	/** Called for left and right movements. */
	void MoveRight(float val);

	/** Called for yaw rotation. */
	void Turn(float val);

	/** Called for pitch rotation. */
	void LookUp(float val);

	/** Called via input to turn at a given rate. 
	* @param Rate It is the normalized rate; i.e. 1.0 indicating 100% of desired turn rate.
	*/
	void TurnAtRate(float rate);

	/** Called via input to look up and down at a given rate.
	* @param Rate It is the normalized rate; i.e. 1.0 indicating 100% of desired look up/down rate.
	*/
	void LookUpAtRate(float rate);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const {
		return CameraBoom;
	}

	FORCEINLINE class UCameraComponent* GetCamera() const {
		return Camera;
	}

	bool bLMBDown;

	void LMBDown();

	void LMBUp();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Anims)
	bool bAttacking;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Anims)
	class UAnimMontage* CombatMontage;

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	TSubclassOf<AHostileNPC> HostileFilter;

	void UpdateCombatTarget();

	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	void LoadGameNoSwitch();

	UPROPERTY(EditDefaultsOnly, Category = SaveData)
	TSubclassOf<class AItemContainer> WeaponContainer;

	bool bESCDown;

	void ESCDown();

	void ESCUp();
};
