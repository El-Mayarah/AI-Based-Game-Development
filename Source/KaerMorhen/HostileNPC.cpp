// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileNPC.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"

// Sets default values
AHostileNPC::AHostileNPC()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(75.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	//CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("LeftClawSocket"));
	CombatCollision->SetupAttachment(GetMesh(), FName("LeftAttackSocket"));

	bOverlappingCombatSphere = false;

	MaxHealth = 100.f;
	Health = 75.f;
	Damage = 15.f;

	AttackMinTime = 0.1f;
	AttackMaxTime = 1.f;

	HostileMovementStatus = EHostileMovementStatus::EHMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;
}

// Called when the game starts or when spawned
void AHostileNPC::BeginPlay()
{
	Super::BeginPlay();

	// GetController returns AController (AIController is derived from AController).
	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AHostileNPC::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AHostileNPC::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AHostileNPC::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AHostileNPC::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AHostileNPC::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AHostileNPC::CombatOnOverlapEnd);

	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AHostileNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHostileNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AHostileNPC::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor && Alive()) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			NavToTarget(Main);
		}
		UE_LOG(LogTemp, Warning, TEXT("AgroSphere overlap begun."))
	}
}

void AHostileNPC::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			bHasValidTarget = false;
			Main->SetHasCombatTarget(false);
			if (Main->CombatTarget == this) {
				Main->SetCombatTarget(nullptr);
			}
			Main->UpdateCombatTarget();

			/*if (Main->MainPlayerController) {
				Main->MainPlayerController->RemoveHostileHealthBar();
			}*/
			SetHostileMovementStatus(EHostileMovementStatus::EHMS_Idle);
			if (AIController) {
				AIController->StopMovement();
			}
			UE_LOG(LogTemp, Warning, TEXT("AgroSphere overlap ended."))
		}
	}
}

void AHostileNPC::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor && Alive()) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			UE_LOG(LogTemp, Warning, TEXT("CombatSphere overlap begun."))
			bHasValidTarget = true;
			Main->SetCombatTarget(this);
			Main->SetHasCombatTarget(true);
			/*if (Main->MainPlayerController) {
				Main->MainPlayerController->DisplayHostileHealthBar();
			}*/

			Main->UpdateCombatTarget();

			CombatTarget = Main;
			bOverlappingCombatSphere = true;
			
			//Attack();
			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AHostileNPC::Attack, AttackTime);
		}
	}
}

void AHostileNPC::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if (OtherActor && OtherComp) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			UE_LOG(LogTemp, Warning, TEXT("CombatSphere overlap ended."))
			bOverlappingCombatSphere = false;
			if (HostileMovementStatus != EHostileMovementStatus::EHMS_Attacking) {
				NavToTarget(Main);
				CombatTarget = nullptr;

				if (Main->CombatTarget == this) {
					Main->SetCombatTarget(nullptr);
					Main->SetHasCombatTarget(false);
					Main->UpdateCombatTarget();
				}

				if (Main->MainPlayerController) {
					USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
					if (MainMesh) {
						Main->MainPlayerController->RemoveHostileHealthBar();
					}
				}
			}
			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

void AHostileNPC::NavToTarget(AMain* Target) {
	SetHostileMovementStatus(EHostileMovementStatus::EHMS_NavToTarget);

	if (AIController) {
		UE_LOG(LogTemp, Warning, TEXT("NavToTarget"))

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/* auto PathPoints;
		TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();

		for (auto Point : PathPoints) {
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 24, FLinearColor::Green, 10.f, 0.5f);
		}
		*/
	}
}

void AHostileNPC::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (OtherActor) {
		AMain* Main = Cast<AMain>(OtherActor);
		if (Main) {
			UE_LOG(LogTemp, Warning, TEXT("Claw overlap begun."))
			if (Main->HitParticles) {
				const USkeletalMeshSocket* LeftParticlesSocket = GetMesh()->GetSocketByName("LeftParticlesSocket");
				if (LeftParticlesSocket) {
					FVector SocketLocation = LeftParticlesSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Main->HitParticles, SocketLocation, FRotator(0.f), false);
					UE_LOG(LogTemp, Warning, TEXT("Emitter"))
				}
			}
			if (Main->HitSound) {
				UGameplayStatics::PlaySound2D(this, Main->HitSound);
			}
			if (DamageTypeClass) {
				UGameplayStatics::ApplyDamage(Main, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AHostileNPC::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	UE_LOG(LogTemp, Warning, TEXT("Claw overlap ended."))
}

void AHostileNPC::ActivateCollision() {
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (SwingSound) {
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AHostileNPC::DeactivateCollision() {
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AHostileNPC::Attack() {
	UE_LOG(LogTemp, Warning, TEXT("Attack called."))
	if (Alive() && bHasValidTarget) {
		if (AIController) {
			AIController->StopMovement();
			SetHostileMovementStatus(EHostileMovementStatus::EHMS_Attacking);
		}
		if (!bAttacking) {
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance && CombatMontage) {
				AnimInstance->Montage_Play(CombatMontage, 1.f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				UE_LOG(LogTemp, Warning, TEXT("Attack section played."))
			}
		}
	}
}

void AHostileNPC::AttackEnd() {
	UE_LOG(LogTemp, Warning, TEXT("Attack section ended."))
	bAttacking = false;
	SetHostileMovementStatus(EHostileMovementStatus::EHMS_Idle);
	if (bOverlappingCombatSphere) {
		UE_LOG(LogTemp, Warning, TEXT("Next attack delay initiated."))
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AHostileNPC::Attack, AttackTime);
	}
}

float AHostileNPC::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) {
	if (Health - DamageAmount <= 0.f) {
		Health -= DamageAmount;
		Die(DamageCauser);
	} else {
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AHostileNPC::Die(AActor* Causer) {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance) {
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetHostileMovementStatus(EHostileMovementStatus::EHMS_Dead);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AMain* Main = Cast<AMain>(Causer);
	if (Main) {
		Main->UpdateCombatTarget();
	}
}

void AHostileNPC::DeathEnd() {
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AHostileNPC::Disappear, DeathDelay);
}

bool AHostileNPC::Alive() {
	return GetHostileMovementStatus() != EHostileMovementStatus::EHMS_Dead;
}

void AHostileNPC::Disappear() {
	Destroy();
}