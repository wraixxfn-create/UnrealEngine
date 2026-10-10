#include "../Source/BaseProject/Private/StoneDoorLayout.h"

#include <iostream>
#include <stdexcept>
#include <string>

using namespace BaseProjectStoneDoor;

namespace
{
	void Require(bool Condition, const std::string& Message)
	{
		if (!Condition)
		{
			throw std::runtime_error(Message);
		}
	}

	void CheckLayout(const FLayout& L)
	{
		Require(L.Width >= 200.0f && L.Height >= 300.0f, "Opening dimensions must be clamped");
		Require(L.FrameDepth >= 50.0f && L.FrameThickness >= 25.0f, "Frame dimensions must be clamped");
		Require(L.LeafWidth > 0.0f && L.LeafHeight > 0.0f, "Leaves must have positive dimensions");
		Require(L.StoneWidth > 0.0f && L.StoneHeight > 0.0f, "Relief stones must have positive dimensions");
		Require(L.Bottom > L.SillHeight, "Leaves must clear the threshold");
		Require(L.Bottom + L.LeafHeight < L.Height, "Leaves must clear the lintel");
		Require(L.HingeDepth > L.FrameDepth * 0.65f, "Axes must clear the widest plinths");
		Require(L.SillHeight >= 0.0f && L.SillHeight <= 30.0f, "Threshold must be traversable");

		const double EmblemSize = std::min(L.LeafWidth * 0.24f, L.LeafHeight * 0.16f);
		const double EmblemHalfWidth = EmblemSize / std::sqrt(2.0);
		const double EmblemDepth = L.PanelThickness * 0.5 + L.BorderDepth * 1.75;
		for (int Step = 0; Step <= 180; ++Step)
		{
			const double Angle = Step * 0.5 * 3.14159265358979323846 / 180.0;
			const double C = std::cos(Angle);
			const double S = std::sin(Angle);
			auto CheckCorner = [&](double X, double Y)
			{
				// Left leaf yaw is negative. The right leaf is the mirror image.
				const double WorldX = L.HingeDepth + X * C + Y * S;
				const double WorldY = -L.LeafRadius - X * S + Y * C;
				Require(WorldY < 0.0, "Opposite leaves must never cross the centre plane");
				Require(std::abs(WorldX) < L.TriggerDepth, "Trigger must cover the entire opening arc in X");
				Require(std::abs(WorldY) < L.TriggerHalfWidth, "Trigger must cover the entire opening arc in Y");
				if (WorldY < -L.HalfWidth)
				{
					Require(WorldX > L.FrameDepth * 0.65f - 0.001, "Opening leaves must not enter the jambs or plinths");
				}
			};

			// Conservative XY envelopes include slab, stone reliefs and borders.
			for (const double X : {-double(L.ProfileDepth), double(L.ProfileDepth)})
			{
				CheckCorner(X, 0.0);
				CheckCorner(X, L.LeafWidth);
			}
			// The 45-degree diamond relief has a wider Y projection than its size.
			for (const double X : {-EmblemDepth, EmblemDepth})
			{
				CheckCorner(X, L.LeafWidth * 0.5 - EmblemHalfWidth);
				CheckCorner(X, L.LeafWidth * 0.5 + EmblemHalfWidth);
			}
		}
	}
}

int main()
{
	try
	{
		const FLayout Default = MakeLayout(1200, 1800, 260, 360, 100, 16, 900);
		CheckLayout(Default);
		Require(Default.Width == 1200 && Default.Height == 1800, "Default opening must be 12 x 18 metres");
		Require(Default.PanelThickness == 100, "Default stone leaves must be one metre thick");
		Require(Default.HalfWidth - Default.EdgeGap - Default.LeafWidth < 42.0f,
			"The closed central seam must not admit the player's 84 cm capsule");
		CheckLayout(MakeLayout(-1, -1, -1, -1, -1, -1, -1));

		int LayoutCount = 2;
		for (float W : {200.0f, 1200.0f, 4800.0f, 20000.0f})
		for (float H : {300.0f, 1800.0f})
		for (float FT : {25.0f, 260.0f})
		for (float FD : {50.0f, 360.0f, 700.0f})
		for (float PT : {5.0f, 100.0f, 1000.0f})
		for (float Sill : {0.0f, 16.0f, 30.0f})
		for (float Distance : {100.0f, 900.0f})
		{
			CheckLayout(MakeLayout(W, H, FT, FD, PT, Sill, Distance));
			++LayoutCount;
		}
		std::cout << "Validated " << LayoutCount << " layouts and their full 0-90 degree opening sweeps.\n";
		return 0;
	}
	catch (const std::exception& Error)
	{
		std::cerr << "Stone door layout test failed: " << Error.what() << '\n';
		return 1;
	}
}
