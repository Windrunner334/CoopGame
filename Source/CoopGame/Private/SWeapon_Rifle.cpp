// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon_Rifle.h"
#include "CoopGame/CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

/*Console command*/
/*Impact sphere drawing*/
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);


/*Trace the world from pawn eyes to crosshair location*/
void ASWeapon_Rifle::Fire()
{
	Super::Fire();

	AActor* MyOwner = GetOwner();
	/*Get eye sight direction*/
	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	/*Shot direction is set as sight direction*/
	FVector ShotDirection = EyeRotation.Vector();
	FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
	TraceEndPoint = TraceEnd;

	/*Collision parameters setting*/
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(MyOwner);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bReturnPhysicalMaterial = true;		//设定可返回物理材质
	QueryParams.bTraceComplex = true;				//trace可以定位到网格体的每个三角形，false为简单的盒体碰撞

	/*Time recording*/
	LastFireTime = GetWorld()->TimeSeconds;

	/*Get hit info and simulate the situation of hitting success*/
	FHitResult Hit;
	EPhysicalSurface SurfaceType = SurfaceType_Default;
	if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
	{
		/*Blocking hit! Process damage*/
		AActor* HitActor = Hit.GetActor();
		TraceEndPoint = Hit.ImpactPoint;
		ActualDamage = BaseDamage;

		/*Select impact effect according to physical surface type*/
		SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

		OnRep_HitScanTrace();
		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
	
	}

	/*According to console controlling*/
	if (DebugWeaponDrawing > 0)
	{
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
	}


	if (GetLocalRole() == ROLE_Authority)
	{
		HitScanTrace.SurfaceType = SurfaceType;
		HitScanTrace.TraceTo = TraceEndPoint;
	}
	else {
		PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
	}

}


void ASWeapon_Rifle::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	// Different surface material
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
		SelectedEffect = FleshImpactEffect;
		break;
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		ActualDamage *= 4.0f;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	/*Impact effect*/
	if (SelectedEffect)
	{
		FVector ShotDirection = ImpactPoint - MeshComp->GetSocketLocation(MuzzleSocketName);
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}


	/*Trace effect, a line*/
	if (TraceEffect && ImpactPoint != FVector::ZeroVector)
	{
		
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TraceComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceEffect, MuzzleLocation);
		if (TraceComp)
		{
			TraceComp->SetVectorParameter(TraceTargetName, ImpactPoint);
		}
	}
}

