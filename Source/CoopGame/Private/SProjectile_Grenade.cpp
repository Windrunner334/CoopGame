// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectile_Grenade.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

/*Console Command*/
/*Explosion Range Reminder*/
static int32 DebugProjectileDrawing = 0;
FAutoConsoleVariableRef CVARDebugProjectileDrawing(
	TEXT("COOP.DebugProjectiles"),
	DebugProjectileDrawing,
	TEXT("Draw Debug spheres for Projectiles"),
	ECVF_Cheat);


ASProjectile_Grenade::ASProjectile_Grenade()
{
	/*Damage counting range*/
	ExplosionRange = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionRange"));
	ExplosionRange->SetupAttachment(RootComponent);

	/*Explosion setting*/
	ExplosionPower = 1000;
	ForceStrength = 20000;
}

void ASProjectile_Grenade::Explode()
{
	AActor* MyOwner = GetOwner();
	/*Get actors and components in explosion range*/
	TArray<AActor*> ActorsInRange;
	TArray<UPrimitiveComponent*> CompsInRange;
	ExplosionRange->GetOverlappingActors(ActorsInRange);
	ExplosionRange->GetOverlappingComponents(CompsInRange);

	/*Parameters setting*/
	float SphereRadius = ExplosionRange->GetScaledSphereRadius();
	FVector ExplosionPoint = GetActorLocation();

	/*Damage calculate*/
// 	TArray<AActor*> IgnoreActors;
// 	IgnoreActors.Add(this);
// 	UGameplayStatics::ApplyRadialDamage(GetWorld(), 20.0f, ExplosionPoint, SphereRadius, DamageType, IgnoreActors, MyOwner);

	/*Blow up characters*/
	for (int32 i = 0; i < ActorsInRange.Num(); i++)
	{
		AActor* actor = ActorsInRange[i];
		if (actor)
		{
			ASCharacter* Enemy = Cast<ASCharacter>(actor);
			if (Enemy)
			{
				FVector ExplosionVelocity = (Enemy->GetActorLocation() - ExplosionPoint) / (Enemy->GetActorLocation() - ExplosionPoint).Size() * ExplosionPower;
				UE_LOG(LogTemp, Log, TEXT("Here is enemy!, Velocity: %.2f"), ExplosionVelocity.Size());
				Enemy->LaunchCharacter(ExplosionVelocity, true, true);
			}
			UGameplayStatics::ApplyDamage(actor, 20.0f, this->GetInstigatorController(), MyOwner, DamageType);
		}
	}

	/*Blow up components*/
	for (int32 i = 0; i < CompsInRange.Num(); i++)
	{
		UPrimitiveComponent* component = CompsInRange[i];
		if (component && component->IsSimulatingPhysics())
		{
			UE_LOG(LogTemp, Log, TEXT("Here is object!"));
			component->AddRadialForce(ExplosionPoint, ForceStrength * component->GetMassScale(), ForceStrength * component->GetMassScale(), ERadialImpulseFalloff::RIF_Constant, true);
		}
	}

	/*Impose impact effect*/
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, ExplosionPoint);
	}

	/*Draw explosion range sphere*/
	if (DebugProjectileDrawing > 0)
	{
		DrawDebugSphere(GetWorld(), ExplosionPoint, SphereRadius, 12, FColor::Red, false, 1.0f, 0.0f, 1.0f);
	}

	Destroy();
	GrenadeExplosionShake();
}

void ASProjectile_Grenade::GrenadeExplosionShake()
{
	APawn* MyPlayer = Cast<APawn>(GetOwner());
	if (MyPlayer)
	{
		APlayerController* PC = Cast<APlayerController>(MyPlayer->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(ExplosionShake);
		}
	}
}
