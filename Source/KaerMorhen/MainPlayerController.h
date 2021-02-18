// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class KAERMORHEN_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Reference to the UMG asset in the Editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<class UUserWidget> HUD_OverlayAsset;

	/** Variable which holds the widgets after creation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* HUD_Overlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* PauseMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets)
	TSubclassOf<UUserWidget> WHostileHealthBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Widgets)
	UUserWidget* HostileHealthBar;

	FVector HostileLocation;

	bool bHostileHealthBarVisible;

	void DisplayHostileHealthBar();

	void RemoveHostileHealthBar();

	bool bPauseMenuVisible;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = HUD)
	void DisplayPauseMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = HUD)
	void RemovePauseMenu();

	void TogglePauseMenu();

	void GameModeOnly();

protected:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
};
