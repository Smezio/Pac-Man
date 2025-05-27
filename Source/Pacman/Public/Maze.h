// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Paper2DClasses.h"
#include "Components/BoxComponent.h"
#include "PacmanGameMode.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Maze.generated.h"


UENUM()
enum class EMazeElement {
	EMA_Empty,
	EMA_Pacman,
	EMA_Blinky,
	EMA_Inky,
	EMA_Pinky,
	EMA_Clyde,
	EMA_Dot,
	EMA_PowerDot,
	EMA_Wall,
	EMA_Teleport,
	EMA_Room,
	EMA_Fruit,
};

USTRUCT()
struct FMazeNode {

	GENERATED_BODY()

	EMazeElement Type;
	FVector Location;

	FMazeNode* UpperNode;
	FMazeNode* LowerNode;
	FMazeNode* RightNode;
	FMazeNode* LeftNode;

	FMazeNode() {
		this->Type = EMazeElement::EMA_Empty;
		this->Location = FVector(0.0);

		this->UpperNode = nullptr;
		this->LowerNode = nullptr;
		this->RightNode = nullptr;
		this->LeftNode = nullptr;
	};

	FMazeNode(EMazeElement Type, FVector Location) {
		this->Type = Type;
		this->Location = Location;

		this->UpperNode = nullptr;
		this->LowerNode = nullptr;
		this->RightNode = nullptr;
		this->LeftNode = nullptr;
	};

	bool operator==(const FMazeNode& Other) const
	{
		return Location == Other.Location;
	}

	TArray<FMazeNode*> GetAdjacentNodes() {
		return TArray<FMazeNode*>({ UpperNode, LowerNode, RightNode, LeftNode });
	};

	bool IsIsolated() {
		if (RightNode == nullptr
			&& LeftNode == nullptr
			&& UpperNode == nullptr
			&& LowerNode == nullptr)
			return true;

		return false;
	}

	TArray<FMazeNode> GetFreeNodes() {
		TArray<FMazeNode> Nodes = {};

		if (RightNode->Type != EMazeElement::EMA_Wall
			&& RightNode->Type != EMazeElement::EMA_Room)
			Nodes.Add(*(RightNode));
		if (LeftNode->Type != EMazeElement::EMA_Wall
			&& LeftNode->Type != EMazeElement::EMA_Room)
			Nodes.Add(*(LeftNode));
		if (UpperNode->Type != EMazeElement::EMA_Wall
			&& UpperNode->Type != EMazeElement::EMA_Room)
			Nodes.Add(*(UpperNode));
		if (LowerNode->Type != EMazeElement::EMA_Wall
			&& LowerNode->Type != EMazeElement::EMA_Room)
			Nodes.Add(*(LowerNode));

		return Nodes;
	}
};

UCLASS()
class PACMAN_API AMaze : public AActor
{
	GENERATED_BODY()

public:
	const uint8 WallSpriteSize = 16;

	UPROPERTY(VisibleAnywhere, Category = "Maze")
	uint8 NumberOfRows;
	UPROPERTY(VisibleAnywhere, Category = "Maze")
	uint8 NumberOfColumns;

protected:
	TArray<FMazeNode> MazeSchema;
	TArray<FMazeNode> OriginalMazeSchema;

	FMazeNode PacmanLocation;
	FMazeNode BlinkyLocation;
	FMazeNode InkyLocation;
	FMazeNode PinkyLocation;
	FMazeNode ClydeLocation;

	FMazeNode RoomEntrance;
	FMazeNode RoomExit;
	
public:	
	// Sets default values for this actor's properties
	AMaze();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to fetch maze schema from a file
	TArray<FMazeNode> ReadMazeSchema();
	
	// Called to instantiate maze elements in the world
	void GenerateMaze();

	// Check GameMode
	TObjectPtr<APacmanGameMode> CheckGameMode();

	// Called to assign the close node
	void AssignAdjacentNode(int Index, FMazeNode*& CloseNode);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to set new position of Characters
	void SetNewLocation(EMazeElement Type, FMazeNode& PrevNode, FMazeNode& NextNode);
	
	// Called to get Pacman location
	FMazeNode GetPacmanLocation();

	// Called to get Ghost location
	FMazeNode GetGhostLocation(EMazeElement Type);

	// Called to get scattering target
	FMazeNode GetGhostScatteringTarget(EMazeElement Type);

	// Called to get chasing target
	FMazeNode GetGhostChasingTarget(EMazeElement Type);

	// Called to get Room entrance
	FMazeNode GetRoomEntrance();

	// Called to get Room exit
	FMazeNode GetRoomExit();
};
