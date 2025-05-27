// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Paper2DClasses.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameMode.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dot.generated.h"

UCLASS()
class PACMAN_API ADot : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Dot")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;
	UPROPERTY(EditAnywhere, Category = "Dot")
	TObjectPtr<UBoxComponent> BoxCollider;

	uint16 Score;
	
public:	
	// Sets default values for this actor's properties
	ADot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Check GameMode
	TObjectPtr<APacmanGameMode> CheckGameMode();

public:
	// Callback for collision with Pacman
	UFUNCTION()
	void GetEaten(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
