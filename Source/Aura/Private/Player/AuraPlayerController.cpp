// Copyright Tentacle 51, LLC

#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	check(AuraContext);
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if(CursorHit.bBlockingHit)
	{
		LastActor = ThisActor;
		ThisActor = CursorHit.GetActor();

		/*
		 * Line trace from cursor. There are several scenarios
		 * A. LastActor is null AND ThisActor is null
		 *		- do nothing
		 * B. LastActor is null AND ThisActor is valid
		 *		- Highlight ThisActor
		 * C. LastActor is valid AND ThisActor is null
		 *		- Unhighlight LastActor
		 * D. Both actor is valid and LastActor != ThisActor
		 *		- Unhighlight LastActor AND Highlight ThisActor
		 * E. Both actors are valid, and are the same
		 *		- do nothing
		 */

		if(LastActor == nullptr)
		{
			if(ThisActor != nullptr)
			{
				// B
				ThisActor->HighlightActor();
			}else
			{
				// A - both are null, do nothing				
			}
		}
		else
		{
			if(ThisActor == nullptr)
			{
				// C
				LastActor->UnHighlightActor();
			}
			else
			{
				if(LastActor != ThisActor)
				{
					// D - both actors valid, are they the same?
					LastActor->UnHighlightActor();
					ThisActor->HighlightActor();
				}
				else
				{
					// E - do nothing, they are the same
				}
			}
		}
	}
}
