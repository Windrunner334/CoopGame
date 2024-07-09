// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class USphereComponent;
class URadialForceComponent;
class ASCharacter;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	virtual void BeginPlay() override;

	void CameraShake();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ExplosionEffect();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barrel")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	UPROPERTY(ReplicatedUsing = OnRep_ExplosionEffect, BlueprintReadOnly, Category = "Barrel")
	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	float ImpulseStrength;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	UMaterialInterface* OriginMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	TSubclassOf<UDamageType>  ExplosionDamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	TSubclassOf<UCameraShakeBase> ExplosionShakeCam;

	UPROPERTY(EditDefaultsOnly, Category = "Barrel")
	TSubclassOf<ASCharacter> c;

	UFUNCTION()
	void CollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BarrelHealthChange(USHealthComponent* HealthComponent, float Health, float HealthDelta,
		const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
};
