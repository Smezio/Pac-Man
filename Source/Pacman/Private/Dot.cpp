// Fill out your copyright notice in the Description page of Project Settings.


#include "Dot.h"

#include "PacmanPawn.h"

// Sets default values
ADot::ADot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxCollider->SetBoxExtent(FVector(2.0));
	BoxCollider->SetGenerateOverlapEvents(true);
	BoxCollider->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ADot::GetEaten);

	ConstructorHelpers::FObjectFinder<UPaperSprite> SpriteFinder(TEXT("/Game/dot_Sprite.dot_Sprite"));
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));

	if(SpriteFinder.Succeeded()) {
		SpriteComponent->SetSprite(SpriteFinder.Object);
	}

	RootComponent = BoxCollider;
	SpriteComponent->SetupAttachment(BoxCollider);	


	// Init properties
	Score = 10;
}

// Called when the game starts or when spawned
void ADot::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADot::GetEaten(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	
	
	if (dynamic_cast<APacmanPawn*>(OtherActor) == nullptr)
		return;

	BoxCollider->SetGenerateOverlapEvents(false);
	RootComponent->SetVisibility(false, true);
	
	TObjectPtr<APacmanGameMode> PacmanGameMode = CheckGameMode();
	if (PacmanGameMode != nullptr) {
		PacmanGameMode->AddScore(Score);
		PacmanGameMode->IncreaseDotCounter();
		
	}
}

TObjectPtr<APacmanGameMode> ADot::CheckGameMode() {
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