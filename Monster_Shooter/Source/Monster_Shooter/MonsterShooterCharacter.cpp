// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterShooterCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"

#include "Projectile.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"

#include "MonsterShooterGameMode.h"

// Sets default values
//Metodo constructor MonsterShooterCharacter
AMonsterShooterCharacter::AMonsterShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(40.0f, 95.0f);//Para definirle el tama�o a la capsula de nuestro personaje.

	TurnRate = 45.0f;
	LookUpRate = 45.0f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent()); //Donde vamos a atar nuestra First Person Camera, osea la camara de nuestro personaje.
	FirstPersonCamera->AddRelativeLocation(FVector(-39.65f, 1.75f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Character Mesh"));
	
	HandsMesh->SetOnlyOwnerSee(true);
	HandsMesh->SetupAttachment(FirstPersonCamera);
	HandsMesh->bCastDynamicShadow = false;
	HandsMesh->CastShadow = false;
	HandsMesh->AddRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	HandsMesh->AddRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun"));
	GunMesh->SetOnlyOwnerSee(true);
	GunMesh->bCastDynamicShadow = false;
	GunMesh->CastShadow = false;

	MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Muzzle Location"));
	MuzzleLocation->SetupAttachment(GunMesh);
	MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

// Called when the game starts or when spawned
void AMonsterShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	GunMesh->AttachToComponent(HandsMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TEXT("GripPoint"));

	World = GetWorld();

	AnimInstance = HandsMesh->GetAnimInstance();
}

// Called every frame
void AMonsterShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMonsterShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMonsterShooterCharacter::OnFire);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMonsterShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMonsterShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMonsterShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMonsterShooterCharacter::LookAtRate);
}

void AMonsterShooterCharacter::OnFire()
{
	if (World != NULL)
	{
		SpawnRotation = GetControlRotation();

		SpawnLocation = ((MuzzleLocation != nullptr) ?
			MuzzleLocation->GetComponentLocation() :
			GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		World->SpawnActor<AProjectile>(Projectile,
			SpawnLocation, SpawnRotation, ActorSpawnParams);

		if (FireSound != NULL) 
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		if (FireAnimation != NULL && AnimInstance != NULL) 
		{
			AnimInstance->Montage_Play(FireAnimation, 1.0f);
		}
	}
}

void AMonsterShooterCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMonsterShooterCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMonsterShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRate * GetWorld()->GetDeltaSeconds());
}

void AMonsterShooterCharacter::LookAtRate(float Rate)
{
	AddControllerPitchInput(Rate * LookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMonsterShooterCharacter::DealDamage(float DamageAmount)
{
	Health -= DamageAmount;

	if (Health <= 0.0f) 
	{
		// restart game
		AMonsterShooterGameMode* MyGameMode =
			Cast<AMonsterShooterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));

		if(MyGameMode)
		{
			MyGameMode->RestartGameplay(false);
		}

		Destroy();
	}
}

