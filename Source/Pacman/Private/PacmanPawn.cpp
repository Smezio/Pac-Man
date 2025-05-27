// Fill out your copyright notice in the Description page of Project Settings.


#include "PacmanPawn.h"

#include "Maze.h"

// Sets default values
APacmanPawn::APacmanPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	BoxComponent->SetBoxExtent(FVector(4.0));
	BoxComponent->SetGenerateOverlapEvents(true);
	BoxComponent->SetCollisionProfileName(TEXT("Pacman"));

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));

	RootComponent = BoxComponent;
	SpriteComponent->SetupAttachment(BoxComponent);

	GameMode = nullptr;

	// Init properties
	Direction = FVector(1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 0.0f, 0.0f);
	Speed = 4.0f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set InputMappingContext
	TObjectPtr<APlayerController> PlayerController = Cast<APlayerController>(GetController());
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

	if (Subsystem && InputContext)
		Subsystem->AddMappingContext(InputContext, 0);

	GameMode = (APacmanGameMode*) GetWorld()->GetAuthGameMode();

	Maze = (AMaze*)UGameplayStatics::GetActorOfClass(GetWorld(), AMaze::StaticClass());

	CurrentNode = Maze->GetPacmanLocation();
	InitNode = CurrentNode;
	NextNode = *(CurrentNode.RightNode);
	CurrentDirection = FVector::XAxisVector;
	SetActorLocation(Maze->GetActorLocation() + Maze->GetPacmanLocation().Location);
}

// Called every frame
void APacmanPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GameMode->IsPlaying()
		&& !GameMode->IsPaused()) {

		// Compute movement
		FVector TargetWorldLocation = Maze->GetActorLocation() + NextNode.Location;
		FVector MovementOffset = CurrentDirection * (16 * Speed) * DeltaTime;

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
			Maze->SetNewLocation(EMazeElement::EMA_Pacman, CurrentNode, NextNode);
			CurrentNode = NextNode;
			
			// Change direction if path node is available
			if (Direction == FVector::XAxisVector
				&& CurrentNode.RightNode->Type != EMazeElement::EMA_Wall) {
				NextNode = *(CurrentNode.RightNode);
				CurrentDirection = Direction;
				SpriteComponent->SetRelativeRotation(Rotation);
			}
			else if (Direction == -FVector::XAxisVector
				&& CurrentNode.LeftNode->Type != EMazeElement::EMA_Wall) {
				NextNode = *(CurrentNode.LeftNode);
				CurrentDirection = Direction;
				SpriteComponent->SetRelativeRotation(Rotation);
			}
			else if (Direction == FVector::ZAxisVector
				&& CurrentNode.UpperNode->Type != EMazeElement::EMA_Wall) {
				NextNode = *(CurrentNode.UpperNode);
				CurrentDirection = Direction;
				SpriteComponent->SetRelativeRotation(Rotation);
			}
			else if (Direction == -FVector::ZAxisVector
				&& CurrentNode.LowerNode->Type != EMazeElement::EMA_Wall
				&& CurrentNode.LowerNode->Type != EMazeElement::EMA_Room) {
				NextNode = *(CurrentNode.LowerNode);
				CurrentDirection = Direction;
				SpriteComponent->SetRelativeRotation(Rotation);
			}
			else {
				// Continue for the current direction until next crossing
				if (CurrentDirection == FVector::XAxisVector
					&& CurrentNode.RightNode->Type != EMazeElement::EMA_Wall) {
					NextNode = *(CurrentNode.RightNode);
				}
				else if (CurrentDirection == -FVector::XAxisVector
					&& CurrentNode.LeftNode->Type != EMazeElement::EMA_Wall) {
					NextNode = *(CurrentNode.LeftNode);
				}
				else if (CurrentDirection == FVector::ZAxisVector
					&& CurrentNode.UpperNode->Type != EMazeElement::EMA_Wall) {
					NextNode = *(CurrentNode.UpperNode);
				}
				else if (CurrentDirection == -FVector::ZAxisVector
					&& CurrentNode.LowerNode->Type != EMazeElement::EMA_Wall
					&& CurrentNode.LowerNode->Type != EMazeElement::EMA_Room) {
					NextNode = *(CurrentNode.LowerNode);
				}
			}
		}
	}
}

// Called to bind functionality to input
void APacmanPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent && InputContext) {
		for (const FEnhancedActionKeyMapping& Action : InputContext->GetMappings()) {
			
			if (Action.Action) {
				FName ActionName = Action.Action->GetFName();

				if (ActionName == "MoveR") {
					EnhancedInputComponent->BindAction(Action.Action, ETriggerEvent::Triggered, this, &APacmanPawn::OnMoveRight);
				}
				else if (ActionName == "MoveL") {
					EnhancedInputComponent->BindAction(Action.Action, ETriggerEvent::Triggered, this, &APacmanPawn::OnMoveLeft);
				}
				else if (ActionName == "MoveU") {
					EnhancedInputComponent->BindAction(Action.Action, ETriggerEvent::Triggered, this, &APacmanPawn::OnMoveUp);
				}
				else if (ActionName == "MoveD") {
					EnhancedInputComponent->BindAction(Action.Action, ETriggerEvent::Triggered, this, &APacmanPawn::OnMoveDown);
				}
				else if (ActionName == "Pause") {
					EnhancedInputComponent->BindAction(Action.Action, ETriggerEvent::Triggered, this, &APacmanPawn::OnPause);
				}
			}
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APacmanPawn::OnMoveUp() {
	Direction.Set(0.0f, 0.0f, 1.0f);
	Rotation = FRotator(90.0f, 0.0f, 0.0f);
}

void APacmanPawn::OnMoveDown() {
	Direction.Set(0.0f, 0.0f, -1.0f);
	Rotation = FRotator(-90.0f, 0.0f, 0.0f);
}

void APacmanPawn::OnMoveRight() {
	Direction.Set(1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 0.0f, 0.0f);
}

void APacmanPawn::OnMoveLeft() {
	Direction.Set(-1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 180.0f, 0.0f);
}

void APacmanPawn::OnPause() {
	GameMode->TogglePause();
}


// Called to retrieve two nodes in front of Pacman
FMazeNode APacmanPawn::GetTwoFrontLocation() {
	FMazeNode FrontNode = CurrentNode;

	for (int i = 0; i < 2; i++) {
		if (CurrentDirection == FVector::XAxisVector) {
			if (FrontNode.RightNode != nullptr
				&& FrontNode.RightNode->Type != EMazeElement::EMA_Wall)
				FrontNode = *(FrontNode.RightNode);
			else
				return FrontNode;
		}
		else if (CurrentDirection == -FVector::XAxisVector) {
			if (FrontNode.LeftNode != nullptr
				&& FrontNode.LeftNode->Type != EMazeElement::EMA_Wall)
				FrontNode = *(FrontNode.LeftNode);
			else
				return FrontNode;
		}
		else if (CurrentDirection == FVector::ZAxisVector) {
			if (FrontNode.UpperNode != nullptr
				&& FrontNode.UpperNode->Type != EMazeElement::EMA_Wall)
				FrontNode = *(FrontNode.UpperNode);
			else
				return FrontNode;
		}
		else if (CurrentDirection == -FVector::ZAxisVector) {
			if (FrontNode.LowerNode != nullptr
				&& FrontNode.LowerNode->Type != EMazeElement::EMA_Wall)
				FrontNode = *(FrontNode.LowerNode);
			else
				return FrontNode;
		}
	}

	return FrontNode;
}


// Called to reset Pacman's properties
void APacmanPawn::Reset() {
	Direction = FVector(1.0f, 0.0f, 0.0f);
	Rotation = FRotator(0.0f, 0.0f, 0.0f);

	CurrentNode = InitNode;
	NextNode = *(CurrentNode.RightNode);
	CurrentDirection = FVector::XAxisVector;
	SetActorLocation(Maze->GetActorLocation() + InitNode.Location);
}