// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileNPCAnimInstance.h"
#include "HostileNPC.h"


void UHostileNPCAnimInstance::NativeInitializeAnimation() {
	if (Pawn == nullptr) {
		Pawn = TryGetPawnOwner();
		if (Pawn) {
			HostileNPC = Cast<AHostileNPC>(Pawn);
		}
	}
}

void UHostileNPCAnimInstance::UpdateAnimProperties() {
	if (Pawn == nullptr) {
		Pawn = TryGetPawnOwner();
		if (Pawn) {
			HostileNPC = Cast<AHostileNPC>(Pawn);
		}
	}
	if (Pawn) {
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
	}
}