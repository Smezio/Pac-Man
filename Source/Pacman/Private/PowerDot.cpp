// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerDot.h"

#include "PacmanPawn.h"

// Sets default values
APowerDot::APowerDot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetBoxExtent(FVector(6.0));
	BoxCollider->SetGenerateOverlapEvents(true);
	BoxCollider->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &APowerDot::GetEaten);

	ConstructorHelpers::FObjectFinder<UPaperSprite> SpriteFinder(TEXT("/Game/powerdot_Sprite.powerdot_Sprite"));
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));

	if (SpriteFinder.Succeeded()) {
		SpriteComponent->SetSprite(SpriteFinder.Object);
	}

	RootComponent = BoxCollider;
	SpriteComponent->SetupAttachment(BoxCollider);


	// Init properties
	Score = 50;

}

// Called when the game starts or when spawned
void APowerDot::BeginPlay()
{
	Super::BeginPlay();
	
}

void APowerDot::GetEaten(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if (dynamic_cast<APacmanPawn*>(OtherActor) == nullptr)
		return;

	BoxCollider->SetGenerateOverlapEvents(false);
	RootComponent->SetVisibility(false, true);
	
	TObjectPtr<APacmanGameMode> PacmanGameMode = CheckGameMode();
	if (PacmanGameMode != nullptr) {
		PacmanGameMode->AddScore(Score);
		PacmanGameMode->IncreaseDotCounter();

		// if not win
		PacmanGameMode->ActivePowerUp();
	}
}

TObjectPtr<APacmanGameMode> APowerDot::CheckGameMode() {
	TObjectPtr<AGameModeBase> GameMode = UGameplayStatics::GetGameMode(GetWorld());
	if (GameMode) {
		TObjectPtr<APacmanGameMode> PacmanGameMode = Cast<APacmanGameMode>(GameMode);

		if (PacmanGameMode) {
			return PacmanGameMode;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("PowerDot: Game Mode cannot be casted!"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("PowerDot: Game Mode not found!"));

	return nullptr;
}

