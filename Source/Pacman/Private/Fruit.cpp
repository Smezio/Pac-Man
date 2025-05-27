// Fill out your copyright notice in the Description page of Project Settings.


#include "Fruit.h"

#include "PacmanPawn.h"

// Sets default values
AFruit::AFruit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetBoxExtent(FVector(2.0));
	BoxCollider->SetGenerateOverlapEvents(true);
	BoxCollider->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &AFruit::GetEaten);

	ConstructorHelpers::FObjectFinder<UPaperSprite> SpriteFinder(TEXT("/Game/fruit_Sprite.fruit_Sprite"));
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));

	if (SpriteFinder.Succeeded()) {
		SpriteComponent->SetSprite(SpriteFinder.Object);
	}

	RootComponent = BoxCollider;
	SpriteComponent->SetupAttachment(BoxCollider);

	// Init properties
	bIsActive = false;
	bIsEaten = false;
	Score = 300;

}

// Called when the game starts or when spawned
void AFruit::BeginPlay()
{
	Super::BeginPlay();

	RootComponent->SetVisibility(false, true);
}

void AFruit::Active() {
	if (bIsEaten) return;

	RootComponent->SetVisibility(true, true);
	bIsActive = true;
}

void AFruit::Deactive() {
	RootComponent->SetVisibility(false, true);
	bIsActive = false;
	bIsEaten = false;
}

void AFruit::GetEaten(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if (dynamic_cast<APacmanPawn*>(OtherActor) == nullptr
		|| !bIsActive)
		return;

	if (bIsEaten) return;

	BoxCollider->SetGenerateOverlapEvents(false);
	RootComponent->SetVisibility(false, true);

	TObjectPtr<APacmanGameMode> PacmanGameMode = CheckGameMode();
	if (PacmanGameMode != nullptr) {
		PacmanGameMode->AddScore(Score);
		bIsEaten = true;
	}
}

TObjectPtr<APacmanGameMode> AFruit::CheckGameMode() {
	TObjectPtr<AGameModeBase> GameMode = UGameplayStatics::GetGameMode(GetWorld());
	if (GameMode) {
		TObjectPtr<APacmanGameMode> PacmanGameMode = Cast<APacmanGameMode>(GameMode);

		if (PacmanGameMode) {
			return PacmanGameMode;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Fruit: Game Mode cannot be casted!"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Fruit: Game Mode not found!"));

	return nullptr;
}

