// Fill out your copyright notice in the Description page of Project Settings.


#include "PacmanAmbientSound.h"

APacmanAmbientSound::APacmanAmbientSound() {
	
	PrimaryActorTick.bCanEverTick = true;

}

void APacmanAmbientSound::BeginPlay() {
	
	Super::BeginPlay();

	if (StartSound == nullptr)
		UE_LOG(LogTemp, Error, TEXT("PacmanAmbientSound: StartSound must be set!"));

	if (LoseSound == nullptr)
		UE_LOG(LogTemp, Error, TEXT("PacmanAmbientSound: StartSound must be set!"));

	
	GetAudioComponent()->SetSound(StartSound);
}

void APacmanAmbientSound::PlayStart() {
	GetAudioComponent()->SetSound(StartSound);
	Play();
}

void APacmanAmbientSound::PlayLose() {
	GetAudioComponent()->SetSound(LoseSound);
	Play();
}

bool APacmanAmbientSound::IsPlaying() {
	return GetAudioComponent()->IsPlaying();
}

FString APacmanAmbientSound::GetSoundName() {
	if (GetAudioComponent()->GetSound() == StartSound)
		return "Start";
	else if (GetAudioComponent()->GetSound() == LoseSound)
		return "Lose";

	return "None";
}