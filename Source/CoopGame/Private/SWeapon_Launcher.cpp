// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon_Launcher.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "SProjectile_Grenade.h"
#include "SCharacter.h"


/*榴弹发射器，很多原fire函数功能移植到projectile类中，包括impact effect, draw debug graph*/
void ASWeapon_Launcher::Fire()
{
	Super::Fire();

	/*Weapon owner*/
	AActor* MyOwner = GetOwner();

	/*Get the line of owner sight*/
	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	/*Shooting trace is sight line*/
	FVector ShotDirection = EyeRotation.Vector();

	/*Time recording*/
	LastFireTime = GetWorld()->GetTimeSeconds();

	/*try and fire a projectile*/
	if (ProjectileClass)
	{
		/*Grabs location from the mesh that must have a socket*/
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		/*Use controller rotation which is our view direction in first person*/
		FRotator MuzzleRotation = EyeRotation;

		/*Set Spawn Collision Handling Override*/
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		ActorSpawnParams.Instigator = Cast<ASCharacter>(MyOwner);

		/*Spawn the projectile at the muzzle, set owner as player*/
		ASProjectile_Grenade* bullet = GetWorld()->SpawnActor<ASProjectile_Grenade>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
		if (bullet)
		{
			bullet->SetOwner(MyOwner);
		}
	}

	PlayFireEffects();
}

