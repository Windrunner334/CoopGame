// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile.h"
#include "Components/SphereComponent.h"
#include "SCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"


/*Sets default values*/
ASProjectile::ASProjectile()
{
 	/*Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.*/
	PrimaryActorTick.bCanEverTick = true;

	/*Project material*/
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshConp"));

	/*Object collision*/
	CollisionRange = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionRange"));
	CollisionRange->SetCollisionProfileName("Projectile");

	/*Players can't walk on it*/
	CollisionRange->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionRange->CanCharacterStepUpOn = ECB_No;

	/*Use a ProjectileMovementComponent to govern this projectile's movement*/
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = MeshComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	/*Attachment relationship*/
	RootComponent = MeshComp;
 	CollisionRange->SetupAttachment(RootComponent);

	CollisionRange->OnComponentBeginOverlap.AddDynamic(this, &ASProjectile::CollisionOverlap);

}


void ASProjectile::BeginPlay()
{
	Super::BeginPlay();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASProjectile::Explode, 0.8f, false);

	/*Impose bullet trace, not yet implemented*/
	if (TraceEffect)
	{
		//FVector ProjectileLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TraceComp = UGameplayStatics::SpawnEmitterAttached(TraceEffect, MeshComp);
	}
}


void ASProjectile::Explode()
{

}


void ASProjectile::CollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	Explode();
}


