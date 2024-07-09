// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*Dynamic material to pulse on damage*/
	UMaterialInstanceDynamic* MatInst;

	FTimerHandle TimerHandle_SelfDamage;

	FVector NextPathPoint;

	bool bExploded;

	bool bStartedSelfDestruction;

	int32 PowerLevel;

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	USphereComponent* SenseRange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditInstanceOnly, Category = "TrackerBot")
	bool bExcitedState;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplodeSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	TSubclassOf<AActor> DetectedClass;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UMaterialInterface* NormalMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UMaterialInterface* ExcitedMaterial;



	FVector GetNextPathPoint();

	void SelfDestruct();

	void DamageSelf();

	void OnCheckNearbyBots();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UFUNCTION()
	void OnRep_Excited();

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
