// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Paper2D/Classes/PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanPawn.h"
#include "PacmanGameMode.h"
#include "Maze.h"

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Ghost.generated.h"


USTRUCT()
struct FPathNode {

	GENERATED_BODY()

	FMazeNode* Node;
	FPathNode* CameFrom;
	double CostG;
	double CostF;

	FPathNode() {
		Node = nullptr;
		CameFrom = nullptr;
		CostG = 0.0;
		CostF = 0.0;
	}

	FPathNode(FMazeNode& Node) {
		this->Node = &Node;
		CameFrom = nullptr;
		CostG = 0.0;
		CostF = 0.0;
	}

	FPathNode(FMazeNode& Node, double CostG, double CostF) {
		this->Node = &Node;
		this->CameFrom = nullptr;
		this->CostG = CostG;
		this->CostF = CostF;
	}

	FPathNode(FMazeNode& Node, FPathNode& CameFrom, double CostG, double CostF) {
		this->Node = &Node;
		this->CameFrom = &CameFrom;
		this->CostG = CostG;
		this->CostF = CostF;
	}

	bool operator==(const FPathNode& Other) const
	{
		return Node == Other.Node;
	}

	friend uint32 GetTypeHash(const FPathNode& PathNode) {
		return GetTypeHash(PathNode.Node->Location);
	}

};

UCLASS()
class PACMAN_API AGhost : public APawn
{
	GENERATED_BODY()

public:
	/* Components */
	UPROPERTY(VisibleAnywhere, Instanced)
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(VisibleAnywhere, Category = "Ghost", Instanced)
	TObjectPtr <UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	TObjectPtr<UPaperSprite> DefaultSprite;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	TObjectPtr<UPaperSprite> EscapingSprite;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	TObjectPtr<UPaperSprite> EatenSprite;

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
	FVector Direction;
	FRotator Rotation;

	FMazeNode InitNode;
	FMazeNode CurrentNode;

	UPROPERTY(EditAnywhere, Category = "Ghost", meta = (ValidEnumValues = "EMA_Blinky, EMA_Inky, EMA_Pinky, EMA_Clyde"))
	EMazeElement GhostType;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	float DefaultSpeed;
	float Speed;

	EGhostState State;
	bool bIsEscaping;
	bool bWasEaten; // avoid escaping after being eaten

	FMazeNode Target;
	FMazeNode ScatteringTarget;

	TArray<FMazeNode> Path;
	int CurrentPathNode;

	// Use to define the order of activation during the game
	UPROPERTY(EditAnywhere, Category = "Ghost")
	uint8 ActivationOrder;

public:
	// Sets default values for this pawn's properties
	AGhost();

	// Called for ActivationOrder comparison
	friend bool operator<(const AGhost& l, const AGhost& r);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called to set the target, regarding the state
	void DecideTarget();

	// Callback function for collisions
	UFUNCTION()
	void ManageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called to compute the path to target
	TArray<FMazeNode> ComputePath(FMazeNode TargetNode, FMazeNode PrevNode);
	// Util function for path computing
	FPathNode* GetCheaperNode(TSet<FPathNode>& NodeSet);
	bool IsCostHighest(TSet<FPathNode>& NodeSet, float CostF);
	TArray<FMazeNode> GetResultingPath(TSet<FPathNode> NodeSet, FPathNode* TargetNode);
	
	EMazeElement GetGhostType() const;
	EGhostState GetState() const;
	void SetState(EGhostState State);

	bool WasEaten() const;

	// Called to get the node through a given direction
	FMazeNode GetNodeByDirection(FVector RefDirection);

	// Called to reset the ghost's properties
	void Reset();
};
