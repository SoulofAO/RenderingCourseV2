#include "Abstracts/Components/GridFloorRenderingComponent.h"
#include <algorithm>
#include <cmath>

GridFloorRenderingComponent::GridFloorRenderingComponent()
	: MeshUniversalComponent()
	, GridHalfExtent(100.0f)
	, MinorGridStep(1.0f)
	, MajorLineStep(5)
	, LineWidth(0.025f)
	, MinorLineColor(0.20f, 0.20f, 0.20f, 1.0f)
	, MajorLineColor(0.35f, 0.35f, 0.35f, 1.0f)
{
	VertexShaderName = "./Shaders/Abstracts/GridFloor.hlsl";
	PixelShaderName = "./Shaders/Abstracts/GridFloor.hlsl";
	DeferredVertexShaderName = "./Shaders/Deferred/GridFloorDeferred.hlsl";
	DeferredPixelShaderName = "./Shaders/Deferred/GridFloorDeferred.hlsl";
	SetRenderOrder(-100);
}

GridFloorRenderingComponent::~GridFloorRenderingComponent() = default;

void GridFloorRenderingComponent::SetGridHalfExtent(float NewGridHalfExtent)
{
	GridHalfExtent = (std::max)(1.0f, NewGridHalfExtent);
}

float GridFloorRenderingComponent::GetGridHalfExtent() const
{
	return GridHalfExtent;
}

void GridFloorRenderingComponent::SetMinorGridStep(float NewMinorGridStep)
{
	MinorGridStep = (std::max)(0.1f, NewMinorGridStep);
}

float GridFloorRenderingComponent::GetMinorGridStep() const
{
	return MinorGridStep;
}

void GridFloorRenderingComponent::SetMajorLineStep(int NewMajorLineStep)
{
	MajorLineStep = (std::max)(1, NewMajorLineStep);
}

int GridFloorRenderingComponent::GetMajorLineStep() const
{
	return MajorLineStep;
}

void GridFloorRenderingComponent::SetLineWidth(float NewLineWidth)
{
	LineWidth = (std::max)(0.001f, NewLineWidth);
}

float GridFloorRenderingComponent::GetLineWidth() const
{
	return LineWidth;
}

void GridFloorRenderingComponent::SetMinorLineColor(const DirectX::XMFLOAT4& NewMinorLineColor)
{
	MinorLineColor = NewMinorLineColor;
}

const DirectX::XMFLOAT4& GridFloorRenderingComponent::GetMinorLineColor() const
{
	return MinorLineColor;
}

void GridFloorRenderingComponent::SetMajorLineColor(const DirectX::XMFLOAT4& NewMajorLineColor)
{
	MajorLineColor = NewMajorLineColor;
}

const DirectX::XMFLOAT4& GridFloorRenderingComponent::GetMajorLineColor() const
{
	return MajorLineColor;
}

bool GridFloorRenderingComponent::InitializeRenderResources(ID3D11Device* Device, SceneViewportSubsystem* SceneViewport)
{
	BuildGridGeometry();
	return MeshUniversalComponent::InitializeRenderResources(Device, SceneViewport);
}

void GridFloorRenderingComponent::BuildGridGeometry()
{
	Vertices.clear();
	Indices.clear();

	const float SafeGridHalfExtent = (std::max)(1.0f, GridHalfExtent);
	const float SafeMinorGridStep = (std::max)(0.1f, MinorGridStep);
	const int SafeMajorLineStep = (std::max)(1, MajorLineStep);
	const float LineHalfWidth = (std::max)(0.001f, LineWidth * 0.5f);
	const int GridLineCount = static_cast<int>(std::floor((SafeGridHalfExtent * 2.0f) / SafeMinorGridStep));

	for (int GridLineIndex = 0; GridLineIndex <= GridLineCount; GridLineIndex++)
	{
		const float PositionValue = -SafeGridHalfExtent + static_cast<float>(GridLineIndex) * SafeMinorGridStep;
		const bool IsMajorLine = (GridLineIndex % SafeMajorLineStep) == 0;
		const DirectX::XMFLOAT4& SelectedColor = IsMajorLine ? MajorLineColor : MinorLineColor;

		const DirectX::XMFLOAT3 StartXLine(-SafeGridHalfExtent, 0.0f, PositionValue);
		const DirectX::XMFLOAT3 EndXLine(SafeGridHalfExtent, 0.0f, PositionValue);
		AppendGridLineQuad(StartXLine, EndXLine, LineHalfWidth, SelectedColor);

		const DirectX::XMFLOAT3 StartZLine(PositionValue, 0.0f, -SafeGridHalfExtent);
		const DirectX::XMFLOAT3 EndZLine(PositionValue, 0.0f, SafeGridHalfExtent);
		AppendGridLineQuad(StartZLine, EndZLine, LineHalfWidth, SelectedColor);
	}
}

void GridFloorRenderingComponent::AppendGridLineQuad(
	const DirectX::XMFLOAT3& StartPoint,
	const DirectX::XMFLOAT3& EndPoint,
	float LineHalfWidth,
	const DirectX::XMFLOAT4& LineColor)
{
	const float DirectionX = EndPoint.x - StartPoint.x;
	const float DirectionZ = EndPoint.z - StartPoint.z;
	const float DirectionLength = std::sqrt(DirectionX * DirectionX + DirectionZ * DirectionZ);
	if (DirectionLength <= 0.0001f)
	{
		return;
	}

	const float InverseDirectionLength = 1.0f / DirectionLength;
	const float NormalizedDirectionX = DirectionX * InverseDirectionLength;
	const float NormalizedDirectionZ = DirectionZ * InverseDirectionLength;
	const float PerpendicularX = -NormalizedDirectionZ;
	const float PerpendicularZ = NormalizedDirectionX;
	const float OffsetX = PerpendicularX * LineHalfWidth;
	const float OffsetZ = PerpendicularZ * LineHalfWidth;
	const float SurfaceHeight = 0.002f;

	MeshUniversalVertex FirstVertex = {};
	FirstVertex.Position = DirectX::XMFLOAT4(StartPoint.x + OffsetX, SurfaceHeight, StartPoint.z + OffsetZ, 1.0f);
	FirstVertex.Color = LineColor;
	FirstVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	FirstVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	FirstVertex.TextureCoordinates = DirectX::XMFLOAT2(0.0f, 0.0f);

	MeshUniversalVertex SecondVertex = {};
	SecondVertex.Position = DirectX::XMFLOAT4(StartPoint.x - OffsetX, SurfaceHeight, StartPoint.z - OffsetZ, 1.0f);
	SecondVertex.Color = LineColor;
	SecondVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	SecondVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	SecondVertex.TextureCoordinates = DirectX::XMFLOAT2(1.0f, 0.0f);

	MeshUniversalVertex ThirdVertex = {};
	ThirdVertex.Position = DirectX::XMFLOAT4(EndPoint.x + OffsetX, SurfaceHeight, EndPoint.z + OffsetZ, 1.0f);
	ThirdVertex.Color = LineColor;
	ThirdVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	ThirdVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	ThirdVertex.TextureCoordinates = DirectX::XMFLOAT2(0.0f, 1.0f);

	MeshUniversalVertex FourthVertex = {};
	FourthVertex.Position = DirectX::XMFLOAT4(EndPoint.x - OffsetX, SurfaceHeight, EndPoint.z - OffsetZ, 1.0f);
	FourthVertex.Color = LineColor;
	FourthVertex.Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	FourthVertex.Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	FourthVertex.TextureCoordinates = DirectX::XMFLOAT2(1.0f, 1.0f);

	const unsigned int BaseIndex = static_cast<unsigned int>(Vertices.size());
	Vertices.push_back(FirstVertex);
	Vertices.push_back(SecondVertex);
	Vertices.push_back(ThirdVertex);
	Vertices.push_back(FourthVertex);

	Indices.push_back(BaseIndex + 0);
	Indices.push_back(BaseIndex + 1);
	Indices.push_back(BaseIndex + 2);
	Indices.push_back(BaseIndex + 2);
	Indices.push_back(BaseIndex + 1);
	Indices.push_back(BaseIndex + 3);
}
