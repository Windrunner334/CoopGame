// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ASWeapon::ASWeapon()
{
 	/*Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.*/
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComp;
	MuzzleSocketName = "MuzzleSocket";
	TraceTargetName = "BeamEnd";

	/*Basic parameters setting*/
	BaseDamage = 20.0f;
	RateOfFire = 1.0f;
	ClipSize = 30;

	/*Spawn weapons in client synchronously*/
	SetReplicates(true);

	/*Frames limitation*/
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	
}


void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*Record time for firing interval*/
	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimeHandle_TimeBetweenShots, this, &ASWeapon::Fire,  TimeBetweenShots, true, FirstDelay);
}


void ASWeapon::HoldFire()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_TimeBetweenShots);
}


/*Trace the world from pawn eyes to crosshair location*/
void ASWeapon::Fire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		/*Client return, and call Fire function in server*/
		ServerFire();
		return;
	}

	if (!GetOwner()) {
		return;
	}
	
}

/*Only run in server*/
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

/*Including muzzle effect, and camera shaking*/
void ASWeapon::PlayFireEffects()
{
	// Muzzle effect
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	// Camera shake
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{

}

void ASWeapon::OnRep_HitScanTrace()
{
	/*Muzzle effect and camera shaking */
	PlayFireEffects();
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}


/*Specify content need replicated*/
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*Ensure replicating*/
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
