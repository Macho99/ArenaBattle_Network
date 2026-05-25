// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"
#include "Components/PointLightComponent.h"
#include "EngineUtils.h"

// Sets default values
AABFountain::AABFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	bReplicates = true;
	NetUpdateFrequency = 10.f;
    NetCullDistanceSquared = 2000.f * 2000.f;
	//NetDormancy = DORM_Initial;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
    if (HasAuthority())
    {
		//ServerRotationYaw = 0.f;
		//{
		//	FTimerHandle TimerHandle;
		//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		//		{

		//		}), 1.0f, true, 0.f);
		//}

		//{
		//	FTimerHandle TimerHandle;
		//	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		//		{
  //                  const FLinearColor NewColor = FLinearColor::MakeRandomColor();
  //                  MulticastRPCChangeLightColor(NewColor);
		//		}), 5.0f, false, -1.f);
		//}
    }
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));
        ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
    }
	else
	{
        ClientTimeSinceUpdate += DeltaTime;
		if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER)
		{
			return;
		}

        const float EstimateRotationYaw = ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdates;
        const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

        FRotator ClientRotator = RootComponent->GetComponentRotation();
        ClientRotator.Yaw = FMath::Lerp(ServerRotationYaw, EstimateRotationYaw, LerpRatio);
        RootComponent->SetWorldRotation(ClientRotator);
	}
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AABFountain, ServerRotationYaw);
    DOREPLIFETIME(AABFountain, ServerLightColor);
}

//void AABFountain::OnActorChannelOpen(FInBunch& InBunch, UNetConnection* Connection)
//{
//	AB_LOG(LogABNetwork, Log, TEXT("Begin"));
//    Super::OnActorChannelOpen(InBunch, Connection);
//	AB_LOG(LogABNetwork, Log, TEXT("End"));
//}
//
//bool AABFountain::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
//{
//    bool NetRelevantResult = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
//	if (!NetRelevantResult)
//	{
//        AB_LOG(LogABNetwork, Log, TEXT("Not Net Relevant[%s] %s"), *RealViewer->GetName(), *SrcLocation.ToCompactString());
//	}
//    return NetRelevantResult;
//}
//
//void AABFountain::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
//{
//    AB_LOG(LogABNetwork, Log, TEXT("PreReplication"));
//    Super::PreReplication(ChangedPropertyTracker);
//}

void AABFountain::OnRep_ServerRotationYaw()
{
    //AB_LOG(LogABNetwork, Log, TEXT("Yaw : %f"), ServerRotationYaw);
	FRotator NewRotation = RootComponent->GetComponentRotation();
	NewRotation.Yaw = ServerRotationYaw;
	RootComponent->SetWorldRotation(NewRotation);

    ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
    ClientTimeSinceUpdate = 0.0f;
}

void AABFountain::OnRep_ServerLightColor()
{
	if (HasAuthority() == false)
	{
		AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *ServerLightColor.ToString());
	}

    UPointLightComponent* PointLight = FindComponentByClass<UPointLightComponent>();
	if (PointLight)
	{
        PointLight->SetLightColor(ServerLightColor);
	}
}

void AABFountain::MulticastRPCChangeLightColor_Implementation(const FLinearColor& NewColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *NewColor.ToString());
	UPointLightComponent* PointLight = FindComponentByClass<UPointLightComponent>();
	if (PointLight)
	{
		PointLight->SetLightColor(NewColor);
	}
}

void AABFountain::ServerRPCChangeLightColor_Implementation()
{
    const FLinearColor NewColor = FLinearColor::MakeRandomColor();
    MulticastRPCChangeLightColor(NewColor);
}

bool AABFountain::ServerRPCChangeLightColor_Validate()
{
	return true;
}

void AABFountain::ClientRPCChangeLightColor_Implementation(const FLinearColor& NewColor)
{
	AB_LOG(LogABNetwork, Log, TEXT("LightColor : %s"), *NewColor.ToString());
	UPointLightComponent* PointLight = FindComponentByClass<UPointLightComponent>();
	if (PointLight)
	{
		PointLight->SetLightColor(NewColor);
	}
}
