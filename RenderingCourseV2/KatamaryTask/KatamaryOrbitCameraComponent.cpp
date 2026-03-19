#include "KatamaryTask/KatamaryOrbitCameraComponent.h"
#include "KatamaryTask/KatamaryOrbitCameraInputHandler.h"
#include "Abstracts/Core/Actor.h"
#include "Abstracts/Core/Game.h"
#include "Abstracts/Core/Transform.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include "Abstracts/Others/MainMathLibrary.h"

KatamaryOrbitCameraComponent::KatamaryOrbitCameraComponent()
	: CameraComponent()
	, OrbitTargetActor(nullptr)
	, OrbitYawRadians(3.14159265358979323846f)
	, OrbitPitchRadians(0.25f)
	, OrbitDistance(7.0f)
	, RotationSensitivity(0.0025f)
	, ZoomStep(1.0f)
	, IsPossessed(false)
	, KatamaryOrbitCameraInputHandlerInstance(nullptr)
{
}

KatamaryOrbitCameraComponent::~KatamaryOrbitCameraComponent() = default;

void KatamaryOrbitCameraComponent::SetOrbitTargetActor(Actor* NewOrbitTargetActor)
{
	OrbitTargetActor = NewOrbitTargetActor;

	ApplyOrbitTransform();
}

Actor* KatamaryOrbitCameraComponent::GetOrbitTargetActor() const
{
	return OrbitTargetActor;
}

void KatamaryOrbitCameraComponent::Posses()
{
	if (IsPossessed)
	{
		return;
	}

	Game* OwningGame = GetOwningGame();
	if (OwningGame == nullptr)
	{
		return;
	}

	if (KatamaryOrbitCameraInputHandlerInstance == nullptr)
	{
		std::unique_ptr<KatamaryOrbitCameraInputHandler> NewKatamaryOrbitCameraInputHandler = std::make_unique<KatamaryOrbitCameraInputHandler>();
		KatamaryOrbitCameraInputHandlerInstance = NewKatamaryOrbitCameraInputHandler.get();
		OwningGame->RegisterInputHandler(std::move(NewKatamaryOrbitCameraInputHandler));
	}

	IsPossessed = true;
}

void KatamaryOrbitCameraComponent::Unposses()
{
	if (IsPossessed == false)
	{
		return;
	}

	Game* OwningGame = GetOwningGame();
	if (OwningGame != nullptr && KatamaryOrbitCameraInputHandlerInstance != nullptr)
	{
		OwningGame->UnregisterInputHandler(KatamaryOrbitCameraInputHandlerInstance);
		KatamaryOrbitCameraInputHandlerInstance = nullptr;
	}

	IsPossessed = false;
}

void KatamaryOrbitCameraComponent::Update(float DeltaTime)
{
	CameraComponent::Update(DeltaTime);
	ApplyOrbitTransform();
}

void KatamaryOrbitCameraComponent::ApplyOrbitInput(float MouseDeltaX, float MouseDeltaY, int MouseWheelDelta, float DeltaTime)
{
	if (OrbitTargetActor == nullptr || DeltaTime <= 0.0f)
	{
		return;
	}

	OrbitYawRadians += MouseDeltaX * RotationSensitivity;
	OrbitPitchRadians += MouseDeltaY * RotationSensitivity;
	OrbitPitchRadians = (std::clamp)(OrbitPitchRadians, -1.3f, 1.3f);

	if (MouseWheelDelta != 0)
	{
		const float MouseWheelStep = static_cast<float>(MouseWheelDelta) / 120.0f;
		OrbitDistance -= MouseWheelStep * ZoomStep;
		OrbitDistance = (std::clamp)(OrbitDistance, 2.0f, 80.0f);
	}

	ApplyOrbitTransform();
}

void KatamaryOrbitCameraComponent::ApplyOrbitTransform()
{
	if (OrbitTargetActor == nullptr)
	{
		return;
	}

	Actor* OwningActor = GetOwningActor();
	if (OwningActor == nullptr)
	{
		return;
	}

	const DirectX::XMFLOAT3 OrbitTargetPosition = OrbitTargetActor->GetTransform(ETransformSpace::World).Position;
	const DirectX::XMFLOAT3 OrbitRotationEuler = DirectX::XMFLOAT3(
		OrbitPitchRadians,
		OrbitYawRadians,
		0.0f);
	const DirectX::XMFLOAT3 OrbitLookDirection = MainMathLibrary::RotationEulerToForwardVector(OrbitRotationEuler);

	DirectX::XMFLOAT3 CameraPosition = OrbitTargetPosition;
	CameraPosition.x -= OrbitLookDirection.x * OrbitDistance;
	CameraPosition.y -= OrbitLookDirection.y * OrbitDistance;
	CameraPosition.z -= OrbitLookDirection.z * OrbitDistance;

	OwningActor->SetLocation(CameraPosition, ETransformSpace::World);
	DirectX::XMFLOAT3 Rotation = OrbitRotationEuler;
	OwningActor->SetRotation(Rotation, ETransformSpace::World);
}
