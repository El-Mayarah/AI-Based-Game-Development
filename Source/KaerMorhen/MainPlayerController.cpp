// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainPlayerController::BeginPlay() {
	Super::BeginPlay();

	if (HUD_OverlayAsset) {
		HUD_Overlay = CreateWidget<UUserWidget>(this, HUD_OverlayAsset);
	}
	HUD_Overlay->AddToViewport();
	HUD_Overlay->SetVisibility(ESlateVisibility::Visible);

	if (WPauseMenu) {
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu) {
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (WHostileHealthBar) {
		HostileHealthBar = CreateWidget<UUserWidget>(this, WHostileHealthBar);
		if (HostileHealthBar) {
			HostileHealthBar->AddToViewport();
			HostileHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0.f, 0.f);
		HostileHealthBar->SetAlignmentInViewport(Alignment);
	}
}

void AMainPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (HostileHealthBar) {
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(HostileLocation, PositionInViewport);
		PositionInViewport.X -= 75.f;
		PositionInViewport.Y -= 100.f;

		FVector2D SizeInViewport(300.f, 20.f);

		HostileHealthBar->SetPositionInViewport(PositionInViewport);
		HostileHealthBar->SetDesiredSizeInViewport(SizeInViewport);
	}
}

void AMainPlayerController::DisplayHostileHealthBar() {
	if (HostileHealthBar) {
		bHostileHealthBarVisible = true;
		HostileHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveHostileHealthBar() {
	if (HostileHealthBar) {
		bHostileHealthBarVisible = false;
		HostileHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation() {
	if (PauseMenu) {
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);

		FInputModeGameAndUI InputModeGameAndUI;
		SetInputMode(InputModeGameAndUI);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation() {
	if (PauseMenu) {
		bPauseMenuVisible = false;

		GameModeOnly();
		bShowMouseCursor = false;
	}
}

void AMainPlayerController::TogglePauseMenu() {
	if (bPauseMenuVisible) {
		RemovePauseMenu();
	} else {
		DisplayPauseMenu();
	}
}

void AMainPlayerController::GameModeOnly() {
	FInputModeGameOnly InputModeGameOnly;
	SetInputMode(InputModeGameOnly);
}