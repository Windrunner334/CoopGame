// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "SWeapon.h"
#include "CoopGame/CoopGame.h"
#include "Components/SHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Net/UnrealNetwork.h"

/*Sets default values*/
ASCharacter::ASCharacter()
{
 	/*Set this character to call Tick() every frame.You can turn this off to improve performance if you don't need it.*/
	PrimaryActorTick.bCanEverTick = true;
	/*���ɱ�*/
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	ZoomedFOV = 65.0f;
	WeaponAttachSocketName = "WeaponSocket";
	DefaultFOV = CameraComp->FieldOfView;

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChange);

}


/*Called when the game starts or when spawned*/
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	/*Spawn weapon only in server*/
	if (GetLocalRole() == ROLE_Authority)
	{
		/*Spawn a default weapon*/
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}


/*Called every frame*/
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*Zoom in*/
	float TargetFOV = bWantsTozoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}


/*Called to bind functionality to input*/
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::Zoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}


void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}


void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}


void ASCharacter::BeginCrouch()
{
	Crouch();
}


void ASCharacter::EndCrouch()
{
	UnCrouch();
}


void ASCharacter::Zoom()
{
	bWantsTozoom = !bWantsTozoom;
}


void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}


void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->HoldFire();
	}
}


/*Specify content need replicated*/
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*Ensure replicating*/
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}

void ASCharacter::OnHealthChange(USHealthComponent* HealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// Die
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);

		bDied = true;

	}
}

