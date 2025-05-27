// Fill out your copyright notice in the Description page of Project Settings.


#include "Maze.h"

#include "Kismet/GameplayStatics.h"
#include "Dot.h"
#include "PowerDot.h"
#include "PacmanPawn.h"
#include "Fruit.h"


// Sets default values
AMaze::AMaze()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default maze size
	NumberOfRows = 31;
	NumberOfColumns = 28;

	PacmanLocation = {};
	BlinkyLocation = {};

	MazeSchema = ReadMazeSchema();

	if (MazeSchema.Num() > 0) {
		UE_LOG(LogTemp, Warning, TEXT("Maze: schema created successfully!"));
		
		if (MazeSchema.Num() == NumberOfColumns * NumberOfRows)
			GenerateMaze();
		else
			UE_LOG(LogTemp, Error, TEXT("Maze: size of schema doesn't correspond to given size! Check the file!"));
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Maze: schema creation failed!"));
	}
}

// Called when the game starts or when spawned
void AMaze::BeginPlay()
{
	Super::BeginPlay();

	uint8 DotsCounter = 0;

	if (GetWorld()) {
		// Instantiate dynamic actors
		for (const FMazeNode Element : MazeSchema) {
			if (Element.Type == EMazeElement::EMA_Dot) {
				GetWorld()->SpawnActor<ADot>(GetActorLocation() + Element.Location, FRotator::ZeroRotator);
				++DotsCounter;
			}
			else if (Element.Type == EMazeElement::EMA_PowerDot) {
				GetWorld()->SpawnActor<APowerDot>(GetActorLocation() + Element.Location, FRotator::ZeroRotator);
				++DotsCounter;
			}
			else if (Element.Type == EMazeElement::EMA_Fruit) {
				GetWorld()->SpawnActor<AFruit>(GetActorLocation() + Element.Location, FRotator::ZeroRotator);
			}
		}
	}

	TObjectPtr<APacmanGameMode> PacmanGameMode = CheckGameMode();
	if (PacmanGameMode != nullptr) {
		PacmanGameMode->SetTotalDots(DotsCounter);
	}
	
}

// Called every frame
void AMaze::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<FMazeNode> AMaze::ReadMazeSchema() {
	TArray<FMazeNode> ComputedSchema;
	ComputedSchema.Init({}, 0);

	// Setup file and file manager
	FString MazeFile = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Maze"), TEXT("maze_schema.txt"));
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	TArray<uint8> FileContent;

	if (!FileManager.FileExists(*MazeFile)) {
		UE_LOG(LogTemp, Warning, TEXT("Maze: Maze structure file not found"));
		return ComputedSchema;
	}

	if (!FFileHelper::LoadFileToArray(FileContent, *MazeFile)) {
		UE_LOG(LogTemp, Warning, TEXT("Maze: File not loaded"));
		return ComputedSchema;
	}
	
	// Creating new node
	int Counter = 0;
	for (const uint8 Element : FileContent) {

		// Computing location of element
		const uint8 Column = Counter % NumberOfColumns;
		const uint8 Row = Counter / NumberOfColumns;
		const FVector Location = FVector(static_cast<int32>(Column * WallSpriteSize), 0, -static_cast<int32>(Row * WallSpriteSize));
		
		// Populating Schema
		TCHAR Symbol = static_cast<TCHAR>(Element);

		switch (Symbol) {
		case '.':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Empty, Location));
			++Counter;
			break;

		case '|':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Wall, Location));
			++Counter;
			break;

		case 'o':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Dot, Location));
			++Counter;
			break;

		case 'u':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_PowerDot, Location));
			++Counter;
			break;

		case 'f':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Fruit, Location));
			++Counter;
			break;

		case 'r':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Room, Location));
			++Counter;
			break;

		case 't':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Teleport, Location));
			++Counter;
			break;

		case '*':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Pacman, Location));
			++Counter;
			break;

		case 'b':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Blinky, Location));
			++Counter;
			break;

		case 'i':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Inky, Location));
			++Counter;
			break;

		case 'p':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Pinky, Location));
			++Counter;
			break;

		case 'c':
			ComputedSchema.Add(FMazeNode(EMazeElement::EMA_Clyde, Location));
			++Counter;
			break;

		default:
			UE_LOG(LogTemp, Warning, TEXT("Maze: Symbol in schema not managed! Symbol: %c"), Symbol);
			break;
		}
	}

	return ComputedSchema;
}

void AMaze::GenerateMaze() {
	static ConstructorHelpers::FObjectFinder<UPaperSprite> SpriteFinder(TEXT("/Game/wall_Sprite.wall_Sprite"));

	if (!SpriteFinder.Succeeded())
		return;

	TObjectPtr<UBoxComponent> SceneComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("MazeRoot"), true);
	SceneComponent->SetBoxExtent(FVector::Zero());

	FMazeNode* RoomNode = nullptr;
	FMazeNode* LeftTeleport = nullptr;
	FMazeNode* RightTeleport = nullptr;

	// Managing schema's static elements
	// and linking near node
	for (int i = 0; i < MazeSchema.Num(); i++) {

		TArray<int> CloseNodesIndices = {
			i - NumberOfColumns,
			i + NumberOfColumns,
			i + 1,
			i - 1
		};

		AssignAdjacentNode(CloseNodesIndices[0], MazeSchema[i].UpperNode);
		AssignAdjacentNode(CloseNodesIndices[1], MazeSchema[i].LowerNode);
		AssignAdjacentNode(CloseNodesIndices[2], MazeSchema[i].RightNode);
		AssignAdjacentNode(CloseNodesIndices[3], MazeSchema[i].LeftNode);
		

		// Create static elements of the Maze as Components
		if (MazeSchema[i].Type == EMazeElement::EMA_Wall) { // Wall
			FString SpriteComponentName = "WallSprite_" + FString::FromInt(i);
			TObjectPtr<UPaperSpriteComponent> SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(FName(*SpriteComponentName), true);
			FString BoxComponentName = "WallCollider_" + FString::FromInt(i);
			TObjectPtr<UBoxComponent> BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName(*BoxComponentName), true);

			if (!SpriteComponent || !BoxComponent)
				return;

			BoxComponent->SetRelativeLocation(MazeSchema[i].Location);
			BoxComponent->SetBoxExtent(FVector((WallSpriteSize / 2) - 2));
			BoxComponent->SetGenerateOverlapEvents(false);
			BoxComponent->SetCollisionProfileName(TEXT("BlockAll"));

			SpriteComponent->SetSprite(SpriteFinder.Object);
			SpriteComponent->SetupAttachment(BoxComponent);
			BoxComponent->SetupAttachment(SceneComponent);
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Room) { // Room
			FString BoxComponentName = "RoomCollider_" + FString::FromInt(i);
			TObjectPtr<UBoxComponent> BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName(*BoxComponentName), true);
			
			BoxComponent->SetRelativeLocation(MazeSchema[i].Location);
			BoxComponent->SetBoxExtent(FVector((WallSpriteSize / 2) - 2));
			BoxComponent->SetGenerateOverlapEvents(false);
			BoxComponent->SetCollisionProfileName(TEXT("Room"));
			BoxComponent->SetupAttachment(SceneComponent);

			RoomNode = &MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Teleport) {
			if (LeftTeleport == nullptr
				&& RightTeleport == nullptr)
				LeftTeleport = &MazeSchema[i];
			else
				RightTeleport = &MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Pacman) { // Pacman
			PacmanLocation = MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Blinky) { // Blinky Ghost
			BlinkyLocation = MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Inky) { // Inky Ghost
			InkyLocation = MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Pinky) { // Pinky Ghost
			PinkyLocation = MazeSchema[i];
		}
		else if (MazeSchema[i].Type == EMazeElement::EMA_Clyde) { // Clyde Ghost
			ClydeLocation = MazeSchema[i];
		}
	}

	// Retrieve entrance and exit nodes of Room
	if (RoomNode != nullptr) {
		RoomEntrance = *(RoomNode->LowerNode);
		RoomExit = *(RoomNode->UpperNode);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Maze: No room created!"));

	// Link teleports
	LeftTeleport->LeftNode = &(*RightTeleport);
	RightTeleport->RightNode = &(*LeftTeleport);
		
	OriginalMazeSchema = MazeSchema; // Keep a copy of the schema

	RootComponent = (USceneComponent*) SceneComponent;
}

TObjectPtr<APacmanGameMode> AMaze::CheckGameMode() {
	TObjectPtr<AGameModeBase> GameMode = UGameplayStatics::GetGameMode(GetWorld());
	if (GameMode) {
		TObjectPtr<APacmanGameMode> PacmanGameMode = Cast<APacmanGameMode>(GameMode);

		if (PacmanGameMode) {
			return PacmanGameMode;
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Maze: Game Mode cannot be casted!"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Maze: Game Mode not found!"));

	return nullptr;
}


void AMaze::AssignAdjacentNode(int Index, FMazeNode*& CloseNode) {
	if (Index >= 0
		&& Index < NumberOfColumns * NumberOfRows) {

		//if (MazeSchema[Index].Type != EMazeElement::EMA_Wall)
			CloseNode = &(MazeSchema[Index]);
	}
}

void AMaze::SetNewLocation(EMazeElement Type, FMazeNode& PrevNode, FMazeNode& NextNode) {

	MazeSchema[MazeSchema.Find(PrevNode)].Type = OriginalMazeSchema[MazeSchema.Find(PrevNode)].Type;
	MazeSchema[MazeSchema.Find(NextNode)].Type = Type;

	if (Type == EMazeElement::EMA_Pacman) {
		PacmanLocation = NextNode;
	}
	else if (Type == EMazeElement::EMA_Blinky) {
		BlinkyLocation= NextNode;
	}
	else if (Type == EMazeElement::EMA_Inky) {
		InkyLocation = NextNode;
	}
	else if (Type == EMazeElement::EMA_Pinky) {
		PinkyLocation = NextNode;
	}
	else if (Type == EMazeElement::EMA_Clyde) {
		ClydeLocation = NextNode;
	}
}

FMazeNode AMaze::GetPacmanLocation() {
	return PacmanLocation;
}

FMazeNode AMaze::GetGhostLocation(EMazeElement Type) {
	if (Type == EMazeElement::EMA_Blinky) {
		return BlinkyLocation;
	}
	else if (Type == EMazeElement::EMA_Inky) {
		return InkyLocation;
	}
	else if (Type == EMazeElement::EMA_Pinky) {
		return PinkyLocation;
	}
	else if (Type == EMazeElement::EMA_Clyde) {
		return ClydeLocation;
	}

	return MazeSchema[NumberOfColumns + 1];
}

FMazeNode AMaze::GetGhostScatteringTarget(EMazeElement Type) {

	if (Type == EMazeElement::EMA_Pinky) {
		return MazeSchema[NumberOfColumns + 1];
	}
	else if (Type == EMazeElement::EMA_Blinky) {
		return MazeSchema[(NumberOfColumns*2) - 2];
	}
	else if (Type == EMazeElement::EMA_Clyde) {
		return MazeSchema[(NumberOfColumns * (NumberOfRows - 2)) + 1];
	}
	else if (Type == EMazeElement::EMA_Inky) {
		return MazeSchema[(NumberOfColumns * (NumberOfRows - 1)) - 2];
	}

	return MazeSchema[NumberOfColumns + 1];
}

FMazeNode AMaze::GetGhostChasingTarget(EMazeElement Type) {
	FMazeNode Target = MazeSchema[NumberOfColumns + 1];

	if (Type == EMazeElement::EMA_Blinky) {
		Target = PacmanLocation;
	}
	else if (Type == EMazeElement::EMA_Inky) {
		APacmanPawn* Pacman = dynamic_cast<APacmanPawn*>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

		if (Pacman != nullptr) {
			Target = Pacman->GetTwoFrontLocation();

			FVector TargetLocation = Pacman->GetTwoFrontLocation().Location + (Pacman->GetTwoFrontLocation().Location - BlinkyLocation.Location);
			int32 Index = MazeSchema.Find(FMazeNode(EMazeElement::EMA_Empty, TargetLocation));

			if (Index != INDEX_NONE) {
				if (MazeSchema[Index].Type == EMazeElement::EMA_Wall) {
					if (MazeSchema[Index].RightNode != nullptr
						&& MazeSchema[Index].RightNode->Type != EMazeElement::EMA_Wall)
						Target = *(MazeSchema[Index].RightNode);

					if (MazeSchema[Index].LeftNode != nullptr
						&& MazeSchema[Index].LeftNode->Type != EMazeElement::EMA_Wall)
						Target = *(MazeSchema[Index].LeftNode);

					if (MazeSchema[Index].UpperNode != nullptr
						&& MazeSchema[Index].UpperNode->Type != EMazeElement::EMA_Wall)
						Target = *(MazeSchema[Index].UpperNode);

					if (MazeSchema[Index].LowerNode != nullptr
						&& MazeSchema[Index].LowerNode->Type != EMazeElement::EMA_Wall)
						Target = *(MazeSchema[Index].LowerNode);
				}

			}
		}
	}
	else if (Type == EMazeElement::EMA_Pinky) {
		APacmanPawn* Pacman = dynamic_cast<APacmanPawn*>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

		if (Pacman != nullptr) {
			Target = Pacman->GetTwoFrontLocation();
		}
	}
	else if (Type == EMazeElement::EMA_Clyde) {
		Target = PacmanLocation;
	}

	return Target;
}

FMazeNode AMaze::GetRoomEntrance() {
	return RoomEntrance;
}

FMazeNode AMaze::GetRoomExit() {
	return RoomExit;
}