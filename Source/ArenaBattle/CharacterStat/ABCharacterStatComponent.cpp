// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterStat/ABCharacterStatComponent.h"
#include "GameData/ABGameSingleton.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	CurrentLevel = 1;
	AttackRadius = 50.0f;

	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevelStat(CurrentLevel);
	ResetStat();

    OnStatChanged.AddUObject(this, &UABCharacterStatComponent::SetNewMaxHp);
}

void UABCharacterStatComponent::SetLevelStat(int32 InNewLevel)
{
	CurrentLevel = FMath::Clamp(InNewLevel, 1, UABGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UABGameSingleton::Get().GetCharacterStat(CurrentLevel));
	check(BaseStat.MaxHp > 0.0f);
}

float UABCharacterStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp<float>(InDamage, 0, InDamage);

	SetHp(PrevHp - ActualDamage);

	return ActualDamage;
}

void UABCharacterStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp<float>(NewHp, 0.0f, MaxHp);
	OnRep_CurrentHp();
}

void UABCharacterStatComponent::BeginPlay()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
    Super::BeginPlay();
}

void UABCharacterStatComponent::ReadyForReplication()
{
	AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));
	Super::ReadyForReplication();
}

void UABCharacterStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UABCharacterStatComponent, CurrentHp);
    DOREPLIFETIME(UABCharacterStatComponent, MaxHp);
	DOREPLIFETIME_CONDITION(UABCharacterStatComponent, BaseStat, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UABCharacterStatComponent, ModifierStat, COND_OwnerOnly);
}

void UABCharacterStatComponent::SetNewMaxHp(const FABCharacterStat& InBaseStat, const FABCharacterStat& InModifierStat)
{
    float PrevMaxHp = MaxHp;
    MaxHp = (InBaseStat + InModifierStat).MaxHp;
    if (PrevMaxHp != MaxHp)
    {
        OnHpChanged.Broadcast(CurrentHp, MaxHp);
    }
}

void UABCharacterStatComponent::OnRep_CurrentHp()
{
    OnHpChanged.Broadcast(CurrentHp, MaxHp);
	if (CurrentHp <= KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}
}

void UABCharacterStatComponent::OnRep_MaxHp()
{
    AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("MaxHp Replicated"));
    OnHpChanged.Broadcast(CurrentHp, MaxHp);
}

void UABCharacterStatComponent::OnRep_BaseStat()
{
    AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("BaseStat Replicated"));
    OnStatChanged.Broadcast(GetBaseStat(), GetModifierStat());
}

void UABCharacterStatComponent::OnRep_ModifierStat()
{
    AB_SUBLOG(LogABNetwork, Log, TEXT("%s"), TEXT("ModifierStat Replicated"));
    OnStatChanged.Broadcast(GetBaseStat(), GetModifierStat());
}

void UABCharacterStatComponent::ResetStat()
{
	SetLevelStat(CurrentLevel);
    MaxHp = BaseStat.MaxHp;
    SetHp(MaxHp);
}

