// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SProjectile.h"
#include "SProjectile_Grenade.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASProjectile_Grenade : public ASProjectile
{
	GENERATED_BODY()
public:
	ASProjectile_Grenade();

protected:
	virtual void Explode() override;

	void GrenadeExplosionShake();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* ExplosionRange;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	int32 ExplosionPower;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	int32 ForceStrength;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<UCameraShakeBase> ExplosionShake;

};
