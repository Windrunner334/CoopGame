// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SWeapon_Launcher.generated.h"

class ASProjectile_Grenade;

/**
 * 
 */
UCLASS()
class COOPGAME_API ASWeapon_Launcher : public ASWeapon
{
	GENERATED_BODY()

protected:

	virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 LauncherVelocityMultiple;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float Gravity;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ASProjectile_Grenade> ProjectileClass;
	
};
