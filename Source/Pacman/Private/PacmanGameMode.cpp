// Fill out your copyright notice in the Description page of Project Settings.


#include "PacmanGameMode.h"

#include "Components/AudioComponent.h"
#include "Ghost.h"
#include "Fruit.h"
#include "UMG.h"


void APacmanGameMode::BeginPlay() {

	Super::BeginPlay();

	bStarting = true;
	bPlaying = false;
	bPaused = false;
	bPowerUp = false;

	Lives = 2;
	GameScore = 0;

	if(TotalDots <= 0)
		TotalDots = -1; // set after maze initialization
	DotsCounter = 0;

	FruitSeconds = 0;

	GameSeconds = 0;
	PowerUpSeconds = 0;
	GhostActivationInterval = 10;
	GhostCounter = 0;

	GhostStateTiming = {
		{7, EGhostState::EGS_Scattering},
		{20, EGhostState::EGS_Chasing},
		{7, EGhostState::EGS_Scattering},
		{20, EGhostState::EGS_Chasing},
		{5, EGhostState::EGS_Scattering},
		{20, EGhostState::EGS_Chasing},
		{5, EGhostState::EGS_Scattering}
	};
	GhostStateSeconds = 0;
	CurrentGhostState = 0;


	// Get all ghosts
	TArray<AActor*> GhostsActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhost::StaticClass(), GhostsActors);

	for (auto Ghost : GhostsActors) {
		AGhost* CastedGhost = dynamic_cast<AGhost*>(Ghost);
		if (CastedGhost != nullptr)
			Ghosts.Add(CastedGhost);
	}
	Ghosts.Sort();

	// Check world context
	if (GetWorld()) {
		// Start game timer
		GetWorld()->GetTimerManager().SetTimer(GameTimer, this, &APacmanGameMode::OnGameTick, 1.0f, true);
		
		// Init Ambient sound
		AmbientSound = (APacmanAmbientSound*) UGameplayStatics::GetActorOfClass(GetWorld(), AAmbientSound::StaticClass());
		AmbientSound->GetAudioComponent()->OnAudioFinished.AddDynamic(this, &APacmanGameMode::ManageAudio);
	
		// Init UI
		if (StartingWidgetClass != nullptr) {
			CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), StartingWidgetClass);
			CurrentWidget->AddToViewport();
		}
	}

}

void APacmanGameMode::OnGameTick() {
	// Play starting music
	if (bStarting
		&& !AmbientSound->IsPlaying()) {
		AmbientSound->PlayStart();
	}

	if (!bPlaying || bPaused)
		return;

	++GameSeconds;

	// Manage Ghosts activations
	if (GameSeconds > GhostActivationInterval - 10
		&& (GhostActivationInterval / 10) <= 4) {
		Ghosts[(GhostActivationInterval / 10) - 1]->SetState(EGhostState::EGS_Walking);
		GhostActivationInterval += 10;
	}

	// Manage Fruit lifecycle
	if (DotsCounter >= 70
		&& FruitSeconds == 0) {
		AFruit* Fruit = (AFruit*) UGameplayStatics::GetActorOfClass(GetWorld(), AFruit::StaticClass());

		if (Fruit != nullptr)
			Fruit->Active();
	}
	else if (DotsCounter >= 70
		&& FruitSeconds < 10) {
		++FruitSeconds;
	}
	else if (DotsCounter >= 70
		&& FruitSeconds >= 10) {
		AFruit* Fruit = (AFruit*)UGameplayStatics::GetActorOfClass(GetWorld(), AFruit::StaticClass());

		if (Fruit != nullptr)
			Fruit->Deactive();
	}

	// Manage Ghosts phases
	if (bPowerUp)
		return;
	
	++GhostStateSeconds;

	if (GhostStateSeconds >= GhostStateTiming[CurrentGhostState].Key) {

		UE_LOG(LogTemp, Log, TEXT("Change Ghost Status"));

		GhostStateSeconds = 0;
		++CurrentGhostState;

		if(GhostStateTiming.Num() > CurrentGhostState)
			UpdateGhostsStates(GhostStateTiming[CurrentGhostState].Value);
		else
			UpdateGhostsStates(EGhostState::EGS_Chasing);
	}
}


void APacmanGameMode::OnPowerUpTick() {
	if (!bPlaying || bPaused)
		return;

	++PowerUpSeconds;

	UE_LOG(LogTemp, Warning, TEXT("PowerUp timer: %d s"), PowerUpSeconds);

	if (PowerUpSeconds >= PowerUpDuration) {
		bPowerUp = false;
		PowerUpSeconds = 0.0f;
		GhostCounter = 0;

		UpdateGhostsStates(GhostStateTiming[CurrentGhostState].Value);

		GetWorld()->GetTimerManager().PauseTimer(PowerUpTimer);
	}
}

bool APacmanGameMode::IsPlaying() { return bPlaying; };
void APacmanGameMode::TogglePlay() { bPlaying = !bPlaying; };
bool APacmanGameMode::IsPaused() { return bPaused; };
void APacmanGameMode::TogglePause() { bPaused = !bPaused; };


// Called when Pacman is hit
void APacmanGameMode::LoseLife() {
	if (!bPlaying) return;

	bPlaying = false;
	--Lives;
	AmbientSound->PlayLose();
}

void APacmanGameMode::ResetGame() {

	bPlaying = true;
	bPaused = false;
	bPowerUp = false;

	GameSeconds = 0;
	PowerUpSeconds = 0;
	GhostCounter = 0;
	GhostActivationInterval = 10;

	GhostStateSeconds = 0;
	CurrentGhostState = 0;


	AMaze* Maze = (AMaze*) UGameplayStatics::GetActorOfClass(GetWorld(), AMaze::StaticClass());
	
	AActor* PacmanPawn = UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass());
	PacmanPawn->Reset();

	for (auto Ghost : Ghosts) {
		Ghost->Reset();
	}


	UTextBlock* ScoreTxt = (UTextBlock*)CurrentWidget->GetWidgetFromName("lives_num_txt");
	ScoreTxt->SetText(FText::FromString(FString::FromInt(Lives)));
		
}


// Called to add new points
void APacmanGameMode::AddScore(uint16 Score) {
	if (Score <= 0)
		return;

	GameScore += Score;
	UE_LOG(LogTemp, Log, TEXT("Updated score: %i"), GameScore);

	UTextBlock* ScoreTxt = (UTextBlock*)CurrentWidget->GetWidgetFromName("score_num_txt");
	ScoreTxt->SetText(FText::FromString(FString::FromInt(GameScore)));
}

// Called to add new points from Ghosts
void APacmanGameMode::AddGhostScore() {
	++GhostCounter;

	uint16 NewScore = 100 * FMath::Pow(2.0f, GhostCounter);
	AddScore(NewScore);
}

// Called to set new number of lives
void APacmanGameMode::SetLives(uint8 NumberOfLives) {
	if (NumberOfLives <= 0)
		return;

	Lives = NumberOfLives;

	UTextBlock* ScoreTxt = (UTextBlock*)CurrentWidget->GetWidgetFromName("lives_num_txt");
	ScoreTxt->SetText(FText::FromString(FString::FromInt(Lives)));
}

void APacmanGameMode::SetTotalDots(uint8 NumberOfDots) {
	if (NumberOfDots < 0)
		return;

	TotalDots = NumberOfDots;
}

void APacmanGameMode::IncreaseDotCounter() {

	++DotsCounter;

	if (DotsCounter == TotalDots) {
		bPlaying = false;
		UE_LOG(LogTemp, Warning, TEXT("WIN"));
		CurrentWidget->GetWidgetFromName("quit_btn")->SetVisibility(ESlateVisibility::Visible);
		CurrentWidget->GetWidgetFromName("retry_btn")->SetVisibility(ESlateVisibility::Visible);
		GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	}
}

// Called when Pacman eat PowerDot
void APacmanGameMode::ActivePowerUp() {
	if (bPowerUp)
		PowerUpSeconds = 0;

	bPowerUp = true;

	UpdateGhostsStates(EGhostState::EGS_Escaping);

	if (GetWorld()) {
		GetWorld()->GetTimerManager().SetTimer(PowerUpTimer, this, &APacmanGameMode::OnPowerUpTick, 1.0f, true);
	}
}

//EGhostState APacmanGameMode::GetCurrentPhase(EMazeElement Type = EMazeElement::EMA_Empty, EGhostState State = EGhostState::EGS_Idle) {
EGhostState APacmanGameMode::GetCurrentPhase(AGhost* Ghost) {

	if (Ghost->GetState() == EGhostState::EGS_Eaten)
		return EGhostState::EGS_Walking;

	if (bPowerUp
		&& !Ghost->WasEaten())
		return EGhostState::EGS_Escaping;
	
	if (CurrentGhostState >= GhostStateTiming.Num())
		return EGhostState::EGS_Chasing;

	// Blinky angry mode
	if (Ghost->GetGhostType() == EMazeElement::EMA_Blinky
		&& DotsCounter >= 20)
		return EGhostState::EGS_Chasing;

	return GhostStateTiming[CurrentGhostState].Value;
}

void APacmanGameMode::UpdateGhostsStates(EGhostState State) {
	for (auto Ghost : Ghosts) {

		if (Ghost->GetState() == EGhostState::EGS_Idle
			|| Ghost->GetState() == EGhostState::EGS_Eaten
			|| Ghost->GetState() == EGhostState::EGS_Walking)
			continue;

		if (bPowerUp) {
			Ghost->SetState(EGhostState::EGS_Escaping);
		}
		else {
			// If phases end, all ghosts will chase
			if (CurrentGhostState < GhostStateTiming.Num())
				Ghost->SetState(State);
			else
				Ghost->SetState(EGhostState::EGS_Chasing);
		}

		// Blinky angry mode
		if (Ghost->GetGhostType() == EMazeElement::EMA_Blinky
			&& DotsCounter >= 20
			&& State != EGhostState::EGS_Escaping)
			Ghost->SetState(EGhostState::EGS_Chasing);
	}
}


void APacmanGameMode::ManageAudio() {
	if (AmbientSound->GetSoundName() == "Start") {
		bStarting = false;
		bPlaying = true;
	}
	else if (AmbientSound->GetSoundName() == "Lose") {
		// Check Lose condition
		if (Lives <= 0) {
			UE_LOG(LogTemp, Warning, TEXT("LOSE"));
			CurrentWidget->GetWidgetFromName("quit_btn")->SetVisibility(ESlateVisibility::Visible);
			CurrentWidget->GetWidgetFromName("retry_btn")->SetVisibility(ESlateVisibility::Visible);
			GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
		}
		else {
			ResetGame();
		}
		
	}
};

void APacmanGameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget != nullptr)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}
	if (NewWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), NewWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}
}