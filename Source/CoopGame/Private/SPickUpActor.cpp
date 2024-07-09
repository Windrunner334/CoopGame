// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickUpActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SPowerupActor.h"

// Sets default values
ASPickUpActor::ASPickUpActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));

	RootComponent = SphereComp;
	SphereComp->SetSphereRadius(75.0f);

	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(64, 75, 75);

	CooldownDuration = 10.0f;

	SetReplicates(true);

}

// Called when the game starts or when spawned
void ASPickUpActor::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		Respawn();
	}
	
}

void ASPickUpActor::Respawn()
{
	if (PowerUpClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PowerUpInstance = GetWorld()->SpawnActor<ASPowerupActor>(PowerUpClass, GetTransform(), SpawnParams);

	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Powerup Class is nullptr in %s. Please update your blueprint"), *GetName());
		
	}
}

void ASPickUpActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	/*Grant a power up to player*/
	if (GetLocalRole() == ROLE_Authority && PowerUpInstance)
	{
		PowerUpInstance->ActivatePowerup(OtherActor);
		PowerUpInstance = nullptr;

		/*Setup a timer to respawn*/
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickUpActor::Respawn, CooldownDuration);
	}

}

