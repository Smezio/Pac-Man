// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

#include "CoreMinimal.h"
#include "Sound/AmbientSound.h"
#include "PacmanAmbientSound.generated.h"

/**
 * 
 */
UCLASS()
class PACMAN_API APacmanAmbientSound : public AAmbientSound
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Sounds");
	TObjectPtr<USoundCue> StartSound;

	UPROPERTY(EditAnywhere, Category = "Sounds");
	TObjectPtr<USoundCue> LoseSound;

public:

	APacmanAmbientSound();

	virtual void BeginPlay() override;

	// Called to play starting music
	void PlayStart();

	// Called to play starting music
	void PlayLose();

	// Called to get audio execution
	bool IsPlaying();

	// Called to get audio name
	FString GetSoundName();
};
