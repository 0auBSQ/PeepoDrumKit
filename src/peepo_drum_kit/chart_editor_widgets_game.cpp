#include "chart_editor_widgets.h"

namespace PeepoDrumKit
{
	static constexpr f32 FrameToTime(f32 frame, f32 fps = 60.0f) { return (frame / fps); }
	static constexpr BezierKeyFrame2D GameNoteHitPath[] =
	{
		BezierKeyFrame2D::Linear(FrameToTime({  0 }), vec2 {  615, 386 }),
		BezierKeyFrame2D::Linear(FrameToTime({  1 }), vec2 {  639, 342 }),
		BezierKeyFrame2D::Linear(FrameToTime({  2 }), vec2 {  664, 300 }),
		BezierKeyFrame2D::Linear(FrameToTime({  3 }), vec2 {  693, 260 }),
		BezierKeyFrame2D::Linear(FrameToTime({  4 }), vec2 {  725, 222 }),
		BezierKeyFrame2D::Linear(FrameToTime({  5 }), vec2 {  758, 186 }),
		BezierKeyFrame2D::Linear(FrameToTime({  6 }), vec2 {  793, 153 }),
		BezierKeyFrame2D::Linear(FrameToTime({  7 }), vec2 {  830, 122 }),
		BezierKeyFrame2D::Linear(FrameToTime({  8 }), vec2 {  870, 93  }),
		BezierKeyFrame2D::Linear(FrameToTime({  9 }), vec2 {  912, 66  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 10 }), vec2 {  954, 43  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 11 }), vec2 { 1001, 27  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 12 }), vec2 { 1046, 11  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 13 }), vec2 { 1094, -2  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 14 }), vec2 { 1142, -14 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 15 }), vec2 { 1192, -18 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 16 }), vec2 { 1240, -22 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 17 }), vec2 { 1292, -23 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 18 }), vec2 { 1336, -22 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 19 }), vec2 { 1385, -16 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 20 }), vec2 { 1435, -8  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 21 }), vec2 { 1479, 3   }),
		BezierKeyFrame2D::Linear(FrameToTime({ 22 }), vec2 { 1526, 16  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 23 }), vec2 { 1570, 36  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 24 }), vec2 { 1612, 56  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 25 }), vec2 { 1658, 83  }),
		BezierKeyFrame2D::Linear(FrameToTime({ 26 }), vec2 { 1696, 115 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 27 }), vec2 { 1734, 144 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 28 }), vec2 { 1770, 176 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 29 }), vec2 { 1803, 210 }),
		BezierKeyFrame2D::Linear(FrameToTime({ 30 }), vec2 { 1836, 247 }),
	};

	static constexpr BezierKeyFrame1D GameNoteHitFadeIn[] =
	{
		{ FrameToTime(30.0f), 0.0f, 0.0f, 0.0f },
		{ FrameToTime(39.0f), 1.0f, 1.0f, 1.0f },
	};
	static constexpr BezierKeyFrame1D GameNoteHitFadeOut[] =
	{
		{ FrameToTime(39.0f), 1.0f, 1.0f, 1.0f },
		{ FrameToTime(46.0f), 0.0f, 0.0f, 0.0f },
	};
	static constexpr Time GameNoteHitAnimationDuration = Time::FromSec(GameNoteHitPath[ArrayCount(GameNoteHitPath) - 1].Time); // Time::FromFrames(46.0);
	static constexpr Time GameHandNoteHitSquashDuration = Time::FromSec(0.15);
	static constexpr Time GetTotalGameNoteHitAnimationDuration(NoteType noteType) { return (IsHandNote(noteType) ? GameHandNoteHitSquashDuration + GameNoteHitAnimationDuration : GameNoteHitAnimationDuration); }

	struct NoteHitPathAnimationData
	{
		vec2 PositionOffset = {};
		f32 WhiteFadeIn = 0.0f;
		f32 AlphaFadeOut = 1.0f;
		f32 HandSquashYOffset = 0;
		f32 HandSquashScale = 1.0f;
		Angle HandRotate = Angle::FromRadians(0);
		b8 HasBeenHit = false;
	};

	static NoteHitPathAnimationData GetNoteHitPathAnimation(Time timeSinceHit, f32 extendedLaneWidthFactor, i32 nLanes, i32 iLane, NoteType noteType)
	{
		NoteHitPathAnimationData out {};
		if (out.HasBeenHit = timeSinceHit >= Time::Zero(); out.HasBeenHit) {
			if (nLanes > 2) {
				out.AlphaFadeOut = 0;
				return out;
			}

			if (IsHandNote(noteType)) {
				if (timeSinceHit < GameHandNoteHitSquashDuration) {
					static constexpr f32 squashAmountMax = 0.15;
					// y = 1 ... (1 - dy) ... 1
					// t = 0 ... 1/2      ... 1 => y = (1 - dy) + dy * (2 * (t - 1/2)) ^ 2
					f32 squashFactor = (1 - squashAmountMax) + squashAmountMax * std::pow(2 * (timeSinceHit / GameHandNoteHitSquashDuration - 0.5), 2);
					out.HandSquashScale = squashFactor;
					out.HandSquashYOffset = (GameLaneSlice.TotalHeight() / 2) * (1 - squashFactor);
					if (iLane == 1)
						out.HandSquashYOffset *= -1;
					return out;
				}

				timeSinceHit -= GameHandNoteHitSquashDuration;
			}

			const f32 animationTime = timeSinceHit.ToSec_F32();
			out.PositionOffset = SampleBezierFCurve(GameNoteHitPath, animationTime) - GameNoteHitPath[0].Value;
			out.PositionOffset.x *= extendedLaneWidthFactor;
#if 0 // TODO: Just doesn't really look that great... maybe also needs the other hit effects too
			out.WhiteFadeIn = SampleBezierFCurve(NoteHitFadeIn, animationTime);
			out.AlphaFadeOut = SampleBezierFCurve(NoteHitFadeOut, animationTime);
#endif

			if (IsHandNote(noteType)) {
				// rotation angle is roughly proportional to the height above hit position
				static constexpr Angle rotateMax = Angle::FromDegrees(45);
				static constexpr auto v0 = GameNoteHitPath[1].Value - GameNoteHitPath[0].Value;
				static constexpr auto vt = GameNoteHitPath[std::size(GameNoteHitPath) - 1].Value - GameNoteHitPath[std::size(GameNoteHitPath) - 2].Value;
				// d(cos(theta = w * t + theta0))/dt = -w * sin(theta) = w * cos(thetaV) => cos(thetaV) = sin(90 deg - thetaV) = -sin(theta) = sin(-theta) => thetaV = theta + 90deg
				static auto theta0 = std::atan2(v0.y, v0.x) - PI / 2; // atan2 range: (-pi, pi], clockwise is positive => w > 0
				static auto theta1 = std::atan2(vt.y, vt.x) - PI / 2;
				static auto sin0 = std::sin(theta0);
				f32 theta = ConvertRange(0.0f, GameNoteHitAnimationDuration.ToSec_F32(), theta0, theta1, animationTime);
				out.HandRotate = rotateMax * (sin(theta) - sin0) / ((-1) - sin0); // heightest point is most negative
			}
			if (iLane == 1) {
				out.PositionOffset.y *= -1;
				out.HandRotate *= -1;
			}
		}
		return out;
	}

	// fire, lane
	static std::pair<f32, f32> getGogoZoomAmount(b8 isGogo, Time timeSinceGogo, Time timeAfterGogo)
	{
		static constexpr auto tAttLane = Time::FromSec(0.10);
		f32 laneAmount = !isGogo ? 0 : timeSinceGogo >= tAttLane ? 1 : (timeSinceGogo / tAttLane);
		static constexpr auto tAtt = Time::FromSec(0.05); // attack
		static constexpr auto tDec = Time::FromSec(0.20); // decay
		static constexpr auto tRel = Time::FromSec(0.10); // release
		Time fireTime = isGogo ? std::min(timeSinceGogo, tAtt + tDec) : std::min(timeSinceGogo, tAtt + tDec) + timeAfterGogo;
		f32 fireAmount = (fireTime > tAtt + tDec + tRel) ? 0
			: (fireTime > tAtt + tDec) ? std::max(0.0, 1 - pow(((fireTime - (tAtt + tDec)) / tRel), 2))
			: (fireTime >= tAtt + tDec) ? 1
			: (fireTime >= tAtt) ? 2 - (1 - pow(1 - (fireTime - tAtt) / tDec, 2))
			: 2 * (fireTime / tAtt);
		return { fireAmount, laneAmount };
	}

	static constexpr Time TimeSinceNoteHit(Time noteTime, Time cursorTime) { return (cursorTime - noteTime); }

	static u32 InterpolateDrumrollHitColor(NoteType noteType, f32 hitPercentage)
	{
		u32 hitNoteColor = 0xFFFFFFFF;
		if (hitPercentage > 0.0f)
			hitNoteColor = Gui::ColorConvertFloat4ToU32(ImLerp(Gui::ColorConvertU32ToFloat4(hitNoteColor), Gui::ColorConvertU32ToFloat4(NoteColorDrumrollHit), hitPercentage));
		return hitNoteColor;
	}

	constexpr static std::pair<Angle, bool> GetNoteFaceRotationMirror(Tempo tempo, Complex scrollSpeed, NoteType noteType)
	{
		switch (noteType) {
		default:
			return {
				Angle::FromRadians(arg(((tempo.BPM >= 0) == (scrollSpeed.GetRealPart() >= 0)) ? scrollSpeed.cpx : -scrollSpeed.cpx)),
				(scrollSpeed.GetRealPart() < 0),
			}; // prevent top-down rotation; mirror horizontally for negative scroll balloon
		case NoteType::Bomb:
			return { Angle::FromRadians(0), false }; // no rotation
		}
	}

	static void DrawGamePreviewNote(ChartGraphicsResources& gfx, const GameCamera& camera, ImDrawList* drawList, vec2 center, Tempo tempo, Complex scrollSpeed, NoteType noteType, Time currentTime,
		const NoteHitPathAnimationData& hitAnimation = {}, i32 nLanes = 1, i32 iLane = 0)
	{
		SprID spr = SprID::Count;
		switch (noteType)
		{
		case NoteType::Don: { spr = SprID::Game_Note_Don; } break;
		case NoteType::DonBig: { spr = SprID::Game_Note_DonBig; } break;
		case NoteType::DonBigHand: { spr = (hitAnimation.HasBeenHit ? SprID::Game_Note_DonBig : SprID::Game_Note_DonHand); } break;
		case NoteType::Ka: { spr = SprID::Game_Note_Ka; } break;
		case NoteType::KaBig: { spr = SprID::Game_Note_KaBig; } break;
		case NoteType::KaBigHand: { spr = (hitAnimation.HasBeenHit ? SprID::Game_Note_KaBig : SprID::Game_Note_KaHand); } break;
		case NoteType::Drumroll: { spr = SprID::Game_Note_Drumroll; } break;
		case NoteType::DrumrollBig: { spr = SprID::Game_Note_DrumrollBig; } break;
		case NoteType::Balloon: { spr = SprID::Game_Note_Balloon; } break;
		case NoteType::BalloonSpecial: { spr = SprID::Game_Note_BalloonSpecial; } break;
		case NoteType::KaDon: { spr = SprID::Game_Note_KaDon; } break;
		case NoteType::Adlib: { spr = SprID::Game_Note_Adlib; } break;
		case NoteType::Fuse: { spr = SprID::Game_Note_Fuse; } break;
		case NoteType::Bomb: { spr = SprID::Game_Note_Bomb; } break;
		}

		// TODO: Right part stretch animation
		if (noteType == NoteType::Balloon) { /* ... */ }

		if (IsHandNote(noteType)) {
			static constexpr f32 armMovePeriod = 0.25;
			static constexpr f32 armMoveMax = 20;
			static constexpr f32 armMoveHit = 5;
			f32 phase = fmod(2 * currentTime.Seconds / armMovePeriod + 1.5, 2); // expected 0 to 2, start at 1.5
			if (phase < 0)
				phase += 2;
			f32 amplitude = 2 * abs(phase - 1) - 1; // expected 1 to -1 to 1, start at increasing 0
			auto drawArm = [&](vec2 flip)
			{
				f32 armMove = hitAnimation.HasBeenHit ? flip.y * armMoveHit
					: flip.x * armMoveMax * amplitude;
				gfx.DrawSprite(drawList, SprID::Game_Note_ArmDown, SprTransform::FromCenter(
					camera.WorldToScreenSpace({ center.x, center.y + armMove }),
					flip * camera.WorldToScreenScale(1.0f), hitAnimation.HandRotate));
			};
			if (iLane != nLanes - 1 || nLanes == 1) { // always show arms when not in comparison mode
				drawArm({ 1, 1 });
				drawArm({ -1, 1 });
			}
			if (iLane != 0) {
				drawArm({ 1, -1 });
				drawArm({ -1, -1 });
			}
			center.y += hitAnimation.HandSquashYOffset;
		}

		const auto [angle, mirror] = GetNoteFaceRotationMirror(tempo, scrollSpeed, noteType);
		gfx.DrawSprite(drawList, spr, SprTransform::FromCenter(
			camera.WorldToScreenSpace(center),
			vec2(camera.WorldToScreenScale(mirror ? -1.0f : 1.0f), camera.WorldToScreenScale(hitAnimation.HandSquashScale)),
			angle));
	}

	static void DrawGamePreviewNoteDuration(ChartGraphicsResources& gfx, const GameCamera& camera, ImDrawList* drawList, vec2 centerHead, vec2 centerTail, NoteType noteType, u32 colorTint = 0xFFFFFFFF)
	{
		const SprID spr = IsFuseRoll(noteType)
			? SprID::Game_Note_FuseLong
			: (IsBigNote(noteType) 
				? SprID::Game_Note_DrumrollLongBig 
				: SprID::Game_Note_DrumrollLong);
		const SprInfo sprInfo = gfx.GetInfo(spr);

		const f32 midScaleX = (Distance(centerTail, centerHead) / (sprInfo.SourceSize.x)) * 3.0f;

		const SprStretchtOut split = StretchMultiPartSpr(gfx, spr,
			SprTransform::FromCenter(
				camera.WorldToScreenSpace((centerHead + centerTail) / 2.0f),
				vec2(camera.WorldToScreenScale(1.0f)),
				AngleBetween(centerTail, centerHead)),
				colorTint,
			SprStretchtParam { 1.0f, midScaleX, 1.0f }, 3);

		for (size_t i = 0; i < 3; i++)
			gfx.DrawSprite(drawList, split.Quads[i]);
	}

	static void DrawGamePreviewNoteSEText(ChartGraphicsResources& gfx, const GameCamera& camera, ImDrawList* drawList, vec2 centerHead, vec2 centerTail, Tempo tempo, Complex scrollSpeed, NoteSEType seType)
	{
		static constexpr f32 contentToFooterOffsetY = (GameLaneSlice.FooterCenterY() - GameLaneSlice.ContentCenterY());
		centerHead.y += contentToFooterOffsetY;
		centerTail.y += contentToFooterOffsetY;

		SprID spr = SprID::Count;
		switch (seType)
		{
		case NoteSEType::Do: { spr = SprID::Game_NoteTxt_Do; } break;
		case NoteSEType::Ko: { spr = SprID::Game_NoteTxt_Ko; } break;
		case NoteSEType::Don: { spr = SprID::Game_NoteTxt_Don; } break;
		case NoteSEType::DonBig: { spr = SprID::Game_NoteTxt_DonBig; } break;
		case NoteSEType::DonHand: { spr = SprID::Game_NoteTxt_DonHand; } break;
		case NoteSEType::Ka: { spr = SprID::Game_NoteTxt_Ka; } break;
		case NoteSEType::Katsu: { spr = SprID::Game_NoteTxt_Katsu; } break;
		case NoteSEType::KatsuBig: { spr = SprID::Game_NoteTxt_KatsuBig; } break;
		case NoteSEType::KatsuHand: { spr = SprID::Game_NoteTxt_KatsuHand; } break;
		case NoteSEType::Drumroll: { spr = SprID::Game_NoteTxt_Drumroll; } break;
		case NoteSEType::DrumrollBig: { spr = SprID::Game_NoteTxt_DrumrollBig; } break;
		case NoteSEType::Balloon: { spr = SprID::Game_NoteTxt_Balloon; } break;
		case NoteSEType::BalloonSpecial: { spr = SprID::Game_NoteTxt_BalloonSpecial; } break;
		default: return;
		}

		if (seType == NoteSEType::Drumroll || seType == NoteSEType::DrumrollBig)
		{
			const SprInfo sprInfo = gfx.GetInfo(spr);

			const f32 midAlignmentOffset = (seType == NoteSEType::DrumrollBig) ? 136.0f : 68.0f;
			const f32 distance = Distance(centerTail, centerHead);
			const f32 midScaleX = (ClampBot(distance - midAlignmentOffset, 0.0f) / (sprInfo.SourceSize.x)) * 3.0f;

			const SprStretchtOut split = StretchMultiPartSpr(gfx, spr,
				SprTransform::FromCenter(
					camera.WorldToScreenSpace((centerHead + centerTail) / 2.0f),
					vec2(camera.WorldToScreenScale(1.0f)),
					(distance < 1) ? Angle::FromRadians(arg((tempo.BPM >= 0) ? scrollSpeed.cpx : -scrollSpeed.cpx))
					: AngleBetween(centerTail, centerHead)), // TODO: only rotate the mid part?
				0xFFFFFFFF,
				SprStretchtParam { 1.0f, midScaleX, 1.0f }, 3);

			for (size_t i = 0; i < 3; i++)
				gfx.DrawSprite(drawList, split.Quads[i]);
		}
		else
		{
			gfx.DrawSprite(drawList, spr, SprTransform::FromCenter(camera.WorldToScreenSpace(centerHead), vec2(camera.WorldToScreenScale(1.0f))));
		}
	}

	static void DrawGamePreviewNumericText(ChartGraphicsResources& gfx, const GameCamera& camera, ImDrawList* drawList, SprTransform baseTransform, std::string_view text, u32 color = 0xFFFFFFFF)
	{
		// TODO: Make more generic by taking in an array of glyph rects as lookup table (?)
		static constexpr std::string_view sprFontNumericalCharSet = "0123456789+-./%";
		static constexpr f32 advanceX = 13.0f;
		const SprInfo sprInfo = gfx.GetInfo(SprID::Game_Font_Numerical);
		const vec2 perCharSprSize = vec2(sprInfo.SourceSize.x, sprInfo.SourceSize.y / static_cast<f32>(sprFontNumericalCharSet.size()));

		vec2 pivotOffset = vec2(0.0f);
		if (baseTransform.Pivot.x != 0.0f || baseTransform.Pivot.y != 0.0f)
		{
			vec2 totalSize = vec2(0.0f, sprInfo.SourceSize.y);
			for (const char c : text)
			{
				i32 charIndex = -1;
				for (i32 i = 0; i < static_cast<i32>(sprFontNumericalCharSet.size()); i++)
					if (c == sprFontNumericalCharSet[i])
						charIndex = i;

				if (charIndex < 0)
					continue;

				totalSize.x += advanceX;
			}

			pivotOffset = (totalSize * baseTransform.Pivot);
		}

		// offset the output to pivot the advance box within the character sprite box
		vec2 writeHead = vec2((advanceX - sprInfo.SourceSize.x) * (sprInfo.SourceSize.x / advanceX) * baseTransform.Pivot.x, 0);
		for (const char c : text)
		{
			i32 charIndex = -1;
			for (i32 i = 0; i < static_cast<i32>(sprFontNumericalCharSet.size()); i++)
				if (c == sprFontNumericalCharSet[i])
					charIndex = i;

			if (charIndex < 0)
				continue;

			SprTransform charTransform = SprTransform::FromTL(camera.WorldToScreenSpace(baseTransform.Position), vec2(camera.WorldToScreenScale(1.0f)), baseTransform.Rotation);
			SprUV charUV = SprUV::FromRect(
				vec2(0.0f, static_cast<f32>(charIndex + 0) * perCharSprSize.y / sprInfo.SourceSize.y),
				vec2(1.0f, static_cast<f32>(charIndex + 1) * perCharSprSize.y / sprInfo.SourceSize.y));

			// calculate character box's pivot point from advance box's pivot point
			charTransform.Pivot.x += ((pivotOffset.x - writeHead.x) * (advanceX / sprInfo.SourceSize.x) / sprInfo.SourceSize.x);
			charTransform.Pivot.y += (pivotOffset.y / sprInfo.SourceSize.y);

			charTransform.Scale *= baseTransform.Scale;
			charTransform.Scale.y /= static_cast<f32>(sprFontNumericalCharSet.size());

			gfx.DrawSprite(drawList, SprID::Game_Font_Numerical, charTransform, color, &charUV);

			writeHead.x += advanceX;
		}
	}

	struct ForEachBarLaneData
	{
		Beat Beat;
		Time Time;
		Tempo Tempo;
		Complex ScrollSpeed;
		ScrollMethod ScrollType;
		i32 BarIndex;
	};

	template <typename Func>
	static void ForEachBarOnNoteLane(const ChartCourse& course, BranchType branch, Beat maxBeatDuration, Func perBarFunc)
	{
		BeatSortedForwardIterator<TempoChange> tempoChangeIt {};
		BeatSortedForwardIterator<ScrollChange> scrollChangeIt {};
		BeatSortedForwardIterator<BarLineChange> barLineChangeIt {};
		BeatSortedForwardIterator<ScrollType> scrollTypeIt {};
		BeatSortedForwardIterator<JPOSScrollChange> JPOSscrollChangeIt {};

		course.TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
		{
			if (it.Beat > maxBeatDuration)
				return ControlFlow::Break;

			if (!it.IsBar)
				return ControlFlow::Continue;

			if (!VisibleOrDefault(barLineChangeIt.Next(course.BarLineChanges.Sorted, it.Beat)))
				return ControlFlow::Continue;

			const Time time = course.TempoMap.BeatToTime(it.Beat);
			// Use Normal scroll as base; branch-specific overrides within branch sections
			const Complex scrollAtBeat = [&]() -> Complex {
				if (branch != BranchType::Normal) {
					const auto& branchList = course.GetScrollChanges(branch);
					if (!branchList.empty()) {
						if (const ScrollChange* sc = branchList.TryFindLastAtBeat(it.Beat)) return sc->ScrollSpeed;
					}
				}
				return ScrollOrDefault(scrollChangeIt.Next(course.ScrollChanges.Sorted, it.Beat));
			}();
			const ScrollMethod scrollTypeAtBeat = [&]() -> ScrollMethod {
				if (branch != BranchType::Normal) {
					const auto& branchList = course.GetScrollTypes(branch);
					if (!branchList.empty()) {
						if (const ScrollType* st = branchList.TryFindLastAtBeat(it.Beat)) return st->Method;
					}
				}
				return ScrollTypeOrDefault(scrollTypeIt.Next(course.ScrollTypes.Sorted, it.Beat));
			}();
			perBarFunc(ForEachBarLaneData { it.Beat, time,
				TempoOrDefault(tempoChangeIt.Next(course.TempoMap.Tempo.Sorted, it.Beat)),
				scrollAtBeat, scrollTypeAtBeat,
				it.BarIndex });

			return ControlFlow::Continue;
		});
	}

	struct ForEachNoteLaneData
	{
		const Note* OriginalNote;
		Beat Beat;
		Time Time;
		Tempo Tempo;
		Complex ScrollSpeed;
		ScrollMethod ScrollType;
		struct {
			struct Beat Beat;
			struct Time Time;
			struct Tempo Tempo;
			Complex ScrollSpeed;
			ScrollMethod ScrollType;
		} Tail;
	};

	template <typename Func>
	static void ForEachNoteOnNoteLane(const ChartCourse& course, BranchType branch, Func perNoteFunc)
	{
		BeatSortedForwardIterator<TempoChange> tempoChangeIt {};
		BeatSortedForwardIterator<ScrollChange> scrollChangeIt {};
		BeatSortedForwardIterator<ScrollType> scrollTypeIt {};
		BeatSortedForwardIterator<JPOSScrollChange> JPOSscrollChangeIt {};

		for (const Note& note : course.GetNotes(branch))
		{
			const Beat beat = note.BeatTime;
			const Time head = (course.TempoMap.BeatToTime(beat) + note.TimeOffset);
			const Beat beatTail = (note.BeatDuration > Beat::Zero()) ? (beat + note.BeatDuration) : beat;
			const Time tail = (note.BeatDuration > Beat::Zero()) ? (course.TempoMap.BeatToTime(beatTail) + note.TimeOffset) : head;
			// Use Normal scroll as base; branch-specific overrides when available
		auto getEffectiveScroll = [&](Beat b) -> Complex {
			if (branch != BranchType::Normal) {
				const auto& branchList = course.GetScrollChanges(branch);
				if (!branchList.empty()) { if (const ScrollChange* sc = branchList.TryFindLastAtBeat(b)) return sc->ScrollSpeed; }
			}
			return ScrollOrDefault(scrollChangeIt.Next(course.ScrollChanges.Sorted, b));
		};
		auto getEffectiveScrollType = [&](Beat b) -> ScrollMethod {
			if (branch != BranchType::Normal) {
				const auto& branchList = course.GetScrollTypes(branch);
				if (!branchList.empty()) { if (const ScrollType* st = branchList.TryFindLastAtBeat(b)) return st->Method; }
			}
			return ScrollTypeOrDefault(scrollTypeIt.Next(course.ScrollTypes.Sorted, b));
		};
		perNoteFunc(ForEachNoteLaneData { &note, beat, head,
				TempoOrDefault(tempoChangeIt.Next(course.TempoMap.Tempo.Sorted, beat)),
				getEffectiveScroll(beat), getEffectiveScrollType(beat),
				{
					beatTail, tail,
					TempoOrDefault(tempoChangeIt.Next(course.TempoMap.Tempo.Sorted, beatTail)),
					getEffectiveScroll(beatTail), getEffectiveScrollType(beatTail),
				},
			});
		}
	}

	void ChartCourse::RecalculateSENotes(BranchType branch) const
	{
		enum class SEFormType { Long, Short, Alternate, Final };

		// prev, curr, next, n(ext)2nd
		ForEachNoteLaneData noteDataRingBuffer[4] = {};
		i32 noteDataRingOffset = 0;
		auto getNoteData = [&](i32 idx) -> decltype(auto) { return noteDataRingBuffer[(noteDataRingOffset + idx) & 3]; };

		// distance when curr is on the judgement mark
		// other is NMScroll: visual beat distance = sec_time * visual_beat_per_second_other
		// other is HBScroll: visual beat distance = scroll_other * beat_distance
		auto getVisualBeat = [&](const auto& curr, const auto& other, f32 scrollOther, f32 vbpsOther, Time timeDistance)
		{
			return (other.OriginalNote == nullptr) ? F32Max
				: (other.ScrollType == ScrollMethod::NMSCROLL) ? vbpsOther * timeDistance.Seconds
				: (other.ScrollType == ScrollMethod::HBSCROLL) ? scrollOther * abs(curr.Beat - other.Beat).Ticks / Beat::TicksPerBeat
				: /* (prev.ScrollType == ScrollMethod::BMSCROLL) ? */ abs(curr.Beat - other.Beat).Ticks / Beat::TicksPerBeat;
		};

		auto getNoteDistance = [&]()
		{
			const auto& prev = getNoteData(0);
			const auto& curr = getNoteData(1);
			const auto& next = getNoteData(2);
			const auto& n2nd = getNoteData(3);
			const f32 scrollPrev = abs(prev.ScrollSpeed.cpx);
			const f32 scrollNextCapped = std::min(1.0f, abs(next.ScrollSpeed.cpx));
			// visual beat per second
			const f32 vbpsPrev = scrollPrev * prev.Tempo.BPM / 60;
			const f32 vbpsNextCapped = scrollNextCapped * next.Tempo.BPM / 60;
			// time distance
			const Time tdToPrev = (prev.OriginalNote == nullptr) ? Time::FromSec(F32Max) : (curr.Time - prev.Time);
			const Time tdToNext = (next.OriginalNote == nullptr) ? Time::FromSec(F32Max) : (next.Time - curr.Time);
			const Time tdToN2nd = (n2nd.OriginalNote == nullptr) ? Time::FromSec(F32Max) : (n2nd.Time - next.Time);
			const f32 vbdToPrev = getVisualBeat(curr, prev, scrollPrev, vbpsPrev, tdToPrev);
			const f32 vbdToNextCapped = getVisualBeat(curr, next, scrollNextCapped, vbpsNextCapped, tdToNext);
			return std::tuple{ tdToPrev, vbdToPrev, tdToNext, vbdToNextCapped, tdToN2nd };
		};

		const SortedNotesList& notes = GetNotes(branch);
		std::vector<const Note*> alterChain;
		b8 isAlterChain = true;
		Time timeIntervalAlter = Time::Zero();
		Time timeStartAlter = Time::Zero();

		auto assignSingleNote = [&]()
		{
			auto& curr = getNoteData(1);
			const Note& it = *getNoteData(1).OriginalNote;
			auto [tdToPrev, vbdToPrev, tdToNext, vbdToNextCapped, tdToN2nd] = getNoteDistance();
			const Time timeEpsilon = Time::FromMS(1e-3);
			const b8 denseToSparse = (tdToNext >= tdToPrev + timeEpsilon);
			const b8 sparseToDense = (tdToN2nd <= tdToNext - timeEpsilon);
			const f32 beatsEpsilon = 4 / 192.0;
			const b8 isLongAvoided = (vbdToPrev <= 4 / 16.0 - beatsEpsilon
				|| vbdToNextCapped <= 4 / 12.0 - beatsEpsilon); // avoid text from overlapping or extending under next note
			const b8 isPrePause = (vbdToNextCapped >= 4 / 8.0 + beatsEpsilon);
			auto se = (!isLongAvoided && (denseToSparse || sparseToDense || isPrePause)) ? SEFormType::Long : SEFormType::Short;
			if (isAlterChain) {
				if (it.Type == NoteType::Don && alterChain.empty()) {
					timeIntervalAlter = tdToNext;
					timeStartAlter = curr.Time;
					alterChain.push_back(&it);
				} else if (it.Type == NoteType::Don && abs(tdToPrev - timeIntervalAlter) < timeEpsilon && abs(timeStartAlter - curr.Time) < Time::FromSec(0.5) + timeEpsilon) {
					alterChain.push_back(&it);
				} else {
					isAlterChain = false;
					alterChain.clear();
				}
			}
			if (denseToSparse || sparseToDense) {
				if (denseToSparse && isAlterChain && !isLongAvoided && size(alterChain) % 2 != 0 && abs(timeStartAlter - curr.Time) < Time::FromSec(0.5) + timeEpsilon) {
					for (i32 ia = 0; ia < size(alterChain); ++ia) {
						if (ia % 2 == 1)
							alterChain[ia]->TempSEType = NoteSEType::Ko;
					}
				}
				alterChain.clear();
				isAlterChain = sparseToDense;
			}

			switch (it.Type)
			{
			case NoteType::Don: { it.TempSEType = (se == SEFormType::Long) ? NoteSEType::Don : NoteSEType::Do; } break;
			case NoteType::DonBig: { it.TempSEType = NoteSEType::DonBig; } break;
			case NoteType::DonBigHand: { it.TempSEType = NoteSEType::DonHand; } break;
			case NoteType::Ka: { it.TempSEType = (se == SEFormType::Long) ? NoteSEType::Katsu : NoteSEType::Ka; } break;
			case NoteType::KaBig: { it.TempSEType = NoteSEType::KatsuBig; } break;
			case NoteType::KaBigHand: { it.TempSEType = NoteSEType::KatsuHand; } break;
			case NoteType::Drumroll: { it.TempSEType = NoteSEType::Drumroll; } break;
			case NoteType::DrumrollBig: { it.TempSEType = NoteSEType::DrumrollBig; } break;
			case NoteType::Balloon: { it.TempSEType = NoteSEType::Balloon; } break;
			case NoteType::BalloonSpecial: { it.TempSEType = NoteSEType::BalloonSpecial; } break;
			default: { it.TempSEType = NoteSEType::Count; } break;
			}
		};

		// fetch 2nd next note, update current note
		i32 lastFilled = 0;
		ForEachNoteOnNoteLane(*this, branch, [&](const ForEachNoteLaneData& dataIt)
		{
			if (getNoteData(1).OriginalNote != nullptr)
				assignSingleNote();
			noteDataRingOffset = (noteDataRingOffset + 1) & 3;
			getNoteData(3) = dataIt;
			lastFilled = 3;
		});
		for (; lastFilled >= 1; --lastFilled) {
			if (getNoteData(1).OriginalNote != nullptr)
				assignSingleNote();
			noteDataRingOffset = (noteDataRingOffset + 1) & 3;
			getNoteData(3).OriginalNote = nullptr;
		}
	}

	void ChartGamePreview::DrawGui(ChartContext& context, Time animatedCursorTime)
	{
		const i32 nLanes = size(context.ChartsCompared);

		static constexpr vec2 buttonMargin = vec2(8.0f);
		vec2 minContentRectSize = vec2(128.0f, nLanes * 72.0f);
		Gui::BeginDisabled();
		Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_Button, 0.25f));
		Gui::Button("##GamePreview", ClampBot(vec2(Gui::GetContentRegionAvail()), minContentRectSize));
		Gui::PopStyleColor();
		Gui::EndDisabled();
		Camera.ScreenSpaceViewportRect = Gui::GetItemRect();
		Camera.ScreenSpaceViewportRect.TL = Camera.ScreenSpaceViewportRect.TL + buttonMargin;
		Camera.ScreenSpaceViewportRect.BR = ClampBot(Camera.ScreenSpaceViewportRect.BR - buttonMargin, Camera.ScreenSpaceViewportRect.TL + (minContentRectSize - vec2(buttonMargin.x, 0.0f)));
		{
			f32 newAspectRatio = GetAspectRatio(Camera.ScreenSpaceViewportRect);
			if (const f32 min = GetAspectRatio(*Settings.General.GameViewportAspectRatioMin); min != 0.0f) newAspectRatio = ClampBot(newAspectRatio, min);
			if (const f32 max = GetAspectRatio(*Settings.General.GameViewportAspectRatioMax); max != 0.0f) newAspectRatio = ClampTop(newAspectRatio, max);

			if (newAspectRatio != 0.0f && newAspectRatio != GetAspectRatio(Camera.ScreenSpaceViewportRect))
				Camera.ScreenSpaceViewportRect = FitInsideFixedAspectRatio(Camera.ScreenSpaceViewportRect, newAspectRatio);
		}

		Rect laneRectBase;
		const vec2 standardSize = vec2(GameLaneStandardWidth + GameLanePaddingL + GameLanePaddingR, nLanes * GameLaneSlice.TotalHeight() + GameLanePaddingTop + GameLanePaddingBot);
		const f32 standardAspectRatio = GetAspectRatio(standardSize);
		const f32 viewportAspectRatio = GetAspectRatio(Camera.ScreenSpaceViewportRect);
		if (viewportAspectRatio <= standardAspectRatio) // NOTE: Standard case of (<= 16:9) with a fixed sized lane being centered
		{
			Camera.WorldSpaceSize = vec2(standardSize.x, standardSize.x / viewportAspectRatio);
			Camera.WorldToScreenScaleFactor = Camera.ScreenSpaceViewportRect.GetWidth() / standardSize.x;
			laneRectBase = Rect::FromTLSize(vec2(GameLanePaddingL, GameLanePaddingTop), vec2(GameLaneStandardWidth, GameLaneSlice.TotalHeight()));
			laneRectBase += vec2(0.0f, (Camera.WorldSpaceSize.y * 0.5f) - (standardSize.y * 0.5f));
		}
		else // NOTE: Ultra wide case of (> 16:9) with the lane being extended further to the right
		{
			const f32 extendedLaneWidth = (standardSize.x * (viewportAspectRatio / standardAspectRatio)) - GameLanePaddingL - GameLanePaddingR;
			Camera.WorldSpaceSize = vec2(extendedLaneWidth, standardSize.y);
			Camera.WorldToScreenScaleFactor = Camera.ScreenSpaceViewportRect.GetHeight() / Camera.WorldSpaceSize.y;
			laneRectBase = Rect::FromTLSize(vec2(GameLanePaddingL, GameLanePaddingTop), vec2(extendedLaneWidth, GameLaneSlice.TotalHeight()));
		}

#if 0 // DEBUG: ...
		{
			Gui::GetForegroundDrawList()->AddRect(Camera.ScreenSpaceViewportRect.TL, Camera.ScreenSpaceViewportRect.BR, 0xFFFF00FF);
			Gui::GetForegroundDrawList()->AddRect(Camera.WorldToScreenSpace(Camera.LaneRect.TL), Camera.WorldToScreenSpace(Camera.LaneRect.BR), 0xFF00FFFF);
			char b[64];
			Gui::GetForegroundDrawList()->AddText(Camera.WorldToScreenSpace(vec2(0.0f, 0.0f)), 0xFFFF00FF, b, b + sprintf_s(b, "(%.2f, %.2f)", 0.0f, 0.0f));
			Gui::GetForegroundDrawList()->AddText(Camera.WorldToScreenSpace(Camera.WorldSpaceSize), 0xFFFF00FF, b, b + sprintf_s(b, "(%.2f, %.2f)", Camera.WorldSpaceSize.x, Camera.WorldSpaceSize.y));
		}
#endif

		if (!context.Gfx.IsAsyncLoading())
			context.Gfx.Rasterize(SprGroup::Game, Camera.WorldToScreenScaleFactor);

		ImDrawList* drawList = Gui::GetWindowDrawList();
		drawList->ChannelsSplit(4); // 0: lane, 1: judgement mark, 2: bar lines, 3: notes
		drawList->PushClipRect(Camera.ScreenSpaceViewportRect.TL, Camera.ScreenSpaceViewportRect.BR, true);

		i32 iLane = -1;
		for (auto it = cbegin(context.Chart.Courses); it != cend(context.Chart.Courses); ++it) {
			const auto* course = it->get();
			// Use the selected branch for the focused course; Normal for compared courses
			const BranchType branch = (course == context.ChartSelectedCourse) ? context.ChartSelectedBranch : BranchType::Normal;
			if (!context.IsChartCompared(course, branch))
				continue;
			const b8 isFocusedLane = (context.CompareMode && course == context.ChartSelectedCourse && branch == context.ChartSelectedBranch);
			++iLane;

			Camera.LaneRect = laneRectBase + vec2{ 0, iLane * GameLaneSlice.TotalHeight() };

			const TempoMapAccelerationStructure& tempoChanges = course->TempoMap.AccelerationStructure;
			const SortedJPOSScrollChangesList& jposScrollChanges = course->JPOSScrollChanges;
			const std::vector<TempoChange>& tempos = course->TempoMap.Tempo.Sorted;
			const SortedGoGoRangesList& gogoRanges = course->GoGoRanges;

			const b8 isPlayback = context.GetIsPlayback();
			const BeatAndTime exactCursorBeatAndTime = context.GetCursorBeatAndTime(course, true);
			const Time cursorTimeOrAnimated = isPlayback ? exactCursorBeatAndTime.Time : animatedCursorTime;
			const Beat cursorBeatOrAnimatedTrunc = isPlayback ? exactCursorBeatAndTime.Beat : course->TempoMap.TimeToBeat(animatedCursorTime, true);
			const f64 cursorHBScrollBeatOrAnimated = course->TempoMap.BeatAndTimeToHBScrollBeatTick(cursorBeatOrAnimatedTrunc, cursorTimeOrAnimated);
			const Beat chartBeatDuration = course->TempoMap.TimeToBeat(context.Chart.GetDurationOrDefault());

			const auto* lastGogo = gogoRanges.TryFindLastAtBeat(cursorBeatOrAnimatedTrunc);
			const b8 isGogo = (lastGogo != nullptr && cursorBeatOrAnimatedTrunc < lastGogo->GetEnd());
			const Time timeSinceGogo = (lastGogo == nullptr) ? Time::FromSec(F64Max)
				: TimeSinceNoteHit(course->TempoMap.BeatToTime(lastGogo->BeatTime), cursorTimeOrAnimated);
			const Time timeAfterGogo = (lastGogo == nullptr) ? Time::FromSec(F64Max)
				: TimeSinceNoteHit(course->TempoMap.BeatToTime(lastGogo->GetEnd()), cursorTimeOrAnimated);
			const auto [gogoFireZoomAmount, gogoLaneZoomAmount] = getGogoZoomAmount(isGogo, timeSinceGogo, timeAfterGogo);

			auto laneBorderColor = isFocusedLane ? GameLaneBorderFocusedColor : GameLaneBorderColor;

			Rect stdLaneRectBR = { Camera.LaneRect.TL, vec2{ Camera.LaneRect.TL.x + GameLaneStandardWidth, Camera.LaneRect.BR.y } };
			// NOTE: Lane background and borders
			drawList->ChannelsSetCurrent(0);
			{
				drawList->AddRectFilled( // NOTE: Top, middle and bottom border, truncated at standard lane width
					Camera.WorldToScreenSpace(stdLaneRectBR.TL),
					Camera.WorldToScreenSpace(stdLaneRectBR.BR),
					laneBorderColor);
				drawList->AddRectFilled( // NOTE: Keep middle border color unfocused and untruncated
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(0.0f, GameLaneSlice.TopBorder + GameLaneSlice.Content)),
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(Camera.LaneWidth(), GameLaneSlice.TopBorder + GameLaneSlice.Content + GameLaneSlice.MidBorder + GameLaneSlice.Footer)),
					GameLaneBorderColor);
				drawList->AddRectFilled( // NOTE: Content
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(0.0f, GameLaneSlice.TopBorder)),
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(Camera.LaneWidth(), GameLaneSlice.TopBorder + GameLaneSlice.Content)),
					GameLaneContentBackgroundColor);
				if (gogoLaneZoomAmount > 0) {
					f32 worldHeightZoomOffset = GameLaneSlice.Content / 2 * (1 - gogoLaneZoomAmount);
					drawList->AddRectFilled( // NOTE: Gogo layer
						Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(0.0f, GameLaneSlice.TopBorder + worldHeightZoomOffset)),
						Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(Camera.LaneWidth(), GameLaneSlice.TopBorder + GameLaneSlice.Content - worldHeightZoomOffset)),
						GameLaneContentBackgroundColorGogo);
				}
				drawList->AddRectFilled( // NOTE: Footer
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(0.0f, GameLaneSlice.TopBorder + GameLaneSlice.Content + GameLaneSlice.MidBorder)),
					Camera.WorldToScreenSpace(Camera.LaneRect.TL + vec2(Camera.LaneWidth(), GameLaneSlice.TopBorder + GameLaneSlice.Content + GameLaneSlice.MidBorder + GameLaneSlice.Footer)),
					GameLaneFooterBackgroundColor);
			}

			// NOTE: Lane left / right foreground borders, showing the standard lane size
			defer
			{
				drawList->ChannelsSetCurrent(3);
				drawList->AddRectFilled(Camera.WorldToScreenSpace(stdLaneRectBR.GetTL()), Camera.WorldToScreenSpace(stdLaneRectBR.GetBL() - vec2(GameLanePaddingL, 0.0f)), laneBorderColor);
				drawList->AddRectFilled(Camera.WorldToScreenSpace(stdLaneRectBR.GetTR()), Camera.WorldToScreenSpace(stdLaneRectBR.GetBR() + vec2(GameLanePaddingR, 0.0f)), laneBorderColor);
				if (isFocusedLane) {
					drawList->AddRect( // NOTE: all-side outline
						Camera.WorldToScreenSpace(stdLaneRectBR.TL - vec2(GameLanePaddingL, 0.0f)),
						Camera.WorldToScreenSpace(stdLaneRectBR.BR + vec2(GameLanePaddingR, 0.0f)),
						GameLaneOutlineFocusedColor);
				}
			};


			// NOTE: Hit indicator circle
			drawList->ChannelsSetCurrent(1);
			const vec2 hitCirclePosJPos = Camera.GetHitCircleCoordinatesJPOSScroll(jposScrollChanges, cursorTimeOrAnimated, tempoChanges);
			const vec2 hitCirclePosLane = Camera.JPOSScrollToLaneSpace(hitCirclePosJPos);
			const vec2 hitCirclePos = Camera.LaneToScreenSpace(hitCirclePosLane);
			if (gogoFireZoomAmount > 0) {
				context.Gfx.DrawSprite(drawList, SprID::Game_Lane_GogoFire, SprTransform::FromCenter(
					hitCirclePos,
					vec2(Camera.WorldToScreenScale(gogoFireZoomAmount), Camera.WorldToScreenScale(gogoFireZoomAmount))));
			}
			drawList->AddCircleFilled(
				hitCirclePos,
				Camera.WorldToScreenScale(GameHitCircle.InnerFillRadius), isGogo ? GameLaneHitCircleInnerFillColorGogo : GameLaneHitCircleInnerFillColor);
			drawList->AddCircle(
				hitCirclePos,
				Camera.WorldToScreenScale(GameHitCircle.InnerOutlineRadius), isGogo ? GameLaneHitCircleInnerOutlineColorGogo : GameLaneHitCircleInnerOutlineColor, 0, Camera.WorldToScreenScale(GameHitCircle.InnerOutlineThickness));
			drawList->AddCircle(
				hitCirclePos,
				Camera.WorldToScreenScale(GameHitCircle.OuterOutlineRadius), isGogo ? GameLaneHitCircleOuterOutlineColorGogo : GameLaneHitCircleOuterOutlineColor, 0, Camera.WorldToScreenScale(GameHitCircle.OuterOutlineThickness));

			if (hitCirclePosJPos != vec2{ 0, 0 }) {
				std::string str = Complex(hitCirclePosJPos.x, hitCirclePosJPos.y).toStringCompat();
				vec2 posTxtJPos = hitCirclePos + vec2{ -1, -1 } * Camera.WorldToScreenScale(GameHitCircle.OuterOutlineRadius);
				drawList->AddText(posTxtJPos, 0xFFFFFFFF, str.c_str(), str.c_str() + str.length());
			}

			drawList->ChannelsSetCurrent(2);
			ForEachBarOnNoteLane(*course, branch, chartBeatDuration, [&](const ForEachBarLaneData& it)
			{
				const vec2 lane = Camera.GetNoteCoordinatesLane(hitCirclePosLane, cursorTimeOrAnimated, cursorHBScrollBeatOrAnimated, it.Time, it.Beat, it.Tempo, it.ScrollSpeed, it.ScrollType, tempoChanges, jposScrollChanges);
				const f32 laneX = lane.x, laneY = lane.y;

				if (Camera.IsPointVisibleOnLane(laneX))
				{
					const vec2 tl = Camera.LaneToWorldSpace(laneX, laneY) - Rotate(vec2(0.0f, GameLaneSlice.Content * 0.5f), GetNoteFaceRotationMirror(it.Tempo, it.ScrollSpeed, NoteType::Count).first);
					const vec2 br = Camera.LaneToWorldSpace(laneX, laneY) + Rotate(vec2(0.0f, GameLaneSlice.Content * 0.5f), GetNoteFaceRotationMirror(it.Tempo, it.ScrollSpeed, NoteType::Count).first);
					drawList->AddLine(Camera.WorldToScreenSpace(tl), Camera.WorldToScreenSpace(br), GameLaneBarLineColor, Camera.WorldToScreenScale(GameLaneBarLineThickness));

					char barLineStr[32];
					DrawGamePreviewNumericText(context.Gfx, Camera, drawList, SprTransform::FromTL(tl + vec2(5.0f, 1.0f), vec2(1.0f)),
						std::string_view(barLineStr, sprintf_s(barLineStr, "%d", it.BarIndex)));
				}
			});

#if 0 // DEBUG: ...
			if (Gui::Begin("Game Preview - Debug", nullptr, ImGuiWindowFlags_NoDocking))
			{
				static char text[64] = "0123456789+-./%";
				static u32 color = 0xFFFFFFFF;
				static SprTransform transform = SprTransform::FromTL(vec2(0.0f));
				Gui::InputText("Text", text, sizeof(text));
				Gui::ColorEdit4_U32("Color", &color);

				Gui::DragFloat2("Position", &transform.Position[0], 1.0f);
				Gui::SliderFloat2("Pivot", &transform.Pivot[0], 0.0f, 1.0f);
				Gui::DragFloat2("Scale", &transform.Scale[0], 0.1f);
				if (Gui::DragFloat("Scale (Uniform)", &transform.Scale[0], 0.1f)) { transform.Scale = vec2(transform.Scale[0]); }
				Gui::SliderFloat("Rotation", &transform.Rotation.Radians, 0.0f, PI * 2.0f);

				DrawGamePreviewNumericText(context.Gfx, Camera, drawList, transform, text, color);
			}
			Gui::End();
#endif

			drawList->ChannelsSetCurrent(3);
			ForEachNoteOnNoteLane(*course, branch, [&](const ForEachNoteLaneData& it)
			{
				vec2 laneHead = Camera.GetNoteCoordinatesLane(hitCirclePosLane, cursorTimeOrAnimated, cursorHBScrollBeatOrAnimated, it.Time, it.Beat, it.Tempo, it.ScrollSpeed, it.ScrollType, tempoChanges, jposScrollChanges);
				vec2 laneTail = Camera.GetNoteCoordinatesLane(hitCirclePosLane, cursorTimeOrAnimated, cursorHBScrollBeatOrAnimated, it.Tail.Time, it.Tail.Beat, it.Tail.Tempo, it.Tail.ScrollSpeed, it.Tail.ScrollType, tempoChanges, jposScrollChanges);

				b8 isVisible = true;

				const Time timeSinceHeadHit = TimeSinceNoteHit(it.Time, cursorTimeOrAnimated);
				const Time timeSinceTailHit = TimeSinceNoteHit(it.Tail.Time, cursorTimeOrAnimated);
				if (IsRegularNote(it.OriginalNote->Type)) {
					if (timeSinceHeadHit >= Time::Zero())
						laneHead = laneTail = hitCirclePosLane; // temporary value, override when drawn
					if (timeSinceHeadHit > GetTotalGameNoteHitAnimationDuration(it.OriginalNote->Type))
						isVisible = false;
				}
				else if (IsBalloonNote(it.OriginalNote->Type)) {
					if (timeSinceTailHit >= Time::Zero()) {
						laneHead = laneTail;
						isVisible = false;
					}
					else if (timeSinceHeadHit >= Time::Zero())
						laneHead = hitCirclePosLane;
				}
				else { // is bar roll note
					// flying notes in screen?
					isVisible = ((timeSinceHeadHit >= Time::Zero() && timeSinceTailHit <= GameNoteHitAnimationDuration)
						// roll body in screen?
						|| Camera.IsRangeVisibleOnLane(Min(laneHead.x, laneTail.x), Max(laneHead.x, laneTail.x)));
				}
				if (isVisible)
					ReverseNoteDrawBuffer.push_back(DeferredNoteDrawData{ laneHead.x, laneTail.x, laneHead.y, laneTail.y, it.Tempo, it.ScrollSpeed, it.OriginalNote, it.Time, it.Tail.Time });
			});

			const Beat drummrollHitInterval = GetGridBeatSnap(*Settings.General.DrumrollAutoHitBarDivision);
			for (auto it = ReverseNoteDrawBuffer.rbegin(); it != ReverseNoteDrawBuffer.rend(); it++)
			{
				const Time timeSinceHit = TimeSinceNoteHit(it->NoteStartTime, cursorTimeOrAnimated);

				if (IsLongNote(it->OriginalNote->Type))
				{
					if (IsBalloonNote(it->OriginalNote->Type))
					{
						if (IsFuseRoll(it->OriginalNote->Type))
							DrawGamePreviewNoteDuration(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), Camera.LaneToWorldSpace(it->LaneTailX, it->LaneTailY), it->OriginalNote->Type, 0xFFFFFFFF);
						DrawGamePreviewNote(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), it->Tempo, it->ScrollSpeed, it->OriginalNote->Type, cursorTimeOrAnimated);
						DrawGamePreviewNoteSEText(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), {}, it->Tempo, it->ScrollSpeed, it->OriginalNote->TempSEType);
						if (timeSinceHit >= Time::Zero())
							DrawGamePreviewNumericText(context.Gfx, Camera, drawList, SprTransform::FromCenter(Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), vec2(2)),
								std::to_string(it->OriginalNote->BalloonPopCount).c_str(), 0xFFFFFFFF);
					}
					else
					{
						const i32 maxHitCount = (it->OriginalNote->BeatDuration.Ticks / drummrollHitInterval.Ticks);
						const Beat hitIntervalRoundedDuration = (drummrollHitInterval * maxHitCount);

						i32 drumrollHitsSoFar = 0;
						if (timeSinceHit >= Time::Zero())
						{
							for (Beat subBeat = hitIntervalRoundedDuration; subBeat >= Beat::Zero(); subBeat -= drummrollHitInterval)
							{
								const Time subHitTime = course->TempoMap.BeatToTime(it->OriginalNote->BeatTime + subBeat) + it->OriginalNote->TimeOffset;
								if (subHitTime <= cursorTimeOrAnimated)
									drumrollHitsSoFar++;
							}
						}

						const f32 hitPercentage = ConvertRangeClampOutput(0.0f, static_cast<f32>(ClampBot(maxHitCount, 4)), 0.0f, 1.0f, static_cast<f32>(drumrollHitsSoFar));
						const u32 hitNoteColor = InterpolateDrumrollHitColor(it->OriginalNote->Type, hitPercentage);
						DrawGamePreviewNoteDuration(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), Camera.LaneToWorldSpace(it->LaneTailX, it->LaneTailY), it->OriginalNote->Type, hitNoteColor);
						DrawGamePreviewNote(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), it->Tempo, it->ScrollSpeed, it->OriginalNote->Type, cursorTimeOrAnimated);
						DrawGamePreviewNoteSEText(context.Gfx, Camera, drawList, Camera.LaneToWorldSpace(it->LaneHeadX, it->LaneHeadY), Camera.LaneToWorldSpace(it->LaneTailX, it->LaneTailY), it->Tempo, it->ScrollSpeed, it->OriginalNote->TempSEType);

						if (timeSinceHit >= Time::Zero())
						{
							for (Beat subBeat = hitIntervalRoundedDuration; subBeat >= Beat::Zero(); subBeat -= drummrollHitInterval)
							{
								const Time subHitTime = course->TempoMap.BeatToTime(it->OriginalNote->BeatTime + subBeat) + it->OriginalNote->TimeOffset;
								const Time timeSinceSubHit = TimeSinceNoteHit(subHitTime, cursorTimeOrAnimated);
								// `>` to avoid displaying extra notes when editing (still fails sometimes)
								if (timeSinceSubHit > Time::Zero() && timeSinceSubHit <= GameNoteHitAnimationDuration)
								{
									// TODO: Scale duration, animation speed and path by extended lane width
									const auto hitAnimation = GetNoteHitPathAnimation(timeSinceSubHit, Camera.ExtendedLaneWidthFactor(), nLanes, iLane, it->OriginalNote->Type);
									const vec2 laneOrigin = Camera.GetHitCircleCoordinatesLane(jposScrollChanges, subHitTime, tempoChanges);
									const vec2 noteCenter = Camera.LaneToWorldSpace(laneOrigin.x, laneOrigin.y) + hitAnimation.PositionOffset;

									if (hitAnimation.AlphaFadeOut >= 1.0f)
										DrawGamePreviewNote(context.Gfx, Camera, drawList, noteCenter, it->Tempo, it->ScrollSpeed, ToBigNoteIf(NoteType::Don, IsBigNote(it->OriginalNote->Type)), cursorTimeOrAnimated, hitAnimation);
								}
							}
						}
					}
				}
				else
				{
					// TODO: Instead of offseting the lane x position just draw as HitCenter + PositionOffset directly (?)
					auto hitAnimation = GetNoteHitPathAnimation(timeSinceHit, Camera.ExtendedLaneWidthFactor(), nLanes, iLane, it->OriginalNote->Type);
					const vec2 noteOrigin = (timeSinceHit >= Time::Zero()) ? Camera.GetHitCircleCoordinatesLane(jposScrollChanges, it->NoteEndTime, tempoChanges) // keep flying note's start position
						: vec2{ it->LaneHeadX, it->LaneHeadY };
					const vec2 noteCenter = Camera.LaneToWorldSpace(noteOrigin.x, noteOrigin.y) + hitAnimation.PositionOffset;

					if (hitAnimation.AlphaFadeOut >= 1.0f)
						DrawGamePreviewNote(context.Gfx, Camera, drawList, noteCenter, it->Tempo, it->ScrollSpeed, it->OriginalNote->Type, cursorTimeOrAnimated, hitAnimation, nLanes, iLane);

					if (timeSinceHit <= Time::Zero())
						DrawGamePreviewNoteSEText(context.Gfx, Camera, drawList, noteCenter, {}, it->Tempo, it->ScrollSpeed, it->OriginalNote->TempSEType);

					if (const f32 whiteAlpha = (hitAnimation.WhiteFadeIn * hitAnimation.AlphaFadeOut); whiteAlpha > 0.0f)
					{
						// TODO: ...
						// const auto radii = IsBigNote(it->OriginalNote->Type) ? GameRefNoteRadiiBig : GameRefNoteRadiiSmall;
						// drawList->AddCircleFilled(Camera.RefToScreenSpace(refSpaceCenter), Camera.RefToScreenScale(radii.BlackOuter), ImColor(1.0f, 1.0f, 1.0f, whiteAlpha));
					}
				}
			}
			ReverseNoteDrawBuffer.clear();
		}
		drawList->PopClipRect();
	}
}
