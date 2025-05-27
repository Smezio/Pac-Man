// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PacmanAmbientSound.h"
#include "Blueprint/UserWidget.h"

class AGhost;
enum class EMazeElement;

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameMode.generated.h"


UENUM()
enum class EGhostState {
	EGS_Idle,
	EGS_Walking,
	EGS_Chasing,
	EGS_Scattering,
	EGS_Escaping,
	EGS_Eaten,
}; 

/**
 * 
 */
UCLASS()
class PACMAN_API APacmanGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	bool bStarting;
	bool bPlaying;
	bool bPaused;
	bool bPowerUp;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint8 Lives;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 GameScore;
	uint8 TotalDots;
	uint8 DotsCounter;

	uint8 FruitSeconds;
	uint8 FruitDuration = 10;

	FTimerHandle GameTimer;
	uint32 GameSeconds;

	TArray<TPair<uint8, EGhostState>> GhostStateTiming;
	uint32 GhostStateSeconds;
	uint8 CurrentGhostState;

	FTimerHandle PowerUpTimer;
	uint8 PowerUpSeconds;
	const uint8 PowerUpDuration = 10;

	TArray<AGhost*> Ghosts;
	uint32 GhostActivationInterval;
	uint8 GhostCounter;

	TObjectPtr<APacmanAmbientSound> AmbientSound;

	UPROPERTY()
	UUserWidget* CurrentWidget;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartingWidgetClass;
	
public:
	virtual void BeginPlay() override;

	// Called every GameTimer's second
	void OnGameTick();

	// Called every PowerUp's second
	void OnPowerUpTick();
	
	// Called to manage game status
	bool IsPlaying();
	void TogglePlay();
	bool IsPaused();
	void TogglePause();

	// Called when Pacman is hit
	void LoseLife();

	// Called to load new match
	void ResetGame();

	// Called to add new points
	void AddScore(uint16 Score);

	// Called to add new points from Ghosts
	void AddGhostScore();

	// Called to set new number of lives
	void SetLives(uint8 NumberOfLives);

	// Called to set initial dots' counter value
	void SetTotalDots(uint8 NumberOfDots);

	// Called after Pacman eats a dot
	void IncreaseDotCounter();

	// Called when Pacman eat PowerDot
	void ActivePowerUp();

	// Called to retrieve current phase
	EGhostState GetCurrentPhase(AGhost* Ghost);

	// Called to update ghosts states
	void UpdateGhostsStates(EGhostState State);

	// Callback function to manage audio
	UFUNCTION()
	void ManageAudio();

	// Called to manage UI
	UFUNCTION()
	void ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass);
};
