// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "HostileNPC.h"
#include "MainPlayerController.h"
#include "Save.h"
#include "ItemContainer.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Pulls towards the player if there's a collision.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraTPP"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; // Camera distance from the char.
	CameraBoom->bUsePawnControlRotation = true; // Rotates arm based on controller.

	// Setting size for collision capsule.
	GetCapsuleComponent()->SetCapsuleSize(36.f, 87.f);

	// Creates camera for the springarm to follow the player.
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the spring arm and let the arm adjust to match the controller orientation.
	Camera->bUsePawnControlRotation = false;

	// Set our turn rates for input.
	BaseTurnRate = 65.f; 
	BaseLookUpRate = 65.f;

	// Stops character rotation with the camera.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// Configuring character movement.
	GetCharacterMovement()->bOrientRotationToMovement = true; // Char moves in direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // ...at this rotation rate.
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 100.f;
	Stamina = 20.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;
	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;

	// Initializing Enums.
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 30.f;
	MinStaminaToSprint = 20.f;

	InterpSpeed = 15.f;
	bInterpToHostile = false;
	bHasCombatTarget = false;
	bMovingForward = false;
	bMovingRight = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	if (Map != "SunTemple") {
		LoadGameNoSwitch();

		if (MainPlayerController) {
			MainPlayerController->GameModeOnly();
		}
	}	
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementStatus == EMovementStatus::EMS_Dead) {
		return;
	}

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus) {
		case EStaminaStatus::ESS_Normal:
			if (bShiftKeyDown && (bMovingForward || bMovingRight)) {
				if (Stamina - DeltaStamina <= MinStaminaToSprint) {
					SetStaminaStatus(EStaminaStatus::ESS_BelowMin);
				}
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			} else {	// ShiftKeyUp.
				if (Stamina + DeltaStamina >= MaxStamina) {
					Stamina = MaxStamina;
				} else {
					Stamina += DeltaStamina;
				}
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminaStatus::ESS_BelowMin:
			if (bShiftKeyDown && (bMovingForward || bMovingRight)) {
				if (Stamina - DeltaStamina <= 0.f) {
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0.f;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				} else {
					Stamina -= DeltaStamina;
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
			} else {	// ShiftKeyUp.
				if (Stamina + DeltaStamina >= MinStaminaToSprint) {
					SetStaminaStatus(EStaminaStatus::ESS_Normal);
				}
				Stamina += DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminaStatus::ESS_Exhausted:
			if (bShiftKeyDown) {
				Stamina = 0.f;
			} else {	// ShiftKeyUp.
				SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		case EStaminaStatus::ESS_ExhaustedRecovering:
			if (Stamina + DeltaStamina >= MinStaminaToSprint) {
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			Stamina += DeltaStamina;
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		default:;
	}

	if (bInterpToHostile && CombatTarget) {
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController) {
			MainPlayerController->HostileLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);
}

bool AMain::CanMove(float Value) {
	if (MainPlayerController) {
		return Value != 0.f &&
			!bAttacking &&
			MovementStatus != EMovementStatus::EMS_Dead &&
			!MainPlayerController->bPauseMenuVisible;
	}
	return false;
}

void AMain::MoveForward(float val) {
	bMovingForward = false;
	if (CanMove(val)) {
		// Finds which way is forward.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, val);
		bMovingForward = true;
	}
}

void AMain::MoveRight(float val) {
	bMovingRight = false;
	if (CanMove(val)) {
		// Finds which way is right.
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, val);
		bMovingRight = true;
	}
}

void AMain::Turn(float val) {
	//if (CanMove(val)) {
		AddControllerYawInput(val);
	//}
}

void AMain::LookUp(float val) {
	//if (CanMove(val)) {
		AddControllerPitchInput(val);
	//}
}

void AMain::TurnAtRate(float rate) {
	AddControllerYawInput(rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float rate) {
	AddControllerPitchInput(rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

FRotator AMain::GetLookAtRotationYaw(FVector Target) {
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AMain::LMBDown() {
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) {
		return;
	}

	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}

	if (ActiveOverlappingItem) {
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon) {
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	} else if (EquippedWeapon) {
		Attack();
	}
}

void AMain::LMBUp() {
	bLMBDown = false;
}

void AMain::ESCDown() {
	bESCDown = true;

	if (MainPlayerController) {
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::ESCUp() {
	bESCDown = false;
}

void AMain::DecrementHealth(float Amount) {
	
}

void AMain::IncrementCoins(int32 Amount) {
	Coins += Amount;
}

void AMain::IncrementHealth(float Amount) {
	if (Health + Amount >= MaxHealth) {
		Health = MaxHealth;
	} else {
		Health += Amount;
	}
}

void AMain::Jump() {
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}

	if (MovementStatus != EMovementStatus::EMS_Dead) {
		ACharacter::Jump();
	}
}

void AMain::Die() {
	if (MovementStatus == EMovementStatus::EMS_Dead) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::DeathEnd() {
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::SetMovementStatus(EMovementStatus Status) {
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	} else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown() {
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp() {
	bShiftKeyDown = false;
}

void AMain::PostPickupLocations() {
	/*for (int32 i = 0; i < PickupLocations.Num(); i++) {
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.f, 8, FLinearColor::Green, 10.f, 0.5f);
	}*/

	// for (auto <variable> : <container>)
	for (FVector Loc : PickupLocations) {
		UKismetSystemLibrary::DrawDebugSphere(this, Loc, 25.f, 8, FLinearColor::Green, 10.f, 0.5f);
	}
}

void AMain::SetEquippedWeapon(AWeapon* Weapon) {
	if (EquippedWeapon) {
		EquippedWeapon->Destroy();
	}
	EquippedWeapon = Weapon;
}

void AMain::Attack() {
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToHostile(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) {
			int32 Section = FMath::RandRange(0, 4);
			switch (Section) {
				case 0:
					AnimInstance->Montage_Play(CombatMontage, 1.5f);
					AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.5f);
					AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
					break;
				case 2:
					AnimInstance->Montage_Play(CombatMontage, 1.5f);
					AnimInstance->Montage_JumpToSection(FName("Attack_3"), CombatMontage);
					break;
				case 3:
					AnimInstance->Montage_Play(CombatMontage, 1.5f);
					AnimInstance->Montage_JumpToSection(FName("Attack_4"), CombatMontage);
					break;
				case 4:
					AnimInstance->Montage_Play(CombatMontage, 1.5f);
					AnimInstance->Montage_JumpToSection(FName("Attack_5"), CombatMontage);
					break;
				default:;
			}
		}
	}
}

void AMain::AttackEnd() {
	bAttacking = false;
	SetInterpToHostile(false);

	if (bLMBDown) {
		Attack();
	}
}

void AMain::PlaySwingSound() {
	if (EquippedWeapon->SwingSound) {
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMain::SetInterpToHostile(bool Interp) {
	bInterpToHostile = Interp;
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController* EventInstigator, AActor* DamageCauser) {
	Health -= DamageAmount;
	if (Health <= 0.f) {
		Die();

		if (DamageCauser) {
			AHostileNPC* HostileNPC = Cast<AHostileNPC>(DamageCauser);
			if (HostileNPC) {
				HostileNPC->bHasValidTarget = false;
			}
		}
	}
	return DamageAmount;
}

void AMain::UpdateCombatTarget() {
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, HostileFilter);

	if (OverlappingActors.Num() == 0) {
		if (MainPlayerController) {
			MainPlayerController->RemoveHostileHealthBar();
		}
		return;
	}

	AHostileNPC* ClosestHostile = Cast<AHostileNPC>(OverlappingActors[0]);
	if (ClosestHostile) {
		FVector CharLocation = GetActorLocation();
		float MinDistance = (ClosestHostile->GetActorLocation() - CharLocation).Size();

		for (auto Actor : OverlappingActors) {
			AHostileNPC* Hostile = Cast<AHostileNPC>(Actor);
			if (Hostile) {
				float DistanceToActor = (Hostile->GetActorLocation() - CharLocation).Size();
				if (DistanceToActor < MinDistance) {
					MinDistance = DistanceToActor;
					ClosestHostile = Hostile;
				}
			}
		}

		if (MainPlayerController) {
			MainPlayerController->DisplayHostileHealthBar();
		}

		SetCombatTarget(ClosestHostile);
		SetHasCombatTarget(true);
	}
}

void AMain::SwitchLevel(FName LevelName) {
	UWorld* World = GetWorld();
	if (World) {
		FString CurrentLevel = World->GetMapName();

		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName) {
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame() {
	USave* SaveGameInstance = Cast<USave>(UGameplayStatics::CreateSaveGameObject(USave::StaticClass()));

	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	if (EquippedWeapon) {
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}

	FString MapName = GetWorld()->GetMapName();
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	// UE_LOG(LogTemp, Warning, TEXT("MapName: %s"), *MapName)

	SaveGameInstance->CharacterStats.LevelName = MapName;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition) {
	USave* LoadGameInstance = Cast<USave>(UGameplayStatics::CreateSaveGameObject(USave::StaticClass()));

	LoadGameInstance = Cast<USave>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (SetPosition) {
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	}

	if (WeaponContainer) {
		AItemContainer* Weapons = GetWorld()->SpawnActor<AItemContainer>(WeaponContainer);
		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;

	if (LoadGameInstance->CharacterStats.LevelName != TEXT("")) {
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		SwitchLevel(LevelName);
	}
}

void AMain::LoadGameNoSwitch() {
	USave* LoadGameInstance = Cast<USave>(UGameplayStatics::CreateSaveGameObject(USave::StaticClass()));

	LoadGameInstance = Cast<USave>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponContainer) {
		AItemContainer* Weapons = GetWorld()->SpawnActor<AItemContainer>(WeaponContainer);
		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}