// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Paper2DClasses.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameMode.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fruit.generated.h"

UCLASS()
class PACMAN_API AFruit : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Fruit")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;
	UPROPERTY(EditAnywhere, Category = "Fruit")
	TObjectPtr<UBoxComponent> BoxCollider;

	bool bIsActive;
	bool bIsEaten;
	uint16 Score;
	
public:	
	// Sets default values for this actor's properties
	AFruit();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Check GameMode
	virtual TObjectPtr<APacmanGameMode> CheckGameMode();

public:	
	// Called to active fruit
	void Active();
	// Called to deactive fruit
	void Deactive();

	// Callback for collision with Pacman
	UFUNCTION()
	void GetEaten(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
