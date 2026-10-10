#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseProjectCharacter.generated.h"

class UCameraComponent;

UCLASS()
class BASEPROJECT_API ABaseProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseProjectCharacter();

	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera.Get(); }

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BaseLookUpRate = 45.0f;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void TurnAtRate(float Rate);
	void LookUp(float Value);
	void LookUpAtRate(float Rate);
};
