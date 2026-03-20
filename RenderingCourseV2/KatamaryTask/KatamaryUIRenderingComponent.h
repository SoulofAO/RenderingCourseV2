#pragma once

#include "Abstracts/Components/UIRenderingComponent.h"

class KatamaryGame;

class KatamaryUIRenderingComponent : public UIRenderingComponent
{
public:
	KatamaryUIRenderingComponent();
	~KatamaryUIRenderingComponent() override;

protected:
	void RenderUI() override;

private:
	KatamaryGame* GetOwningKatamaryGame() const;
	bool UseWeldCollectMode;
};
