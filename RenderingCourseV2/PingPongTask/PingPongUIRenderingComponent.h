#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class PingPongGame;

class PingPongUIRenderingComponent : public UIRenderingComponent
{
public:
	PingPongUIRenderingComponent();
	~PingPongUIRenderingComponent() override;

protected:
	void RenderUI() override;

private:
	PingPongGame* GetOwningPingPongGame() const;
};
