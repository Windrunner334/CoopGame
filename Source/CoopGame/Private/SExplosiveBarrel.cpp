// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "Components/SHealthComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "CoopGame/CoopGame.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthConp"));
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));

	RadialForceComp->SetupAttachment(MeshComp);
	RootComponent = MeshComp;
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	SetReplicates(true);
	SetReplicateMovement(true);

	ExplosionRadius = 500.0f;
	ExplosionDamage = 20.0f;
	ImpulseStrength = 400.0f;

	RadialForceComp->Radius = ExplosionRadius;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false;//由下方的FireImpulse触发
	RadialForceComp->bIgnoreOwningActor = true;

	MeshComp->OnComponentBeginOverlap.AddDynamic(this, &ASExplosiveBarrel::CollisionOverlap);
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::BarrelHealthChange);
}

void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

}

void ASExplosiveBarrel::CameraShake()
{
	TArray<AActor*> charas_array;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), c, charas_array);
	for (uint8 i = 0; i < charas_array.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("Search success!"));
		APawn* chara = Cast<APawn>(charas_array[i]);
		if (chara && ExplosionShakeCam)
		{
			APlayerController* PC = Cast<APlayerController>(chara->GetController());
			if (PC)
			{
				PC->ClientPlayCameraShake(ExplosionShakeCam);
				UE_LOG(LogTemp, Log, TEXT("Shake success!"));
			}
		}
	}
}

void ASExplosiveBarrel::CollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void ASExplosiveBarrel::BarrelHealthChange(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("Health changed: %.2f"), Health);
	/*Explosion Occurs*/
	if (Health <= 0.0f && !bExploded)
	{
		FVector Location = GetActorLocation();
		bExploded = true;
		OnRep_ExplosionEffect();

		/*Radial damage*/
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(this);
		UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, Location, ExplosionRadius, ExplosionDamageType, IgnoreActors);
		
		/*Impulse actors and components*/
		FVector BoostIntensity = FVector::UpVector * ImpulseStrength;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);
		RadialForceComp->FireImpulse();

 	}
}


/*Specify content need replicated*/
void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*Ensure replicating*/
	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}

void ASExplosiveBarrel::OnRep_ExplosionEffect()
{
	FVector Location = GetActorLocation();

	/*Material change*/
	MeshComp->SetMaterial(0, ExplodedMaterial);
	DrawDebugSphere(GetWorld(), Location, RadialForceComp->Radius, 12, FColor::Red, false, 1.0f, 0.0f, 1.0f);
	CameraShake();

	/*Explosion effect*/
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Location);
	}
}

