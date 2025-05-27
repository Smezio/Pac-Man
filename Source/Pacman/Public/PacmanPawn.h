// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Paper2D/Classes/PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Maze.h"


#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"



UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> InputContext;

	UPROPERTY(VisibleAnywhere, Category = "Pacman")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pacman")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

protected:
	APacmanGameMode* GameMode;
	TObjectPtr<AMaze> Maze;
	/*
	*	2D directions
	*	Right: (1, 0)
	*	Up: (0, 1)
	*	Left: (-1, 0)
	*	Down: (0, -1)
	*/
	FVector CurrentDirection;
	FVector Direction;
	FRotator Rotation;

	FMazeNode CurrentNode;
	FMazeNode InitNode;
	FMazeNode NextNode;

	UPROPERTY(EditAnywhere, Category = "Pacman")
	float Speed;

public:
	// Sets default values for this pawn's properties
	APacmanPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Callback function for input
	virtual void OnMoveUp();
	virtual void OnMoveDown();
	virtual void OnMoveRight();
	virtual void OnMoveLeft();
	virtual void OnPause();

	// Called to retrieve two nodes in front of Pacman
	FMazeNode GetTwoFrontLocation();

	// Called to reset Pacman's properties
	void Reset();
};
