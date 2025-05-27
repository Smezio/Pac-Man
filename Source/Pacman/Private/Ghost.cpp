// Fill out your copyright notice in the Description page of Project Settings.


#include "Ghost.h"


// Sets default values
AGhost::AGhost()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxComponent->SetBoxExtent(FVector(4.0));
	BoxComponent->SetGenerateOverlapEvents(true);
	BoxComponent->SetCollisionProfileName(TEXT("Ghost"));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AGhost::ManageCollision);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetSprite(DefaultSprite);

	RootComponent = BoxComponent;
	SpriteComponent->SetupAttachment(BoxComponent);

	GameMode = nullptr;

	// Init properties
	Direction = FVector(1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 0.0f, 0.0f);
	CurrentNode = {};

	DefaultSpeed = 3.0f;
	Speed = DefaultSpeed;

	State = EGhostState::EGS_Idle;
	bIsEscaping = false;
	bWasEaten = false;
}

bool operator<(const AGhost& l, const AGhost& r) {
	return l.ActivationOrder < r.ActivationOrder;
}

// Called when the game starts or when spawned
void AGhost::BeginPlay()
{
	Super::BeginPlay();

	GameMode = (APacmanGameMode*) GetWorld()->GetAuthGameMode();

	Maze = (AMaze*)UGameplayStatics::GetActorOfClass(GetWorld(), AMaze::StaticClass());

	CurrentNode = Maze->GetGhostLocation(GhostType);
	InitNode = CurrentNode;

	ScatteringTarget = Maze->GetGhostScatteringTarget(GhostType);
	Target = ScatteringTarget;

	SetActorLocation(Maze->GetActorLocation() + CurrentNode.Location);
	Path = ComputePath(Target, FMazeNode());
	CurrentPathNode = Path.Num() - 1;
}

// Called every frame
void AGhost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GameMode->IsPaused()
		&& GameMode->IsPlaying()
		&& State != EGhostState::EGS_Idle) {

		// Change Sprite
		if (State == EGhostState::EGS_Escaping)
			SpriteComponent->SetSprite(EscapingSprite);
		else if (State == EGhostState::EGS_Eaten)
			SpriteComponent->SetSprite(EatenSprite);
		else
			SpriteComponent->SetSprite(DefaultSprite);

		// Change Speed
		if (State == EGhostState::EGS_Eaten)
			Speed = DefaultSpeed * 1.5f;
		else if (State == EGhostState::EGS_Walking)
			Speed = DefaultSpeed * 0.8f;
		else if (State == EGhostState::EGS_Escaping)
			Speed = DefaultSpeed / 2;
		else
			Speed = DefaultSpeed;

		// Compute movement
		FMazeNode NextNode = (Path.Num() > 0 ? Path[CurrentPathNode] : CurrentNode);

		FVector TargetWorldLocation = Maze->GetActorLocation() + NextNode.Location;
		FVector MovementOffset = Direction * (16 * Speed) * DeltaTime;

		FVector NextLocation = GetActorLocation() + MovementOffset;
		SetActorLocation(NextLocation, true);

		FVector RelativeLocationFromTarget = TargetWorldLocation - NextLocation;	
		
		// Compute path when next node is reached
		double CheckValue = FVector::DotProduct(RelativeLocationFromTarget, MovementOffset);
		if (CheckValue < 0) {

			SetActorLocation(TargetWorldLocation);

			// Manage teleport
			if (NextNode.Type == EMazeElement::EMA_Teleport) {
				if (NextNode.LeftNode->Type == EMazeElement::EMA_Teleport) { // from left to right
					SetActorLocation(Maze->GetActorLocation() + NextNode.LeftNode->Location);
				}
				if (NextNode.RightNode->Type == EMazeElement::EMA_Teleport) { // from right to left
					SetActorLocation(Maze->GetActorLocation() + NextNode.RightNode->Location);
				}
			}
			
			// Update nodes
			FMazeNode PrevNode = CurrentNode;
			Maze->SetNewLocation(GhostType, CurrentNode, NextNode);
			CurrentNode = NextNode;

			// If Room reached, change state
			if ((State == EGhostState::EGS_Eaten
				&& CurrentNode == Maze->GetRoomEntrance())
				|| (State == EGhostState::EGS_Walking
					&& CurrentNode == Maze->GetRoomExit())) {

				//State = GameMode->GetCurrentPhase(GhostType, State);
				State = GameMode->GetCurrentPhase(this);
				PrevNode = FMazeNode();
			}

			// If escaping, ghost must turn back as first movement
			if (State == EGhostState::EGS_Escaping)
				PrevNode = FMazeNode();

			// Compute path
			DecideTarget();
			Path = ComputePath(Target, PrevNode);
			CurrentPathNode = Path.Num() - 1;

			if (Path.Num() > 0)
				Direction = (Path[CurrentPathNode].Location - CurrentNode.Location).GetSafeNormal();
		}
	}
}

void AGhost::DecideTarget() {
	

	if (State == EGhostState::EGS_Idle) { // Idle phase
		Target = CurrentNode;
	}
	else if (State == EGhostState::EGS_Scattering) { // Scattering phase
		Target = ScatteringTarget;

		bIsEscaping = false;
		Speed = DefaultSpeed;
	}
	else if (State == EGhostState::EGS_Chasing) { // Chasing phase
		Target = Maze->GetGhostChasingTarget(GhostType);

		// Clyde chase Pacman if Pacman is 8 cell far away
		if (GhostType == EMazeElement::EMA_Clyde) {
			if (Path.Num() > 8)
				Target = ScatteringTarget;
			else
				Target = Maze->GetGhostChasingTarget(GhostType);
		}

		bIsEscaping = false;
		//Speed = DefaultSpeed;
	}
	else if (State == EGhostState::EGS_Escaping) { // Escaping phase

		//Speed = DefaultSpeed / 2;
		FMazeNode EscapingNode;

		if (!bIsEscaping) {
			EscapingNode = GetNodeByDirection(-Direction);
			bIsEscaping = true;
		}
		else {
			TArray<FMazeNode> FreeNodes = CurrentNode.GetFreeNodes();
			FMazeNode PreviousNode = GetNodeByDirection(-Direction);

			FreeNodes.Remove(PreviousNode);

			if (FreeNodes.Num() > 0) {
				int32 RandomIndex = FMath::RandHelper(FreeNodes.Num());
				EscapingNode = FreeNodes[RandomIndex];
			}
		}

		Target = EscapingNode;
	}
	else if (State == EGhostState::EGS_Eaten) {
		Target = Maze->GetRoomEntrance();

		//Speed = DefaultSpeed * 1.5f;
	}
	else if (State == EGhostState::EGS_Walking) {
		Target = Maze->GetRoomExit();

		//Speed = DefaultSpeed * 0.8f;
	}
}

void AGhost::ManageCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if (dynamic_cast<APacmanPawn*>(OtherActor) == nullptr)
		return;

	if (GameMode == nullptr)
		return;
	

	if (State == EGhostState::EGS_Escaping) {

		State = EGhostState::EGS_Eaten;
		bWasEaten = true;
		GameMode->AddGhostScore();
		
	}
	else if (State == EGhostState::EGS_Chasing
		|| State == EGhostState::EGS_Scattering) {

		GameMode->LoseLife();
	}
}

// Called to bind functionality to input
void AGhost::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Called to compute the path to target
TArray<FMazeNode> AGhost::ComputePath(FMazeNode TargetNode, FMazeNode PrevNode) {

	TSet<FPathNode> OpenList = {}; 
	OpenList.Add(FPathNode(CurrentNode, 0.0f, 0.0f));
	TSet<FPathNode> ClosedList = {};

	uint8 CostOfMove = 1;

	while (!OpenList.IsEmpty()) {

		FPathNode* Current = new FPathNode(*GetCheaperNode(OpenList));
		OpenList.Remove(*Current);
		ClosedList.Add(*Current);

		if (*(Current->Node) == TargetNode
			&& ClosedList.Num() > 1) {
			return GetResultingPath(ClosedList, Current);
		}

		TArray<FMazeNode*> AdjacentNodes = {
			Current->Node->UpperNode,
			Current->Node->LowerNode,
			Current->Node->RightNode,
			Current->Node->LeftNode,
		};

		for (auto AdjacentNode : AdjacentNodes) {

			if (AdjacentNode == nullptr) continue;

			if (AdjacentNode->Type == EMazeElement::EMA_Wall) continue;

			// Avoid to go into the room when ghosts don't need
			if (AdjacentNode->Type == EMazeElement::EMA_Room
				&& (State == EGhostState::EGS_Chasing
				|| State == EGhostState::EGS_Escaping
				|| State == EGhostState::EGS_Scattering))
				continue;

			// Avoid previous node of starting one
			if (!PrevNode.IsIsolated())
				if(*(Current->Node) == CurrentNode
					&& *AdjacentNode == PrevNode)
					continue;

			// Blocks ghosts to turn back during the path
			if (Current->CameFrom != nullptr)
				if (*AdjacentNode == *(Current->CameFrom->Node)) continue; // try to deference

			if (ClosedList.Contains({ *AdjacentNode })) continue;

			float CostG = Current->CostG + CostOfMove;
			float CostH = abs(AdjacentNode->Location.X - Target.Location.X) +
				abs(AdjacentNode->Location.Z - Target.Location.Z);
			float CostF = CostG + CostH;

			FPathNode NewNode = { *AdjacentNode, *Current, CostG, CostF };

			if (OpenList.Contains(NewNode)) {
				if (IsCostHighest(OpenList, NewNode.CostF))
					continue;
			}

			OpenList.Add(NewNode);
		}
	}


	return {};
}

FPathNode* AGhost::GetCheaperNode(TSet<FPathNode>& NodeSet) {
	FPathNode* ResultingNodeId = nullptr;
	float LeastCost = INFINITY;

	TSet<FPathNode>::TConstIterator Iterator = NodeSet.CreateConstIterator();
	
	while (Iterator) {

		FPathNode* Node = &(NodeSet.Get(Iterator.GetId()));

		if (Node->CostF < LeastCost) {
			ResultingNodeId = Node;
			LeastCost = Node->CostF;
		}

		++Iterator;
	}

	return ResultingNodeId;
}

bool AGhost::IsCostHighest(TSet<FPathNode>& NodeSet, float CostF) {
	TSet<FPathNode>::TConstIterator Iterator = NodeSet.CreateConstIterator();

	while (Iterator) {

		FPathNode* Node = &(NodeSet.Get(Iterator.GetId()));

		if (Node->CostF < CostF)
			return true;

		++Iterator;
	}

	return false;
}

TArray<FMazeNode> AGhost::GetResultingPath(TSet<FPathNode> NodeSet, FPathNode* TargetNode) {

	TArray<FMazeNode> Result = { };
	FPathNode* Temp = TargetNode;

	while (Temp->CameFrom != nullptr) {
		Result.Add(*(Temp->Node));
		Temp = Temp->CameFrom;
	}

	return Result;
}

EMazeElement AGhost::GetGhostType() const { return GhostType; }

EGhostState AGhost::GetState() const { return State; }
void AGhost::SetState(EGhostState NewState) { this->State = NewState; }

bool AGhost::WasEaten() const { return bWasEaten; }


FMazeNode AGhost::GetNodeByDirection(FVector RefDirection) {
	if (CurrentNode.IsIsolated())
		return CurrentNode;

	if (RefDirection == FVector::XAxisVector
		&& CurrentNode.RightNode != nullptr)
		return *(CurrentNode.RightNode);
	else if (RefDirection == -FVector::XAxisVector
		&& CurrentNode.LeftNode != nullptr)
		return *(CurrentNode.LeftNode);
	else if (RefDirection == FVector::ZAxisVector
		&& CurrentNode.UpperNode != nullptr)
		return *(CurrentNode.UpperNode);
	else if (RefDirection == -FVector::ZAxisVector
		&& CurrentNode.LowerNode != nullptr)
		return *(CurrentNode.LowerNode);

	return CurrentNode;
}

void AGhost::Reset() {
	Direction = FVector(1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 0.0f, 0.0f);
	Speed = DefaultSpeed;

	State = EGhostState::EGS_Idle;
	bIsEscaping = false;

	CurrentNode = InitNode;
	Target = ScatteringTarget;

	SetActorLocation(Maze->GetActorLocation() + CurrentNode.Location);
	Path = ComputePath(Target, FMazeNode());
	CurrentPathNode = Path.Num() - 1;
}