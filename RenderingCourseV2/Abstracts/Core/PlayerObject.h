#pragma once

#include "Abstracts/Core/Object.h"
#include "Abstracts/Core/GameConfigurator.h"

class PlayerObject : public Object
{
public:
	PlayerObject();
	~PlayerObject() override;

	void SetPlayerIdentifier(int NewPlayerIdentifier);
	int GetPlayerIdentifier() const;
	void SetInputSourceIdentifier(int NewInputSourceIdentifier);
	int GetInputSourceIdentifier() const;
	void SetPreferredCameraIndex(int NewPreferredCameraIndex);
	int GetPreferredCameraIndex() const;
	void SetViewportRectangle(const ViewportRectangleNormalized& NewViewportRectangle);
	const ViewportRectangleNormalized& GetViewportRectangle() const;
	void SetActiveGameIdentifier(int NewActiveGameIdentifier);
	int GetActiveGameIdentifier() const;

private:
	int PlayerIdentifier;
	int InputSourceIdentifier;
	int PreferredCameraIndex;
	ViewportRectangleNormalized ViewportRectangle;
	int ActiveGameIdentifier;
};
