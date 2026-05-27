// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ABGameMode.h"
#include "ABGameMode.h"
#include "Player/ABPlayerController.h"
#include "ArenaBattle.h"
#include "ABGameState.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "ABPlayerState.h"

AABGameMode::AABGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> DefaultPawnClassRef(TEXT("/Script/Engine.Blueprint'/Game/ArenaBattle/Blueprint/BP_ABCharacterPlayer.BP_ABCharacterPlayer_C'"));
    if (DefaultPawnClassRef.Class)
    {
        DefaultPawnClass = DefaultPawnClassRef.Class;
    }

    static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/ArenaBattle.ABPlayerController"));
    if (PlayerControllerClassRef.Class)
    {
        PlayerControllerClass = PlayerControllerClassRef.Class;
    }

    GameStateClass = AABGameState::StaticClass();
    PlayerStateClass = AABPlayerState::StaticClass();
}

FTransform AABGameMode::GetRandomStartTransform() const
{
    if (PlayerStartArray.Num() == 0)
    {
        return FTransform(FVector(0.f, 0.f, 230.f));
    }

    int32 RandomIndex = FMath::RandRange(0, PlayerStartArray.Num() - 1);
    return PlayerStartArray[RandomIndex]->GetActorTransform();
}

void AABGameMode::OnPlayerKilled(AController* Killer, AController* KilledPlayer, APawn* KilledPawn)
{
    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

    APlayerState* KillerState = Killer->PlayerState;
    if (KillerState)
    {
        KillerState->SetScore(KillerState->GetScore() + 1);

        if (KillerState->GetScore() >= 2)
        {
            FinishMatch();
        }
    }
}

/*
void AABGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("============================================================"));
    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    //ErrorMessage = TEXT("Server is full");

    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

APlayerController* AABGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

    APlayerController* NewPlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));

    return NewPlayerController;
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

    Super::PostLogin(NewPlayer);

    UNetDriver* NetDriver = GetWorld()->GetNetDriver();
    if (NetDriver)
    {
        if (NetDriver->ClientConnections.Num() == 0)
        {
            AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No ClientConnections"));
        }
        else
        {
            AB_LOG(LogABNetwork, Log, TEXT("%s"), *FString::Printf(TEXT("ClientConnections: %d"), NetDriver->ClientConnections.Num()));
            for (int32 i = 0; i < NetDriver->ClientConnections.Num(); ++i)
            {
                FString Name = NetDriver->ClientConnections[i]->GetName();
                AB_LOG(LogABNetwork, Log, TEXT("%s"), *FString::Printf(TEXT("ClientConnection %d: %s"), i, *Name));
            }
        }
    }
    else
    {
        AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("No NetDriver"));
    }

    AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}
*/

void AABGameMode::StartPlay()
{
    Super::StartPlay();

    for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld()))
    {
        PlayerStartArray.Add(PlayerStart);
    }
}

void AABGameMode::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AABGameMode::DefaultGameTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AABGameMode::DefaultGameTimer()
{
    AABGameState* ABGameState = Cast<AABGameState>(GameState);

    if (ABGameState && ABGameState->RemainingTime > 0)
    {
        ABGameState->RemainingTime--;
        AB_LOG(LogABNetwork, Log, TEXT("RemainingTime: %d"), ABGameState->RemainingTime);

        if (ABGameState->RemainingTime <= 0)
        {
            if (GetMatchState() == MatchState::InProgress)
            {
                FinishMatch();
            }
            else if (GetMatchState() == MatchState::WaitingPostMatch)
            {
                GetWorld()->ServerTravel(TEXT("/Game/ArenaBattle/Maps/Part3Step2?listen"));
            }
            else
            {
                AB_LOG(LogABNetwork, Warning, TEXT("%s"), TEXT("Invalid MatchState"));
            }
        }
    }
}

void AABGameMode::FinishMatch()
{
    AABGameState* ABGameState = Cast<AABGameState>(GameState);
    if (IsMatchInProgress())
    {
        EndMatch();
        ABGameState->RemainingTime = ABGameState->ShowResultWaitingTime;
    }
}
