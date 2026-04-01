#pragma once

#include "Abstracts/Components/MeshUniversalComponent.h"
#include <directxmath.h>

class GridFloorRenderingComponent : public MeshUniversalComponent
{
public:
	GridFloorRenderingComponent();
	~GridFloorRenderingComponent() override;

	void SetGridHalfExtent(float NewGridHalfExtent);
	float GetGridHalfExtent() const;

	void SetMinorGridStep(float NewMinorGridStep);
	float GetMinorGridStep() const;

	void SetMajorLineStep(int NewMajorLineStep);
	int GetMajorLineStep() const;

	void SetLineWidth(float NewLineWidth);
	float GetLineWidth() const;

	void SetMinorLineColor(const DirectX::XMFLOAT4& NewMinorLineColor);
	const DirectX::XMFLOAT4& GetMinorLineColor() const;

	void SetMajorLineColor(const DirectX::XMFLOAT4& NewMajorLineColor);
	const DirectX::XMFLOAT4& GetMajorLineColor() const;

	bool InitializeRenderResources(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport) override;

private:
	void BuildGridGeometry();
	void AppendGridLineQuad(
		const DirectX::XMFLOAT3& StartPoint,
		const DirectX::XMFLOAT3& EndPoint,
		float LineHalfWidth,
		const DirectX::XMFLOAT4& LineColor);

	float GridHalfExtent;
	float MinorGridStep;
	int MajorLineStep;
	float LineWidth;
	DirectX::XMFLOAT4 MinorLineColor;
	DirectX::XMFLOAT4 MajorLineColor;
};
