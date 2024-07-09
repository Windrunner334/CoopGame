// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/STrackerBot.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "GameFramework\Character.h"
#include "SCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Sound\SoundCue.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SenseRange = CreateDefaultSubobject<USphereComponent>(TEXT("SenseRange"));

	bUseVelocityChange = false;
	MovementForce = 1000.0f;
	RequiredDistanceToTarget = 100.0f;
	ExplosionRadius = 200.0f;
	SelfDamageInterval = 0.25f;

	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp->SetSphereRadius(200);
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SenseRange->SetSphereRadius(400);
	SenseRange->SetupAttachment(RootComponent);
	SenseRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SenseRange->SetCollisionResponseToAllChannels(ECR_Ignore);
	SenseRange->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//SenseRange->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);


}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	/*Find initial move to*/
	if (GetLocalRole() == ROLE_Authority)
	{
		NextPathPoint = GetNextPathPoint();
	}

}

/*Find next path point while reaching target*/
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLocalRole() == ROLE_Authority && !bExploded)
	{
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();
		/*Detect the distance to target*/
		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			NextPathPoint = GetNextPathPoint();
			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");
		}
		else {
			/*Keep moving towards to target */
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 4.0f, 1.0f);

		/*Detect all partners*/
		TArray<AActor*> DetectedActors;
		int32 DetechedNum = 0;
		SenseRange->GetOverlappingActors(DetectedActors, DetectedClass);
		for (int i = 0; i < DetectedActors.Num(); i++)
		{
			ASTrackerBot* tb = Cast<ASTrackerBot>(DetectedActors[i]);
			if (tb)
			{
				DetechedNum++;
			}
		}
		bExcitedState = DetechedNum >= 2 ? true : false;
		UE_LOG(LogTemp, Log, TEXT("Detected Num: %d"), DetectedActors.Num());

		OnRep_Excited();
	}

}

/*Get next path point*/
FVector ASTrackerBot::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath =  UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		/*return next point*/
		return NavPath->PathPoints[1];
	}
	/*fail*/
	return GetActorLocation();
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	/*Explode on hitpoints == 0*/
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

/*Destruct self while reaching target*/
void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}
	bExploded = true;

	/*Spawn effect, sound and apply damage*/
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetLocalRole() == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		/*Damage range*/
		float DamageValue = bExcitedState ? 200 : 100;
		UGameplayStatics::ApplyRadialDamage(GetWorld(), DamageValue, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1);
		SetLifeSpan(2.0f);	
		UE_LOG(LogTemp, Log, TEXT("Self Destruct"));

	}

}


void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
	if (PlayerPawn && !bStartedSelfDestruction && !bExploded)
	{
		/*Overlapped with a player*/
		bStartedSelfDestruction = true;
		UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		UE_LOG(LogTemp, Log, TEXT("Overlap!"));

		if (GetLocalRole() == ROLE_Authority)
		{
			/*Self Destruction*/
			GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
		}

	}
}


void ASTrackerBot::OnRep_Excited()
{
	/*Material change*/
	UMaterialInterface* NewMaterial = bExcitedState ? ExcitedMaterial : NormalMaterial;
	MeshComp->SetMaterial(0, NewMaterial);

}


/*Damage self function used at overlap function*/
void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}


void ASTrackerBot::OnCheckNearbyBots()
{
	const float DetectRadius = 300;
	const int32 MaxPowerLevel = 4;

	/*temporary collision shape*/
	FCollisionShape CollShape;
	CollShape.SetSphere(DetectRadius);

	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);
	DrawDebugSphere(GetWorld(), GetActorLocation(), DetectRadius, 12, FColor::White, false, 1.0f);

	int32 BotNum = 0;
	for (FOverlapResult Result : Overlaps)
	{
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		if (Bot && Bot != this)
		{
			BotNum++;
		}
	}

	PowerLevel = FMath::Clamp(BotNum, 0, MaxPowerLevel);
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}
}

