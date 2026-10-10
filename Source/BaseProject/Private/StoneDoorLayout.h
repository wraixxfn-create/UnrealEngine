#pragma once

#include <algorithm>
#include <cmath>

/** Pure layout math, shared by the Unreal actor and standalone geometry tests. */
namespace BaseProjectStoneDoor
{
	inline constexpr int FrameCourseCount = 6;
	inline constexpr int LintelBlockCount = 5;
	inline constexpr int PanelStoneColumns = 2;
	inline constexpr int PanelStoneRows = 5;
	inline constexpr int LeafCount = 2;
	inline constexpr int FaceCount = 2;

	struct FLayout
	{
		float Width, Height, HalfWidth;
		float FrameThickness, FrameDepth, PanelThickness;
		float JointGap, CourseHeight, LintelWidth, SillHeight;
		float EdgeGap, Bottom, LeafHeight, LeafWidth, LeafRadius;
		float ReliefDepth, BorderDepth, ProfileDepth, BorderWidth, HingeDepth;
		float StoneGap, StoneWidth, StoneHeight, TriggerDepth, TriggerHalfWidth;
	};

	inline FLayout MakeLayout(float Width, float Height, float FrameThickness,
		float FrameDepth, float PanelThickness, float ThresholdHeight, float ActivationDistance)
	{
		FLayout L{};
		L.Width = std::max(200.0f, Width);
		L.Height = std::max(300.0f, Height);
		L.HalfWidth = L.Width * 0.5f;
		L.FrameThickness = std::max(25.0f, FrameThickness);
		L.FrameDepth = std::max(50.0f, FrameDepth);
		L.PanelThickness = std::clamp(PanelThickness, 5.0f, L.Width * 0.20f);
		L.CourseHeight = L.Height / FrameCourseCount;
		L.JointGap = std::clamp(std::min(L.FrameThickness, L.CourseHeight) * 0.025f, 1.0f, 8.0f);
		L.LintelWidth = L.Width + 2.0f * L.FrameThickness;
		L.SillHeight = std::clamp(ThresholdHeight, 0.0f, 30.0f);
		L.EdgeGap = std::clamp(L.PanelThickness * 0.06f, 2.0f, std::min(L.Width * 0.02f, L.Height * 0.025f));
		L.Bottom = L.SillHeight + L.EdgeGap;
		L.LeafHeight = L.Height - L.Bottom - L.EdgeGap;
		L.ReliefDepth = std::clamp(L.PanelThickness * 0.10f, 1.0f, 14.0f);
		L.BorderDepth = L.ReliefDepth * 1.4f;
		L.ProfileDepth = L.PanelThickness * 0.5f + L.BorderDepth * 0.90f;
		L.LeafRadius = L.HalfWidth - L.EdgeGap;
		// Include the thickness in the central seam, so the meeting edges cannot
		// cross each other at small opening angles.
		L.LeafWidth = std::sqrt(L.LeafRadius * L.LeafRadius - L.ProfileDepth * L.ProfileDepth) - L.EdgeGap * 0.5f;
		L.BorderWidth = std::clamp(L.LeafWidth * 0.055f, 3.0f, 40.0f);
		// Rear-mounted axes clear even the widest plinths during the full swing.
		L.HingeDepth = L.FrameDepth * 0.65f + L.EdgeGap;
		L.StoneGap = std::clamp(L.LeafWidth * 0.015f, 1.0f, 10.0f);
		L.StoneWidth = (L.LeafWidth - L.BorderWidth * 2.0f - L.StoneGap * (PanelStoneColumns + 1)) / PanelStoneColumns;
		L.StoneHeight = (L.LeafHeight - L.BorderWidth * 2.0f - L.StoneGap * (PanelStoneRows + 1)) / PanelStoneRows;
		// Keep all moving geometry, including reliefs, inside the activation area.
		L.TriggerDepth = std::max(L.FrameDepth * 0.5f + std::max(100.0f, ActivationDistance),
			L.LeafWidth + L.HingeDepth + L.ProfileDepth + 100.0f);
		L.TriggerHalfWidth = std::max(L.HalfWidth + L.FrameThickness * 0.5f,
			L.LeafRadius + L.PanelThickness * 0.5f + L.BorderDepth * 1.75f + 50.0f);
		return L;
	}
}
