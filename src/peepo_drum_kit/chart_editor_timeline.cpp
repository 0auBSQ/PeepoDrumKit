#include "chart_editor_timeline.h"
#include "chart_editor_undo.h"
#include "chart_editor_theme.h"
#include "chart_editor_i18n.h"

namespace PeepoDrumKit
{
	static void DrawTimelineDebugWindowContent(ChartTimeline& timeline, ChartContext& context)
	{
		static constexpr ImGuiColorEditFlags colorEditFlags = ImGuiColorEditFlags_AlphaPreviewHalf;
		static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInner;

		Gui::PushID(&timeline);
		defer { Gui::PopID(); };

		Gui::PushStyleColor(ImGuiCol_Text, 0xFFABE1E0);
		Gui::PushStyleColor(ImGuiCol_Border, 0x3EABE1E0);
		Gui::BeginChild("BackgroundChild", Gui::GetContentRegionAvail(), true);
		defer { Gui::EndChild(); Gui::PopStyleColor(2); };

		Gui::UpdateSmoothScrollWindow();

		if (Gui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (Gui::Property::BeginTable(tableFlags))
			{
				Gui::Property::PropertyTextValueFunc("Camera.PositionCurrent", [&] { Gui::SetNextItemWidth(-1.0f); Gui::DragFloat2("##PositionCurrent", timeline.Camera.PositionCurrent.data()); });
				Gui::Property::PropertyTextValueFunc("Camera.PositionTarget", [&] { Gui::SetNextItemWidth(-1.0f); Gui::DragFloat2("##PositionTarget", timeline.Camera.PositionTarget.data()); });
				Gui::Property::PropertyTextValueFunc("Camera.ZoomCurrent", [&] { Gui::SetNextItemWidth(-1.0f); Gui::DragFloat2("##ZoomCurrent", timeline.Camera.ZoomCurrent.data(), 0.01f); });
				Gui::Property::PropertyTextValueFunc("Camera.ZoomTarget", [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (auto v = timeline.Camera.ZoomTarget; Gui::DragFloat2("##ZoomTarget", v.data(), 0.01f))
						timeline.Camera.SetZoomTargetAroundLocalPivot(v, timeline.ScreenToLocalSpace(vec2(timeline.Regions.Content.GetCenter().x, 0.0f)));
				});
				Gui::Property::EndTable();
			}
		}

		if (Gui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen))
		{
			struct NamedColorU32Pointer { cstr Label; u32* ColorPtr; u32 Default; };
			static NamedColorU32Pointer namedColors[] =
			{
				{ "Timeline Background", &TimelineBackgroundColor },
				{ "Timeline Out Of Bounds Dim", &TimelineOutOfBoundsDimColor },
				{ "Timeline Waveform Base", &TimelineWaveformBaseColor },
				NamedColorU32Pointer {},
				{ "Timeline Cursor", &TimelineCursorColor },
				{ "Timeline Item Text", &TimelineItemTextColor },
				{ "Timeline Item Text (Shadow)", &TimelineItemTextColorShadow },
				{ "Timeline Item Text (Warning)", &TimelineItemTextColorWarning },
				{ "Timeline Item Text Underline", &TimelineItemTextUnderlineColor },
				{ "Timeline Item Text Underline (Shadow)", &TimelineItemTextUnderlineColorShadow },
				NamedColorU32Pointer {},
				{ "Timeline GoGo Background Border", &TimelineGoGoBackgroundColorBorder },
				{ "Timeline GoGo Background Border (Selected)", &TimelineGoGoBackgroundColorBorderSelected },
				{ "Timeline GoGo Background Outer", &TimelineGoGoBackgroundColorOuter },
				{ "Timeline GoGo Background Inner", &TimelineGoGoBackgroundColorInner },
				NamedColorU32Pointer {},
				{ "Timeline Lyrics Text", &TimelineLyricsTextColor },
				{ "Timeline Lyrics Text (Shadow)", &TimelineLyricsTextColorShadow },
				{ "Timeline Lyrics Background Border", &TimelineLyricsBackgroundColorBorder },
				{ "Timeline Lyrics Background Border (Selected)", &TimelineLyricsBackgroundColorBorderSelected },
				{ "Timeline Lyrics Background Outer", &TimelineLyricsBackgroundColorOuter },
				{ "Timeline Lyrics Background Inner", &TimelineLyricsBackgroundColorInner },
				NamedColorU32Pointer {},
				{ "Timeline JPosScroll Background Border", &TimelineJPOSScrollBackgroundColorBorder },
				{ "Timeline JPosScroll Background Border (Selected)", &TimelineJPOSScrollBackgroundColorBorderSelected },
				{ "Timeline JPosScroll Background Outer", &TimelineJPOSScrollBackgroundColorOuter },
				{ "Timeline JPosScroll Background Inner", &TimelineJPOSScrollBackgroundColorInner },
				NamedColorU32Pointer {},
				{ "Timeline Branch Section Background Border", &TimelineBranchSectionBackgroundColorBorder },
				{ "Timeline Branch Section Background Border (Selected)", &TimelineBranchSectionBackgroundColorBorderSelected },
				{ "Timeline Branch Section Background Outer", &TimelineBranchSectionBackgroundColorOuter },
				{ "Timeline Branch Section Background Inner", &TimelineBranchSectionBackgroundColorInner },
				{ "Timeline Section Marker Line", &TimelineSectionMarkerLineColor },
				NamedColorU32Pointer {},
				{ "Timeline Horizontal Row Line", &TimelineHorizontalRowLineColor },
				{ "Grid Bar Line", &TimelineGridBarLineColor },
				{ "Grid Beat Line", &TimelineGridBeatLineColor },
				{ "Grid Snap Line", &TimelineGridSnapLineColor },
				{ "Grid Snap Line (Tuplet)", &TimelineGridSnapTupletLineColor },
				NamedColorU32Pointer {},
				{ "Box-Selection Background", &TimelineBoxSelectionBackgroundColor },
				{ "Box-Selection Border", &TimelineBoxSelectionBorderColor },
				{ "Box-Selection Fill", &TimelineBoxSelectionFillColor },
				{ "Box-Selection Inner", &TimelineBoxSelectionInnerColor },
				{ "Range-Selection Background", &TimelineRangeSelectionBackgroundColor },
				{ "Range-Selection Border", &TimelineRangeSelectionBorderColor },
				{ "Range-Selection Header Highlight", &TimelineRangeSelectionHeaderHighlightColor },
				{ "Selected Note Box Background", &TimelineSelectedNoteBoxBackgroundColor },
				{ "Selected Note Box Border", &TimelineSelectedNoteBoxBorderColor },
				NamedColorU32Pointer {},
				{ "Timeline Default Line", &TimelineDefaultLineColor },
				{ "Timeline Tempo Change Line", &TimelineTempoChangeLineColor },
				{ "Timeline Signature Change Line", &TimelineSignatureChangeLineColor },
				{ "Timeline Scroll Change Line", &TimelineScrollChangeLineColor },
				{ "Timeline Scroll Change (Complex) Line", &TimelineScrollChangeComplexLineColor },
				{ "Timeline Scroll Type Line", &TimelineScrollTypeLineColor },
				{ "Timeline Bar Line Change Line", &TimelineBarLineChangeLineColor },
				{ "Timeline Selected Item Line", &TimelineSelectedItemLineColor },
				NamedColorU32Pointer {},
				{ "Timeline Song Demo Start Marker Fill", &TimelineSongDemoStartMarkerColorFill },
				{ "Timeline Song Demo Start Marker Border", &TimelineSongDemoStartMarkerColorBorder },
				NamedColorU32Pointer {},
				{ "Game Lane Outline Focused", &GameLaneOutlineFocusedColor },
				{ "Game Lane Border", &GameLaneBorderColor },
				{ "Game Lane Bar Line", &GameLaneBarLineColor },
				{ "Game Lane Content Background", &GameLaneContentBackgroundColor },
				{ "Game Lane Footer Background", &GameLaneFooterBackgroundColor },
				{ "Game Lane Hit Circle Inner Fill", &GameLaneHitCircleInnerFillColor },
				{ "Game Lane Hit Circle Inner Outline", &GameLaneHitCircleInnerOutlineColor },
				{ "Game Lane Hit Circle Outer Outline", &GameLaneHitCircleOuterOutlineColor },
				{ "Game Lane Content Background (Go-go time)", &GameLaneContentBackgroundColorGogo },
				{ "Game Lane Hit Circle Inner Fill (Go-go time)", &GameLaneHitCircleInnerFillColorGogo },
				{ "Game Lane Hit Circle Inner Outline (Go-go time)", &GameLaneHitCircleInnerOutlineColorGogo },
				{ "Game Lane Hit Circle Outer Outline (Go-go time)", &GameLaneHitCircleOuterOutlineColorGogo },
				NamedColorU32Pointer {},
				{ "Note Red", &NoteColorRed },
				{ "Note Blue", &NoteColorBlue },
				{ "Note Orange", &NoteColorOrange },
				{ "Note Yellow", &NoteColorYellow },
				{ "Note White", &NoteColorWhite },
				{ "Note Black", &NoteColorBlack },
				{ "Note Drumroll Hit", &NoteColorDrumrollHit },
				{ "Note Balloon Text", &NoteBalloonTextColor },
				{ "Note Balloon Text (Shadow)", &NoteBalloonTextColorShadow },
				NamedColorU32Pointer {},
				{ "Drag Text (Hovered)", &DragTextHoveredColor },
				{ "Drag Text (Active)", &DragTextActiveColor },
				{ "Input Text Warning Text", &InputTextWarningTextColor },
			};

			if (static b8 firstFrame = true; firstFrame) { for (auto& e : namedColors) { e.Default = (e.ColorPtr != nullptr) ? *e.ColorPtr : 0xFFFF00FF; } firstFrame = false; }

			static ImGuiTextFilter colorFilter;
			Gui::SetNextItemWidth(-1.0f);
			if (Gui::InputTextWithHint("##ColorFilter", "Color Filter...", colorFilter.InputBuf, ArrayCount(colorFilter.InputBuf)))
				colorFilter.Build();

			if (Gui::Property::BeginTable(tableFlags))
			{
				i32 passedColorCount = 0;
				for (const auto& namedColor : namedColors)
				{
					if (namedColor.ColorPtr == nullptr || !colorFilter.PassFilter(namedColor.Label))
						continue;

					passedColorCount++;
					Gui::PushID(namedColor.ColorPtr);
					Gui::Property::Property([&]()
					{
						Gui::AlignTextToFramePadding();
						Gui::TextUnformatted(namedColor.Label);
						if (*namedColor.ColorPtr != namedColor.Default)
						{
							Gui::SameLine(ClampBot(Gui::GetContentRegionAvail().x - Gui::GetFontSize(), 1.0f), -1.0f);
							if (Gui::ColorButton("Reset Default", ImColor(namedColor.Default), ImGuiColorEditFlags_None))
								*namedColor.ColorPtr = namedColor.Default;
						}
					});
					Gui::Property::Value([&]() { Gui::SetNextItemWidth(-1.0f); Gui::ColorEdit4_U32(namedColor.Label, namedColor.ColorPtr, colorEditFlags); });
					Gui::PopID();
				}

				if (passedColorCount <= 0) { Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_TextDisabled)); Gui::Property::PropertyText("No matching color found :Sadge:"); Gui::PopStyleColor(); }

				Gui::Property::EndTable();
			}
		}

		if (Gui::CollapsingHeader("Size Variables", ImGuiTreeNodeFlags_DefaultOpen))
		{
			struct NamedVariablePtr { cstr Label; f32* ValuePtr; f32 Default; };
			static NamedVariablePtr namedVariables[] =
			{
				{ "TimelineRowHeight", &TimelineRowHeight, },
				{ "TimelineRowHeightNotes", &TimelineRowHeightNotes, },
				{ "TimelineCursorHeadHeight", &TimelineCursorHeadHeight, },
				{ "TimelineCursorHeadWidth", &TimelineCursorHeadWidth, },
				NamedVariablePtr {},
				{ "TimelineSongDemoStartMarkerWidth", &TimelineSongDemoStartMarkerWidth, },
				{ "TimelineSongDemoStartMarkerHeight", &TimelineSongDemoStartMarkerHeight, },
				{ "TimelineSelectedNoteHitBoxSizeSmall", &TimelineSelectedNoteHitBoxSizeSmall, },
				{ "TimelineSelectedNoteHitBoxSizeBig", &TimelineSelectedNoteHitBoxSizeBig, },
			};

			if (static b8 firstFrame = true; firstFrame) { for (auto& e : namedVariables) { e.Default = (e.ValuePtr != nullptr) ? *e.ValuePtr : 0.0f; } firstFrame = false; }

			static ImGuiTextFilter variableFilter;
			Gui::SetNextItemWidth(-1.0f);
			if (Gui::InputTextWithHint("##VariableFilter", "Variable Filter...", variableFilter.InputBuf, ArrayCount(variableFilter.InputBuf)))
				variableFilter.Build();

			if (Gui::Property::BeginTable(tableFlags))
			{
				i32 passedVariableCount = 0;
				for (const auto& namedVariable : namedVariables)
				{
					if (namedVariable.ValuePtr == nullptr || !variableFilter.PassFilter(namedVariable.Label))
						continue;

					passedVariableCount++;
					Gui::PushID(namedVariable.ValuePtr);
					Gui::Property::Property([&]()
					{
						Gui::AlignTextToFramePadding();
						Gui::TextUnformatted(namedVariable.Label);
						if (*namedVariable.ValuePtr != namedVariable.Default)
						{
							Gui::SameLine(ClampBot(Gui::GetContentRegionAvail().x - Gui::GetFontSize(), 1.0f), -1.0f);
							if (Gui::ColorButton("Reset Default", ImColor(0xFF6384AA), ImGuiColorEditFlags_NoDragDrop))
								*namedVariable.ValuePtr = namedVariable.Default;
						}
					});
					Gui::Property::Value([&]() { Gui::SetNextItemWidth(-1.0f); Gui::DragFloat(namedVariable.Label, namedVariable.ValuePtr, 0.25f); });
					Gui::PopID();
				}

				if (passedVariableCount <= 0) { Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_TextDisabled)); Gui::Property::PropertyText("No matching variable found :Sadge:"); Gui::PopStyleColor(); }

				Gui::Property::EndTable();
			}
		}

		if (Gui::CollapsingHeader("Highlight Region"))
		{
			auto region = [](cstr name, Rect region) { Gui::Selectable(name); if (Gui::IsItemHovered()) { Gui::GetForegroundDrawList()->AddRect(region.TL, region.BR, ImColor(1.0f, 0.0f, 1.0f)); } };
			region("Regions.Window", timeline.Regions.Window);
			region("Regions.SidebarHeader", timeline.Regions.SidebarHeader);
			region("Regions.Sidebar", timeline.Regions.Sidebar);
			region("Regions.ContentHeader", timeline.Regions.ContentHeader);
			region("Regions.Content", timeline.Regions.Content);
			region("Regions.ContentScrollbarX", timeline.Regions.ContentScrollbarX);
		}
	}
}

namespace PeepoDrumKit
{
	// TODO: Maybe use f64 for all timeline units? "dvec2"?
	struct ForEachRowData
	{
		TimelineRowType RowType;
		f32 LocalY;
		f32 LocalHeight;
		std::string_view Label;
	};

	template <typename Func>
	static void ForEachTimelineRow(ChartTimeline& timeline, Func perRowFunc)
	{
		f32 localY = 0.0f;
		for (TimelineRowType rowType = {}; rowType < TimelineRowType::Count; IncrementEnum(rowType))
		{
				// All branch rows get the full notes height when the course has branches; Normal-only height otherwise
			const b8 isNotesRow =
				(rowType >= TimelineRowType::NoteBranches_First && rowType <= TimelineRowType::NoteBranches_Last) &&
				(timeline.SelectedCourseHasBranches || TimelineRowToBranchType(rowType) == BranchType::Normal);

			const f32 localHeight = GuiScale(isNotesRow ? TimelineRowHeightNotes : TimelineRowHeight);

			perRowFunc(ForEachRowData { rowType, localY, localHeight, UI_StrRuntime(TimelineRowTypeNames[EnumToIndex(rowType)]) });
			localY += localHeight;
		}
	}

	static f32 GetTotalTimelineRowsHeight(const ChartTimeline& timeline)
	{
		f32 totalHeight = 0.0f;
		ForEachTimelineRow(*const_cast<ChartTimeline*>(&timeline), [&](const ForEachRowData& it) { totalHeight += it.LocalHeight; });
		return totalHeight;
	}

	struct ForEachGridLineData
	{
		Time Time;
		i32 BarIndex;
		b8 IsBar;
	};

	template <typename Func>
	static void ForEachTimelineVisibleGridLine(ChartTimeline& timeline, ChartContext& context, Time visibleTimeOverdraw, Func perGridFunc)
	{
		// TODO: Rewrite all of this to correctly handle tempo map changes and take GuiScaleFactor text size into account
		/* // TEMP: */ static constexpr Time GridTimeStep = Time::FromSec(1.0);
		if (GridTimeStep.Seconds <= 0.0)
			return;

		// TODO: Not too sure about the exact values here, these just came from randomly trying out numbers until it looked about right
		const f32 minAllowedSpacing = GuiScale(128.0f);
		const i32 maxSubDivisions = 6;

		// TODO: Go the extra step and add a fade animation for these too
		i32 gridLineSubDivisions = 0;
		for (gridLineSubDivisions = 0; gridLineSubDivisions < maxSubDivisions; gridLineSubDivisions++)
		{
			const f32 localSpaceXPerGridLine = timeline.Camera.WorldToLocalSpaceScale(vec2(timeline.Camera.TimeToWorldSpaceX(GridTimeStep), 0.0f)).x;
			const f32 localSpaceXPerGridLineSubDivided = localSpaceXPerGridLine * static_cast<f32>(gridLineSubDivisions + 1);

			if (localSpaceXPerGridLineSubDivided >= (minAllowedSpacing / (gridLineSubDivisions + 1)))
				break;
		}

		const auto minMaxVisibleTime = timeline.GetMinMaxVisibleTime(visibleTimeOverdraw);
		const i32 gridLineModToSkip = (1 << gridLineSubDivisions);
		i32 gridLineIndex = 0;

		const Time chartDuration = context.Chart.GetDurationOrDefault();
		context.ChartSelectedCourse->TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
		{
			const Time timeIt = context.ChartSelectedCourse->TempoMap.BeatToTime(it.Beat);

			if ((gridLineIndex++ % gridLineModToSkip) == 0)
			{
				if (timeIt >= minMaxVisibleTime.Min && timeIt <= minMaxVisibleTime.Max)
					perGridFunc(ForEachGridLineData { timeIt, it.BarIndex, it.IsBar });
			}

			if (timeIt >= minMaxVisibleTime.Max || (it.IsBar && timeIt >= chartDuration))
				return ControlFlow::Break;
			else
				return ControlFlow::Fallthrough;
		});
	}

	// NOTE: Drawing the waveform on top was originally intended to ensure it is always visible, though it can admittedly looks a bit weird at times...
	enum class WaveformDrawOrder { Background, Foreground };
	static constexpr WaveformDrawOrder TimelineWaveformDrawOrder = WaveformDrawOrder::Background;

	static constexpr f32 NoteHitAnimationDuration = 0.06f;
	static constexpr f32 NoteHitAnimationScaleStart = 1.35f, NoteHitAnimationScaleEnd = 1.0f;
	static constexpr f32 NoteDeleteAnimationDuration = 0.04f;

	static constexpr void PlaySoundEffectTypeForNoteType(ChartContext& context, NoteType noteType, Time startTime = Time::Zero(), std::optional<Time> externalClock = {}, f32 pan = 0)
	{
		if (!IsKaNote(noteType))
			context.SfxVoicePool.PlaySound(SoundEffectType::TaikoDon, startTime, externalClock, pan);
		if (IsKaNote(noteType) || IsKaDonNote(noteType))
			context.SfxVoicePool.PlaySound(SoundEffectType::TaikoKa, startTime, externalClock, pan);
	}

	static b8 IsTimelineCursorVisibleOnScreen(const TimelineCamera& camera, const TimelineRegions& regions, const Time cursorTime, const f32 edgePixelThreshold = 0.0f)
	{
		assert(edgePixelThreshold >= 0.0f);
		const f32 cursorLocalSpaceX = camera.TimeToLocalSpaceX(cursorTime);
		return (cursorLocalSpaceX >= edgePixelThreshold && cursorLocalSpaceX <= ClampBot(regions.Content.GetWidth() - edgePixelThreshold, edgePixelThreshold));
	}

	static void ScrollToTimelinePosition(TimelineCamera& camera, const TimelineRegions& regions, const ChartProject& chart, f32 normalizedTargetPosition)
	{
		const f32 visibleWidth = regions.Content.GetWidth();
		const f32 totalTimelineWidth = camera.WorldToLocalSpaceScale(vec2(camera.TimeToWorldSpaceX(chart.GetDurationOrDefault()), 0.0f)).x + 2.0f;

		const f32 cameraTargetPosition = TimelineCameraBaseScrollX + (totalTimelineWidth - visibleWidth - TimelineCameraBaseScrollX) * normalizedTargetPosition;
		camera.PositionTarget.x = ClampBot(cameraTargetPosition, TimelineCameraBaseScrollX);
	}

	static f32 GetNotesWaveAnimationTimeAtIndex(i32 noteIndex, i32 notesCount, i32 direction)
	{
		// TODO: Proper "wave" placement animation (also maybe scale by beat instead of index?)
		const i32 animationIndex = (direction > 0) ? noteIndex : (notesCount - noteIndex);
		const f32 noteCountAnimationFactor = (1.0f / notesCount) * 2.0f;
		return NoteHitAnimationDuration + (animationIndex * (NoteHitAnimationDuration * noteCountAnimationFactor));
	}

	static void SetNotesWaveAnimationTimes(std::vector<Note>& notesToAnimate, i32 direction = +1)
	{
		const i32 notesCount = static_cast<i32>(notesToAnimate.size());
		for (i32 noteIndex = 0; noteIndex < notesCount; noteIndex++)
			notesToAnimate[noteIndex].ClickAnimationTimeDuration = notesToAnimate[noteIndex].ClickAnimationTimeRemaining = GetNotesWaveAnimationTimeAtIndex(noteIndex, notesCount, direction);
	}

	static void SetNotesWaveAnimationTimes(std::vector<GenericListStructWithType>& noteItemsToAnimate, i32 direction = +1)
	{
		i32 notesCount = 0, noteIndex = 0;
		for (auto& item : noteItemsToAnimate) { if (IsNotesList(item.List)) notesCount++; }

		for (auto& item : noteItemsToAnimate)
			if (IsNotesList(item.List))
				item.Value.POD.Note.ClickAnimationTimeDuration = item.Value.POD.Note.ClickAnimationTimeRemaining = GetNotesWaveAnimationTimeAtIndex(noteIndex++, notesCount, direction);
	}

	static f32 GetTimelineNoteScaleFactor(b8 isPlayback, Time cursorTime, Beat cursorBeatOnPlaybackStart, const Note& note, Time noteTime)
	{
		// TODO: Handle AnimationTime > AnimationDuration differently so that range selected multi note placement can have a nice "wave propagation" effect
		if (note.ClickAnimationTimeRemaining > 0.0f)
			return ConvertRange<f32>(note.ClickAnimationTimeDuration, 0.0f, NoteHitAnimationScaleStart, NoteHitAnimationScaleEnd, note.ClickAnimationTimeRemaining);

		if (isPlayback && note.BeatTime >= cursorBeatOnPlaybackStart)
		{
			// TODO: Rewirte cleanup using ConvertRange
			const f32 timeUntilNote = (noteTime - cursorTime).ToSec_F32();
			if (timeUntilNote <= 0.0f && timeUntilNote >= -NoteHitAnimationDuration)
			{
				const f32 delta = (timeUntilNote / -NoteHitAnimationDuration);
				return Lerp(NoteHitAnimationScaleStart, NoteHitAnimationScaleEnd, delta);
			}
		}

		return 1.0f;
	}

	static void DrawTimelineNote(ChartGraphicsResources& gfx, ImDrawList* drawList, vec2 center, f32 scale, NoteType noteType, f32 alpha = 1.0f)
	{
		SprID spr = SprID::Count;
		switch (noteType)
		{
		case NoteType::Don: { spr = SprID::Timeline_Note_Don; } break;
		case NoteType::DonBig: { spr = SprID::Timeline_Note_DonBig; } break;
		case NoteType::Ka: { spr = SprID::Timeline_Note_Ka; } break;
		case NoteType::KaBig: { spr = SprID::Timeline_Note_KaBig; } break;
		case NoteType::Drumroll: { spr = SprID::Timeline_Note_Drumroll; } break;
		case NoteType::DrumrollBig: { spr = SprID::Timeline_Note_DrumrollBig; } break;
		case NoteType::Balloon: { spr = SprID::Timeline_Note_Balloon; } break;
		case NoteType::BalloonSpecial: { spr = SprID::Timeline_Note_BalloonSpecial; } break;
		case NoteType::DonBigHand: { spr = SprID::Timeline_Note_DonHand; } break;
		case NoteType::KaBigHand: { spr = SprID::Timeline_Note_KaHand; } break;
		case NoteType::KaDon: { spr = SprID::Timeline_Note_KaDon; } break;
		case NoteType::Adlib: { spr = SprID::Timeline_Note_Adlib; } break;
		case NoteType::Fuse: { spr = SprID::Timeline_Note_Fuse; } break;
		case NoteType::Bomb: { spr = SprID::Timeline_Note_Bomb; } break;
		}

		if (IsHandNote(noteType))
			gfx.DrawSprite(drawList, SprID::Timeline_Note_Arms, SprTransform::FromCenter(center, vec2(scale * GuiScaleFactorCurrent)), ImColor(1.0f, 1.0f, 1.0f, alpha));
		gfx.DrawSprite(drawList, spr, SprTransform::FromCenter(center, vec2(scale * GuiScaleFactorCurrent)), ImColor(1.0f, 1.0f, 1.0f, alpha));
	}

	static void DrawTimelineNoteDuration(ChartGraphicsResources& gfx, ImDrawList* drawList, vec2 centerHead, vec2 centerTail, NoteType noteType, f32 alpha = 1.0f)
	{
		SprID spr = SprID::Count;
		switch (noteType)
		{
		case NoteType::Don: { spr = SprID::Timeline_Note_DrumrollLong; } break;
		case NoteType::DonBig: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::Ka: { spr = SprID::Timeline_Note_DrumrollLong; } break;
		case NoteType::KaBig: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::Drumroll: { spr = SprID::Timeline_Note_DrumrollLong; } break;
		case NoteType::DrumrollBig: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::Balloon: { spr = SprID::Timeline_Note_BalloonLong; } break;
		case NoteType::BalloonSpecial: { spr = SprID::Timeline_Note_BalloonLongSpecial; } break;
		case NoteType::DonBigHand: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::KaBigHand: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::KaDon: { spr = SprID::Timeline_Note_DrumrollLongBig; } break;
		case NoteType::Adlib: { spr = SprID::Timeline_Note_DrumrollLong; } break;
		case NoteType::Fuse: { spr = SprID::Timeline_Note_FuseLong; } break;
		case NoteType::Bomb: { spr = SprID::Timeline_Note_DrumrollLong; } break;
		}

		const SprInfo sprInfo = gfx.GetInfo(spr);

		const f32 longL = Min(centerHead.x, centerTail.x);
		const f32 longR = Max(centerHead.x, centerTail.x);
		const f32 midScaleX = ((longR - longL) / (sprInfo.SourceSize.x)) * 3.0f;

		const SprStretchtOut split = StretchMultiPartSpr(gfx, spr,
			SprTransform::FromCenter((centerHead + centerTail) / 2.0f, vec2(GuiScaleFactorCurrent)), ImColor(1.0f, 1.0f, 1.0f, alpha),
			SprStretchtParam { 1.0f, midScaleX / GuiScaleFactorCurrent, 1.0f }, 3);

		for (size_t i = 0; i < 3; i++)
			gfx.DrawSprite(drawList, split.Quads[i]);
	}

	static void DrawTimelineNoteBalloonPopCount(ChartGraphicsResources& gfx, ImDrawList* drawList, vec2 center, f32 scale, i32 popCount)
	{
		char buffer[32]; const auto text = std::string_view(buffer, sprintf_s(buffer, "%d", popCount));
		ImFont* const font = FontMain;
		const f32 fontSize = (FontBaseSizes::Large * scale);
		const vec2 textSize = font->CalcTextSizeA(fontSize, F32Max, -1.0f, Gui::StringViewStart(text), Gui::StringViewEnd(text));
		const vec2 textPosition = (center - (textSize * 0.5f)) - vec2(0.0f, 1.0f);

		Gui::DisableFontPixelSnap(true);
		Gui::AddTextWithDropShadow(drawList, font, fontSize, textPosition, NoteBalloonTextColor, text, 0.0f, nullptr, NoteBalloonTextColorShadow);
		Gui::DisableFontPixelSnap(false);
	}

	struct DrawTimelineRectBaseParam { vec2 TL, BR; f32 TriScaleL, TriScaleR; u32 ColorBorder, ColorOuter, ColorInner; b8 selected; };
	static void DrawTimelineRectBaseWithStartEndTriangles(ImDrawList* drawList, DrawTimelineRectBaseParam param)
	{
		const f32 outerOffset = ClampBot(GuiScale(2.0f), 1.0f);
		const f32 innerOffset = ClampBot(GuiScale(5.0f), 1.0f);
		Rect borderRect = Rect(param.TL, param.BR);

		drawList->AddRectFilled(borderRect.TL, borderRect.BR, param.ColorBorder);

		Rect outerRect = Rect(param.TL + vec2(outerOffset), param.BR - vec2(outerOffset));
		if (outerRect.GetWidth() < 1.0f) {
			// prevent shrinking further but cannot preserve full outline; center and draw as a single line
			outerRect.BR.x = outerRect.TL.x = param.TL.x + ClampBot(borderRect.GetWidth() / 2, 0.0f);
			drawList->AddLine(outerRect.TL, outerRect.BR, param.ColorOuter);
		}
		else {
			drawList->AddRectFilled(outerRect.TL, outerRect.BR, param.ColorOuter);
		}

		Rect innerRect = Rect(param.TL + vec2(innerOffset), param.BR - vec2(innerOffset));
		if (innerRect.GetWidth() > 0) {
			drawList->AddRectFilled(innerRect.TL, innerRect.BR, param.ColorInner);

			const f32 triH = (param.BR.y - param.TL.y) - innerOffset;
			const f32 triW = ClampTop(triH, (param.BR.x - param.TL.x) - innerOffset);
			if (param.TriScaleL > 0.0f)
				drawList->AddTriangleFilled(outerRect.TL, outerRect.TL + vec2(triW, triH), outerRect.TL + vec2(0.0f, triH), param.ColorOuter);
			else if (param.TriScaleL < 0.0f)
				drawList->AddTriangleFilled(outerRect.TL, outerRect.TL + vec2(triW, 0.0f), outerRect.TL + vec2(0.0f, triH), param.ColorOuter);

			if (param.TriScaleR > 0.0f)
				drawList->AddTriangleFilled(outerRect.BR, outerRect.BR - vec2(triW, 0.0f), outerRect.BR - vec2(0.0f, triH), param.ColorOuter);
			else if (param.TriScaleR < 0.0f)
				drawList->AddTriangleFilled(outerRect.BR, outerRect.BR - vec2(triW, triH), outerRect.BR - vec2(0.0f, triH), param.ColorOuter);
		}

		if (param.selected && borderRect.GetWidth() < 2 * outerOffset) { // border would be too thin; re-highlight
			if (borderRect.GetWidth() < 1.0f) { // too thin even if re-highlighted; draw as single line
				borderRect.BR.x = borderRect.TL.x = param.TL.x;
				drawList->AddLine(borderRect.TL, borderRect.BR, param.ColorBorder);
			}
			else {
				drawList->AddRectFilled(borderRect.TL, borderRect.BR, param.ColorBorder);
			}
		}
	}

	static void DrawTimelineGoGoTimeBackground(ImDrawList* drawList, vec2 tl, vec2 br, f32 animationScale, b8 selected)
	{
		const f32 centerX = (br.x + tl.x) * 0.5f;
		tl.x = Lerp(centerX, tl.x, animationScale);
		br.x = Lerp(centerX, br.x, animationScale);
		DrawTimelineRectBaseWithStartEndTriangles(drawList, DrawTimelineRectBaseParam { tl, br, 1.0f, 1.0f, selected ? TimelineGoGoBackgroundColorBorderSelected : TimelineGoGoBackgroundColorBorder, TimelineGoGoBackgroundColorOuter, TimelineGoGoBackgroundColorInner, selected });
	}

	static void DrawTimelineLyricsBackground(ImDrawList* drawList, vec2 tl, vec2 br, b8 selected)
	{
		DrawTimelineRectBaseWithStartEndTriangles(drawList, DrawTimelineRectBaseParam { tl, br, 1.0f, 0.0f, selected ? TimelineLyricsBackgroundColorBorderSelected : TimelineLyricsBackgroundColorBorder, TimelineLyricsBackgroundColorOuter, TimelineLyricsBackgroundColorInner, selected });
	}

	static void DrawTimelineJPOSScrollBackground(ImDrawList* drawList, vec2 tl, vec2 br, b8 selected)
	{
		DrawTimelineRectBaseWithStartEndTriangles(drawList, DrawTimelineRectBaseParam{ tl, br, 1.0f, 1.0f, selected ? TimelineJPOSScrollBackgroundColorBorderSelected : TimelineJPOSScrollBackgroundColorBorder, TimelineJPOSScrollBackgroundColorOuter, TimelineJPOSScrollBackgroundColorInner, selected });
	}

	static void DrawTimelineBranchSectionBackground(ImDrawList* drawList, vec2 tl, vec2 br, b8 selected)
	{
		DrawTimelineRectBaseWithStartEndTriangles(drawList, DrawTimelineRectBaseParam{ tl, br, 1.0f, 1.0f, selected ? TimelineBranchSectionBackgroundColorBorderSelected : TimelineBranchSectionBackgroundColorBorder, TimelineBranchSectionBackgroundColorOuter, TimelineBranchSectionBackgroundColorInner, selected });
	}

	static void DrawTimelineContentWaveform(const ChartTimeline& timeline, ImDrawList* drawList, Time chartSongOffset, const Audio::WaveformMipChain& waveformL, const Audio::WaveformMipChain& waveformR, f32 waveformAnimation)
	{
		const f32 waveformAnimationScale = Clamp(waveformAnimation, 0.0f, 1.0f);
		const f32 waveformAnimationAlpha = (waveformAnimationScale * waveformAnimationScale);
		const u32 waveformColor = Gui::ColorU32WithAlpha(TimelineWaveformBaseColor, waveformAnimationAlpha * 0.215f * (waveformR.IsEmpty() ? 2.0f : 1.0f));

		const Time waveformTimePerPixel = timeline.Camera.LocalSpaceXToTime(1.0f) - timeline.Camera.LocalSpaceXToTime(0.0f);
		const Time waveformDuration = waveformL.Duration;

		const Rect contentRect = timeline.Regions.Content;
		const f32 rowsHeight = GetTotalTimelineRowsHeight(timeline);

		const f32 minAmplitude = (2.0f / rowsHeight);
		for (size_t waveformIndex = 0; waveformIndex < 2; waveformIndex++)
		{
			const auto& waveform = (waveformIndex == 0) ? waveformL : waveformR;
			if (waveform.IsEmpty())
				continue;

			const auto& waveformMip = waveform.FindClosestMip(waveformTimePerPixel);
			for (i32 visiblePixel = 0; visiblePixel < contentRect.GetWidth(); /*visiblePixel++*/)
			{
				CustomDraw::WaveformChunk chunk;
				const Rect chunkRect = Rect::FromTLSize(timeline.LocalToScreenSpace(vec2(static_cast<f32>(visiblePixel), 0.5f)), vec2(static_cast<f32>(CustomDraw::WaveformPixelsPerChunk), rowsHeight));

				for (i32 chunkPixel = 0; chunkPixel < CustomDraw::WaveformPixelsPerChunk; chunkPixel++)
				{
					const Time timeAtPixel = timeline.Camera.LocalSpaceXToTime(static_cast<f32>(visiblePixel)) - chartSongOffset;
					const b8 outOfBounds = (timeAtPixel < Time::Zero() || (timeAtPixel > waveformDuration));

					chunk.PerPixelAmplitude[chunkPixel] = outOfBounds ? 0.0f : (waveformAnimationScale * ClampBot(waveform.GetAmplitudeAt(waveformMip, timeAtPixel, waveformTimePerPixel), minAmplitude));
					visiblePixel++;
				}

				CustomDraw::DrawWaveformChunk(drawList, chunkRect, waveformColor, chunk);
			}
		}
	}

	struct DrawTimelineContentItemRowParam
	{
		ChartTimeline& Timeline;
		ChartContext& Context;
		ImDrawList* DrawListContent;
		ChartTimeline::MinMaxTime VisibleTime;
		b8 IsPlayback;
		Time CursorTime;
		Beat CursorBeatOnPlaybackStart;
	};

	template <typename T, TimelineRowType RowType>
	static void DrawTimelineContentItemRowT(DrawTimelineContentItemRowParam param, const ForEachRowData& rowIt, const BeatSortedList<T>& list)
	{
		ChartTimeline& timeline = param.Timeline;
		ChartContext& context = param.Context;
		ImDrawList* drawListContent = param.DrawListContent;
		const TimelineCamera& camera = timeline.Camera;
		const ChartTimeline::MinMaxTime visibleTime = param.VisibleTime;

		// TODO: Do culling by first determining min/max visible beat times then use those to continue / break inside loop (although problematic with TimeOffset?)
		if constexpr (std::is_same_v<T, Note>)
		{
			// TODO: Draw unselected branch notes grayed and at a slightly smaller scale (also nicely animate between selecting different branched!)

			// TODO: It looks like there'll also have to be one scroll speed lane per branch type
			//		 which means the scroll speed change line should probably extend all to the way down to its corresponding note lane (?)

			for (const Note& it : list)
			{
				const Time startTime = context.BeatToTime(it.GetStart()) + it.TimeOffset;
				const Time endTime = (it.BeatDuration > Beat::Zero()) ? context.BeatToTime(it.GetEnd()) + it.TimeOffset : startTime;
				if (endTime < visibleTime.Min || startTime > visibleTime.Max)
					continue;

				const vec2 localTL = vec2(timeline.Camera.TimeToLocalSpaceX(startTime), rowIt.LocalY);
				const vec2 localCenter = localTL + vec2(0.0f, rowIt.LocalHeight * 0.5f);
				vec2 localTR = localTL;
				vec2 localCenterEnd = localCenter;

				if (it.BeatDuration > Beat::Zero())
				{
					localTR = vec2(timeline.Camera.TimeToLocalSpaceX(endTime), rowIt.LocalY);
					localCenterEnd = localTR + vec2(0.0f, rowIt.LocalHeight * 0.5f);
					DrawTimelineNoteDuration(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), timeline.LocalToScreenSpace(localCenterEnd), it.Type);
				}

				const f32 noteScaleFactor = GetTimelineNoteScaleFactor(param.IsPlayback, param.CursorTime, param.CursorBeatOnPlaybackStart, it, startTime);
				DrawTimelineNote(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), noteScaleFactor, it.Type);

				if (IsBalloonNote(it.Type) || it.BalloonPopCount > 0)
					DrawTimelineNoteBalloonPopCount(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), noteScaleFactor, it.BalloonPopCount);

				if (it.IsSelected)
				{
					// NOTE: Draw the note itself with the time offset applied but draw the hitbox at the original beat center
					const f32 localSpaceTimeOffsetX = timeline.Camera.WorldToLocalSpaceScale(vec2(timeline.Camera.TimeToWorldSpaceX(it.TimeOffset), 0.0f)).x;

					const vec2 hitBoxSize = vec2(GuiScale((IsBigNote(it.Type) ? TimelineSelectedNoteHitBoxSizeBig : TimelineSelectedNoteHitBoxSizeSmall)));
					timeline.TempSelectionBoxesDrawBuffer.push_back(ChartTimeline::TempDrawSelectionBox { Rect::FromCenterSize(timeline.LocalToScreenSpace(localCenter - vec2(localSpaceTimeOffsetX, 0.0f)), hitBoxSize), TimelineSelectedNoteBoxBackgroundColor, TimelineSelectedNoteBoxBorderColor });
					if (it.BeatDuration > Beat::Zero())
						timeline.TempSelectionBoxesDrawBuffer.push_back(ChartTimeline::TempDrawSelectionBox{ Rect::FromCenterSize(timeline.LocalToScreenSpace(localCenterEnd - vec2(localSpaceTimeOffsetX, 0.0f)), hitBoxSize), TimelineSelectedNoteBoxBackgroundColor, TimelineSelectedNoteBoxBorderColor });
				}
			}

			static constexpr BranchType branchForThisRow = TimelineRowToBranchType(RowType);
			if (!timeline.TempDeletedNoteAnimationsBuffer.empty())
			{
				for (const auto& data : timeline.TempDeletedNoteAnimationsBuffer)
				{
					if (data.Branch != branchForThisRow)
						continue;

					const Time startTime = context.BeatToTime(data.OriginalNote.BeatTime) + data.OriginalNote.TimeOffset;
					const Time endTime = startTime;
					if (endTime < visibleTime.Min || startTime > visibleTime.Max)
						continue;

					const vec2 localTL = vec2(camera.TimeToLocalSpaceX(startTime), rowIt.LocalY);
					const vec2 localCenter = localTL + vec2(0.0f, rowIt.LocalHeight * 0.5f);

					const f32 noteScaleFactor = ConvertRange(0.0f, NoteDeleteAnimationDuration, 1.0f, 0.0f, ClampBot(data.ElapsedTimeSec, 0.0f));
					DrawTimelineNote(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), noteScaleFactor, data.OriginalNote.Type);
					// TODO: Also animate duration fading out or "collapsing" some other way (?)
				}
			}

			if (!timeline.TempSelectionBoxesDrawBuffer.empty())
			{
				for (const auto& box : timeline.TempSelectionBoxesDrawBuffer)
				{
					drawListContent->AddRectFilled(box.ScreenSpaceRect.TL, box.ScreenSpaceRect.BR, box.FillColor);
					drawListContent->AddRect(box.ScreenSpaceRect.TL, box.ScreenSpaceRect.BR, box.BorderColor);
				}
				timeline.TempSelectionBoxesDrawBuffer.clear();
			}

			// NOTE: Long note placement preview
			if (timeline.LongNotePlacement.IsActive && context.ChartSelectedBranch == branchForThisRow)
			{
				const Beat minBeatAfter = timeline.LongNotePlacement.GetMin(), maxBeat = timeline.LongNotePlacement.GetMax();
				const vec2 localTL = vec2(timeline.Camera.TimeToLocalSpaceX(context.BeatToTime(minBeatAfter)), rowIt.LocalY);
				const vec2 localCenter = localTL + vec2(0.0f, rowIt.LocalHeight * 0.5f);
				const vec2 localTR = vec2(timeline.Camera.TimeToLocalSpaceX(context.BeatToTime(maxBeat)), rowIt.LocalY);
				const vec2 localCenterEnd = localTR + vec2(0.0f, rowIt.LocalHeight * 0.5f);
				DrawTimelineNoteDuration(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), timeline.LocalToScreenSpace(localCenterEnd), timeline.LongNotePlacement.NoteType, 0.7f);
				DrawTimelineNote(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), 1.0f, timeline.LongNotePlacement.NoteType, 0.7f);

				if (IsBalloonNote(timeline.LongNotePlacement.NoteType))
					DrawTimelineNoteBalloonPopCount(context.Gfx, drawListContent, timeline.LocalToScreenSpace(localCenter), 1.0f, DefaultBalloonPopCount(maxBeat - minBeatAfter, timeline.CurrentGridBarDivision));
			}
		}
		else if constexpr (std::is_same_v<T, GoGoRange>)
		{
			for (const GoGoRange& it : list)
			{
				const Time startTime = context.BeatToTime(it.GetStart());
				const Time endTime = context.BeatToTime(it.GetEnd());
				if (endTime < visibleTime.Min || startTime > visibleTime.Max)
					continue;

				static constexpr f32 margin = 1.0f;
				const vec2 localTL = vec2(camera.TimeToLocalSpaceX(startTime), 0.0f) + vec2(0.0f, rowIt.LocalY + margin);
				const vec2 localBR = vec2(camera.TimeToLocalSpaceX(endTime), 0.0f) + vec2(0.0f, rowIt.LocalY + rowIt.LocalHeight - (margin * 2.0f));
				DrawTimelineGoGoTimeBackground(drawListContent, timeline.LocalToScreenSpace(localTL) + vec2(0.0f, 2.0f), timeline.LocalToScreenSpace(localBR), it.ExpansionAnimationCurrent, it.IsSelected);
			}
		}
		else if constexpr (std::is_same_v<T, BranchSection>)
		{
			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
			static constexpr u32 BranchBarText = IM_COL32(255, 245, 210, 230);

			char condBuf[40];
			i32 rightClickedIdx = -1;
			for (i32 bsi = 0; bsi < (i32)list.size(); bsi++)
			{
				const BranchSection& it = list[bsi];
				const Time startTime = context.BeatToTime(it.GetStart());
				const Time endTime   = context.BeatToTime(it.GetEnd());
				if (endTime < visibleTime.Min || startTime > visibleTime.Max)
					continue;

				static constexpr f32 margin = 1.0f;
				const vec2 localTL = vec2(camera.TimeToLocalSpaceX(startTime), rowIt.LocalY + margin);
				const vec2 localBR = vec2(camera.TimeToLocalSpaceX(endTime),   rowIt.LocalY + rowIt.LocalHeight - margin * 2.0f);
				const vec2 screenTL = timeline.LocalToScreenSpace(localTL);
				const vec2 screenBR = timeline.LocalToScreenSpace(localBR);

				DrawTimelineBranchSectionBackground(drawListContent, screenTL + vec2(0.0f, 2.0f), screenBR, it.IsSelected);

				// Condition label
				const f32 barWidth = screenBR.x - screenTL.x;
				if (barWidth > GuiScale(30.0f))
				{
					sprintf_s(condBuf, "%c,%d,%d", TJA::BranchConditionToChar(it.Condition), it.RequirementExpert, it.RequirementMaster);
					const vec2 textPos = screenTL + GuiScale(vec2(3.0f, 2.0f));
					const ImVec4 clip = { screenTL.x + GuiScale(2.0f), screenTL.y, screenBR.x - GuiScale(2.0f), screenBR.y };
					Gui::AddTextWithDropShadow(drawListContent, nullptr, 0.0f, textPos, BranchBarText, condBuf, 0.0f, &clip, IM_COL32(0, 0, 0, 120));
				}

				// Right-click to open section-specific context menu
				if (timeline.IsContentWindowHovered && Gui::IsMouseHoveringRect(screenTL, screenBR, false) && Gui::IsMouseClicked(ImGuiMouseButton_Right))
					rightClickedIdx = bsi;
			}

			// Right-click anywhere on row (not on existing section) → add
			{
				const vec2 rowScreenTL = timeline.LocalToScreenSpace(vec2(0.0f, rowIt.LocalY));
				const vec2 rowScreenBR = timeline.LocalToScreenSpace(vec2(timeline.Regions.Content.GetWidth(), rowIt.LocalY + rowIt.LocalHeight));
				if (timeline.IsContentWindowHovered && Gui::IsMouseHoveringRect(rowScreenTL, rowScreenBR, false) && Gui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					if (rightClickedIdx < 0)
						Gui::OpenPopup("BranchSec_Add");
					else
						Gui::OpenPopup("BranchSec_Edit");
				}
			}

			// "Add" popup
			if (Gui::BeginPopup("BranchSec_Add"))
			{
				const Time mouseTime = camera.LocalSpaceXToTime(timeline.ScreenToLocalSpace(Gui::GetMousePosOnOpeningCurrentPopup()).x);
				const Beat startBeat = timeline.RoundBeatToCurrentGrid(context.TimeToBeat(mouseTime));
				const Beat defDuration = Beat::FromBars(2);
				if (Gui::MenuItem("Add Branch Section Here"))
				{
					BranchSection newSec; newSec.BeatTime = startBeat; newSec.BeatDuration = defDuration;
					context.Undo.Execute<Commands::AddSingleChartEvent<BranchSection>>(context.ChartSelectedCourse, &context.ChartSelectedCourse->BranchSections, newSec);
					context.ChartSelectedCourse->HasBranches(); // triggers rebuild of SelectedCourseHasBranches next frame
				}
				Gui::EndPopup();
			}

			// "Edit/Delete" popup for clicked section
			if (rightClickedIdx >= 0 && rightClickedIdx < (i32)list.size())
			{
				BranchSection& editSec = context.ChartSelectedCourse->BranchSections[rightClickedIdx];
				if (Gui::BeginPopup("BranchSec_Edit"))
				{
					Gui::TextUnformatted("Branch Section");
					Gui::Separator();
					static const char* condNames[] = { "Roll (r)", "Precise (p)", "Score (s)" };
					int condIdx = static_cast<int>(editSec.Condition);
					if (Gui::Combo("Type", &condIdx, condNames, 3))
					{ context.Undo.NotifyChangesWereMade(); editSec.Condition = static_cast<TJA::BranchCondition>(condIdx); }
					int reqE = editSec.RequirementExpert, reqM = editSec.RequirementMaster;
					if (Gui::InputInt("Expert Req.", &reqE)) { context.Undo.NotifyChangesWereMade(); editSec.RequirementExpert = reqE; }
					if (Gui::InputInt("Master Req.", &reqM)) { context.Undo.NotifyChangesWereMade(); editSec.RequirementMaster = reqM; }
					Gui::Separator();
					if (Gui::MenuItem("Delete Section"))
						context.Undo.Execute<Commands::RemoveSingleChartEvent<BranchSection>>(context.ChartSelectedCourse, &context.ChartSelectedCourse->BranchSections, editSec);
					Gui::TextDisabled("(changes to condition persist on Save, no undo)");
					Gui::EndPopup();
				}
			}
			Gui::PopFont();
		}
		else if constexpr (std::is_same_v<T, SectionMarker>)
		{
			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
			for (i32 smi = 0; smi < (i32)list.size(); smi++)
			{
				const SectionMarker& it = list[smi];
				const Time time = context.BeatToTime(it.BeatTime);
				if (time < visibleTime.Min || time > visibleTime.Max)
					continue;

				const vec2 localSpaceTL = vec2(camera.TimeToLocalSpaceX(time), rowIt.LocalY);
				const vec2 localSpaceBL = vec2(localSpaceTL.x, rowIt.LocalY + rowIt.LocalHeight);
				const u32 lineCol = it.IsSelected ? TimelineSelectedItemLineColor : TimelineSectionMarkerLineColor;
				drawListContent->AddLine(timeline.LocalToScreenSpace(localSpaceTL + vec2(0.0f, 1.0f)), timeline.LocalToScreenSpace(localSpaceBL), lineCol, GuiScale(2.0f));

				const vec2 textPos = timeline.LocalToScreenSpace(localSpaceTL + vec2(2.0f, 1.0f));
				const ImVec4 clip = { textPos.x, textPos.y, textPos.x + GuiScale(32.0f), textPos.y + rowIt.LocalHeight };
				Gui::AddTextWithDropShadow(drawListContent, nullptr, 0.0f, textPos, lineCol, "#SECTION", 0.0f, &clip, IM_COL32(0, 0, 0, 120));
			}
			Gui::PopFont();
		}
		else if constexpr (std::is_same_v<T, LyricChange>)
		{
			const Beat chartBeatDuration = context.TimeToBeat(context.Chart.GetDurationOrDefault());

			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
			for (size_t i = 0; i < list.size(); i++)
			{
				const LyricChange* prevLyric = IndexOrNull(static_cast<i32>(i) - 1, list);
				const LyricChange& thisLyric = list[i];
				const LyricChange* nextLyric = IndexOrNull(i + 1, list);
				if (thisLyric.Lyric.empty() && prevLyric != nullptr && !prevLyric->Lyric.empty())
					continue;

				const Beat nowBeat = (thisLyric.BeatTime <= chartBeatDuration) ? chartBeatDuration : Beat::FromTicks(I32Max);
				const Time startTime = context.BeatToTime(thisLyric.BeatTime);
				const Time endTime = context.BeatToTime(thisLyric.Lyric.empty() ? thisLyric.BeatTime : (nextLyric != nullptr) ? nextLyric->BeatTime : nowBeat);
				if (endTime < visibleTime.Min || startTime > visibleTime.Max)
					continue;

				const vec2 localSpaceTL = vec2(camera.TimeToLocalSpaceX(startTime), rowIt.LocalY);
				const vec2 localSpaceBR = vec2(camera.TimeToLocalSpaceX(endTime), rowIt.LocalY + rowIt.LocalHeight);
				const vec2 localSpaceCenter = localSpaceTL + vec2(0.0f, rowIt.LocalHeight * 0.5f);

				const f32 textHeight = Gui::GetFontSize();
				const vec2 textPosition = timeline.LocalToScreenSpace(localSpaceCenter + vec2(4.0f, textHeight * -0.5f));

				Gui::DisableFontPixelSnap(true);
				{
					const Rect lyricsBarRect = Rect(timeline.LocalToScreenSpace(localSpaceTL + vec2(0.0f, 1.0f)), timeline.LocalToScreenSpace(localSpaceBR));

					DrawTimelineLyricsBackground(drawListContent, lyricsBarRect.TL, lyricsBarRect.BR, thisLyric.IsSelected);

					// HACK: Try to at least somewhat visalize the tail being selected, for now
					if (thisLyric.IsSelected || (nextLyric != nullptr && nextLyric->IsSelected))
					{
						const vec2 width = vec2(GuiScale(1.0f), 0.0f);
						drawListContent->AddRectFilled(lyricsBarRect.GetTR() - width, lyricsBarRect.GetBR() + width, (nextLyric != nullptr && nextLyric->IsSelected) ? TimelineLyricsBackgroundColorBorderSelected : TimelineLyricsBackgroundColorBorder);
					}

					static constexpr f32 borderLeft = 2.0f, borderRight = 5.0f;
					const ImVec4 clipRect = { lyricsBarRect.TL.x + borderLeft, lyricsBarRect.TL.y, lyricsBarRect.BR.x - borderRight, lyricsBarRect.BR.y };

					if (Absolute(clipRect.z - clipRect.x) > (borderLeft + borderRight))
						Gui::AddTextWithDropShadow(drawListContent, nullptr, 0.0f, textPosition, TimelineLyricsTextColor, thisLyric.Lyric, 0.0f, &clipRect, TimelineLyricsTextColorShadow);
				}
				Gui::DisableFontPixelSnap(false);
			}
			Gui::PopFont();
		}
		else
		{
			static constexpr f32 compactFormatStringZoomLevelThreshold = 0.25f;
			const b8 useCompactFormat = (camera.ZoomTarget.x < compactFormatStringZoomLevelThreshold);
			const f32 textHeight = Gui::GetFontSize();

			for (const auto& it : list)
			{
				const Time startTime = context.BeatToTime(GetBeat(it));
				Time endTime = startTime;
				if constexpr (std::is_same_v<T, JPOSScrollChange>) {
					endTime = startTime + Time::FromSec(it.Duration);
				}
				if (endTime < visibleTime.Min || startTime > visibleTime.Max)
					continue;

				const vec2 localSpaceTL = vec2(camera.TimeToLocalSpaceX(startTime), rowIt.LocalY);
				const vec2 localSpaceBL = localSpaceTL + vec2(0.0f, rowIt.LocalHeight);
				const vec2 localSpaceCenter = localSpaceTL + vec2(0.0f, rowIt.LocalHeight * 0.5f);
				const vec2 textPosition = timeline.LocalToScreenSpace(localSpaceCenter + vec2(3.0f, textHeight * -0.5f));

				Gui::DisableFontPixelSnap(true);
				{
					[[maybe_unused]] char b[32]; std::string_view text; u32 lineColor = TimelineDefaultLineColor; u32 textColor = TimelineItemTextColor;
					if constexpr (std::is_same_v<T, TempoChange>) { text = std::string_view(b, sprintf_s(b, useCompactFormat ? "%.0f BPM" : "%g BPM", it.Tempo.BPM)); lineColor = TimelineTempoChangeLineColor; }
					if constexpr (std::is_same_v<T, TimeSignatureChange>) { text = std::string_view(b, sprintf_s(b, "%d/%d", it.Signature.Numerator, it.Signature.Denominator)); lineColor = TimelineSignatureChangeLineColor; textColor = IsTimeSignatureSupported(it.Signature) ? TimelineItemTextColor : TimelineItemTextColorWarning; }
					if constexpr (std::is_same_v<T, ScrollChange>) { text = std::string_view(b, sprintf_s(b, "%sx", it.ScrollSpeed.toString().c_str())); lineColor = it.ScrollSpeed.IsReal() ? TimelineScrollChangeLineColor : TimelineScrollChangeComplexLineColor; }
					if constexpr (std::is_same_v<T, BarLineChange>) { text = it.IsVisible ? "On" : "Off"; lineColor = TimelineBarLineChangeLineColor; }
					if constexpr (std::is_same_v<T, ScrollType>) { text = std::string_view(b, sprintf_s(b, "%s", it.Method_ToString().c_str())); lineColor = TimelineScrollTypeLineColor; }
					if constexpr (std::is_same_v<T, JPOSScrollChange>) { text = std::string_view(b, sprintf_s(b, "%s", it.Move.toString().c_str())); }

					const vec2 textSize = Gui::CalcTextSize(text);

					drawListContent->AddRectFilled(vec2(timeline.LocalToScreenSpace(localSpaceTL).x, textPosition.y), textPosition + textSize, TimelineBackgroundColor);
					if constexpr (std::is_same_v<T, JPOSScrollChange>) {
						// draw bar background; still need the simple text background if too narrow
						static constexpr f32 margin = 1.0f;
						const vec2 localTL = vec2(camera.TimeToLocalSpaceX(startTime), 0.0f) + vec2(0.0f, rowIt.LocalY + margin);
						const vec2 localBR = vec2(camera.TimeToLocalSpaceX(endTime), 0.0f) + vec2(0.0f, rowIt.LocalY + rowIt.LocalHeight - (margin * 2.0f));
						DrawTimelineJPOSScrollBackground(drawListContent, timeline.LocalToScreenSpace(localTL) + vec2(0.0f, 2.0f), timeline.LocalToScreenSpace(localBR), it.IsSelected);
					}
					else {
						drawListContent->AddLine(timeline.LocalToScreenSpace(localSpaceTL + vec2(0.0f, 1.0f)), timeline.LocalToScreenSpace(localSpaceBL), it.IsSelected ? TimelineSelectedItemLineColor : lineColor);
					}
					Gui::AddTextWithDropShadow(drawListContent, textPosition, textColor, text, TimelineItemTextColorShadow);

					if (it.IsSelected)
					{
						const vec2 lineStartEnd[2] = { textPosition + vec2(0.0f, textSize.y), textPosition + textSize };
						drawListContent->AddLine(lineStartEnd[0] + vec2(1.0f), lineStartEnd[1] + vec2(1.0f), TimelineItemTextUnderlineColorShadow);
						drawListContent->AddLine(lineStartEnd[0], lineStartEnd[1], TimelineItemTextUnderlineColor);
					}
				}
				Gui::DisableFontPixelSnap(false);
			}
		}
	}

	static constexpr f32 TimeToScrollbarLocalSpaceX(Time time, const TimelineRegions& regions, Time chartDuration)
	{
		return static_cast<f32>(ConvertRange<f64>(0.0, chartDuration.ToSec(), 0.0, regions.ContentScrollbarX.GetWidth(), time.ToSec()));
	}

	static constexpr f32 TimeToScrollbarLocalSpaceXClamped(Time time, const TimelineRegions& regions, Time chartDuration)
	{
		return Clamp(TimeToScrollbarLocalSpaceX(time, regions, chartDuration), 1.0f, regions.ContentScrollbarX.GetWidth() - 2.0f);
	}

	static void DrawTimelineScrollbarXWaveform(const ChartTimeline& timeline, ImDrawList* drawList, Time chartSongOffset, Time chartDuration, const Audio::WaveformMipChain& waveformL, const Audio::WaveformMipChain& waveformR, f32 waveformAnimation)
	{
		assert(!waveformL.IsEmpty());
		const f32 waveformAnimationScale = Clamp(waveformAnimation, 0.0f, 1.0f);
		const f32 waveformAnimationAlpha = (waveformAnimationScale * waveformAnimationScale);
		const u32 waveformColor = Gui::ColorU32WithAlpha(TimelineWaveformBaseColor, waveformAnimationAlpha * 0.5f * (waveformR.IsEmpty() ? 2.0f : 1.0f));

		const Time waveformTimePerPixel = Time::FromSec(chartDuration.ToSec() / ClampBot(timeline.Regions.ContentScrollbarX.GetWidth(), 1.0f));
		const Time waveformDuration = waveformL.Duration;

		const Rect scrollbarRect = timeline.Regions.ContentScrollbarX;
		const f32 scrollbarHeight = scrollbarRect.GetHeight();

		const f32 minAmplitude = (2.0f / scrollbarHeight);
		for (size_t waveformIndex = 0; waveformIndex < 2; waveformIndex++)
		{
			const auto& waveform = (waveformIndex == 0) ? waveformL : waveformR;
			if (waveform.IsEmpty())
				continue;

			const auto& waveformMip = waveform.FindClosestMip(waveformTimePerPixel);
			for (i32 visiblePixel = 0; visiblePixel < scrollbarRect.GetWidth(); /*visiblePixel++*/)
			{
				CustomDraw::WaveformChunk chunk;
				const Rect chunkRect = Rect::FromTLSize(timeline.LocalToScreenSpace_ScrollbarX(vec2(static_cast<f32>(visiblePixel), 0.5f)), vec2(static_cast<f32>(CustomDraw::WaveformPixelsPerChunk), scrollbarHeight));

				for (i32 chunkPixel = 0; chunkPixel < CustomDraw::WaveformPixelsPerChunk; chunkPixel++)
				{
					const Time timeAtPixel = (waveformTimePerPixel * static_cast<f64>(visiblePixel)) - chartSongOffset;
					const b8 outOfBounds = (timeAtPixel < Time::Zero() || (timeAtPixel > waveformDuration));

					chunk.PerPixelAmplitude[chunkPixel] = outOfBounds ? 0.0f : (waveformAnimationScale * ClampBot(waveform.GetAmplitudeAt(waveformMip, timeAtPixel, waveformTimePerPixel), minAmplitude));
					visiblePixel++;
				}

				CustomDraw::DrawWaveformChunk(drawList, chunkRect, waveformColor, chunk);
			}
		}
	}

	static void DrawTimelineScrollbarXMinimap(const ChartTimeline& timeline, ImDrawList* drawList, const ChartCourse& course, BranchType branch, Time chartDuration)
	{
		const vec2 localNoteRectSize = GuiScale(vec2(2.0f, 4.0f)); // timeline.Regions.ContentScrollbarX.GetHeight() * 0.25f;
		const f32 localNoteCenterY = timeline.Regions.ContentScrollbarX.GetHeight() * /*0.5f*//*0.75f*/0.25f;

		// TODO: Also draw other timeline items... tempo / signature changes, gogo-time etc. (?)
		for (const Note& note : course.GetNotes(branch))
		{
			const f32 localHeadX = TimeToScrollbarLocalSpaceX(course.TempoMap.BeatToTime(note.GetStart()) + note.TimeOffset, timeline.Regions, chartDuration);
			Rect screenNoteRect = Rect::FromCenterSize(timeline.LocalToScreenSpace_ScrollbarX(vec2(localHeadX, localNoteCenterY)), localNoteRectSize);

			if (note.BeatDuration > Beat::Zero())
			{
				const f32 localTailX = TimeToScrollbarLocalSpaceX(course.TempoMap.BeatToTime(note.GetEnd()) + note.TimeOffset, timeline.Regions, chartDuration);
				screenNoteRect.BR.x += (localTailX - localHeadX);
			}

			drawList->AddRectFilled(screenNoteRect.TL, screenNoteRect.BR, note.IsSelected ? NoteColorWhite : *NoteTypeToColorMap[EnumToIndex(note.Type)]);
			// drawList->AddRect(screenNoteRect.TL, screenNoteRect.BR, NoteColorWhite, 0.0f, ImDrawFlags_None, 2.0f);
		}
	}

	static void UpdateTimelinePlaybackAndMetronomneSounds(ChartContext& context, b8 playbackSoundsEnabled, ChartTimeline::MetronomeData& metronome)
	{
		static constexpr Time frameTimeThresholdAtWhichPlayingSoundsMakesNoSense = Time::FromMS(250.0);
		static constexpr Time playbackSoundFutureOffset = Time::FromSec(1.0 / 25.0);

		Time& nonSmoothCursorThisFrame = context.CursorNonSmoothTimeThisFrame;
		Time& nonSmoothCursorLastFrame = context.CursorNonSmoothTimeLastFrame;

		nonSmoothCursorLastFrame = nonSmoothCursorThisFrame;
		nonSmoothCursorThisFrame = (context.SongVoice.GetPosition() + context.Chart.SongOffset);

		const Time elapsedCursorTimeSinceLastUpdate = (nonSmoothCursorLastFrame - nonSmoothCursorThisFrame);
		if (elapsedCursorTimeSinceLastUpdate >= frameTimeThresholdAtWhichPlayingSoundsMakesNoSense)
			return;

		const Time futureOffset = (playbackSoundFutureOffset * Min(context.SongVoice.GetPlaybackSpeed(), 1.0f));

		if (playbackSoundsEnabled)
		{
			auto checkAndPlayNoteSound = [&](Time noteTime, NoteType noteType, f32 pan)
			{
				const Time offsetNoteTime = noteTime - futureOffset;
				if (offsetNoteTime >= nonSmoothCursorLastFrame && offsetNoteTime < nonSmoothCursorThisFrame)
				{
					// NOTE: Don't wanna cause any audio cutoffs. If this happens the future threshold is either set too low for the current frame time
					//		 or playback was started on top of an existing note
					const Time startTime = Min((nonSmoothCursorThisFrame - noteTime), Time::Zero());
					const Time externalClock = noteTime;

					PlaySoundEffectTypeForNoteType(context, noteType, startTime, externalClock, pan);
				}
			};

			auto handleNotePlayback = [&](const ChartCourse* course, BranchType branch, const Note& note, i32 nLanes, i32 iLane)
			{
				f32 pan = (nLanes <= 1) ? 0 : 2.0 * iLane / (nLanes - 1) - 1;
				if (note.BeatDuration > Beat::Zero())
				{
					if (IsBalloonNote(note.Type))
					{
						for (i32 iPop = 0; iPop < note.BalloonPopCount; ++iPop)
							checkAndPlayNoteSound(course->TempoMap.BeatToTime(ConvertRange(0, i32{ note.BalloonPopCount }, note.BeatTime, note.GetEnd(), iPop)) + note.TimeOffset, note.Type, pan);
					}
					else
					{
						const Beat drummrollBeatInterval = GetGridBeatSnap(*Settings.General.DrumrollAutoHitBarDivision);
						for (Beat subBeat = Beat::Zero(); subBeat <= note.BeatDuration; subBeat += drummrollBeatInterval)
							checkAndPlayNoteSound(course->TempoMap.BeatToTime(note.BeatTime + subBeat) + note.TimeOffset, note.Type, pan);
					}
				}
				else
				{
					checkAndPlayNoteSound(course->TempoMap.BeatToTime(note.BeatTime) + note.TimeOffset, note.Type, pan);
				}
			};

			const i32 nLanes = size(context.ChartsCompared);
			i32 iLane = -1;
			for (auto it = cbegin(context.Chart.Courses); it != cend(context.Chart.Courses); ++it) {
				const auto* course = it->get();
				const BranchType branch = (course == context.ChartSelectedCourse) ? context.ChartSelectedBranch : BranchType::Normal;
				if (!context.IsChartCompared(course, branch))
					continue;
				const b8 isFocusedLane = (context.CompareMode && course == context.ChartSelectedCourse && branch == context.ChartSelectedBranch);
				++iLane;

				for (const Note& note : course->GetNotes(branch))
					handleNotePlayback(course, branch, note, nLanes, iLane);
			}
		}

		if (metronome.IsEnabled)
		{
			if (nonSmoothCursorLastFrame != metronome.LastProvidedNonSmoothCursorTime)
			{
				metronome.LastPlayedBeatTime = Time::FromSec(F64Min);
				metronome.HasOnPlaybackStartTimeBeenPlayed = false;
			}
			metronome.LastProvidedNonSmoothCursorTime = nonSmoothCursorThisFrame;

			const Beat cursorBeatStart = context.TimeToBeat(nonSmoothCursorThisFrame);
			const Beat cursorBeatEnd = cursorBeatStart + Beat::FromBars(1);
			const Time cursorTimeOnPlaybackStart = context.CursorTimeOnPlaybackStart;

			context.ChartSelectedCourse->TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
			{
				if (it.Beat >= cursorBeatEnd)
					return ControlFlow::Break;

				const Time beatTime = context.BeatToTime(it.Beat);
				const Time offsetBeatTime = (beatTime - futureOffset);

				// BUG: This is definitely too lenient and easily falsely triggers 1/16th after a bar start
				// HACK: Imperfect solution for making up for the future offset skipping past the first beat(s) if they are too close to the playback start time
				if (!metronome.HasOnPlaybackStartTimeBeenPlayed && ApproxmiatelySame(beatTime.Seconds, cursorTimeOnPlaybackStart.Seconds, 0.01))
				{
					metronome.HasOnPlaybackStartTimeBeenPlayed = true;
					metronome.LastPlayedBeatTime = beatTime;
					context.SfxVoicePool.PlaySound(it.IsBar ? SoundEffectType::MetronomeBar : SoundEffectType::MetronomeBeat);
					return ControlFlow::Break;
				}

				if (offsetBeatTime >= nonSmoothCursorLastFrame && offsetBeatTime < nonSmoothCursorThisFrame)
				{
					if (metronome.LastPlayedBeatTime != beatTime)
					{
						metronome.LastPlayedBeatTime = beatTime;
						const Time startTime = Min((nonSmoothCursorThisFrame - beatTime), Time::Zero());
						context.SfxVoicePool.PlaySound(it.IsBar ? SoundEffectType::MetronomeBar : SoundEffectType::MetronomeBeat, startTime);
					}
					return ControlFlow::Break;
				}

				return ControlFlow::Fallthrough;
			});
		}
	}

	void ChartTimeline::DrawGui(ChartContext& context)
	{
		UpdateInputAtStartOfFrame(context);
		UpdateAllAnimationsAfterUserInput(context);

#if PEEPO_DEBUG // DEBUG: Submit empty window first for more natural tab order sorting
		if (Gui::Begin(UI_WindowName("TAB_TIMELINE_DEBUG"))) { DrawTimelineDebugWindowContent(*this, context); } Gui::End();
#endif

		const auto& style = Gui::GetStyle();

		// NOTE: Big enough to fit both the bar-index, bar-time and some padding
		static constexpr f32 contentAndSidebarHeaderHeight = 34.0f;
		const f32 sidebarWidth = GuiScale(CurrentSidebarWidth);
		const f32 sidebarHeight = GuiScale(contentAndSidebarHeaderHeight);

		const f32 scrollbarHeight = Gui::GetStyle().ScrollbarSize * 3.0f;
		Regions.Window = Rect::FromTLSize(Gui::GetCursorScreenPos(), Gui::GetContentRegionAvail());
		Regions.SidebarHeader = Rect::FromTLSize(Regions.Window.TL, vec2(sidebarWidth, sidebarHeight));
		Regions.Sidebar = Rect::FromTLSize(Regions.SidebarHeader.GetBL(), vec2(sidebarWidth, Regions.Window.GetHeight() - sidebarHeight));
		Regions.ContentHeader = Rect(Regions.SidebarHeader.GetTR(), vec2(Regions.Window.BR.x, Regions.SidebarHeader.BR.y));
		Regions.Content = Rect::FromTLSize(Regions.ContentHeader.GetBL(), Max(vec2(0.0f), vec2(Regions.Window.GetWidth() - sidebarWidth, Regions.Window.GetHeight() - sidebarHeight - scrollbarHeight)));
		Regions.ContentScrollbarX = Rect::FromTLSize(Regions.Content.GetBL(), vec2(Regions.Content.GetWidth(), scrollbarHeight));

		auto timelineRegionBegin = [](const Rect region, cstr name, b8 padding = false)
		{
			if (!padding) Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });

			Gui::SetCursorScreenPos(region.GetTL());
			Gui::BeginChild(name, region.GetSize(), true, ImGuiWindowFlags_NoScrollbar);

			if (!padding) Gui::PopStyleVar(1);
		};

		auto timelineRegionEnd = []()
		{
			Gui::EndChild();
		};

		IsAnyChildWindowFocused = Gui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

		timelineRegionBegin(Regions.Window, "TimelineWindow");
		{
			timelineRegionBegin(Regions.SidebarHeader, "TimelineSideBarHeader");
			{
				DrawListSidebarHeader = Gui::GetWindowDrawList();
				char buffer[64];

				// TODO: ...
				static constexpr f32 buttonAlpha = 0.35f;
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_Button, buttonAlpha));
				Gui::PushStyleColor(ImGuiCol_ButtonHovered, Gui::GetColorU32(ImGuiCol_ButtonHovered, buttonAlpha));
				Gui::PushStyleColor(ImGuiCol_ButtonActive, Gui::GetColorU32(ImGuiCol_ButtonActive, buttonAlpha));
				{
					Gui::BeginDisabled();
					const vec2 perButtonSize = vec2(Gui::GetContentRegionAvail()) * vec2(1.0f / 3.0f, 1.0f);
					{
						Time displayTime = {};
						if (*Settings.General.DisplayTimeInSongSpace)
							displayTime = ClampBot(context.GetCursorTime() - context.Chart.SongOffset, Min(-context.Chart.SongOffset, Time::Zero()));
						else
							displayTime = ClampBot(context.GetCursorTime(), Time::Zero());

						Gui::Button(displayTime.ToString().Data, perButtonSize);
					}
					Gui::SameLine(0.0f, 0.0f);
					{
						sprintf_s(buffer, "%g%%", ToPercent(context.GetPlaybackSpeed()));
						Gui::Button(buffer, perButtonSize);
					}
					Gui::SameLine(0.0f, 0.0f);
					{
						sprintf_s(buffer, "1 / %d", CurrentGridBarDivision);
						Gui::Button(buffer, perButtonSize);
					}
					Gui::EndDisabled();
				}
				Gui::PopStyleColor(3);
				Gui::PopFont();
			}
			timelineRegionEnd();

			const ImGuiHoveredFlags hoveredFlags = BoxSelection.IsActive ? ImGuiHoveredFlags_AllowWhenBlockedByActiveItem : ImGuiHoveredFlags_None;

			timelineRegionBegin(Regions.Sidebar, "TimelineSideBar");
			{
				DrawListSidebar = Gui::GetWindowDrawList();
				IsSidebarWindowHovered = Gui::IsWindowHovered(hoveredFlags);
			}
			timelineRegionEnd();

			timelineRegionBegin(Regions.ContentHeader, "TimelineContentHeader");
			{
				DrawListContentHeader = Gui::GetWindowDrawList();
				IsContentHeaderWindowHovered = Gui::IsWindowHovered(hoveredFlags);
			}
			timelineRegionEnd();

			timelineRegionBegin(Regions.Content, "TimelineContent");
			{
				if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && !Gui::IsWindowFocused())
				{
					if (Gui::IsMouseClicked(ImGuiMouseButton_Middle) || Gui::IsMouseClicked(ImGuiMouseButton_Right))
						Gui::SetWindowFocus();
				}

				DrawListContent = Gui::GetWindowDrawList();
				IsContentWindowHovered = Gui::IsWindowHovered(hoveredFlags);
				IsContentWindowFocused = Gui::IsWindowFocused();
			}
			timelineRegionEnd();

			timelineRegionBegin(Regions.ContentScrollbarX, "TimelineContentScrollbarX");
			{
				Gui::PushStyleColor(ImGuiCol_ScrollbarGrab, Gui::GetColorU32(ImGuiCol_ScrollbarGrab, 0.2f));
				Gui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, Gui::GetColorU32(ImGuiCol_ScrollbarGrabHovered, 0.2f));
				Gui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, Gui::GetColorU32(ImGuiCol_ScrollbarGrabActive, 0.2f));

				// NOTE: Waveform and cursor on top of scrollbar!
				{
					const Time cursorTime = context.GetCursorTime();
					const Time chartDuration = context.Chart.GetDurationOrDefault();
					const b8 isPlayback = context.GetIsPlayback();

					if (!context.SongWaveformL.IsEmpty())
						DrawTimelineScrollbarXWaveform(*this, Gui::GetWindowDrawList(), context.Chart.SongOffset, chartDuration, context.SongWaveformL, context.SongWaveformR, context.SongWaveformFadeAnimationCurrent);

					DrawTimelineScrollbarXMinimap(*this, Gui::GetWindowDrawList(), *context.ChartSelectedCourse, context.ChartSelectedBranch, chartDuration);

					const f32 animatedCursorLocalSpaceX = TimeToScrollbarLocalSpaceXClamped(Camera.WorldSpaceXToTime(WorldSpaceCursorXAnimationCurrent), Regions, chartDuration);
					const f32 currentCursorLocalSpaceX = TimeToScrollbarLocalSpaceXClamped(cursorTime, Regions, chartDuration);
					const f32 cursorLocalSpaceX = isPlayback ? currentCursorLocalSpaceX : animatedCursorLocalSpaceX;

					// BUG: Drawn cursor doesn't perfectly line up on the left side with the scroll grab hand (?)
					Gui::GetWindowDrawList()->AddLine(
						LocalToScreenSpace_ScrollbarX(vec2(cursorLocalSpaceX, 2.0f)),
						LocalToScreenSpace_ScrollbarX(vec2(cursorLocalSpaceX, Regions.ContentScrollbarX.GetHeight() - 2.0f)),
						TimelineCursorColor);
				}

				// NOTE: Nice extra bit of user feedback
				if (IsCameraMouseGrabActive) Gui::PushStyleColor(ImGuiCol_ScrollbarGrab, Gui::GetStyleColorVec4(ImGuiCol_ScrollbarGrabHovered));

				const f32 localSpaceVisibleWidth = Regions.Content.GetWidth();
				const f32 localSpaceTimelineWidth = Camera.WorldToLocalSpaceScale(vec2(Camera.TimeToWorldSpaceX(context.Chart.GetDurationOrDefault()), 0.0f)).x + 2.0f;

				// BUG: Scrollbar should still be interactable while box selecting
				static constexpr ImS64 padding = 1;
				ImS64 inOutScrollValue = static_cast<ImS64>(Camera.PositionCurrent.x - TimelineCameraBaseScrollX);
				const ImS64 inSizeAvail = static_cast<ImS64>(localSpaceVisibleWidth + TimelineCameraBaseScrollX);
				const ImS64 inContentSize = static_cast<ImS64>(localSpaceTimelineWidth);
				if (Gui::ScrollbarEx(ImRect(Regions.ContentScrollbarX.TL, Regions.ContentScrollbarX.BR), Gui::GetID("ContentScrollbarX"), ImGuiAxis_X,
					&inOutScrollValue, inSizeAvail, inContentSize, ImDrawFlags_RoundCornersNone))
				{
					// BUG: Only setting PositionTarget results in glitchy behavior when clicking somewhere on the scrollbar for shorter than the animation duration
					//		however setting both PositionCurrent and PositionTarget means no smooth scrolling while dragging around
					// Camera.PositionTarget.x = static_cast<f32>(inOutScrollValue);
					Camera.PositionCurrent.x = Camera.PositionTarget.x = static_cast<f32>(inOutScrollValue) + TimelineCameraBaseScrollX;
				}

				if (IsCameraMouseGrabActive) Gui::PopStyleColor();
				Gui::PopStyleColor(3);
			}
			timelineRegionEnd();

			DrawAllAtEndOfFrame(context);
			DrawListSidebarHeader = DrawListSidebar = DrawListContentHeader = DrawListContent = nullptr;
		}
		timelineRegionEnd();
	}

	void ChartTimeline::StartEndRangeSelectionAtCursor(ChartContext& context)
	{
		if (!RangeSelection.IsActive || RangeSelection.HasEnd)
		{
			RangeSelection.Start = RangeSelection.End = RoundBeatToCurrentGrid(context.GetCursorBeat());
			RangeSelection.HasEnd = false;
			RangeSelection.IsActive = true;
			RangeSelectionExpansionAnimationTarget = 0.0f;
		}
		else
		{
			RangeSelection.End = RoundBeatToCurrentGrid(context.GetCursorBeat());
			RangeSelection.HasEnd = true;
			RangeSelectionExpansionAnimationTarget = 1.0f;
			if (RangeSelection.End == RangeSelection.Start)
				RangeSelection = {};
		}
	}

	void ChartTimeline::PlayNoteSoundAndHitAnimationsAtBeat(ChartContext& context, Beat cursorBeat)
	{
		b8 soundHasBeenPlayed = false;
		for (Note& note : context.ChartSelectedCourse->GetNotes(context.ChartSelectedBranch))
		{
			if (note.BeatTime == cursorBeat)
			{
				note.ClickAnimationTimeRemaining = note.ClickAnimationTimeDuration = NoteHitAnimationDuration;
				if (!soundHasBeenPlayed) { PlaySoundEffectTypeForNoteType(context, note.Type); soundHasBeenPlayed = true; }
			}
		}
	}

	void ChartTimeline::ExecuteClipboardAction(ChartContext& context, ClipboardAction action)
	{
		static constexpr cstr clipboardTextHeader = "// PeepoDrumKit Clipboard";
		static constexpr auto itemsToClipboardText = [](const std::vector<GenericListStructWithType>& inItems, Beat baseBeat) -> std::string
		{
			std::string out; out.reserve(512); out += clipboardTextHeader; out += '\n';
			for (const auto& item : inItems)
			{
				char buffer[256]; i32 bufferLength = 0;
				switch (item.List)
				{
				case GenericList::TempoChanges:
				{
					const auto& in = item.Value.POD.Tempo;
					bufferLength = sprintf_s(buffer, "Tempo { %d, %g };\n", (in.Beat - baseBeat).Ticks, in.Tempo.BPM);
				} break;
				case GenericList::SignatureChanges:
				{
					const auto& in = item.Value.POD.Signature;
					bufferLength = sprintf_s(buffer, "TimeSignature { %d, %d, %d };\n", (in.Beat - baseBeat).Ticks, in.Signature.Numerator, in.Signature.Denominator);
				} break;
				case GenericList::Notes_Normal:
				case GenericList::Notes_Expert:
				case GenericList::Notes_Master:
				{
					const auto& in = item.Value.POD.Note;
					bufferLength = sprintf_s(buffer, "Note { %d, %d, %d, %d, %g };\n", (in.BeatTime - baseBeat).Ticks, in.BeatDuration.Ticks, static_cast<i32>(in.Type), in.BalloonPopCount, in.TimeOffset.ToMS());
				} break;
				case GenericList::ScrollChanges:
				{
					const auto& in = item.Value.POD.Scroll;
					bufferLength = sprintf_s(buffer, "ScrollSpeed { %d, %s };\n", (in.BeatTime - baseBeat).Ticks, in.ScrollSpeed.toString().c_str());
				} break;
				case GenericList::BarLineChanges:
				{
					const auto& in = item.Value.POD.BarLine;
					bufferLength = sprintf_s(buffer, "BarLine { %d, %d };\n", (in.BeatTime - baseBeat).Ticks, in.IsVisible ? 1 : 0);
				} break;
				case GenericList::GoGoRanges:
				{
					const auto& in = item.Value.POD.GoGo;
					bufferLength = sprintf_s(buffer, "GoGo { %d, %d };\n", (in.BeatTime - baseBeat).Ticks, in.BeatDuration.Ticks);
				} break;
				case GenericList::Lyrics:
				{
					// TODO: Properly handle escape characters (?)
					const auto& in = item.Value.NonTrivial.Lyric;
					out += "Lyric { ";
					out += std::string_view(buffer, sprintf_s(buffer, "%d, ", (in.BeatTime - baseBeat).Ticks));
					out += in.Lyric;
					out += " };\n";
				} break;
				case GenericList::ScrollType:
				{
					const auto& in = item.Value.POD.ScrollType;
					bufferLength = sprintf_s(buffer, "ScrollType { %d, %d };\n", (in.BeatTime - baseBeat).Ticks, in.Method);
				} break;
				case GenericList::JPOSScroll:
				{
					const auto& in = item.Value.POD.JPOSScroll;
					bufferLength = sprintf_s(buffer, "JPOSScroll { %d, %s, %g };\n", (in.BeatTime - baseBeat).Ticks, in.Move.toString().c_str(), in.Duration);
				} break;
				default: { assert(false); } break;
				}

				if (bufferLength > 0)
					out += std::string_view(buffer, bufferLength);
			}

			if (!out.empty() && out.back() == '\n')
				out.erase(out.end() - 1);

			return out;
		};
		static constexpr auto itemsFromClipboatdText = [](std::string_view clipboardText) -> std::vector<GenericListStructWithType>
		{
			std::vector<GenericListStructWithType> out;
			if (!ASCII::StartsWith(clipboardText, clipboardTextHeader))
				return out;

			// TODO: Split and parse by ';' instead of '\n' (?)
			ASCII::ForEachLineInMultiLineString(clipboardText, false, [&](std::string_view line)
			{
				if (line.empty() || ASCII::StartsWith(line, "//"))
					return;

				line = ASCII::TrimSuffix(ASCII::TrimSuffix(line, "\r"), "\n");
				const size_t openIndex = line.find_first_of('{');
				const size_t closeIndex = line.find_last_of('}');
				if (openIndex == std::string_view::npos || closeIndex == std::string_view::npos || closeIndex <= openIndex)
					return;

				const std::string_view itemType = ASCII::Trim(line.substr(0, openIndex));
				const std::string_view itemParam = ASCII::Trim(line.substr(openIndex + sizeof('{'), (closeIndex - openIndex) - sizeof('}')));

				if (itemType == "Lyric")
				{
					auto& newItem = out.emplace_back(); newItem.List = GenericList::Lyrics;
					auto& newItemValue = newItem.Value.NonTrivial.Lyric;

					const size_t commaIndex = itemParam.find_first_of(',');
					if (commaIndex != std::string_view::npos)
					{
						const std::string_view beatSubStr = ASCII::Trim(itemParam.substr(0, commaIndex));
						const std::string_view lyricSubStr = ASCII::Trim(itemParam.substr(commaIndex + sizeof(',')));

						ASCII::TryParse(beatSubStr, newItemValue.BeatTime.Ticks);
						newItemValue.Lyric = lyricSubStr;
					}
				}
				else
				{
					struct { i32 I32; f32 F32; Complex CPX; b8 IsValidI32, IsValidF32, IsValidCPX; } parsedParams[6] = {};
					ASCII::ForEachInCommaSeparatedList(itemParam, [&, paramIndex = 0](std::string_view v) mutable
					{
						if (paramIndex < ArrayCount(parsedParams))
						{
							if (v = ASCII::Trim(v); !v.empty())
							{
								parsedParams[paramIndex].IsValidI32 = ASCII::TryParse(v, parsedParams[paramIndex].I32);
								parsedParams[paramIndex].IsValidF32 = ASCII::TryParse(v, parsedParams[paramIndex].F32);
								parsedParams[paramIndex].IsValidCPX = ASCII::TryParse(v, parsedParams[paramIndex].CPX);
							}
						}
						paramIndex++;
					});

					if (itemType == "Tempo")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::TempoChanges;
						auto& newItemValue = newItem.Value.POD.Tempo;
						newItemValue = TempoChange {};
						newItemValue.Beat.Ticks = parsedParams[0].I32;
						newItemValue.Tempo.BPM = parsedParams[1].F32;
					}
					else if (itemType == "TimeSignature")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::SignatureChanges;
						auto& newItemValue = newItem.Value.POD.Signature;
						newItemValue = TimeSignatureChange {};
						newItemValue.Beat.Ticks = parsedParams[0].I32;
						newItemValue.Signature.Numerator = parsedParams[1].I32;
						newItemValue.Signature.Denominator = parsedParams[2].I32;
					}
					else if (itemType == "Note")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::Notes_Normal;
						auto& newItemValue = newItem.Value.POD.Note;
						newItemValue = Note {};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.BeatDuration.Ticks = parsedParams[1].I32;
						newItemValue.Type = static_cast<NoteType>(parsedParams[2].I32);
						newItemValue.BalloonPopCount = static_cast<i16>(parsedParams[3].I32);
						newItemValue.TimeOffset = Time::FromMS(parsedParams[4].F32);
					}
					else if (itemType == "ScrollSpeed")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::ScrollChanges;
						auto& newItemValue = newItem.Value.POD.Scroll;
						newItemValue = ScrollChange {};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.ScrollSpeed = parsedParams[1].CPX;
					}
					else if (itemType == "BarLine")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::BarLineChanges;
						auto& newItemValue = newItem.Value.POD.BarLine;
						newItemValue = BarLineChange {};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.IsVisible = (parsedParams[1].I32 != 0);
					}
					else if (itemType == "GoGo")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::GoGoRanges;
						auto& newItemValue = newItem.Value.POD.GoGo;
						newItemValue = GoGoRange {};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.BeatDuration.Ticks = parsedParams[1].I32;
					}
					else if (itemType == "ScrollType")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::ScrollType;
						auto& newItemValue = newItem.Value.POD.ScrollType;
						newItemValue = ScrollType{};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.Method = static_cast<ScrollMethod>(parsedParams[1].I32);
					}
					else if (itemType == "JPOSScroll")
					{
						auto& newItem = out.emplace_back(); newItem.List = GenericList::JPOSScroll;
						auto& newItemValue = newItem.Value.POD.JPOSScroll;
						newItemValue = JPOSScrollChange{};
						newItemValue.BeatTime.Ticks = parsedParams[0].I32;
						newItemValue.Move = parsedParams[1].CPX;
						newItemValue.Duration = parsedParams[2].F32;
					}
#if PEEPO_DEBUG
					else { assert(false); }
#endif
				}
			});
			return out;
		};
		static constexpr auto copyAllSelectedItems = [](const ChartCourse& course) -> std::vector<GenericListStructWithType>
		{
			std::vector<GenericListStructWithType> out;
			size_t selectionCount = 0; ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it) { selectionCount++; });
			if (selectionCount > 0)
			{
				out.reserve(selectionCount);
				ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it)
				{
					auto& itemValue = out.emplace_back();
					itemValue.List = it.List;
					TryGetGenericStruct(course, it.List, it.Index, itemValue.Value);
				});
			}
			return out;
		};
		static constexpr auto findBaseBeat = [](const std::vector<GenericListStructWithType>& items)
		{
			Beat minBase = Beat::FromTicks(I32Max);
			for (const auto& item : items)
				minBase = Min(GetBeat(item), minBase);
			return (minBase.Ticks != I32Max) ? minBase : Beat::Zero();
		};

		ChartCourse& course = *context.ChartSelectedCourse;
		switch (action)
		{
		case ClipboardAction::Cut:
		{
			if (auto selectedItems = copyAllSelectedItems(course); !selectedItems.empty())
			{
				size_t selectedNoteIndex = 0;
				for (const auto& item : selectedItems)
				{
					if (IsNotesList(item.List))
					{
						TempDeletedNoteAnimationsBuffer.push_back(
							DeletedNoteAnimation { item.Value.POD.Note, context.ChartSelectedBranch, ConvertRange(0.0f, static_cast<f32>(selectedItems.size()), 0.0f, -0.08f, static_cast<f32>(selectedNoteIndex)) });
						selectedNoteIndex++;
					}
				}

				Gui::SetClipboardText(itemsToClipboardText(selectedItems, findBaseBeat(selectedItems)).c_str());
				context.Undo.Execute<Commands::RemoveMultipleGenericItems_Cut>(&course, std::move(selectedItems));
			}
		} break;
		case ClipboardAction::Copy:
		{
			if (auto selectedItems = copyAllSelectedItems(course); !selectedItems.empty())
			{
				// TODO: Maybe also animate original notes being copied (?)
				Gui::SetClipboardText(itemsToClipboardText(selectedItems, findBaseBeat(selectedItems)).c_str());
			}
		} break;
		case ClipboardAction::Paste:
		{
			std::vector<GenericListStructWithType> clipboardItems = itemsFromClipboatdText(Gui::GetClipboardTextView());
			if (!clipboardItems.empty())
			{
				const Beat baseBeat = FloorBeatToCurrentGrid(context.GetCursorBeat()) - findBaseBeat(clipboardItems);
				for (auto& item : clipboardItems) { SetBeat(GetBeat(item) + baseBeat, item); }

				auto itemAlreadyExistsOrIsBad = [&](const GenericListStructWithType& item)
				{
					const b8 inclusiveBeatCheck = ListUsesInclusiveBeatCheck(item.List);
					auto check = [&](auto& list, auto& i) { return (GetBeat(i) < Beat::Zero()) || (list.TryFindOverlappingBeatUntrusted(GetBeat(i), GetBeat(i) + GetBeatDuration(i), inclusiveBeatCheck) != nullptr); };
					switch (item.List)
					{
					case GenericList::TempoChanges: return check(course.TempoMap.Tempo, item.Value.POD.Tempo);
					case GenericList::SignatureChanges: return check(course.TempoMap.Signature, item.Value.POD.Signature);
					case GenericList::Notes_Normal: return check(course.Notes_Normal, item.Value.POD.Note);
					case GenericList::Notes_Expert: return check(course.Notes_Expert, item.Value.POD.Note);
					case GenericList::Notes_Master: return check(course.Notes_Master, item.Value.POD.Note);
					case GenericList::ScrollChanges: return check(course.ScrollChanges, item.Value.POD.Scroll);
					case GenericList::BarLineChanges: return check(course.BarLineChanges, item.Value.POD.BarLine);
					case GenericList::GoGoRanges: return check(course.GoGoRanges, item.Value.POD.GoGo);
					case GenericList::Lyrics: return check(course.Lyrics, item.Value.NonTrivial.Lyric);
					case GenericList::ScrollType: return check(course.ScrollTypes, item.Value.POD.ScrollType);
					case GenericList::JPOSScroll: return check(course.JPOSScrollChanges, item.Value.POD.JPOSScroll);
					default: assert(false); return false;
					}
				};
				erase_remove_if(clipboardItems, itemAlreadyExistsOrIsBad);

				if (!clipboardItems.empty())
				{
					SetNotesWaveAnimationTimes(clipboardItems, +1);

					b8 isFirstNote = true;
					for (const auto& item : clipboardItems)
						if (isFirstNote && IsNotesList(item.List)) { PlaySoundEffectTypeForNoteType(context, item.Value.POD.Note.Type); isFirstNote = false; }

					context.Undo.Execute<Commands::AddMultipleGenericItems_Paste>(&course, std::move(clipboardItems));
				}
			}
		} break;
		case ClipboardAction::Delete:
		{
			if (auto selectedItems = copyAllSelectedItems(course); !selectedItems.empty())
			{
				for (const auto& item : selectedItems)
				{
					if (IsNotesList(item.List))
						TempDeletedNoteAnimationsBuffer.push_back(DeletedNoteAnimation { item.Value.POD.Note, context.ChartSelectedBranch, 0.0f });
				}

				context.Undo.Execute<Commands::RemoveMultipleGenericItems>(&course, std::move(selectedItems));
			}
		} break;
		default: { assert(false); } break;
		}
	}

	void ChartTimeline::ExecuteSelectionAction(ChartContext& context, SelectionAction action, const SelectionActionParam& param)
	{
		ChartCourse& course = *context.ChartSelectedCourse;
		switch (action)
		{
		default: { assert(false); } break;
		case SelectionAction::SelectAll: { ForEachChartItem(course, [&](const ForEachChartItemData& it) { SetIsSelected(true, it, course); }); } break;
		case SelectionAction::UnselectAll: { ForEachChartItem(course, [&](const ForEachChartItemData& it) { SetIsSelected(false, it, course); }); } break;
		case SelectionAction::InvertAll: { ForEachChartItem(course, [&](const ForEachChartItemData& it) { SetIsSelected(!GetIsSelected(it, course), it, course); }); } break;
		case SelectionAction::SelectToEnd:
			ForEachChartItem(course, [&](const ForEachChartItemData& it)
			{
				if (GetBeat(it, course) >= context.GetCursorBeat())
					SetIsSelected(true, it, course);
			});
			break;
		case SelectionAction::SelectAllWithinRangeSelection:
		{
			if (RangeSelection.IsActiveAndHasEnd())
			{
				const Beat rangeSelectionMin = RoundBeatToCurrentGrid(RangeSelection.GetMin());
				const Beat rangeSelectionMax = RoundBeatToCurrentGrid(RangeSelection.GetMax());
				ForEachChartItem(course, [&](const ForEachChartItemData& it)
				{
					const Beat itStart = GetBeat(it, course);
					const Beat itEnd = itStart + GetBeatDuration(it, course);
					if ((itStart <= rangeSelectionMax) && (itEnd >= rangeSelectionMin))
						SetIsSelected(true, it, course);
				});
			}
		} break;
		case SelectionAction::PerRowShiftSelected:
		{
			struct ItemProxy
			{
				ChartCourse& Course; GenericList List; i32 Index;
				inline b8 Exists() const { return (Index >= 0); }
				constexpr b8 IsSelected() const { return GetOrEmpty<GenericMember::B8_IsSelected>(Course, List, Index); }
				constexpr void IsSelected(b8 isSelected) { TrySet<GenericMember::B8_IsSelected>(Course, List, Index, isSelected); }
				inline static ItemProxy At(ChartCourse& course, GenericList list, i32 listCount, i32 i) { return ItemProxy { course, list, (i >= 0) && (i < listCount) ? i : -1 }; }
			};

			for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
			{
				const i32 listCount = static_cast<i32>(GetGenericListCount(course, list));
				if (param.ShiftDelta > 0)
				{
					for (i32 i = listCount - 1; i >= 0; i--)
					{
						ItemProxy thisIt = ItemProxy::At(course, list, listCount, i);
						ItemProxy nextIt = ItemProxy::At(course, list, listCount, i + 1);
						if (nextIt.Exists() && thisIt.IsSelected())
							nextIt.IsSelected(true);
						thisIt.IsSelected(false);
					}
				}
				else if (param.ShiftDelta < 0)
				{
					for (i32 i = 0; i < listCount; i++)
					{
						ItemProxy thisIt = ItemProxy::At(course, list, listCount, i);
						ItemProxy prevIt = ItemProxy::At(course, list, listCount, i - 1);
						if (prevIt.Exists() && thisIt.IsSelected())
							prevIt.IsSelected(true);
						thisIt.IsSelected(false);
					}
				}
			}
		} break;
		case SelectionAction::PerRowSelectPattern:
		{
			if (param.Pattern == nullptr || param.Pattern[0] == '\0')
				break;

			const std::string_view pattern = param.Pattern;
			for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
			{
				for (size_t i = 0, patternIndex = 0; i < GetGenericListCount(course, list); i++)
				{
					if (const ForEachChartItemData it = { list, i }; GetIsSelected(it, course))
					{
						if (pattern[patternIndex] != 'x')
							SetIsSelected(false, it, course);
						if (++patternIndex >= pattern.size())
							patternIndex = 0;
					}
				}
			}
		} break;
		}
	}

	static auto GetScaleChartItemRatios(const TransformActionParam& param)
	{
		bool byTempo = *Settings.General.TransformScale_ByTempo;
		bool keepTimePos = *Settings.General.TransformScale_KeepTimePosition;
		bool keepItemDur = *Settings.General.TransformScale_KeepItemDuration;
		std::array ratioBeat = { param.TimeRatio[0], param.TimeRatio[1] };
		if (byTempo) {
			if (keepTimePos) // scale tempo, adjust beat to compensate
				std::swap(ratioBeat[0], ratioBeat[1]);
			else // just scale tempo
				ratioBeat[0] = ratioBeat[1] = 1;
		}
		std::array ratioBeatAbs = { abs(ratioBeat[0]), abs(ratioBeat[1]) };
		if (byTempo || keepTimePos)
			ratioBeat = ratioBeatAbs; // reverse scroll instead
		bool reverseBeat = !(byTempo || keepTimePos) && (ratioBeat[0] / ratioBeat[1] < 0); // flip the selected region
		return std::tuple{ byTempo, keepTimePos, keepItemDur, ratioBeat, ratioBeatAbs, reverseBeat };
	}

	template <typename Arr0, typename Arr1>
	static b8 ScaleChartItemValues(GenericListStructWithType& item, const Arr0& ratio, const Arr1& ratioBeat, b8 byTempo, b8 keepTimePos, b8 keepTimeSignature)
	{
		static_assert(std::size(decltype(ratio){}) == 2 && std::size(decltype(ratioBeat){}) == 2);

		if (ratio[0] == ratio[1])
			return false; // no change

		b8 changed = false;
		if (Tempo v; TryGet<GenericMember::Tempo_V>(item, v)) {
			changed = true;
			if (byTempo) // scale beat interval (60/BPM)
				v.BPM = v.BPM * ratio[1] / ratio[0];
			else if (keepTimePos) // scale beat, adjust beat interval (60/BPM) to compensate
				v.BPM = v.BPM * ratio[0] / ratio[1];
			else
				changed = false;
			TrySet<GenericMember::Tempo_V>(item, v);
		}
		if (TimeSignature v; !keepTimeSignature && TryGet<GenericMember::TimeSignature_V>(item, v)) {
			if (std::abs(ratioBeat[0]) != std::abs(ratioBeat[1])) {
				const auto denomOrig = v.Denominator;
				v.Numerator *= std::abs(ratioBeat[0]);
				v.Denominator *= std::abs(ratioBeat[1]);
				v.Simplify(denomOrig); // use original denominator if possible
				changed = true;
			}
			if ((byTempo || keepTimePos) && Sign(ratio[0]) * Sign(ratio[1]) < 0) { // flip sign when BPM does
				v.Numerator *= -1;
				changed = true;
			}
			TrySet<GenericMember::TimeSignature_V>(item, v);
		}
		return changed;
	}

	template <typename Arr0, typename Arr1>
	static auto FragmentChartItem(const GenericListStructWithType& item, Beat beatDuration, Beat beatSplit, const Arr0& ratio, const Arr1& ratioBeat, b8 reverseBeat, b8 byTempo, b8 keepTimePos, b8 keepTimeSignature)
	{
		std::vector<std::tuple<GenericListStructWithType, Beat, b8>> res = {}; // {itemSplit, beatDurationSplit, isSplitPoint}
		Beat beatHead = GetBeat(item), beatEnd = beatHead + beatDuration;
		Beat beatMin = std::min(beatHead, beatEnd), beatMax = std::max(beatHead, beatEnd);
		if (!(beatSplit > beatMin && beatSplit < beatMax))
			return res;

		if (TimeSignature v; !keepTimeSignature && TryGet<GenericMember::TimeSignature_V>(item, v)) {

			if (reverseBeat || std::abs(ratioBeat[0]) != std::abs(ratioBeat[1]) // scaled
				|| ((byTempo || keepTimePos) && Sign(ratio[0]) * Sign(ratio[1]) < 0) // flip sign when BPM does
				) {
				Beat beatPerBar = abs(v.GetDurationPerBar());
				Beat beatEffectPre = (beatPerBar == Beat::Zero()) ? beatMin : beatMin + std::floor((beatSplit - beatMin) / beatPerBar) * beatPerBar;

				// /|| - - - - //| - - - - /| - - - - /|| -> process as usual; no fragmenting
				if (beatEffectPre != beatSplit) {
					// /|| - - - - /| - - -//- /| - - - - /||, ||: time sig change, |: measure border, -: beat, //: split, /: wanted measure border
					res.emplace_back(item, Beat::Zero(), false);
					if (beatEffectPre != beatMin) {
						// -> /|| - - - - /|| - - -//- /| - - - - /||
						res.emplace_back(item, Beat::Zero(), false);
						SetBeat(beatEffectPre, get<0>(res.back()));
					}
					// -> /|| - - - - /|| - - -//| - / - - | - - /(||), (||): mid-measure time sig change (unspecified behavior on defined measure)
					TimeSignature vPre = { Sign(v) * (beatSplit - beatEffectPre).Ticks, Beat::FromBars(1).Ticks };
					vPre.Simplify(v.Denominator);
					TrySet<GenericMember::TimeSignature_V>(get<0>(res.back()), vPre);
					// -> /|| - - - - /|| - - -//|| - / - - - | - /(||)
					res.emplace_back(item, Beat::Zero(), true);
					SetBeat(beatSplit, get<0>(res.back()));
					// -> /|| - - - - /|| - - -//|| - /| - | - | - | - /||
					TimeSignature vPost = { Sign(v) * (beatEffectPre + beatPerBar - beatSplit).Ticks, Beat::FromBars(1).Ticks };
					vPost.Simplify(v.Denominator);
					TrySet<GenericMember::TimeSignature_V>(get<0>(res.back()), vPost);
					if (Beat beatEffectPost = beatEffectPre + beatPerBar; beatEffectPost < beatMax) {
						// -> /|| - - - - /|| - - -//|| - /|| - - - - /||, done
						res.emplace_back(item, Beat::Zero(), false);
						SetBeat(beatEffectPost, get<0>(res.back()));
					}

					// calculate duration
					for (size_t i = 1; i < std::size(res); ++i) {
						auto& [itemPrev, beatDurationPrev, _] = res[i - 1];
						beatDurationPrev = GetBeat(get<0>(res[i])) - GetBeat(itemPrev);
					}
					auto& [itemLast, beatDurationLast, _] = res.back();
					beatDurationLast = beatMax - GetBeat(itemLast);
				}
			}
		}
		return res;
	}

	template <typename ArrB, typename Arr0, typename Arr1, std::enable_if_t<expect_type_v<decltype(std::declval<ArrB>()[0]), Beat>, b8> = true>
	static auto FragmentChartItem(const GenericListStructWithType& item, Beat beatDuration, ArrB&& beatSplits,
		const Arr0& ratio, const Arr1& ratioBeat, b8 reverseBeat, b8 byTempo, b8 keepTimePos, b8 keepTimeSignature)
	{
		b8 fragmented = false;
		std::vector res = { std::tuple{ item, beatDuration, false } }; // {itemSplit, beatDurationSplit, isSplitPoint}
		for (Beat beatSplit : beatSplits) {
			decltype(res) fragmentsI = {};
			for (auto& fragJ : res) {
				auto& [itemJ, beatDurationJ, isSplitPointJ] = fragJ;
				auto fragmentsJ = FragmentChartItem(itemJ, beatDurationJ, beatSplit, ratio, ratioBeat, reverseBeat, byTempo, keepTimePos, keepTimeSignature);
				if (fragmentsJ.empty()) {
					fragmentsI.push_back(std::move(fragJ));
				} else {
					auto& isSplitPointJ0 = get<2>(fragmentsJ[0]);
					isSplitPointJ0 |= isSplitPointJ;
					std::move(begin(fragmentsJ), end(fragmentsJ), std::back_insert_iterator(fragmentsI));
					fragmented = true;
				}
			}
			res = std::move(fragmentsI);
		}
		return fragmented ? res : (res = {});
	}

	static auto MergeChartItem(const GenericListStructWithType& itemX, Beat beatDurationX, const GenericListStructWithType& itemY, Beat beatDurationY, b8 keepTimeSignature)
	{
		assert(GetBeat(itemX) <= GetBeat(itemY) && "unhandled item order");

		std::vector<GenericListStructWithType> res = {};
		if (itemX.List != itemY.List)
			return res;
		if ((beatDurationX > Beat::Zero()) != (beatDurationY > Beat::Zero())) // cannot determine head
			return res;

		Beat beatHeadX = GetBeat(itemX), beatEndX = beatHeadX + beatDurationX;
		Beat beatMinX = std::min(beatHeadX, beatEndX), beatMaxX = std::max(beatHeadX, beatEndX);
		Beat beatHeadY = GetBeat(itemY), beatEndY = beatHeadY + beatDurationY;
		Beat beatMinY = std::min(beatHeadY, beatEndY), beatMaxY = std::max(beatHeadY, beatEndY);
		if (beatMaxX != beatMinY)
			return res;
		Beat beatMid = beatMaxX;

		if (TimeSignature vX, vY; !keepTimeSignature && TryGet<GenericMember::TimeSignature_V>(itemX, vX) && TryGet<GenericMember::TimeSignature_V>(itemY, vY)) {
			if (Sign(vX) * Sign(vY) >= 0) { // only zero or same-direction time sigs are mergeable
				Beat beatPerBarX = abs(vX.GetDurationPerBar()), beatPerBarY = abs(vY.GetDurationPerBar());
				Beat beatEffectPre = (beatPerBarX == Beat::Zero()) ? beatMinX : beatMinX + std::floor((beatMid - beatMinX) / beatPerBarX) * beatPerBarX;
				TimeSignature vPre = vX;

				// /|| - - - - /| - - -//|| - - /| - - /||, ||: time sig change, |: measure border, -: beat, //: merge, /: wanted measure border
				res.push_back(itemX);
				if (beatEffectPre == beatMid) // split before merge, not at merge
					beatEffectPre -= beatPerBarX;
				if (beatEffectPre > beatMinX && beatEffectPre != beatMid) {
					// -> /|| - - - - /|| - - -//(||) - | - / - | - /(||), (||): mid-measure time sig change (unspecified behavior on defined measure)
					res.push_back(itemX);
					SetBeat(beatEffectPre, res.back());
				}
				if (beatEffectPre != beatMinX) {
					vPre = { Sign(vX) * (beatMid - std::max(beatMinX, beatEffectPre)).Ticks, Beat::FromBars(1).Ticks };
					vPre.Simplify(vX.Denominator);
				}
				// -> /|| - - - - /|| - - -//- - /| - - /(||)
				TimeSignature vMerged = (vPre + vY).GetSimplified(vX.Denominator);
				TrySet<GenericMember::TimeSignature_V>(res.back(), vMerged);
				if (vMerged == vX && std::size(res) > 1)
					res.pop_back();
				if (Beat beatEffectPost = beatMid + beatPerBarY; beatEffectPost < beatMaxY) {
					// -> /|| - - - - /|| - - -//- - /|| - - /||, done
					res.push_back(itemY);
					SetBeat(beatEffectPost, res.back());
				}
			}
		}
		return res;
	}

	template <typename ArrB, typename Arr0, typename Arr1, typename Func>
	static auto RefragmentChartItem(const GenericListStructWithType& item, const Beat origBeatDuration, ArrB&& beatSplits,
		const Arr0& ratio, const Arr1& ratioBeat, b8 reverseBeat, b8 byTempo, b8 keepTimePos, b8 keepTimeSignature,
		Func&& mapFragment)
	{
		std::vector<GenericListStructWithType> res = {};
		std::vector<i8> mergeWays = {};
		auto fragments = FragmentChartItem(item, origBeatDuration, beatSplits, ratio, ratioBeat, reverseBeat, byTempo, keepTimePos, keepTimeSignature);
		b8 needSplitByMap = fragments.empty();
		if (needSplitByMap)
			fragments = { std::tuple{ item, origBeatDuration, false } };

		for (size_t i = 0; i < std::size(fragments); ++i) {
			auto& [itemI, beatDurationI, isSplitPointI] = fragments[i];
			const auto [hasPast, hasPresent, hasFuture] = mapFragment(itemI, beatDurationI, needSplitByMap, res);
			if (!needSplitByMap) {
				i32 nFragments = (static_cast<i32>(hasPast) + hasPresent + hasFuture);
				assert(nFragments <= 1 && "Fragment failed. Expected exactly 1 or no items to be inserted per fragment");
				if (nFragments > 0)
					mergeWays.push_back(!isSplitPointI ? 0 : (reverseBeat && hasPresent) ? 1 : -1);
			}
		}

		if (needSplitByMap)
			return res;

		// index of merged items in beat order
		std::vector<i8> orderToIdxItems (size(mergeWays));
		std::iota(begin(orderToIdxItems), end(orderToIdxItems), 0);
		std::sort(begin(orderToIdxItems), end(orderToIdxItems), [&](size_t i, size_t j)
		{
			return GetBeat(res[i]) < GetBeat(res[j]);
		});
		{ // rearrange added items in beat order
			decltype(res) itemToAddOrdered = {};
			itemToAddOrdered.reserve(size(res));
			for (auto idx : orderToIdxItems)
				itemToAddOrdered.push_back(std::move(res[idx]));
			res = std::move(itemToAddOrdered);
		}

		for (size_t i = size(res); i-- > 0;) { // merge from back to allow removing and inserting
			const auto mergeWay = mergeWays[orderToIdxItems[i]];
			if (mergeWay == 0)
				continue;
			auto ordI = i, ordJ = i + Sign(mergeWay);
			if (!(ordJ >= 0 && ordJ < size(mergeWays))) // only merge between fragments
				continue;
			if (ordI > ordJ)
				std::swap(ordI, ordJ);
			const auto& itemI = res[ordI], & itemJ = res[ordJ];
			const auto idxI = orderToIdxItems[ordI], idxJ = orderToIdxItems[ordJ];
			const auto& fragI = fragments[idxI], & fragJ = fragments[idxJ];
			auto mergedItems = MergeChartItem(itemI, get<1>(fragI), itemJ, get<1>(fragJ), keepTimeSignature);
			if (!mergedItems.empty()) {
				if (mergeWays[idxI] > 0)
					mergeWays[idxI] = 0; // already merged
				res.erase(begin(res) + ordI, begin(res) + ordJ + 1);
				std::move(std::begin(mergedItems), std::end(mergedItems), std::insert_iterator(res, begin(res) + ordI));
			}
		}

		return res;
	}

	void ChartTimeline::ExecuteTransformAction(ChartContext& context, TransformAction action, const TransformActionParam& param)
	{
		ChartCourse& course = *context.ChartSelectedCourse;
		switch (action)
		{
		default: { assert(false); } break;
		case TransformAction::FlipNoteType:
		{
			for (BranchType branch = {}; branch < BranchType::Count; IncrementEnum(branch))
			{
				size_t selectedNoteCount = 0;
				SortedNotesList& notes = context.ChartSelectedCourse->GetNotes(branch);
				for (const Note& note : notes) { if (note.IsSelected && IsNoteFlippable(note.Type)) selectedNoteCount++; }
				if (selectedNoteCount <= 0)
					continue;

				std::vector<Commands::ChangeMultipleNoteTypes::Data> noteTypesToChange;
				noteTypesToChange.reserve(selectedNoteCount);

				for (Note& note : notes)
				{
					if (note.IsSelected && IsNoteFlippable(note.Type))
					{
						auto& data = noteTypesToChange.emplace_back();
						data.Index = ArrayItToIndex(&note, &notes[0]);
						data.NewValue = FlipNote(note.Type);
						note.ClickAnimationTimeRemaining = note.ClickAnimationTimeDuration = NoteHitAnimationDuration;
					}
				}

				PlaySoundEffectTypeForNoteType(context, noteTypesToChange[0].NewValue);
				context.Undo.Execute<Commands::ChangeMultipleNoteTypes_FlipTypes>(&course, &notes, std::move(noteTypesToChange));
				context.Undo.DisallowMergeForLastCommand();
			}
		} break;
		case TransformAction::ToggleNoteSize:
		{
			for (BranchType branch = {}; branch < BranchType::Count; IncrementEnum(branch))
			{
				size_t selectedNoteCount = 0;
				SortedNotesList& notes = context.ChartSelectedCourse->GetNotes(branch);
				for (const Note& note : notes) { if (note.IsSelected) selectedNoteCount++; }
				if (selectedNoteCount <= 0)
					continue;

				std::vector<Commands::ChangeMultipleNoteTypes::Data> noteTypesToChange;
				noteTypesToChange.reserve(selectedNoteCount);

				for (Note& note : notes)
				{
					if (note.IsSelected)
					{
						auto& data = noteTypesToChange.emplace_back();
						data.Index = ArrayItToIndex(&note, &notes[0]);
						data.NewValue = ToggleNoteSize(note.Type);
						note.ClickAnimationTimeRemaining = note.ClickAnimationTimeDuration = NoteHitAnimationDuration;
					}
				}

				PlaySoundEffectTypeForNoteType(context, noteTypesToChange[0].NewValue);
				context.Undo.Execute<Commands::ChangeMultipleNoteTypes_ToggleSizes>(&course, &notes, std::move(noteTypesToChange));
				context.Undo.DisallowMergeForLastCommand();
			}
		} break;
		case TransformAction::ScaleItemTime:
		{
			assert(param.TimeRatio[1] != 0);
			if (param.TimeRatio[0] == param.TimeRatio[1])
				break;
			size_t selectedItemCount = 0; ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it) { selectedItemCount++; });
			if (selectedItemCount <= 0)
				return;

			auto [byTempo, keepTimePos, keepItemDur, ratioBeat, ratioBeatAbs, reverseBeat] = GetScaleChartItemRatios(param);
			static constexpr auto scale = [](const auto& now, const auto& first, const auto& ratio) { return (((now - first) / ratio[1]) * ratio[0]) + first; };

			b8 isFirst = true; Beat firstBeat = {}, minBeatBefore = {}, minBeatAfter = {};
			std::vector<GenericListStructWithType> itemsToRemove; itemsToRemove.reserve(selectedItemCount);
			std::vector<GenericListStructWithType> itemsToAdd; itemsToAdd.reserve(selectedItemCount);
			std::vector<size_t> idxItemsToAlignToStart; // move last selected item to start for reversing end-unbounded items
			ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it)
			{
				const Beat origBeat = GetBeat(it, course);
				if (isFirst) { minBeatAfter = minBeatBefore = firstBeat = origBeat; isFirst = false; }
				else if (origBeat < minBeatBefore) // Happens because ForEachSelectedChartItem is not in ascending order across rows
					minBeatBefore = origBeat;

				auto& itemToRemove = itemsToRemove.emplace_back();
				itemToRemove.List = it.List;
				TryGetGenericStruct(course, it.List, it.Index, itemToRemove.Value);

				itemsToAdd.push_back(itemToRemove);
				auto& itemToAdd = itemsToAdd.back();
				Beat nowBeat = scale(origBeat, firstBeat, ratioBeat);
				SetBeat(nowBeat, itemToAdd);

				Beat origBeatDuration = GetBeatDuration(itemToAdd);
				if (reverseBeat && !ListIsItemEndBounded(it.List)) { // use the next item as the end for reversing end-unbounded items
					Beat origBeatLastEffect = GetLastEffectBeat(course, it.List, it.Index);
					if (origBeatLastEffect > origBeat) {
						origBeatDuration = origBeatLastEffect - origBeat;
						if (!idxItemsToAlignToStart.empty() && itemsToAdd[idxItemsToAlignToStart.back()].List == it.List)
							idxItemsToAlignToStart.back() = itemsToAdd.size() - 1;
						else
							idxItemsToAlignToStart.push_back(itemsToAdd.size() - 1);
					}
					if (nowBeat < minBeatAfter) // only consider item head for realigning region
						minBeatAfter = nowBeat;
				}

				if (origBeatDuration > Beat::Zero()) {
					if (!keepItemDur || !ListIsItemEndBounded(it.List)) {
						Beat nowBeatDuration = scale(origBeatDuration, Beat::Zero(), ratioBeatAbs);
						SetBeatDuration(Max(Beat::FromTicks(1), nowBeatDuration), itemToAdd);
						if (reverseBeat)
							SetBeat(nowBeat -= nowBeatDuration, itemToAdd);
					}
				}
				if (const auto [hasTimeDuration, origTimeDuration] = GetTimeDuration(itemToAdd); hasTimeDuration) {
					if (!(keepItemDur || keepTimePos)) {
						SetTimeDuration(scale(origTimeDuration, Time::Zero(), ratioBeatAbs), itemToAdd);
						if (reverseBeat) {
							const Beat origEndBeat = context.TimeToBeat(context.BeatToTime(origBeat) + origTimeDuration, true); // in original timing
							Beat nowBeatEnd = scale(origEndBeat, firstBeat, param.TimeRatio);
							SetBeat(nowBeatEnd, itemToAdd);
							nowBeat = std::min(nowBeat, nowBeatEnd); // in case of negative time duration
						}
					}
				}
				ScaleChartItemValues(itemToAdd, param.TimeRatio, ratioBeat, byTempo, keepTimePos, *Settings.General.TransformScale_KeepTimeSignature);
				if (!(reverseBeat && !ListIsItemEndBounded(it.List)) && nowBeat < minBeatAfter) // handle overlapping items with varying lengths from different row
					minBeatAfter = nowBeat;

				if (IsNotesList(itemToAdd.List))
					itemToAdd.Value.POD.Note.ClickAnimationTimeRemaining = itemToAdd.Value.POD.Note.ClickAnimationTimeDuration = NoteHitAnimationDuration;
			});

			// BUG: Resolve item duration intersections (only *add* notes if they don't interect another non-selected long item (?))
			if (!itemsToRemove.empty() || !itemsToAdd.empty())
			{
				for (auto& it : itemsToAdd) if (IsNotesList(it.List)) { PlaySoundEffectTypeForNoteType(context, it.Value.POD.Note.Type); break; }

				if (minBeatBefore != minBeatAfter) { // realign region to originally earliest item
					for (auto& it : itemsToAdd)
						SetBeat(GetBeat(it) + minBeatBefore - minBeatAfter, it);
				}
				for (auto& idx : idxItemsToAlignToStart)
					SetBeat(minBeatBefore, itemsToAdd[idx]);

				if (reverseBeat)
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_ReverseItems>(&course, std::move(itemsToRemove), std::move(itemsToAdd));
				else if (ratioBeat[0] < ratioBeat[1])
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_CompressItems>(&course, std::move(itemsToRemove), std::move(itemsToAdd));
				else
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_ExpandItems>(&course, std::move(itemsToRemove), std::move(itemsToAdd));
				context.Undo.DisallowMergeForLastCommand();
			}
		} break;
		case TransformAction::ScaleRangeTime:
		{
			assert(param.TimeRatio[1] != 0);
			if (param.TimeRatio[0] == param.TimeRatio[1])
				break;
			if (!RangeSelection.IsActiveAndHasEnd())
				return;

			auto [byTempo, keepTimePos, keepItemDur, ratioBeat, ratioBeatAbs, reverseBeat] = GetScaleChartItemRatios(param);
			const b8 keepTimeSig = *Settings.General.TransformScale_KeepTimeSignature;
			enum RangeSide : u8 { Past, Present, Future, Count };
			static constexpr auto scale = [](const auto& now, const auto& first, const auto& ratio) { return (((now - first) / ratio[1]) * ratio[0]) + first; };

			const Beat firstBeat = RangeSelection.GetMin(), latestBeat = RangeSelection.GetMax();
			Beat minBeatAfter = firstBeat, maxBeatAfter = scale(latestBeat, firstBeat, ratioBeat);
			if (minBeatAfter > maxBeatAfter)
				std::swap(minBeatAfter, maxBeatAfter);

			auto getRangeSide = [&](Beat origBeat) { return (origBeat > latestBeat) ? RangeSide::Future : (origBeat >= firstBeat) ? RangeSide::Present : RangeSide::Past; };
			auto getRangeSideToFtr = [&](Beat origBeat) { return (origBeat >= latestBeat) ? RangeSide::Future : (origBeat >= firstBeat) ? RangeSide::Present : RangeSide::Past; };

			const auto scaleBeatIn = [&](const auto& beat) { return scale(beat, firstBeat, ratioBeat) + (firstBeat - minBeatAfter); };
			const auto scaleBeatAfter = [&](const auto& beat) { return beat + maxBeatAfter - latestBeat + (firstBeat - minBeatAfter); };

			std::vector<GenericListStructWithType> itemsToRemove;
			std::vector<GenericListStructWithType> itemsToAdd;
			ForEachChartItem(course, [&](const ForEachChartItemData& it)
			{
				const Beat origBeat = GetBeat(it, course);
				auto& itemToRemove = itemsToRemove.emplace_back();
				itemToRemove.List = it.List;
				TryGetGenericStruct(course, it.List, it.Index, itemToRemove.Value);

				std::vector<GenericListStructWithType> itemToAdd = {};

				b8 changed = false;
				if (getRangeSide(origBeat) == RangeSide::Future) {
					Beat nowBeat = scaleBeatAfter(origBeat);
					if (nowBeat != origBeat) {
						SetBeat(nowBeat, (itemToAdd.push_back(itemToRemove), itemToAdd.back()));
						changed = true;
					}
				} else {
					auto splitAndScale = [&](const GenericListStructWithType& item, const Beat origBeat, const Beat origBeatDuration, b8 needSplit, auto&& splitItems, auto&& setItemHeadAndEnd)
					{
						const b8 reverseDuration = (origBeatDuration < Beat::Zero());
						const Beat origBeatEnd = origBeat + origBeatDuration;

						const auto headSide = needSplit ? getRangeSide(origBeat) : getRangeSideToFtr(origBeat);
						const auto endSide = needSplit ? getRangeSide(origBeatEnd) : headSide;

						// to calculate using non-negative duration
						const auto& origBeatMin = std::min(origBeat, origBeatEnd), origBeatMax = std::max(origBeat, origBeatEnd);
						const auto& minSide = std::min(headSide, endSide), maxSide = std::max(headSide, endSide);

						// split: s-[->(e … s)-]>e → s₁->e₁[s₂->(e₂ … s₃)->e₃]s₄->e₄
						b8 hasPast = (minSide == RangeSide::Past);
						b8 hasPresent = (minSide <= RangeSide::Present && maxSide >= RangeSide::Present);
						b8 hasFuture = (maxSide == RangeSide::Future);

						const Beat beatMinPst = origBeatMin;
						const Beat beatMaxPst = std::min(firstBeat, origBeatMax);
						const Beat beatMinFtr = scaleBeatAfter(std::max(latestBeat, origBeatMin));
						const Beat beatMaxFtr = (origBeatMax.Ticks >= I32Max) ? origBeatMax : scaleBeatAfter(origBeatMax); // for endless events

						// reverse: move s₄ to effective first beat
						Beat beatMinFtrMoved = beatMinFtr;
						if (reverseBeat && hasFuture && !ListIsItemEndBounded(item.List)) {
							Beat origBeatFirstEffect = GetFirstEffectBeatAfter<false>(course, item.List, latestBeat);
							if (origBeatFirstEffect < origBeatEnd)
								beatMinFtrMoved = scaleBeatAfter(std::max(origBeatFirstEffect, origBeatMin));
							else
								hasFuture = false; // no space to insert
						}

						if (!ListIsItemEndBounded(item.List)) { // prevent 0-duration events if duration is implicit
							if (beatMaxPst <= beatMinPst)
								hasPast = false;
							if (beatMaxFtr <= beatMinFtr)
								hasFuture = false;
						}

						if (hasPresent) {
							auto maxSidePrs = RangeSide::Present;
							Beat beatHeadPrs = scaleBeatIn(std::max(firstBeat, origBeatMin));
							Beat beatEndPrs = scaleBeatIn(std::min(latestBeat, origBeatMax));
							Beat beatMinPrs = std::min(beatHeadPrs, beatEndPrs), beatMaxPrs = std::max(beatHeadPrs, beatEndPrs); // handle reversing
							// merge if the original item was single note or the resulting items have the same value
							splitItems.push_back(item);
							b8 changedIn = ScaleChartItemValues(splitItems.back(), param.TimeRatio, ratioBeat, byTempo, keepTimePos, keepTimeSig);
							if (!changedIn) {
								if (hasPast && beatMaxPst == beatMinPrs) {
									beatMinPrs = beatMinPst;
									hasPast = false;
								}
								if (hasFuture && beatMinFtr == beatMaxPrs) {
									maxSidePrs = RangeSide::Future;
									beatMaxPrs = beatMaxFtr;
									hasFuture = false;
								}
							}
							if (!ListIsItemEndBounded(item.List) && beatMaxPrs - beatMinPrs == Beat::Zero()) {
								splitItems.pop_back();
								hasPresent = false;
							} else {
								setItemHeadAndEnd(beatMinPrs, beatMaxPrs, reverseDuration, maxSide, maxSidePrs, splitItems.back());
							}
						}
						if (hasPast)
							setItemHeadAndEnd(beatMinPst, beatMaxPst, reverseDuration, maxSide, RangeSide::Past, (splitItems.push_back(item), splitItems.back()));
						if (hasFuture)
							setItemHeadAndEnd(beatMinFtrMoved, beatMaxFtr, reverseDuration, maxSide, RangeSide::Future, (splitItems.push_back(item), splitItems.back()));
						changed = true;

						return std::array{ hasPast, hasPresent, hasFuture };
					};

					Beat origBeatDuration = GetBeatDuration(itemToRemove);
					if (!ListIsItemEndBounded(it.List)) {
						Beat origBeatLastEffect = GetLastEffectBeat(course, it.List, it.Index);
						if (origBeatLastEffect > origBeat)
							origBeatDuration = origBeatLastEffect - origBeat;
					}
					if (origBeatDuration > Beat::Zero()) {
						if (!keepItemDur || !ListIsItemEndBounded(it.List)) {
							auto fragments = RefragmentChartItem(itemToRemove, origBeatDuration,
								std::array{ firstBeat, latestBeat }, param.TimeRatio, ratioBeat, reverseBeat, byTempo, keepTimePos, keepTimeSig,
								[&](const GenericListStructWithType& itemI, Beat& beatDurationI, b8 needSplit, auto&& fragments)
								{
									return splitAndScale(itemI, GetBeat(itemI), beatDurationI, needSplit, fragments, [&](Beat head, Beat end, b8 reverse, RangeSide maxSideItem, RangeSide maxSideSet, GenericListStructWithType& item)
									{
										auto duration = end - head;
										SetBeat(reverse ? end : head, item);
										SetBeatDuration(Max(Beat::FromTicks(1), beatDurationI = (reverse ? -duration : duration)), item);
									});
								});
							std::move(begin(fragments), end(fragments), std::back_insert_iterator(itemToAdd));
						}
					}

					if (const auto [hasTimeDuration, origTimeDuration] = GetTimeDuration(itemToRemove); hasTimeDuration) {
						if (!(keepItemDur || keepTimePos)) {
							const Beat origEndBeat = context.TimeToBeat(context.BeatToTime(origBeat) + origTimeDuration, true); // in original timing
							const Time origTimeExtra = context.BeatToTime(origEndBeat) - (context.BeatToTime(origBeat) + origTimeDuration);
							const auto [hasPast, hasPresent, hasFuture] = splitAndScale(itemToRemove, origBeat, origEndBeat - origBeat, true, itemToAdd, [&](Beat head, Beat end, b8 reverse, RangeSide maxSideItem, RangeSide maxSideSet, GenericListStructWithType& item)
							{
								auto duration = context.BeatToTime(end) - context.BeatToTime(head);
								if (maxSideSet == maxSideItem)
									duration += (maxSideSet == RangeSide::Present) ? scale(origTimeExtra, Time::Zero(), param.TimeRatio) : origTimeExtra;
								SetBeat(reverse ? end : head, item);
								SetTimeDuration(reverse ? -duration : duration, item);
							});
						}
					}

					if (!changed && getRangeSide(origBeat) == RangeSide::Present) {
						Beat nowBeat = scaleBeatIn(origBeat);
						itemToAdd.push_back(itemToRemove);
						SetBeat(nowBeat, itemToAdd.back());
						changed |= (nowBeat != origBeat);
						changed |= ScaleChartItemValues(itemToAdd[0], param.TimeRatio, ratioBeat, byTempo, keepTimePos, keepTimeSig);
					}
				}

				// actually insert items
				if (!changed) {
					itemsToRemove.pop_back();
					return;
				}
				for (auto& item : itemToAdd) {
					if (IsNotesList(it.List))
						item.Value.POD.Note.ClickAnimationTimeRemaining = item.Value.POD.Note.ClickAnimationTimeDuration = NoteHitAnimationDuration;
					itemsToAdd.push_back(std::move(item));
				}
			});

			// BUG: Resolve item duration intersections (only *add* notes if they don't interect another non-selected long item (?))
			if (!itemsToRemove.empty() || !itemsToAdd.empty())
			{
				for (auto& it : itemsToAdd) if (IsNotesList(it.List)) { PlaySoundEffectTypeForNoteType(context, it.Value.POD.Note.Type); break; }

				if (firstBeat != minBeatAfter) { // realign region to earliest item
					maxBeatAfter += firstBeat - minBeatAfter;
					minBeatAfter = firstBeat;
				}

				if (reverseBeat) { // for range reversing, it is possible for long events to be reversed into negative beats; clip at beat 0
					for (auto& it : itemsToAdd) {
						if (Beat beat = GetBeat(it); beat < Beat::Zero()) {
							SetBeat(Beat::Zero(), it);
							if (Beat beatDuration = GetBeatDuration(it); beatDuration > Beat::Zero())
								SetBeatDuration(Max(Beat::FromTicks(1), beatDuration - (Beat::Zero() - beat)), it);
							if (auto [hasTimeDuration, timeDuration] = GetTimeDuration(it); hasTimeDuration) {
								const Time zeroTime = context.BeatToTime(Beat::Zero());
								const Time startTime = context.BeatToTime(beat);
								SetTimeDuration(timeDuration - (zeroTime - startTime), it);
							}
						}
					}
				}

				std::pair selectedRange = { &RangeSelection.Start, &RangeSelection.End };
				std::pair newRange = { minBeatAfter, maxBeatAfter };

				if (reverseBeat)
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_ReverseRange>(selectedRange, newRange, &course, std::move(itemsToRemove), std::move(itemsToAdd));
				else if (abs(param.TimeRatio[0]) < abs(param.TimeRatio[1]))
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_CompressRange>(selectedRange, newRange, &course, std::move(itemsToRemove), std::move(itemsToAdd));
				else
					context.Undo.Execute<Commands::RemoveThenAddMultipleGenericItems_ExpandRange>(selectedRange, newRange, &course, std::move(itemsToRemove), std::move(itemsToAdd));
				context.Undo.DisallowMergeForLastCommand();
			}
		} break;
		}
	}

	void ChartTimeline::UpdateInputAtStartOfFrame(ChartContext& context)
	{
		MousePosLastFrame = MousePosThisFrame;
		MousePosThisFrame = Gui::GetMousePos();

		// NOTE: Mouse scroll / zoom
		if (!ApproxmiatelySame(Gui::GetIO().MouseWheel, 0.0f))
		{
			vec2 scrollStep = vec2(Gui::GetIO().KeyShift ? *Settings.General.TimelineScrollDistancePerMouseWheelTickFast : *Settings.General.TimelineScrollDistancePerMouseWheelTick);
			scrollStep.y *= 0.75f;
			if (*Settings.General.TimelineScrollInvertMouseWheel)
				scrollStep.x *= -1.0f;

			if (IsSidebarWindowHovered)
			{
#if 0 // NOTE: Not needed at the moment for small number of rows
				if (!Gui::GetIO().KeyAlt)
					Camera.PositionTarget.y -= (Gui::GetIO().MouseWheel * scrollStep.y);
#endif
			}
			else if (IsContentHeaderWindowHovered || IsContentWindowHovered)
			{
				if (Gui::GetIO().KeyAlt)
				{
					const f32 zoomFactorX = *Settings.General.TimelineZoomFactorPerMouseWheelTick;
					const f32 newZoomX = (Gui::GetIO().MouseWheel > 0.0f) ? (Camera.ZoomTarget.x * zoomFactorX) : (Camera.ZoomTarget.x / zoomFactorX);
					Camera.SetZoomTargetAroundLocalPivot(vec2(newZoomX, Camera.ZoomTarget.y), ScreenToLocalSpace(Gui::GetMousePos()));
				}
				else
				{
					if (Gui::GetIO().KeyCtrl)
						Camera.PositionTarget.y -= (Gui::GetIO().MouseWheel * scrollStep.y);
					else
						Camera.PositionTarget.x += (Gui::GetIO().MouseWheel * scrollStep.x);
				}
			}
		}

		// NOTE: Mouse wheel grab scroll
		{
			if (IsContentWindowHovered && Gui::IsMouseClicked(ImGuiMouseButton_Middle))
				IsCameraMouseGrabActive = true;
			else if (IsCameraMouseGrabActive && !Gui::IsMouseDown(ImGuiMouseButton_Middle))
				IsCameraMouseGrabActive = false;

			if (IsCameraMouseGrabActive)
			{
				Gui::SetActiveID(Gui::GetID(&IsCameraMouseGrabActive), Gui::GetCurrentWindow());
				Gui::SetMouseCursor(ImGuiMouseCursor_Hand);

				// BUG: Kinda freaks out when mouse grabbing and smooth zooming at the same time
				Camera.PositionTarget.x = Camera.PositionCurrent.x += (MousePosLastFrame.x - MousePosThisFrame.x);
				Camera.ZoomTarget.x = Camera.ZoomCurrent.x;
			}
		}

		// NOTE: Auto playback scroll (needs to be updated before the toggle playback input check is run for better result (?))
		{
			if (context.GetIsPlayback() && Audio::Engine.GetIsStreamOpenRunning())
			{
				const Time cursorTime = context.GetCursorTime();
				if (IsTimelineCursorVisibleOnScreen(Camera, Regions, cursorTime) && Camera.TimeToLocalSpaceX(cursorTime) >= Round(Regions.Content.GetWidth() * TimelineAutoScrollLockContentWidthFactor))
				{
					const Time elapsedCursorTime = Time::FromSec(Gui::DeltaTime()) * context.GetPlaybackSpeed();
					const f32 cameraScrollIncrement = Camera.TimeToWorldSpaceX(elapsedCursorTime) * Camera.ZoomCurrent.x;
					Camera.PositionCurrent.x += cameraScrollIncrement;
					Camera.PositionTarget.x += cameraScrollIncrement;
				}
			}
		}

		// NOTE: Cursor controls
		{
			// NOTE: Selected items mouse drag
			{
				ChartCourse& selectedCourse = *context.ChartSelectedCourse;

				size_t selectedItemCount = 0; b8 allSelectedItemsAreNotes = true; b8 atLeastOneSelectedItemIsTempoChange = false;
				ForEachSelectedChartItem(selectedCourse, [&](const ForEachChartItemData& it)
				{
					selectedItemCount++;
					allSelectedItemsAreNotes &= IsNotesList(it.List);
					atLeastOneSelectedItemIsTempoChange |= (it.List == GenericList::TempoChanges);
				});

				if (SelectedItemDrag.ActiveTarget != EDragTarget::None && !Gui::IsMouseDown(ImGuiMouseButton_Left))
					SelectedItemDrag = {};

				SelectedItemDrag.HoverTarget = EDragTarget::None;
				SelectedItemDrag.MouseBeatLastFrame = SelectedItemDrag.MouseBeatThisFrame;
				SelectedItemDrag.MouseBeatThisFrame = FloorBeatToCurrentGrid(context.TimeToBeat(Camera.LocalSpaceXToTime(ScreenToLocalSpace(MousePosThisFrame).x)));

				if (selectedItemCount > 0 && IsContentWindowHovered && SelectedItemDrag.ActiveTarget == EDragTarget::None)
				{
					ForEachTimelineRow(*this, [&](const ForEachRowData& rowIt)
					{
						const GenericList list = GetEffectiveGenericList(rowIt.RowType, context.ChartSelectedBranch);
						const b8 isNotesRow = IsNotesList(list);

						const Rect screenRowRect = Rect(LocalToScreenSpace(vec2(0.0f, rowIt.LocalY)), LocalToScreenSpace(vec2(Regions.Content.GetWidth(), rowIt.LocalY + rowIt.LocalHeight)));
						const vec2 screenRectCenter = screenRowRect.GetCenter();

						for (size_t i = 0; i < GetGenericListCount(selectedCourse, list); i++)
						{
							b8 isSelected {};
							if (TryGet<GenericMember::B8_IsSelected>(selectedCourse, list, i, isSelected) && isSelected)
							{
								Beat beatStart {}, beatDuration {};
								f32 timeDuration {};
								const b8 hasBeatStart = TryGet<GenericMember::Beat_Start>(selectedCourse, list, i, beatStart);
								const b8 hasBeatDuration = TryGet<GenericMember::Beat_Duration>(selectedCourse, list, i, beatDuration);
								const b8 hasTimeDuration = TryGet<GenericMember::F32_JPOSScrollDuration>(selectedCourse, list, i, timeDuration);

								const vec2 center = vec2(LocalToScreenSpace(vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(beatStart)), 0.0f)).x, screenRectCenter.y);
								vec2 centerTail = center;

								f32 hitboxSize = TimelineSelectedNoteHitBoxSizeSmall;
								if (isNotesRow) {
									NoteType noteType = GetOrEmpty<GenericMember::NoteType_V>(selectedCourse, list, i);
									hitboxSize = (IsBigNote(noteType) ? TimelineSelectedNoteHitBoxSizeBig : TimelineSelectedNoteHitBoxSizeSmall);
								}

								Rect screenHitbox = Rect::FromCenterSize(center, vec2(GuiScale(hitboxSize)));
								Rect screenHitboxTail = screenHitbox;
								if (hasBeatDuration && beatDuration > Beat::Zero()) {
									// TODO: Proper hitboxses (at least for gogo range and lyrics?)
									centerTail = vec2(LocalToScreenSpace(vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(beatStart + beatDuration)), 0.0f)).x, screenRectCenter.y);
									screenHitboxTail = Rect::FromCenterSize(centerTail, vec2(GuiScale(hitboxSize)));
								}
								else if (hasTimeDuration) {
									centerTail = vec2(LocalToScreenSpace(vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(beatStart) + Time::FromSec(timeDuration)), 0.0f)).x, screenRectCenter.y);
									screenHitboxTail = Rect::FromCenterSize(centerTail, vec2(GuiScale(hitboxSize)));
								}

								for (const auto& [hitbox, target] : {
									std::make_tuple(screenHitbox, EDragTarget::Body),
									std::make_tuple(screenHitboxTail, EDragTarget::Tail),
									}) {
									if (hitbox.Contains(MousePosThisFrame))
									{
										SelectedItemDrag.HoverTarget = target;
										if (Gui::IsMouseClicked(ImGuiMouseButton_Left))
										{
											SelectedItemDrag.ActiveTarget = target;
											SelectedItemDrag.BeatOnMouseDown = SelectedItemDrag.MouseBeatThisFrame;
											SelectedItemDrag.BeatDistanceMovedSoFar = Beat::Zero();
											context.Undo.DisallowMergeForLastCommand();
										}
										break;
									}
								}
							}
						}
					});
				}

				if (SelectedItemDrag.ActiveTarget != EDragTarget::None || SelectedItemDrag.HoverTarget != EDragTarget::None)
					Gui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

				if (SelectedItemDrag.ActiveTarget != EDragTarget::None)
				{
					// TODO: Maybe should set active ID here too... thought that then conflicts with WindowIsHovered and other actions
					// Gui::SetActiveID(Gui::GetID(&SelectedItemDrag), Gui::GetCurrentWindow());
					SelectedItemDrag.BeatDistanceMovedSoFar += (SelectedItemDrag.MouseBeatThisFrame - SelectedItemDrag.MouseBeatLastFrame);
					context.Undo.ResetMergeTimeThresholdStopwatch();
				}

				if (SelectedItemDrag.ActiveTarget != EDragTarget::None)
				{
					auto itemSelected = [&](GenericList list, size_t i) constexpr { return GetOrEmpty<GenericMember::B8_IsSelected>(selectedCourse, list, i); };
					auto itemStart = [&](GenericList list, size_t i) constexpr { return GetOrEmpty<GenericMember::Beat_Start>(selectedCourse, list, i); };
					auto itemDuration = [&](GenericList list, size_t i) constexpr { return GetOrEmpty<GenericMember::Beat_Duration>(selectedCourse, list, i); };
					auto itemTimeDuration = [&](GenericList list, size_t i) constexpr -> std::tuple<bool, Time> { f32 out {}; return { TryGet<GenericMember::F32_JPOSScrollDuration>(selectedCourse, list, i, out), Time::FromSec(out) }; };
					auto checkCanSelectedItemsBeDragged = [&](GenericList list, Beat beatIncrement, b8 tailOnly) -> b8
					{
						const i32 listCount = static_cast<i32>(GetGenericListCount(selectedCourse, list));
						if (listCount <= 0)
							return true;

						// BUG: It's possible to move two non-empty lyric changes on top of each other, which isn't a critical bug (as it's still handled correctly) 
						//		but definitely annoying and a bit confusing
						// TODO: Rework LyricChange to be Beat+Duration based to avoid the problem all together and also simplify the rest of the code
						const b8 inclusiveBeatCheck = ListUsesInclusiveBeatCheck(list);
						if (beatIncrement > Beat::Zero())
						{
							for (i32 thisIndex = 0; thisIndex < listCount; thisIndex++)
							{
								const i32 nextIndex = thisIndex + 1;
								const b8 hasNext = (nextIndex < listCount);
								if (itemSelected(list, thisIndex) && hasNext
									&& (tailOnly ? (itemDuration(list, thisIndex) > Beat::Zero()) : !itemSelected(list, nextIndex))
									)
								{
									const Beat thisEnd = itemStart(list, thisIndex) + itemDuration(list, thisIndex);
									const Beat nextStart = itemStart(list, nextIndex);
									if (inclusiveBeatCheck)
									{
										if (thisEnd + beatIncrement >= nextStart)
											return false;
									}
									else
									{
										if (thisEnd + beatIncrement > nextStart)
											return false;
									}
								}
							}
						}
						else if (tailOnly)
						{
							for (i32 thisIndex = 0; thisIndex < listCount; thisIndex++)
							{
								if (!itemSelected(list, thisIndex))
									continue;
								if (itemDuration(list, thisIndex) > Beat::Zero()) {
									if (itemDuration(list, thisIndex) + beatIncrement <= Beat::Zero())
										return false;
								}
								else if (auto [hasTimeDuration, timeDuration] = itemTimeDuration(list, thisIndex); hasTimeDuration) {
									const Time startTime = context.BeatToTime(itemStart(list, thisIndex));
									const Time endTime = startTime + timeDuration;
									const Beat endBeatTrunc = context.TimeToBeat(endTime, true);
									const Beat beatDurationTrunc = endBeatTrunc - itemStart(list, thisIndex);
									if (beatDurationTrunc + beatIncrement < Beat::Zero()) {
										return false;
									}
								}
							}
						}
						else
						{
							for (i32 thisIndex = (listCount - 1); thisIndex >= 0; thisIndex--)
							{
								const i32 prevIndex = thisIndex - 1;
								const b8 hasPrev = (prevIndex >= 0);
								if (itemSelected(list, thisIndex))
								{
									const Beat thisStart = itemStart(list, thisIndex);
									if (thisStart + beatIncrement < Beat::Zero())
										return false;

									if (hasPrev && !itemSelected(list, prevIndex))
									{
										const Beat prevEnd = itemStart(list, prevIndex) + itemDuration(list, prevIndex);
										if (inclusiveBeatCheck)
										{
											if (thisStart + beatIncrement <= prevEnd)
												return false;
										}
										else
										{
											if (thisStart + beatIncrement < prevEnd)
												return false;
										}
									}
								}
							}
						}
						return true;
					};

					const Beat cursorBeat = context.GetCursorBeat();
					const Beat dragBeatIncrement = FloorBeatToCurrentGrid(SelectedItemDrag.BeatDistanceMovedSoFar);

					// BUG: Doesn't account for smooth scroll delay and playback auto scrolling
					const b8 wasMouseMovedOrScrolled = (!ApproxmiatelySame(Gui::GetIO().MouseDelta.x, 0.0f) || !ApproxmiatelySame(Gui::GetIO().MouseWheel, 0.0f));

					if (dragBeatIncrement != Beat::Zero() && wasMouseMovedOrScrolled)
					{
						const b8 isTail = (SelectedItemDrag.ActiveTarget == EDragTarget::Tail);
						b8 allItemsCanBeMoved = true;
						for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
							allItemsCanBeMoved &= checkCanSelectedItemsBeDragged(list, dragBeatIncrement, isTail);

						if (allItemsCanBeMoved)
						{
							SelectedItemDrag.BeatDistanceMovedSoFar -= dragBeatIncrement;

							auto& notes = selectedCourse.GetNotes(context.ChartSelectedBranch);
							if (allSelectedItemsAreNotes)
							{
								std::vector<Commands::ChangeMultipleNoteBeats::Data> noteBeatsToChange;
								noteBeatsToChange.reserve(selectedItemCount);
								for (const Note& note : notes)
									if (note.IsSelected && !(isTail && note.BeatDuration <= Beat::Zero())) {
										auto& data = noteBeatsToChange.emplace_back();
										data.Index = ArrayItToIndex(&note, &notes[0]);
										data.NewValue = ((isTail ? note.BeatDuration : note.BeatTime) + dragBeatIncrement);
									}
								if (isTail)
									context.Undo.Execute<Commands::ChangeMultipleNoteBeatDurations_AdjustRollNoteDurations>(&selectedCourse, &notes, std::move(noteBeatsToChange));
								else
									context.Undo.Execute<Commands::ChangeMultipleNoteBeats_MoveNotes>(&selectedCourse, &notes, std::move(noteBeatsToChange));
							}
							else
							{
								std::vector<Commands::ChangeMultipleGenericProperties::Data> itemsToChange;
								itemsToChange.reserve(selectedItemCount);

								ForEachSelectedChartItem(selectedCourse, [&](const ForEachChartItemData& it)
								{
									auto& data = itemsToChange.emplace_back();
									data.Index = it.Index;
									data.List = it.List;
									if (!isTail) {
										data.Member = GenericMember::Beat_Start;
										data.NewValue.Beat = GetBeat(it, selectedCourse) + dragBeatIncrement;
									}
									else if (auto beatDuration = GetBeatDuration(it, selectedCourse); beatDuration > Beat::Zero()) {
										data.Member = GenericMember::Beat_Duration;
										data.NewValue.Beat = beatDuration + dragBeatIncrement;
									}
									else if (auto [hasTimeDuration, timeDuration] = GetTimeDuration(it, selectedCourse); hasTimeDuration) {
										data.Member = GenericMember::F32_JPOSScrollDuration;
										const Beat startBeat = GetBeat(it, selectedCourse);
										const Time startTime = context.BeatToTime(GetBeat(it, selectedCourse));
										const Time endTime = startTime + timeDuration;
										const Beat endBeatTrunc = context.TimeToBeat(endTime, true);
										const Time residualTime = endTime - context.BeatToTime(endBeatTrunc);
										data.NewValue.F32 = (context.BeatToTime(endBeatTrunc + dragBeatIncrement) + residualTime - startTime).Seconds;
									}
									else {
										itemsToChange.pop_back(); // no changes
									}
								});

								if (isTail)
									context.Undo.Execute<Commands::ChangeMultipleGenericProperties_AdjustItemDurations>(&selectedCourse, std::move(itemsToChange));
								else
									context.Undo.Execute<Commands::ChangeMultipleGenericProperties_MoveItems>(&selectedCourse, std::move(itemsToChange));
							}

							for (const Note& note : notes)
								if (note.IsSelected) { if (note.BeatTime == cursorBeat) PlayNoteSoundAndHitAnimationsAtBeat(context, note.BeatTime); }

							// NOTE: Set again to account for a changes in cursor time
							if (atLeastOneSelectedItemIsTempoChange)
								context.SetCursorBeat(cursorBeat);
						}
					}
				}
			}

			if (IsContentWindowHovered && SelectedItemDrag.HoverTarget == EDragTarget::None && Gui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				const Time oldCursorTime = context.GetCursorTime();
				const f32 oldCursorLocalSpaceX = Camera.TimeToLocalSpaceX(oldCursorTime);

				const Time timeAtMouseX = Camera.LocalSpaceXToTime(ScreenToLocalSpace(MousePosThisFrame).x);
				const Beat newCursorBeat = FloorBeatToCurrentGrid(context.TimeToBeat(timeAtMouseX));

				context.SetCursorBeat(newCursorBeat);
				PlayNoteSoundAndHitAnimationsAtBeat(context, newCursorBeat);

				if (context.GetIsPlayback())
				{
					const Time newCursorTime = context.BeatToTime(newCursorBeat);
					const f32 localCursorX = Camera.TimeToLocalSpaceX(newCursorTime);
					const f32 localAutoScrollLockX = Round(Regions.Content.GetWidth() * TimelineAutoScrollLockContentWidthFactor);

					// BUG: *Sometimes* ugly ~1px jutter/lag when jumping forward..? (?)
					// printf("localCursorX: %f\n\n", localCursorX);
					// printf("localAutoScrollLockX: %f\n", localAutoScrollLockX);

					// TODO: Should this be a condition too..?
					// const b8 clickedForward = (newCursorTime >= oldCursorTime);

					const f32 distanceFromAutoScrollPositionX = (localCursorX - localAutoScrollLockX);
					if (/*clickedForward &&*/ distanceFromAutoScrollPositionX > 0.0f)
						Camera.PositionTarget.x += distanceFromAutoScrollPositionX;
				}
			}

			// NOTE: Header bar cursor scrubbing
			{
				if (IsContentHeaderWindowHovered && SelectedItemDrag.HoverTarget == EDragTarget::None && Gui::IsMouseClicked(ImGuiMouseButton_Left))
					IsCursorMouseScrubActive = true;

				if (!Gui::IsMouseDown(ImGuiMouseButton_Left) || !Gui::IsMousePosValid())
					IsCursorMouseScrubActive = false;

				if (IsCursorMouseScrubActive)
				{
					const f32 mouseLocalSpaceX = ScreenToLocalSpace(MousePosThisFrame).x;
					const Time mouseCursorTime = Camera.LocalSpaceXToTime(mouseLocalSpaceX);
					const Beat oldCursorBeat = context.GetCursorBeat();
					const Beat newCursorBeat = context.TimeToBeat(mouseCursorTime);

					const f32 threshold = ClampBot(GuiScale(*Settings.General.TimelineScrubAutoScrollPixelThreshold), 1.0f);
					const f32 speedMin = *Settings.General.TimelineScrubAutoScrollSpeedMin, speedMax = *Settings.General.TimelineScrubAutoScrollSpeedMax;

					const f32 modifier = Gui::GetIO().KeyAlt ? 0.25f : Gui::GetIO().KeyShift ? 2.0f : 1.0f;
					if (const f32 left = threshold; mouseLocalSpaceX < left)
					{
						const f32 scrollIncrementThisFrame = ConvertRange(threshold, 0.0f, speedMin, speedMax, mouseLocalSpaceX) * modifier * Gui::DeltaTime();
						if (*Settings.General.TimelineScrubAutoScrollEnableClamp)
						{
							const f32 minScrollX = TimelineCameraBaseScrollX;
							Camera.PositionCurrent.x = ClampBot(Camera.PositionCurrent.x - scrollIncrementThisFrame, ClampTop(Camera.PositionCurrent.x, minScrollX));
							Camera.PositionTarget.x = ClampBot(Camera.PositionTarget.x - scrollIncrementThisFrame, ClampTop(Camera.PositionTarget.x, minScrollX));
						}
						else
						{
							Camera.PositionCurrent.x -= scrollIncrementThisFrame;
							Camera.PositionTarget.x -= scrollIncrementThisFrame;
						}
					}
					if (const f32 right = (Regions.ContentHeader.GetWidth() - threshold); mouseLocalSpaceX >= right)
					{
						const f32 scrollIncrementThisFrame = ConvertRange(0.0f, threshold, speedMin, speedMax, mouseLocalSpaceX - right) * modifier * Gui::DeltaTime();
						if (*Settings.General.TimelineScrubAutoScrollEnableClamp)
						{
							const f32 maxScrollX = Camera.WorldToLocalSpaceScale(vec2(Camera.TimeToWorldSpaceX(context.Chart.GetDurationOrDefault()), 0.0f)).x - Regions.ContentHeader.GetWidth() + 1.0f;
							Camera.PositionCurrent.x = ClampTop(Camera.PositionCurrent.x + scrollIncrementThisFrame, ClampBot(Camera.PositionCurrent.x, maxScrollX));
							Camera.PositionTarget.x = ClampTop(Camera.PositionTarget.x + scrollIncrementThisFrame, ClampBot(Camera.PositionTarget.x, maxScrollX));
						}
						else
						{
							Camera.PositionCurrent.x += scrollIncrementThisFrame;
							Camera.PositionTarget.x += scrollIncrementThisFrame;
						}
					}

					if (newCursorBeat != oldCursorBeat)
					{
						context.SetCursorBeat(newCursorBeat);
						WorldSpaceCursorXAnimationCurrent = Camera.LocalToWorldSpace(vec2(mouseLocalSpaceX, 0.0f)).x;
						if (MousePosThisFrame.x != MousePosLastFrame.x) PlayNoteSoundAndHitAnimationsAtBeat(context, newCursorBeat);
					}
				}
			}

			if (HasKeyboardFocus())
			{
				if (Gui::GetActiveID() == 0)
				{
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_Cut, false)) ExecuteClipboardAction(context, ClipboardAction::Cut);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_Copy, false)) ExecuteClipboardAction(context, ClipboardAction::Copy);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_Paste, false)) ExecuteClipboardAction(context, ClipboardAction::Paste);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_DeleteSelection, false)) ExecuteClipboardAction(context, ClipboardAction::Delete);

					SelectionActionParam param {};
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectAll, false)) ExecuteSelectionAction(context, SelectionAction::SelectAll, param);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_ClearSelection, false)) ExecuteSelectionAction(context, SelectionAction::UnselectAll, param);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_InvertSelection, false)) ExecuteSelectionAction(context, SelectionAction::InvertAll, param);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectToChartEnd, false)) ExecuteSelectionAction(context, SelectionAction::SelectToEnd, param);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectAllWithinRangeSelection, false)) ExecuteSelectionAction(context, SelectionAction::SelectAllWithinRangeSelection, param);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_ShiftSelectionLeft, true)) ExecuteSelectionAction(context, SelectionAction::PerRowShiftSelected, param.SetShiftDelta(-1));
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_ShiftSelectionRight, true)) ExecuteSelectionAction(context, SelectionAction::PerRowShiftSelected, param.SetShiftDelta(+1));
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectItemPattern_xo, false)) ExecuteSelectionAction(context, SelectionAction::PerRowSelectPattern, param.SetPattern("xo"));
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectItemPattern_xoo, false)) ExecuteSelectionAction(context, SelectionAction::PerRowSelectPattern, param.SetPattern("xoo"));
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectItemPattern_xooo, false)) ExecuteSelectionAction(context, SelectionAction::PerRowSelectPattern, param.SetPattern("xooo"));
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_SelectItemPattern_xxoo, false)) ExecuteSelectionAction(context, SelectionAction::PerRowSelectPattern, param.SetPattern("xxoo"));

					const MultiInputBinding* customBindings[] =
					{
						&*Settings.Input.Timeline_SelectItemPattern_CustomA, &*Settings.Input.Timeline_SelectItemPattern_CustomB, &*Settings.Input.Timeline_SelectItemPattern_CustomC,
						&*Settings.Input.Timeline_SelectItemPattern_CustomD, &*Settings.Input.Timeline_SelectItemPattern_CustomE, &*Settings.Input.Timeline_SelectItemPattern_CustomF,
					};

					for (size_t i = 0; i < ArrayCount(customBindings); i++)
					{
						if (i < Settings.General.CustomSelectionPatterns->V.size() && Gui::IsAnyPressed(*customBindings[i], false))
							ExecuteSelectionAction(context, SelectionAction::PerRowSelectPattern, param.SetPattern(Settings.General.CustomSelectionPatterns->V[i].Data));
					}

					if (Gui::IsAnyPressed(*Settings.Input.Timeline_ConvertSelectionToScrollChanges, false)) ExecuteConvertSelectionToEvents<GenericList::ScrollChanges>(context);
				}

				if (const auto& io = Gui::GetIO(); !io.KeyCtrl)
				{
					auto stepCursorByBeat = [&](Beat beatIncrement)
					{
						const auto oldCursorBeatAndTime = context.GetCursorBeatAndTime();
						const Beat oldCursorBeat = RoundBeatToCurrentGrid(oldCursorBeatAndTime.Beat);
						const Beat newCursorBeat = oldCursorBeat + beatIncrement;

						context.SetCursorBeat(newCursorBeat);
						PlayNoteSoundAndHitAnimationsAtBeat(context, newCursorBeat);

						// BUG: It's still possible to move the cursor off screen without it auto-scrolling if the cursor is on the far right edge of the screen
						const f32 cursorLocalSpaceXOld = Camera.TimeToLocalSpaceX_AtTarget(oldCursorBeatAndTime.Time);
						if (cursorLocalSpaceXOld >= 0.0f && cursorLocalSpaceXOld <= Regions.Content.GetWidth())
						{
							const f32 cursorLocalSpaceX = Camera.TimeToLocalSpaceX(context.BeatToTime(newCursorBeat));
							Camera.PositionTarget.x += (cursorLocalSpaceX - Camera.TimeToLocalSpaceX(oldCursorBeatAndTime.Time));
							WorldSpaceCursorXAnimationCurrent = Camera.LocalToWorldSpace(vec2(cursorLocalSpaceX, 0.0f)).x;
						}
					};

					// TODO: Maybe refine further...
					const Beat cursorStepKeyDistance = context.GetIsPlayback() ?
						(io.KeyAlt ? GetGridBeatSnap(CurrentGridBarDivision) : io.KeyShift ? Beat::FromBeats(2) : Beat::FromBeats(1)) :
						(io.KeyShift ? Beat::FromBeats(1) : GetGridBeatSnap(CurrentGridBarDivision));

					// NOTE: Separate down checks for right priority, although ideally the *last held* binding should probably be prioritized instead (which would be a bit tricky)
					if (Gui::IsAnyDown(*Settings.Input.Timeline_StepCursorRight, InputModifierBehavior::Relaxed))
					{
						if (Gui::IsAnyPressed(*Settings.Input.Timeline_StepCursorRight, true, InputModifierBehavior::Relaxed)) { stepCursorByBeat(cursorStepKeyDistance * +1); }
					}
					else if (Gui::IsAnyDown(*Settings.Input.Timeline_StepCursorLeft, InputModifierBehavior::Relaxed))
					{
						if (Gui::IsAnyPressed(*Settings.Input.Timeline_StepCursorLeft, true, InputModifierBehavior::Relaxed)) { stepCursorByBeat(cursorStepKeyDistance * -1); }
					}
				}

				if (Gui::IsAnyPressed(*Settings.Input.Timeline_JumpToTimelineStart, false)) ScrollToTimelinePosition(Camera, Regions, context.Chart, 0.0f);
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_JumpToTimelineEnd, false)) ScrollToTimelinePosition(Camera, Regions, context.Chart, 1.0f);
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_StartEndRangeSelection, false)) StartEndRangeSelectionAtCursor(context);
			}

			// NOTE: Grid snap controls
			if (IsContentWindowHovered || HasKeyboardFocus())
			{
				i32 currentGridDivisionIndex = 0;
				for (const i32& it : AllowedGridBarDivisions) if (it == CurrentGridBarDivision) currentGridDivisionIndex = ArrayItToIndexI32(&it, &AllowedGridBarDivisions[0]);

				const b8 keyboardFocus = HasKeyboardFocus() && !Gui::GetIO().KeyCtrl;
				const b8 increaseGrid = (IsContentWindowHovered && Gui::IsMouseClicked(ImGuiMouseButton_X2, true)) || (keyboardFocus && Gui::IsAnyPressed(*Settings.Input.Timeline_IncreaseGridDivision, true, InputModifierBehavior::Relaxed));
				const b8 decreaseGrid = (IsContentWindowHovered && Gui::IsMouseClicked(ImGuiMouseButton_X1, true)) || (keyboardFocus && Gui::IsAnyPressed(*Settings.Input.Timeline_DecreaseGridDivision, true, InputModifierBehavior::Relaxed));
				if (increaseGrid) CurrentGridBarDivision = AllowedGridBarDivisions[Clamp(currentGridDivisionIndex + 1, 0, ArrayCountI32(AllowedGridBarDivisions) - 1)];
				if (decreaseGrid) CurrentGridBarDivision = AllowedGridBarDivisions[Clamp(currentGridDivisionIndex - 1, 0, ArrayCountI32(AllowedGridBarDivisions) - 1)];
			}
			if (HasKeyboardFocus())
			{
				const struct { const WithDefault<MultiInputBinding>& V; i32 BarDivision; } allBindings[] =
				{
					{ Settings.Input.Timeline_SetGridDivision_1_4, 4 },
					{ Settings.Input.Timeline_SetGridDivision_1_5, 5 },
					{ Settings.Input.Timeline_SetGridDivision_1_6, 6 },
					{ Settings.Input.Timeline_SetGridDivision_1_7, 7 },
					{ Settings.Input.Timeline_SetGridDivision_1_8, 8 },
					{ Settings.Input.Timeline_SetGridDivision_1_9, 9 },
					{ Settings.Input.Timeline_SetGridDivision_1_10, 10 },
					{ Settings.Input.Timeline_SetGridDivision_1_12, 12 },
					{ Settings.Input.Timeline_SetGridDivision_1_14, 14 },
					{ Settings.Input.Timeline_SetGridDivision_1_16, 16 },
					{ Settings.Input.Timeline_SetGridDivision_1_18, 18 },
					{ Settings.Input.Timeline_SetGridDivision_1_20, 20 },
					{ Settings.Input.Timeline_SetGridDivision_1_24, 24 },
					{ Settings.Input.Timeline_SetGridDivision_1_28, 28 },
					{ Settings.Input.Timeline_SetGridDivision_1_32, 32 },
					{ Settings.Input.Timeline_SetGridDivision_1_36, 36 },
					{ Settings.Input.Timeline_SetGridDivision_1_40, 40 },
					{ Settings.Input.Timeline_SetGridDivision_1_48, 48 },
					{ Settings.Input.Timeline_SetGridDivision_1_64, 64 },
					{ Settings.Input.Timeline_SetGridDivision_1_96, 96 },
					{ Settings.Input.Timeline_SetGridDivision_1_192, 192 },
				};

				for (const auto& binding : allBindings)
				{
					if (Gui::IsAnyPressed(*binding.V, false))
						CurrentGridBarDivision = binding.BarDivision;
				}
			}

			// NOTE: Playback speed controls
			if (HasKeyboardFocus())
			{
				if (const auto& io = Gui::GetIO(); !io.KeyCtrl)
				{
					const std::vector<f32>& speeds = io.KeyAlt ? Settings.General.PlaybackSpeedStepsPrecise->V : io.KeyShift ? Settings.General.PlaybackSpeedStepsRough->V : Settings.General.PlaybackSpeedSteps->V;

					i32 closetPlaybackSpeedIndex = 0;
					const f32 currentPlaybackSpeed = context.GetPlaybackSpeed();
					for (const f32& it : speeds) if (it >= currentPlaybackSpeed) closetPlaybackSpeedIndex = ArrayItToIndexI32(&it, &speeds[0]);

					if (Gui::IsAnyPressed(*Settings.Input.Timeline_IncreasePlaybackSpeed, true, InputModifierBehavior::Relaxed)) context.SetPlaybackSpeed(speeds[Clamp(closetPlaybackSpeedIndex - 1, 0, speeds.empty() ? 0 : static_cast<i32>(speeds.size() - 1))]);
					if (Gui::IsAnyPressed(*Settings.Input.Timeline_DecreasePlaybackSpeed, true, InputModifierBehavior::Relaxed)) context.SetPlaybackSpeed(speeds[Clamp(closetPlaybackSpeedIndex + 1, 0, speeds.empty() ? 0 : static_cast<i32>(speeds.size() - 1))]);
				}

				if (Gui::IsAnyPressed(*Settings.Input.Timeline_SetPlaybackSpeed_100, false)) context.SetPlaybackSpeed(FromPercent(100.0f));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_SetPlaybackSpeed_75, false)) context.SetPlaybackSpeed(FromPercent(75.0f));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_SetPlaybackSpeed_50, false)) context.SetPlaybackSpeed(FromPercent(50.0f));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_SetPlaybackSpeed_25, false)) context.SetPlaybackSpeed(FromPercent(25.0f));
			}

			if (HasKeyboardFocus() && Gui::IsAnyPressed(*Settings.Input.Timeline_TogglePlayback, false, InputModifierBehavior::Relaxed))
			{
				if (context.GetIsPlayback())
				{
					context.SetIsPlayback(false);
					WorldSpaceCursorXAnimationCurrent = Camera.TimeToWorldSpaceX(context.GetCursorTime());
				}
				else
				{
					Audio::Engine.EnsureStreamRunning();
					const Beat cursorBeat = context.GetCursorBeat();
					PlayNoteSoundAndHitAnimationsAtBeat(context, cursorBeat);

					context.SetIsPlayback(true);
				}
			}

			auto updateNotePlacementBinding = [this, &context](const MultiInputBinding& inputBinding, NoteType noteTypeToInsert)
			{
				if (Gui::IsAnyPressed(inputBinding, false, InputModifierBehavior::Relaxed))
				{
					ChartCourse& course = *context.ChartSelectedCourse;
					SortedNotesList& notes = course.GetNotes(context.ChartSelectedBranch);

					if (Gui::GetIO().KeyShift && RangeSelection.IsActiveAndHasEnd())
					{
						const Beat startTick = RoundBeatToCurrentGrid(RangeSelection.GetMin());
						const Beat endTick = RoundBeatToCurrentGrid(RangeSelection.GetMax());
						const Beat beatPerNote = GetGridBeatSnap(CurrentGridBarDivision);
						const i32 maxExpectedNoteCountToAdd = ((endTick - startTick).Ticks / beatPerNote.Ticks) + 1;

						std::vector<Note> newNotesToAdd;
						newNotesToAdd.reserve(maxExpectedNoteCountToAdd);

						for (i32 i = 0; i < maxExpectedNoteCountToAdd; i++)
						{
							const Beat beatForThisNote = Beat(Min(startTick, endTick).Ticks + (i * beatPerNote.Ticks));
							if (notes.TryFindOverlappingBeat(beatForThisNote, beatForThisNote) == nullptr)
							{
								Note& newNote = newNotesToAdd.emplace_back();
								newNote.BeatTime = beatForThisNote;
								newNote.Type = noteTypeToInsert;
							}
						}

						if (!newNotesToAdd.empty())
						{
							SetNotesWaveAnimationTimes(newNotesToAdd, (RangeSelection.Start < RangeSelection.End) ? +1 : -1);
							PlaySoundEffectTypeForNoteType(context, newNotesToAdd.front().Type);
							context.Undo.Execute<Commands::AddMultipleNotes>(&course, &notes, std::move(newNotesToAdd));
						}
					}
					else
					{
						const b8 isPlayback = context.GetIsPlayback();
						const Beat cursorBeat = isPlayback ? RoundBeatToCurrentGrid(context.GetCursorBeat()) : FloorBeatToCurrentGrid(context.GetCursorBeat());

						Note* existingNoteAtCursor = notes.TryFindOverlappingBeat(cursorBeat, cursorBeat);
						if (existingNoteAtCursor != nullptr)
						{
							if (existingNoteAtCursor->BeatTime == cursorBeat)
							{
								existingNoteAtCursor->ClickAnimationTimeRemaining = existingNoteAtCursor->ClickAnimationTimeDuration = NoteHitAnimationDuration;
								if (!isPlayback)
								{
									if (ToSmallNote(existingNoteAtCursor->Type) == ToSmallNote(noteTypeToInsert) || (existingNoteAtCursor->BeatDuration > Beat::Zero()))
									{
										TempDeletedNoteAnimationsBuffer.push_back(DeletedNoteAnimation { *existingNoteAtCursor, context.ChartSelectedBranch, 0.0f });
										context.Undo.Execute<Commands::RemoveSingleNote>(&course, &notes, *existingNoteAtCursor);
									}
									else
									{
										context.Undo.Execute<Commands::ChangeSingleNoteType>(&course, &notes, Commands::ChangeSingleNoteType::Data { ArrayItToIndex(existingNoteAtCursor, &notes[0]), noteTypeToInsert });
										context.Undo.DisallowMergeForLastCommand();
									}
								}
							}
							PlaySoundEffectTypeForNoteType(context, existingNoteAtCursor->Type);
						}
						else if (cursorBeat >= Beat::Zero())
						{
							Note newNote {};
							newNote.BeatTime = cursorBeat;
							newNote.Type = noteTypeToInsert;
							newNote.ClickAnimationTimeRemaining = newNote.ClickAnimationTimeDuration = NoteHitAnimationDuration;
							context.Undo.Execute<Commands::AddSingleNote>(&course, &notes, newNote);
							PlaySoundEffectTypeForNoteType(context, noteTypeToInsert);
						}
					}
				}
			};

			PlaceBalloonBindingDownLastFrame = PlaceBalloonBindingDownThisFrame;
			PlaceBalloonBindingDownThisFrame = HasKeyboardFocus() && Gui::IsAnyDown(*Settings.Input.Timeline_PlaceNoteBalloon, InputModifierBehavior::Relaxed);
			PlaceDrumrollBindingDownLastFrame = PlaceDrumrollBindingDownThisFrame;
			PlaceDrumrollBindingDownThisFrame = HasKeyboardFocus() && Gui::IsAnyDown(*Settings.Input.Timeline_PlaceNoteDrumroll, InputModifierBehavior::Relaxed);
			if (HasKeyboardFocus())
			{
				updateNotePlacementBinding(*Settings.Input.Timeline_PlaceNoteDon, ToBigNoteIf(NoteType::Don, Gui::GetIO().KeyAlt));
				updateNotePlacementBinding(*Settings.Input.Timeline_PlaceNoteKa, ToBigNoteIf(NoteType::Ka, Gui::GetIO().KeyAlt));

				if (PlaceBalloonBindingDownThisFrame || PlaceDrumrollBindingDownThisFrame)
				{
					const Beat cursorBeat = context.GetIsPlayback() ? RoundBeatToCurrentGrid(context.GetCursorBeat()) : FloorBeatToCurrentGrid(context.GetCursorBeat());
					LongNotePlacement.NoteType = ToBigNoteIf(PlaceBalloonBindingDownThisFrame ? NoteType::Balloon : NoteType::Drumroll, Gui::GetIO().KeyAlt);
					if (!LongNotePlacement.IsActive)
					{
						LongNotePlacement.IsActive = true;
						LongNotePlacement.CursorBeatHead = cursorBeat;
						PlaySoundEffectTypeForNoteType(context, LongNotePlacement.NoteType);
					}
					LongNotePlacement.CursorBeatTail = cursorBeat;
				}
			}
			else
			{
				LongNotePlacement = {};
			}

			auto placeLongNoteOnBindingRelease = [this, &context](NoteType longNoteType)
			{
				ChartCourse& course = *context.ChartSelectedCourse;
				SortedNotesList& notes = course.GetNotes(context.ChartSelectedBranch);
				const Beat minBeatAfter = LongNotePlacement.GetMin();
				const Beat maxBeat = LongNotePlacement.GetMax();

				std::vector<Note> notesToRemove;
				for (const Note& existingNote : notes)
				{
					if (existingNote.GetStart() <= maxBeat && minBeatAfter <= existingNote.GetEnd())
						notesToRemove.push_back(existingNote);
				}

				Note newLongNote {};
				newLongNote.BeatTime = minBeatAfter;
				newLongNote.BeatDuration = (maxBeat - minBeatAfter);
				newLongNote.BalloonPopCount = IsBalloonNote(longNoteType) ? DefaultBalloonPopCount(newLongNote.BeatDuration, CurrentGridBarDivision) : 0;
				newLongNote.Type = longNoteType;
				newLongNote.ClickAnimationTimeRemaining = newLongNote.ClickAnimationTimeDuration = NoteHitAnimationDuration;
				context.Undo.Execute<Commands::AddSingleLongNote>(&course, &notes, newLongNote, std::move(notesToRemove));

				PlaySoundEffectTypeForNoteType(context, longNoteType);
			};

			const b8 activeFocusedAndHasLength = HasKeyboardFocus() && LongNotePlacement.IsActive && (LongNotePlacement.CursorBeatHead != LongNotePlacement.CursorBeatTail);
			if (PlaceBalloonBindingDownLastFrame && !PlaceBalloonBindingDownThisFrame) { if (activeFocusedAndHasLength) placeLongNoteOnBindingRelease(LongNotePlacement.NoteType); LongNotePlacement = {}; }
			if (PlaceDrumrollBindingDownLastFrame && !PlaceDrumrollBindingDownThisFrame) { if (activeFocusedAndHasLength) placeLongNoteOnBindingRelease(LongNotePlacement.NoteType); LongNotePlacement = {}; }

			if (HasKeyboardFocus())
			{
				TransformActionParam param {};
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_FlipNoteType, false)) ExecuteTransformAction(context, TransformAction::FlipNoteType, param);
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_ToggleNoteSize, false)) ExecuteTransformAction(context, TransformAction::ToggleNoteSize, param);

				// NOTE: tentatively use the same set of keybinds for item and range scale
				TransformAction scaleAction = RangeSelection.IsActiveAndHasEnd() ? TransformAction::ScaleRangeTime : TransformAction::ScaleItemTime;;
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_ExpandItemTime_2To1, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(2, 1));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_ExpandItemTime_3To2, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(3, 2));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_ExpandItemTime_4To3, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(4, 3));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_CompressItemTime_1To2, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(1, 2));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_CompressItemTime_2To3, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(2, 3));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_CompressItemTime_3To4, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(3, 4));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_CompressItemTime_0To1, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(0, 1));
				if (Gui::IsAnyPressed(*Settings.Input.Timeline_ReverseItemTime_N1To1, false)) ExecuteTransformAction(context, scaleAction, param.SetTimeRatio(-1, 1));

				const MultiInputBinding* customBindings[] =
				{
					&*Settings.Input.Timeline_ScaleItemTime_CustomA, &*Settings.Input.Timeline_ScaleItemTime_CustomB, &*Settings.Input.Timeline_ScaleItemTime_CustomC,
					&*Settings.Input.Timeline_ScaleItemTime_CustomD, &*Settings.Input.Timeline_ScaleItemTime_CustomE, &*Settings.Input.Timeline_ScaleItemTime_CustomF,
				};

				for (size_t i = 0; i < ArrayCount(customBindings); i++)
				{
					if (i < Settings.General.CustomScaleRatios->size() && Gui::IsAnyPressed(*customBindings[i], false))
						ExecuteTransformAction(context, scaleAction, param.SetTimeRatio((*Settings.General.CustomScaleRatios)[i].TimeRatio));
				}
			}
		}

		if (HasKeyboardFocus() && Gui::IsAnyPressed(*Settings.Input.Timeline_ToggleMetronome))
		{
			Metronome.IsEnabled ^= true;
			if (!context.GetIsPlayback())
				context.SfxVoicePool.PlaySound(SoundEffectType::MetronomeBeat);
		}

		// NOTE: Playback preview sounds / metronome
		if (context.GetIsPlayback() && (PlaybackSoundsEnabled || Metronome.IsEnabled))
			UpdateTimelinePlaybackAndMetronomneSounds(context, PlaybackSoundsEnabled, Metronome);

		// NOTE: Mouse selection box
		{
			static constexpr auto getBoxSelectionAction = [](const ImGuiIO& io)
			{
				const b8 shift = io.KeyShift, alt = io.KeyAlt;
				return (shift && alt) ? BoxSelectionAction::XOR : shift ? BoxSelectionAction::Add : alt ? BoxSelectionAction::Sub : BoxSelectionAction::Clear;
			};

			if (IsContentWindowHovered && Gui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				BoxSelection.IsActive = true;
				BoxSelection.Action = getBoxSelectionAction(Gui::GetIO());
				BoxSelection.WorldSpaceRect.TL = BoxSelection.WorldSpaceRect.BR = Camera.LocalToWorldSpace(ScreenToLocalSpace(Gui::GetMousePos()));
			}
			else if (BoxSelection.IsActive && Gui::IsMouseDown(ImGuiMouseButton_Right))
			{
				BoxSelection.WorldSpaceRect.BR = Camera.LocalToWorldSpace(ScreenToLocalSpace(Gui::GetMousePos()));
				BoxSelection.Action = getBoxSelectionAction(Gui::GetIO());
				RangeSelection = {};
			}
			else
			{
				if (BoxSelection.IsActive)
					BoxSelection.Action = getBoxSelectionAction(Gui::GetIO());

				if (BoxSelection.IsActive && Gui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					static constexpr f32 minBoxSizeThreshold = 4.0f;
					const Rect screenSpaceRect = Rect(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.TL), Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.BR));
					if (Absolute(screenSpaceRect.GetWidth()) >= minBoxSizeThreshold && Absolute(screenSpaceRect.GetHeight()) >= minBoxSizeThreshold)
					{
						const vec2 screenSelectionMin = LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.GetMin()));
						const vec2 screenSelectionMax = LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.GetMax()));
						const Time selectionTimeMin = Camera.WorldSpaceXToTime(BoxSelection.WorldSpaceRect.GetMin().x);
						const Time selectionTimeMax = Camera.WorldSpaceXToTime(BoxSelection.WorldSpaceRect.GetMax().x);
						const Beat selectionBeatMin = context.TimeToBeat(selectionTimeMin);
						const Beat selectionBeatMax = context.TimeToBeat(selectionTimeMax);

						ForEachTimelineRow(*this, [&](const ForEachRowData& rowIt)
						{
							const GenericList list = GetEffectiveGenericList(rowIt.RowType, context.ChartSelectedBranch);
							const b8 isNotesRow = IsNotesList(list);

							enum class XIntersectionTest : u8 { Tips, Full };
							enum class YIntersectionTest : u8 { Center, FullRow };
							const XIntersectionTest xIntersectionTest = IsNotesList(list) ? XIntersectionTest::Tips : XIntersectionTest::Full;
							const YIntersectionTest yIntersectionTest = (list == GenericList::GoGoRanges || list == GenericList::Lyrics || list == GenericList::JPOSScroll || list == GenericList::JPOSScroll_Expert || list == GenericList::JPOSScroll_Master || list == GenericList::BranchSections) ?
								YIntersectionTest::FullRow
								: YIntersectionTest::Center;

							const Rect screenRowRect = Rect(LocalToScreenSpace(vec2(0.0f, rowIt.LocalY)), LocalToScreenSpace(vec2(Regions.Content.GetWidth(), rowIt.LocalY + rowIt.LocalHeight)));
							const f32 screenMinY = (yIntersectionTest == YIntersectionTest::Center) ? screenRowRect.GetCenter().y : screenRowRect.TL.y;
							const f32 screenMaxY = (yIntersectionTest == YIntersectionTest::Center) ? screenRowRect.GetCenter().y : screenRowRect.BR.y;

							for (size_t i = 0; i < GetGenericListCount(*context.ChartSelectedCourse, list); i++)
							{
								Beat beatStart {}, beatDuration {};
								f32 timeDuration {};
								bool isSelected {};
								const b8 hasBeatStart = TryGet<GenericMember::Beat_Start>(*context.ChartSelectedCourse, list, i, beatStart);
								const b8 hasBeatDuration = TryGet<GenericMember::Beat_Duration>(*context.ChartSelectedCourse, list, i, beatDuration);
								const b8 hasTimeDuration = TryGet<GenericMember::F32_JPOSScrollDuration>(*context.ChartSelectedCourse, list, i, timeDuration);
								const b8 hasIsSelected = TryGet<GenericMember::B8_IsSelected>(*context.ChartSelectedCourse, list, i, isSelected);
								assert(hasBeatStart && hasIsSelected);

								// Note: Ignore negative-length body
								const Beat beatMin = beatStart;
								const Beat beatMax = hasBeatDuration ? (beatStart + ClampBot(beatDuration, Beat::Zero())) : beatStart;
								b8 isXInsideSelectionBox;
								if (hasTimeDuration && timeDuration > 0) {
									const Time timeMax = context.BeatToTime(beatMin) + Time::FromSec(timeDuration);
									isXInsideSelectionBox = (((beatMin <= selectionBeatMax) && (timeMax >= selectionTimeMin))
										&& (!(xIntersectionTest == XIntersectionTest::Tips) || (beatMin >= selectionBeatMin) || (timeMax <= selectionTimeMax)));
								}
								else {
									isXInsideSelectionBox = (((beatMin <= selectionBeatMax) && (beatMax >= selectionBeatMin))
										&& (!(xIntersectionTest == XIntersectionTest::Tips) || (beatMin >= selectionBeatMin) || (beatMax <= selectionBeatMax)));
								}
								const b8 isInsideSelectionBox = isXInsideSelectionBox && (screenMinY <= screenSelectionMax.y) && (screenMaxY >= screenSelectionMin.y);

								switch (BoxSelection.Action)
								{
								case BoxSelectionAction::Clear: { isSelected = isInsideSelectionBox; } break;
								case BoxSelectionAction::Add: { if (isInsideSelectionBox) isSelected = true; } break;
								case BoxSelectionAction::Sub: { if (isInsideSelectionBox) isSelected = false; } break;
								case BoxSelectionAction::XOR: { isSelected ^= isInsideSelectionBox; } break;
								}

								TrySet<GenericMember::B8_IsSelected>(*context.ChartSelectedCourse, list, i, isSelected);
							}
						});
					}
				}

				BoxSelection.IsActive = false;
				BoxSelection.WorldSpaceRect.TL = BoxSelection.WorldSpaceRect.BR = vec2(0.0f);
			}

			// TODO: Animate notes when including them in selection box (?)
			if (BoxSelection.IsActive)
			{
				// BUG: Doesn't work perfectly (mostly with the scrollbar) but need to prevent other window widgets from being interactable
				Gui::SetActiveID(Gui::GetID(&BoxSelection), Gui::GetCurrentWindow());

				// NOTE: Auto scroll timeline if selection offscreen and mouse moved (similar to how it works in reaper)
				if ((MousePosThisFrame.x < Regions.Content.TL.x) || (MousePosThisFrame.x > Regions.Content.BR.x))
				{
					const vec2 mouseDelta = (MousePosLastFrame - MousePosThisFrame);
					if (!ApproxmiatelySame(mouseDelta.x, 0.0f) || !ApproxmiatelySame(mouseDelta.y, 0.0f))
					{
						// TODO: Make global setting and adjust default value
						static constexpr f32 scrollSpeed = 3.0f; // 4.0f;

						// NOTE: Using the max axis feels more natural than the vector length
						const f32 mouseDistanceMoved = Max(Absolute(mouseDelta.x), Absolute(mouseDelta.y));
						const f32 scrollDirection = (MousePosThisFrame.x < Regions.Content.TL.x) ? -1.0f : +1.0f;
						Camera.PositionTarget.x = ClampBot(Min(TimelineCameraBaseScrollX, Camera.PositionTarget.x), Camera.PositionTarget.x + (scrollDirection * mouseDistanceMoved * scrollSpeed));
					}
				}
			}
		}
	}

	void ChartTimeline::UpdateAllAnimationsAfterUserInput(ChartContext& context)
	{
		// BUG: Freaks out when zoomed in too far / the timeline is too long (16min song for example) float precision issue (?)
		Camera.UpdateAnimations();
		Gui::AnimateExponential(&context.SongWaveformFadeAnimationCurrent, context.SongWaveformFadeAnimationTarget, *Settings.Animation.TimelineWaveformFadeSpeed);
		Gui::AnimateExponential(&RangeSelectionExpansionAnimationCurrent, RangeSelectionExpansionAnimationTarget, *Settings.Animation.TimelineRangeSelectionExpansionSpeed);

		const f32 worldSpaceCursorXAnimationTarget = Camera.TimeToWorldSpaceX(context.GetCursorTime());
		Gui::AnimateExponential(&WorldSpaceCursorXAnimationCurrent, worldSpaceCursorXAnimationTarget, *Settings.Animation.TimelineWorldSpaceCursorXSpeed);

		for (auto& course : context.Chart.Courses)
		{
			const f32 elapsedAnimationTimeSec = Gui::DeltaTime();
			for (BranchType branch = {}; branch < BranchType::Count; IncrementEnum(branch))
			{
				for (Note& note : course->GetNotes(branch))
					note.ClickAnimationTimeRemaining = ClampBot(note.ClickAnimationTimeRemaining - elapsedAnimationTimeSec, 0.0f);
			}

			for (auto& gogo : course->GoGoRanges)
				Gui::AnimateExponential(&gogo.ExpansionAnimationCurrent, gogo.ExpansionAnimationTarget, *Settings.Animation.TimelineGoGoRangeExpansionSpeed);
		}

		if (!TempDeletedNoteAnimationsBuffer.empty())
		{
			for (auto& data : TempDeletedNoteAnimationsBuffer)
				data.ElapsedTimeSec += Gui::DeltaTime();
			erase_remove_if(TempDeletedNoteAnimationsBuffer, [](auto& v) { return (v.ElapsedTimeSec >= NoteDeleteAnimationDuration); });
		}

		static constexpr f32 maxZoomLevelAtWhichToFadeOutGridBeatSnapLines = 0.5f;
		const f32 gridBeatSnapLineAnimationTarget = (Camera.ZoomCurrent.x <= maxZoomLevelAtWhichToFadeOutGridBeatSnapLines) ? 0.0f : 1.0f;
		Gui::AnimateExponential(&GridSnapLineAnimationCurrent, gridBeatSnapLineAnimationTarget, *Settings.Animation.TimelineGridSnapLineSpeed);
		GridSnapLineAnimationCurrent = Clamp(GridSnapLineAnimationCurrent, 0.0f, 1.0f);
	}

	void ChartTimeline::DrawAllAtEndOfFrame(ChartContext& context)
	{
		// Cache per-frame flags used by ForEachTimelineRow and branch overlays
		SelectedCourseHasBranches = (context.ChartSelectedCourse != nullptr && context.ChartSelectedCourse->HasBranches());

		if (!context.Gfx.IsAsyncLoading())
			context.Gfx.Rasterize(SprGroup::Timeline, GuiScaleFactorTarget);

		const b8 isPlayback = context.GetIsPlayback();
		const BeatAndTime cursorBeatAndTime = context.GetCursorBeatAndTime();
		const Time cursorTime = cursorBeatAndTime.Time;
		const Beat cursorBeat = cursorBeatAndTime.Beat;
		const Beat cursorBeatOnPlaybackStart = context.TimeToBeat(context.CursorTimeOnPlaybackStart);

		// HACK: Drawing outside the BeginChild() / EndChild() means the clipping rect has already been popped...
		DrawListSidebarHeader->PushClipRect(Regions.SidebarHeader.TL, Regions.SidebarHeader.BR);
		DrawListSidebar->PushClipRect(Regions.Sidebar.TL, Regions.Sidebar.BR);
		DrawListContentHeader->PushClipRect(Regions.ContentHeader.TL, Regions.ContentHeader.BR);
		DrawListContent->PushClipRect(Regions.Content.TL, Regions.Content.BR);
		defer { DrawListContent->PopClipRect(); DrawListContentHeader->PopClipRect(); DrawListSidebar->PopClipRect(); DrawListSidebarHeader->PopClipRect(); };

		context.ElapsedProgramTimeSincePlaybackStarted = isPlayback ? context.ElapsedProgramTimeSincePlaybackStarted + Time::FromSec(Gui::DeltaTime()) : Time::Zero();
		context.ElapsedProgramTimeSincePlaybackStopped = !isPlayback ? context.ElapsedProgramTimeSincePlaybackStopped + Time::FromSec(Gui::DeltaTime()) : Time::Zero();
		const f32 animatedCursorLocalSpaceX = Camera.WorldToLocalSpace(vec2(WorldSpaceCursorXAnimationCurrent, 0.0f)).x;
		const f32 currentCursorLocalSpaceX = Camera.TimeToLocalSpaceX(cursorTime);

		const f32 cursorLocalSpaceX = isPlayback ? currentCursorLocalSpaceX : animatedCursorLocalSpaceX;
		const f32 cursorHeaderTriangleLocalSpaceX = cursorLocalSpaceX + 0.5f;

		// NOTE: Base background
		{
			static constexpr f32 borders = 1.0f;
			DrawListContent->AddRectFilled(Regions.Content.TL + vec2(borders), Regions.Content.BR - vec2(borders), TimelineBackgroundColor);
		}

		// NOTE: Demo start time marker
		{
			if (context.Chart.SongDemoStartTime > Time::Zero())
			{
				const Time songDemoStartTimeChartSpace = context.Chart.SongDemoStartTime + context.Chart.SongOffset;
				const f32 songDemoStartTimeLocalSpaceX = Camera.TimeToLocalSpaceX(songDemoStartTimeChartSpace);

				const vec2 headerScreenSpaceTL = LocalToScreenSpace_ContentHeader(vec2(songDemoStartTimeLocalSpaceX, 0.0f));
				const vec2 triangle[3]
				{
					headerScreenSpaceTL + vec2(0.0f, Regions.ContentHeader.GetHeight() - GuiScale(TimelineSongDemoStartMarkerHeight)),
					headerScreenSpaceTL + vec2(GuiScale(TimelineSongDemoStartMarkerWidth), Regions.ContentHeader.GetHeight()),
					headerScreenSpaceTL + vec2(0.0f, Regions.ContentHeader.GetHeight()),
				};

				DrawListContentHeader->AddTriangleFilled(triangle[0], triangle[1], triangle[2], TimelineSongDemoStartMarkerColorFill);
				DrawListContentHeader->AddTriangle(triangle[0], triangle[1], triangle[2], TimelineSongDemoStartMarkerColorBorder);
			}
		}


		// NOTE: Tempo map bar/beat lines and bar-index/time text
		{
			const f32 screenSpaceTimeTextWidth = Gui::CalcTextSize(Time::Zero().ToString().Data).x;
			vec2 lastDrawnScreenSpaceTextTL = vec2(F32Min);

			const vec2 screenSpaceTextOffsetBarIndex = GuiScale(vec2(4.0f, 1.0f));
			const vec2 screenSpaceTextOffsetBarTime = GuiScale(vec2(4.0f, 13.0f));

			const b8 displayTimeInSongSpace = (*Settings.General.DisplayTimeInSongSpace && Absolute(context.Chart.SongOffset.ToMS()) > 0.5);
			const Time timeLabelDisplayOffset = displayTimeInSongSpace ? -context.Chart.SongOffset : Time::Zero();

			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
			const Time visibleTimeOverdraw = Camera.TimePerScreenPixel() * (Gui::CalcTextSize("00:00.000").x + Gui::GetFrameHeight());
			ForEachTimelineVisibleGridLine(*this, context, visibleTimeOverdraw, [&](const ForEachGridLineData& gridIt)
			{
				const u32 lineColor = gridIt.IsBar ? TimelineGridBarLineColor : TimelineGridBeatLineColor;

				const vec2 screenSpaceTL = LocalToScreenSpace(vec2(Camera.TimeToLocalSpaceX(gridIt.Time), 0.0f));
				DrawListContent->AddLine(screenSpaceTL, screenSpaceTL + vec2(0.0f, Regions.Content.GetHeight()), lineColor);

				const vec2 headerScreenSpaceTL = LocalToScreenSpace_ContentHeader(vec2(Camera.TimeToLocalSpaceX(gridIt.Time), 0.0f));
				DrawListContentHeader->AddLine(headerScreenSpaceTL, headerScreenSpaceTL + vec2(0.0f, Regions.ContentHeader.GetHeight()), lineColor);

				// HACK: Try to prevent overlapping text for the very last grid line at least
				//if (!ApproxmiatelySame(gridIt.Time.Seconds, context.GetDurationOrDefault().Seconds) && (lastDrawnScreenSpaceTextTL.x + screenSpaceTimeTextWidth) > headerScreenSpaceTL.x)
				{
					// TODO: Fix overlapping text when zoomed out really far (while still keeping the grid lines)
					if (gridIt.IsBar)
					{
						Gui::DisableFontPixelSnap(true);

						char buffer[32];
						Gui::AddTextWithDropShadow(DrawListContentHeader, headerScreenSpaceTL + screenSpaceTextOffsetBarIndex, Gui::GetColorU32(ImGuiCol_Text),
							std::string_view(buffer, sprintf_s(buffer, "%d", gridIt.BarIndex)));
						Gui::AddTextWithDropShadow(DrawListContentHeader, headerScreenSpaceTL + screenSpaceTextOffsetBarTime, Gui::GetColorU32(ImGuiCol_Text, 0.5f),
							(gridIt.Time + timeLabelDisplayOffset).ToString().Data);

						lastDrawnScreenSpaceTextTL = headerScreenSpaceTL;
						Gui::DisableFontPixelSnap(false);
					}
				}
			});
			Gui::PopFont();
		}

		// NOTE: Grid snap beat lines
		{
			if (GridSnapLineAnimationCurrent > 0.0f)
			{
				const auto minMaxVisibleTime = GetMinMaxVisibleTime();
				const Beat gridBeatSnap = GetGridBeatSnap(CurrentGridBarDivision);
				const Beat minVisibleBeat = FloorBeatToGrid(context.TimeToBeat(minMaxVisibleTime.Min), gridBeatSnap) - gridBeatSnap;
				const Beat maxVisibleBeat = CeilBeatToGrid(context.TimeToBeat(minMaxVisibleTime.Max), gridBeatSnap) + gridBeatSnap;

				const u32 gridColorHex = IsTupletBarDivision(CurrentGridBarDivision)
					? TimelineGridSnapTupletLineColor
					: IsQuintupletBarDivision(CurrentGridBarDivision)
					? TimelineGridSnapQuintupletLineColor
					: IsSeptupletBarDivision(CurrentGridBarDivision)
					? TimelineGridSnapSeptupletLineColor
					: TimelineGridSnapLineColor
					;


				const u32 gridSnapLineColor = Gui::ColorU32WithAlpha(
					gridColorHex,
					GridSnapLineAnimationCurrent);
				for (Beat beatIt = ClampBot(minVisibleBeat, Beat::Zero()); beatIt <= ClampTop(maxVisibleBeat, context.TimeToBeat(context.Chart.GetDurationOrDefault())); beatIt += gridBeatSnap)
				{
					const vec2 screenSpaceTL = LocalToScreenSpace(vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(beatIt)), 0.0f));
					DrawListContent->AddLine(screenSpaceTL, screenSpaceTL + vec2(0.0f, Regions.Content.GetHeight()), gridSnapLineColor);
				}
			}
		}

		// NOTE: Out of bounds darkening
		{
			const Time gridTimeMin = ClampTop(context.Chart.SongOffset, Time::Zero()), gridTimeMax = context.Chart.GetDurationOrDefault();
			const f32 localSpaceMinL = Clamp(Camera.WorldToLocalSpace(vec2(Camera.TimeToWorldSpaceX(gridTimeMin), 0.0f)).x, 0.0f, Regions.Content.GetWidth());
			const f32 localSpaceMaxR = Clamp(Camera.WorldToLocalSpace(vec2(Camera.TimeToWorldSpaceX(gridTimeMax), 0.0f)).x, 0.0f, Regions.Content.GetWidth());

			const Rect screenSpaceRectL = Rect(Regions.Content.TL, vec2(LocalToScreenSpace(vec2(localSpaceMinL, 0.0f)).x, Regions.Content.BR.y));
			const Rect screenSpaceRectR = Rect(vec2(LocalToScreenSpace(vec2(localSpaceMaxR, 0.0f)).x, Regions.Content.TL.y), Regions.Content.BR);

			if (screenSpaceRectL.GetWidth() > 0.0f) DrawListContent->AddRectFilled(screenSpaceRectL.TL, screenSpaceRectL.BR, TimelineOutOfBoundsDimColor);
			if (screenSpaceRectR.GetWidth() > 0.0f) DrawListContent->AddRectFilled(screenSpaceRectR.TL, screenSpaceRectR.BR, TimelineOutOfBoundsDimColor);
		}

		// NOTE: Range selection rect
		if (RangeSelection.IsActive)
		{
			vec2 localTL = vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(RangeSelection.Start)), 1.0f);
			vec2 localBR = vec2(Camera.TimeToLocalSpaceX(context.BeatToTime(RangeSelection.End)), Regions.Content.GetHeight() - 1.0f);
			localBR.x = LerpClamped(localTL.x, localBR.x, RangeSelectionExpansionAnimationCurrent);
			const vec2 screenSpaceTL = LocalToScreenSpace(localTL) + vec2(!RangeSelection.HasEnd ? -1.0f : 0.0f, 0.0f);
			const vec2 screenSpaceBR = LocalToScreenSpace(localBR) + vec2(!RangeSelection.HasEnd ? +1.0f : 0.0f, 0.0f);

			if ((TimelineRangeSelectionHeaderHighlightColor & IM_COL32_A_MASK) > 0)
			{
				const f32 headerHighlightRectHeight = GuiScale(3.0f);
				const Rect headerHighlightRect = Rect(
					LocalToScreenSpace_ContentHeader(vec2(localTL.x, Regions.ContentHeader.GetHeight() - headerHighlightRectHeight)),
					LocalToScreenSpace_ContentHeader(vec2(localBR.x, Regions.ContentHeader.GetHeight() + 2.0f)));

				DrawListContentHeader->AddRectFilled(headerHighlightRect.TL, headerHighlightRect.BR, TimelineRangeSelectionHeaderHighlightColor);
				DrawListContentHeader->AddRect(headerHighlightRect.TL, headerHighlightRect.BR, TimelineRangeSelectionBorderColor);
			}

			DrawListContent->AddRectFilled(screenSpaceTL, screenSpaceBR, TimelineRangeSelectionBackgroundColor);
			DrawListContent->AddRect(screenSpaceTL, screenSpaceBR, TimelineRangeSelectionBorderColor);
		}

		// NOTE: Background waveform
		if (TimelineWaveformDrawOrder == WaveformDrawOrder::Background && !context.SongWaveformL.IsEmpty())
			DrawTimelineContentWaveform(*this, DrawListContent, context.Chart.SongOffset, context.SongWaveformL, context.SongWaveformR, context.SongWaveformFadeAnimationCurrent);

		// NOTE: Branch section background overlays on note rows (thin tint only; main visual is in the BranchSections row)
		if (SelectedCourseHasBranches)
		{
			// Collect note row Y span
			f32 noteRowsStartY = 0.0f, noteRowsEndY = 0.0f;
			ForEachTimelineRow(*this, [&](const ForEachRowData& rowIt)
			{
				if (rowIt.RowType == TimelineRowType::Notes_Normal)
					noteRowsStartY = rowIt.LocalY;
				if (rowIt.RowType == TimelineRowType::Notes_Master)
					noteRowsEndY = rowIt.LocalY + rowIt.LocalHeight;
			});

			for (const BranchSection& bs : context.ChartSelectedCourse->BranchSections)
			{
				const Time t0 = context.BeatToTime(bs.BeatTime);
				const Time t1 = context.BeatToTime(bs.GetEnd());
				const f32 x0 = Camera.TimeToLocalSpaceX(t0);
				const f32 x1 = Camera.TimeToLocalSpaceX(t1);
				if (x0 > Regions.Content.GetWidth() || x1 < 0.0f) continue;

				// Very light tint across all note rows
				DrawListContent->AddRectFilled(
					LocalToScreenSpace(vec2(x0, noteRowsStartY)),
					LocalToScreenSpace(vec2(x1, noteRowsEndY)),
					IM_COL32(200, 190, 80, 12));
			}
		}

		// NOTE: Row labels, lines and items
		{
			const Time visibleTimeOverdraw = Camera.TimePerScreenPixel() * (Gui::GetFrameHeight() * 4.0f);
			const DrawTimelineContentItemRowParam rowParam = { *this, context, DrawListContent, GetMinMaxVisibleTime(visibleTimeOverdraw), isPlayback, cursorTime, cursorBeatOnPlaybackStart };
			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
			ForEachTimelineRow(*this, [&](const ForEachRowData& rowIt)
			{
				// NOTE: Row label text
				{
					const vec2 sidebarScreenSpaceTL = LocalToScreenSpace_Sidebar(vec2(0.0f, rowIt.LocalY));
					const vec2 sidebarScreenSpaceBL = LocalToScreenSpace_Sidebar(vec2(0.0f, rowIt.LocalY + rowIt.LocalHeight));

					DrawListSidebar->AddLine(
						sidebarScreenSpaceBL,
						sidebarScreenSpaceBL + vec2(Regions.Sidebar.GetWidth(), 0.0f),
						Gui::GetColorU32(ImGuiCol_Separator, 0.35f));

					const f32 textHeight = Gui::GetFontSize();
					const vec2 screenSpaceTextPosition = sidebarScreenSpaceTL + vec2((Gui::GetStyle().FramePadding.x * 2.0f), Floor((rowIt.LocalHeight * 0.5f) - (textHeight * 0.5f)));

					const BranchType rowBranchType = TimelineRowToBranchType(rowIt.RowType);
					const b8 isBranchRow = (rowBranchType < BranchType::Count);
					const b8 isActiveBranch = isBranchRow && SelectedCourseHasBranches && (rowBranchType == context.ChartSelectedBranch);

					// Colored left border for the active branch
					if (isActiveBranch)
					{
						static constexpr u32 branchActiveColors[3] = { IM_COL32(100, 180, 255, 220), IM_COL32(255, 210, 70, 220), IM_COL32(255, 110, 110, 220) };
						DrawListSidebar->AddRectFilled(
							sidebarScreenSpaceTL,
							sidebarScreenSpaceTL + vec2(GuiScale(3.0f), rowIt.LocalHeight),
							branchActiveColors[EnumToIndex(rowBranchType)]);
					}

					// Branch row label color: N=blue, E=yellow, M=red; active = brighter
					u32 labelColor;
					if (isBranchRow && SelectedCourseHasBranches)
					{
						static constexpr u32 branchColors[3]       = { IM_COL32(130, 190, 255, 200), IM_COL32(245, 205, 75, 200), IM_COL32(255, 120, 120, 200) };
						static constexpr u32 branchColorsActive[3] = { IM_COL32(160, 210, 255, 255), IM_COL32(255, 225, 100, 255), IM_COL32(255, 150, 150, 255) };
						labelColor = isActiveBranch ? branchColorsActive[EnumToIndex(rowBranchType)] : branchColors[EnumToIndex(rowBranchType)];
					}
					else
					{
						labelColor = Gui::GetColorU32(ImGuiCol_Text);
					}

					Gui::DisableFontPixelSnap(true);
					DrawListSidebar->AddText(screenSpaceTextPosition, labelColor, Gui::StringViewStart(rowIt.Label), Gui::StringViewEnd(rowIt.Label));
					Gui::DisableFontPixelSnap(false);
				}

				// NOTE: Row separator line
				{
					const vec2 screenSpaceBL = LocalToScreenSpace(vec2(0.0f, rowIt.LocalY + rowIt.LocalHeight));
					DrawListContent->AddLine(screenSpaceBL, screenSpaceBL + vec2(Regions.Content.GetWidth(), 0.0f), TimelineHorizontalRowLineColor);
				}

				// NOTE: Row items
				switch (rowIt.RowType)
				{
				case TimelineRowType::Tempo: DrawTimelineContentItemRowT<TempoChange, TimelineRowType::Tempo>(rowParam, rowIt, context.ChartSelectedCourse->TempoMap.Tempo); break;
				case TimelineRowType::TimeSignature: DrawTimelineContentItemRowT<TimeSignatureChange, TimelineRowType::TimeSignature>(rowParam, rowIt, context.ChartSelectedCourse->TempoMap.Signature); break;
				case TimelineRowType::Notes_Normal: DrawTimelineContentItemRowT<Note, TimelineRowType::Notes_Normal>(rowParam, rowIt, context.ChartSelectedCourse->Notes_Normal); break;
				case TimelineRowType::Notes_Expert: DrawTimelineContentItemRowT<Note, TimelineRowType::Notes_Expert>(rowParam, rowIt, context.ChartSelectedCourse->Notes_Expert); break;
				case TimelineRowType::Notes_Master: DrawTimelineContentItemRowT<Note, TimelineRowType::Notes_Master>(rowParam, rowIt, context.ChartSelectedCourse->Notes_Master); break;
				case TimelineRowType::ScrollSpeed: DrawTimelineContentItemRowT<ScrollChange, TimelineRowType::ScrollSpeed>(rowParam, rowIt, context.ChartSelectedCourse->GetScrollChanges(context.ChartSelectedBranch)); break;
				case TimelineRowType::BarLineVisibility: DrawTimelineContentItemRowT<BarLineChange, TimelineRowType::BarLineVisibility>(rowParam, rowIt, context.ChartSelectedCourse->BarLineChanges); break;
				case TimelineRowType::GoGoTime: DrawTimelineContentItemRowT<GoGoRange, TimelineRowType::GoGoTime>(rowParam, rowIt, context.ChartSelectedCourse->GoGoRanges); break;
				case TimelineRowType::Lyrics: DrawTimelineContentItemRowT<LyricChange, TimelineRowType::Lyrics>(rowParam, rowIt, context.ChartSelectedCourse->Lyrics); break;
				case TimelineRowType::ScrollType: DrawTimelineContentItemRowT<ScrollType, TimelineRowType::ScrollType>(rowParam, rowIt, context.ChartSelectedCourse->GetScrollTypes(context.ChartSelectedBranch)); break;
				case TimelineRowType::JPOSScroll: DrawTimelineContentItemRowT<JPOSScrollChange, TimelineRowType::JPOSScroll>(rowParam, rowIt, context.ChartSelectedCourse->GetJPOSScrollChanges(context.ChartSelectedBranch)); break;
				case TimelineRowType::BranchSections: DrawTimelineContentItemRowT<BranchSection, TimelineRowType::BranchSections>(rowParam, rowIt, context.ChartSelectedCourse->BranchSections); break;
				case TimelineRowType::SectionMarkers: DrawTimelineContentItemRowT<SectionMarker, TimelineRowType::SectionMarkers>(rowParam, rowIt, context.ChartSelectedCourse->SectionMarkers); break;
				default: { assert(!"Missing TimelineRowType switch case"); } break;
				}
			});
			Gui::PopFont();
		}

		// NOTE: Background waveform overlay
		if (TimelineWaveformDrawOrder == WaveformDrawOrder::Foreground && !context.SongWaveformL.IsEmpty())
			DrawTimelineContentWaveform(*this, DrawListContent, context.Chart.SongOffset, context.SongWaveformL, context.SongWaveformR, context.SongWaveformFadeAnimationCurrent);

		// NOTE: Cursor foreground
		{
			const f32 cursorWidth = GuiScale(TimelineCursorHeadWidth);
			const f32 cursorHeight = GuiScale(TimelineCursorHeadHeight);

			// TODO: Maybe animate between triangle and something else depending on isPlayback (?)
			DrawListContentHeader->AddTriangleFilled(
				LocalToScreenSpace_ContentHeader(vec2(cursorHeaderTriangleLocalSpaceX - cursorWidth, 0.0f)),
				LocalToScreenSpace_ContentHeader(vec2(cursorHeaderTriangleLocalSpaceX + cursorWidth, 0.0f)),
				LocalToScreenSpace_ContentHeader(vec2(cursorHeaderTriangleLocalSpaceX, cursorHeight)), TimelineCursorColor);
			DrawListContentHeader->AddLine(
				LocalToScreenSpace_ContentHeader(vec2(cursorLocalSpaceX, cursorHeight)),
				LocalToScreenSpace_ContentHeader(vec2(cursorLocalSpaceX, Regions.ContentHeader.GetHeight())), TimelineCursorColor);

			// TODO: Maybe draw different cursor color or "type" depending on grid snap (?) (see "ArrowVortex/assets/icons snap.png")
			DrawListContent->AddLine(
				LocalToScreenSpace(vec2(cursorLocalSpaceX, 0.0f)),
				LocalToScreenSpace(vec2(cursorLocalSpaceX, Regions.Content.GetHeight())), TimelineCursorColor);

			// NOTE: Immediately draw destination cursor line for better responsiveness
			if (!isPlayback && !ApproxmiatelySame(animatedCursorLocalSpaceX, currentCursorLocalSpaceX, 0.5f))
			{
				DrawListContent->AddLine(
					LocalToScreenSpace(vec2(currentCursorLocalSpaceX, 0.0f)),
					LocalToScreenSpace(vec2(currentCursorLocalSpaceX, Regions.Content.GetHeight())), Gui::ColorU32WithAlpha(TimelineCursorColor, 0.25f));
			}
		}

		// NOTE: Mouse selection box
		if (BoxSelection.IsActive)
		{
			auto clampVisibleScreenSpace = [&](vec2 screenSpace) { return vec2(screenSpace.x, Clamp(screenSpace.y, Regions.Content.TL.y + 2.0f, Regions.Content.BR.y - 2.0f)); };

			DrawListContent->AddRectFilled(
				clampVisibleScreenSpace(LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.TL))),
				clampVisibleScreenSpace(LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.BR))), TimelineBoxSelectionBackgroundColor);
			DrawListContent->AddRect(
				clampVisibleScreenSpace(LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.TL))),
				clampVisibleScreenSpace(LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.BR))), TimelineBoxSelectionBorderColor);

			if (BoxSelection.Action != BoxSelectionAction::Clear)
			{
				const f32 radius = GuiScale(TimelineBoxSelectionRadius);
				const f32 linePadding = GuiScale(TimelineBoxSelectionLinePadding);
				const f32 lineThickness = ClampBot(Round(TimelineBoxSelectionLineThickness * GuiScaleFactorCurrent / 2.0f) * 2.0f, TimelineBoxSelectionLineThickness);

				const vec2 center = LocalToScreenSpace(Camera.WorldToLocalSpace(BoxSelection.WorldSpaceRect.TL));
				DrawListContent->AddCircleFilled(center, radius, TimelineBoxSelectionFillColor);
				DrawListContent->AddCircle(center, radius, TimelineBoxSelectionBorderColor);
				{
					const Rect horizontal = Rect::FromCenterSize(center, vec2((radius - linePadding) * 2.0f, lineThickness));
					const Rect vertical = Rect::FromCenterSize(center, vec2(lineThickness, (radius - linePadding) * 2.0f));

					if (BoxSelection.Action == BoxSelectionAction::Add)
					{
						DrawListContent->AddRectFilled(horizontal.TL, horizontal.BR, TimelineBoxSelectionInnerColor);
						DrawListContent->AddRectFilled(vertical.TL, vertical.BR, TimelineBoxSelectionInnerColor);
					}
					else if (BoxSelection.Action == BoxSelectionAction::Sub)
					{
						DrawListContent->AddRectFilled(horizontal.TL, horizontal.BR, TimelineBoxSelectionInnerColor);
					}
					else if (BoxSelection.Action == BoxSelectionAction::XOR)
					{
						DrawListContent->AddCircleFilled(center, GuiScale(TimelineBoxSelectionXorDotRadius), TimelineBoxSelectionInnerColor);
					}
				}
			}
		}
	}
}
