// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SWeapon_Rifle.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASWeapon_Rifle : public ASWeapon
{
	GENERATED_BODY()
	

protected:
	virtual void Fire() override;


	FVector TraceEndPoint;
	float ActualDamage;

	virtual void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

};
