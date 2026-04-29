#include "chart_editor_widgets.h"
#include "chart_editor_settings.h"
#include "chart_editor_undo.h"
#include "chart_editor_i18n.h"
#include "core_build_info.h"

#include <map>

// TODO: Populate char[U8Max] lookup table using provided flags and index into instead of using a switch (?)
enum class EscapeSequenceFlags : u32 { NewLines };

static void ResolveEscapeSequences(const std::string_view in, std::string& out, EscapeSequenceFlags flags)
{
	assert(flags == EscapeSequenceFlags::NewLines);
	for (size_t i = 0; i < in.size(); i++)
	{
		if (in[i] == '\\' && i + 1 < in.size())
		{
			switch (in[i + 1])
			{
			case '\\': { out += '\\'; i++; } break;
			case 'n': { out += '\n'; i++; } break;
			}
		}
		else { out += in[i]; }
	}
}

static void ConvertToEscapeSequences(const std::string_view in, std::string& out, EscapeSequenceFlags flags)
{
	assert(flags == EscapeSequenceFlags::NewLines);
	out.reserve(out.size() + in.size());
	for (const char c : in)
	{
		switch (c)
		{
		case '\\': { out += "\\\\"; } break;
		case '\n': { out += "\\n"; } break;
		default: { out += c; } break;
		}
	}
}

namespace PeepoDrumKit
{
	static b8 GuiDragLabelScalar(std::string_view label, ImGuiDataType dataType, void* inOutValue, f32 speed = 1.0f, const void* min = nullptr, const void* max = nullptr, ImGuiSliderFlags flags = ImGuiSliderFlags_None)
	{
		b8 valueChanged = false;
		Gui::BeginGroup();
		Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
		{
			const ImVec2 topLeftCursorPos = Gui::GetCursorScreenPos();

			Gui::PushStyleColor(ImGuiCol_Text, 0); Gui::PushStyleColor(ImGuiCol_FrameBg, 0); Gui::PushStyleColor(ImGuiCol_FrameBgHovered, 0); Gui::PushStyleColor(ImGuiCol_FrameBgActive, 0);
			valueChanged = Gui::DragScalar("##DragScalar", dataType, inOutValue, speed, min, max, nullptr, flags | ImGuiSliderFlags_NoInput);
			Gui::SetDragScalarMouseCursor();
			Gui::PopStyleColor(4);

			// NOTE: Calling GetColorU32(ImGuiCol_Text) here instead of GetStyleColorVec4() causes the disabled-item alpha factor to get applied twice
			Gui::SetCursorScreenPos(topLeftCursorPos);
			Gui::PushStyleColor(ImGuiCol_Text, Gui::IsItemActive() ? DragTextActiveColor : Gui::IsItemHovered() ? DragTextHoveredColor : Gui::ColorConvertFloat4ToU32(Gui::GetStyleColorVec4(ImGuiCol_Text)));
			Gui::AlignTextToFramePadding(); Gui::TextUnformatted(Gui::StringViewStart(label), Gui::FindRenderedTextEnd(Gui::StringViewStart(label), Gui::StringViewEnd(label)));
			Gui::PopStyleColor(1);
		}
		Gui::PopID();
		Gui::EndGroup();
		return valueChanged;
	}

	static b8 GuiDragLabelFloat(std::string_view label, f32* inOutValue, f32 speed = 1.0f, f32 min = 0.0f, f32 max = 0.0f, ImGuiSliderFlags flags = ImGuiSliderFlags_None)
	{
		return GuiDragLabelScalar(label, ImGuiDataType_Float, inOutValue, speed, &min, &max, flags);
	}

	b8 GuiInputFraction(cstr label, ivec2* inOutValue, std::optional<ivec2> valueRange, i32 step, i32 stepFast, const u32* textColorOverride, std::string_view divisionText)
	{
		static constexpr i32 components = 2;
		const f32 divisionLabelWidth = Gui::CalcTextSize(Gui::StringViewStart(divisionText), Gui::StringViewEnd(divisionText)).x;
		const f32 perComponentInputFloatWidth = Floor(((Gui::GetContentRegionAvail().x - divisionLabelWidth) / static_cast<f32>(components)));

		b8 anyValueChanged = false;
		for (i32 component = 0; component < components; component++)
		{
			Gui::PushID(&(*inOutValue)[component]);

			const b8 isLastComponent = ((component + 1) == components);
			Gui::SetNextItemWidth(isLastComponent ? (Gui::GetContentRegionAvail().x - 1.0f) : perComponentInputFloatWidth);

			Gui::InputScalarWithButtonsExData exData {}; exData.TextColor = (textColorOverride != nullptr) ? *textColorOverride : Gui::GetColorU32(ImGuiCol_Text); exData.SpinButtons = true;
			Gui::InputScalarWithButtonsResult result = Gui::InputScalarN_WithExtraStuff("##Component", ImGuiDataType_S32, &(*inOutValue)[component], 1, &step, &stepFast, nullptr, ImGuiInputTextFlags_None, &exData);
			if (result.ValueChanged)
			{
				if (valueRange.has_value()) (*inOutValue)[component] = Clamp((*inOutValue)[component], valueRange->x, valueRange->y);
				anyValueChanged = true;
			}

			if (!isLastComponent)
			{
				Gui::SameLine(0.0f, 0.0f);
				Gui::TextUnformatted(divisionText);
				Gui::SameLine(0.0f, 0.0f);
			}

			Gui::PopID();
		}
		return anyValueChanged;
	}

	static b8 GuiDifficultyDecimalLevelStarSliderWidget(cstr label, DifficultyLevelDecimal* inOutLevel, b8& inOutFitOnScreenLastFrame, b8& inOutHoveredLastFrame)
	{
		b8 valueWasChanged = false;

		Gui::PushStyleColor(ImGuiCol_SliderGrab, Gui::GetStyleColorVec4(inOutHoveredLastFrame ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
		Gui::PushStyleColor(ImGuiCol_SliderGrabActive, Gui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		Gui::PushStyleColor(ImGuiCol_FrameBgHovered, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));
		Gui::PushStyleColor(ImGuiCol_FrameBgActive, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));

		
		if (i32 v = static_cast<i32>(*inOutLevel); 
			Gui::SliderInt(label, &v,
			static_cast<i32>(DifficultyLevelDecimal::None), 
			static_cast<i32>(DifficultyLevelDecimal::Max), 
			(v == static_cast <i32>(DifficultyLevelDecimal::None))
				? u8"None"
				: (v >= static_cast <i32>(DifficultyLevelDecimal::PlusThreshold))
				? u8"%d (+)"
				: u8"%d",
			ImGuiSliderFlags_AlwaysClamp))
		{
			*inOutLevel = static_cast<DifficultyLevelDecimal>(v);
			valueWasChanged = true;
		}
		Gui::PopStyleColor(4);

		const Rect sliderRect = Gui::GetItemRect();
		const f32 availableWidth = sliderRect.GetWidth();
		const vec2 starSize = vec2(availableWidth / static_cast<f32>(DifficultyLevelDecimal::Max), Gui::GetFrameHeight());

		inOutHoveredLastFrame = Gui::IsItemHovered();
		return valueWasChanged;
	}

	static b8 GuiDifficultyLevelStarSliderWidget(cstr label, DifficultyLevel* inOutLevel, b8& inOutFitOnScreenLastFrame, b8& inOutHoveredLastFrame)
	{
		b8 valueWasChanged = false;

		auto getStarColor = [](int level, unsigned int def) -> unsigned int {
			if (level >= 11) return IM_COL32(255, 122, 122, 255);
			return def;
		};

		auto _defaultColor = Gui::GetColorU32(ImGuiCol_Text);

		// NOTE: Make text transparent instead of using an empty slider format string 
		//		 so that the slider can still convert the input to a string on the same frame it is turned into an InputText (due to the frame delayed starsFitOnScreen)
		if (inOutFitOnScreenLastFrame) Gui::PushStyleColor(ImGuiCol_Text, 0x00000000);
		else Gui::PushStyleColor(ImGuiCol_Text, getStarColor((i32)(*inOutLevel), _defaultColor));
		Gui::PushStyleColor(ImGuiCol_SliderGrab, Gui::GetStyleColorVec4(inOutHoveredLastFrame ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
		Gui::PushStyleColor(ImGuiCol_SliderGrabActive, Gui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		Gui::PushStyleColor(ImGuiCol_FrameBgHovered, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));
		Gui::PushStyleColor(ImGuiCol_FrameBgActive, Gui::GetStyleColorVec4(ImGuiCol_FrameBg));
		if (i32 v = static_cast<i32>(*inOutLevel); Gui::SliderInt(label, &v,
			static_cast<i32>(DifficultyLevel::Min), static_cast<i32>(DifficultyLevel::Max), u8"★ %d", ImGuiSliderFlags_AlwaysClamp))
		{
			*inOutLevel = static_cast<DifficultyLevel>(v);
			valueWasChanged = true;
		}
		Gui::PopStyleColor(5);

		const Rect sliderRect = Gui::GetItemRect();
		const f32 availableWidth = sliderRect.GetWidth();
		const vec2 starSize = vec2(availableWidth / static_cast<f32>(DifficultyLevel::Max), Gui::GetFrameHeight());

		const b8 starsFitOnScreen = (starSize.x >= Gui::GetFrameHeight()) && !Gui::IsItemBeingEditedAsText();

		// NOTE: Use the last frame result here too to match the slider text as it has already been drawn
		if (inOutFitOnScreenLastFrame)
		{
			// NOTE: Manually tuned for a 16px font size
			struct StarParam { f32 OuterRadius, InnerRadius, Thickness; };
			static constexpr StarParam fontSizedStarParamOutline = { 8.0f, 4.0f, 1.0f };
			static constexpr StarParam fontSizedStarParamFilled = { 10.0f, 4.0f, 0.0f };
			const f32 starScale = Gui::GetFontSize() / 16.0f;

			// TODO: Consider drawing star background manually instead of using the slider grab hand (?)
			for (i32 i = 0; i < static_cast<i32>(DifficultyLevel::Max); i++)
			{
				const Rect starRect = Rect::FromTLSize(sliderRect.TL + vec2(i * starSize.x, 0.0f), starSize);
				const auto star = (i >= static_cast<i32>(*inOutLevel)) ? fontSizedStarParamOutline : fontSizedStarParamFilled;
				Gui::DrawStar(Gui::GetWindowDrawList(), starRect.GetCenter(), starScale * star.OuterRadius, starScale * star.InnerRadius, getStarColor(i + 1, _defaultColor), star.Thickness);
			}
		}

		inOutHoveredLastFrame = Gui::IsItemHovered();
		inOutFitOnScreenLastFrame = starsFitOnScreen;
		return valueWasChanged;
	}

	union MultiEditDataUnion
	{
		f32 F32; i32 I32; i16 I16;
		f32 F32_V[4]; i32 I32_V[4]; i16 I16_V[4];
	};
	struct MultiEditWidgetResult
	{
		u32 HasValueExact;
		u32 HasValueIncrement;
		MultiEditDataUnion ValueExact;
		MultiEditDataUnion ValueIncrement;
	};
	struct MultiEditWidgetParam
	{
		ImGuiDataType DataType = ImGuiDataType_Float;
		i32 Components = 1;
		b8 EnableStepButtons = false;
		b8 UseSpinButtonsInstead = false;
		b8 EnableDragLabel = true;
		b8 EnableClamp = false;
		u32 HasMixedValues = false;
		MultiEditDataUnion Value = {};
		MultiEditDataUnion MixedValuesMin = {};
		MultiEditDataUnion MixedValuesMax = {};
		MultiEditDataUnion ValueClampMin = {};
		MultiEditDataUnion ValueClampMax = {};
		MultiEditDataUnion ButtonStep = {};
		MultiEditDataUnion ButtonStepFast = {};
		f32 DragLabelSpeed = 1.0f;
		cstr FormatString = nullptr;
		u32* TextColorOverride = nullptr;
	};

	static MultiEditWidgetResult GuiPropertyMultiSelectionEditWidget(std::string_view label, const MultiEditWidgetParam& in)
	{
		MultiEditWidgetResult out = {};
		Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
		Gui::Property::Property([&]
		{
			if (in.EnableDragLabel)
			{
				MultiEditDataUnion v = in.Value;
				Gui::SetNextItemWidth(-1.0f);
				if (GuiDragLabelScalar(label, in.DataType, &v, in.DragLabelSpeed, in.EnableClamp ? &in.ValueClampMin : nullptr, in.EnableClamp ? &in.ValueClampMax : nullptr))
				{
					for (i32 c = 0; c < in.Components; c++)
					{
						out.HasValueIncrement |= (1 << c);
						switch (in.DataType)
						{
						case ImGuiDataType_Float: { out.ValueIncrement.F32_V[c] = (v.F32_V[c] - in.Value.F32_V[c]); } break;
						case ImGuiDataType_S32: { out.ValueIncrement.I32_V[c] = (v.I32_V[c] - in.Value.I32_V[c]); } break;
						case ImGuiDataType_S16: { out.ValueIncrement.I16_V[c] = (v.I16_V[c] - in.Value.I16_V[c]); } break;
						default: assert(false); break;
						}
					}
				}
			}
			else
			{
				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted(label);
			}
		});
		Gui::Property::Value([&]()
		{
			const auto& style = Gui::GetStyle();
			cstr formatString = (in.FormatString != nullptr) ? in.FormatString : Gui::DataTypeGetInfo(in.DataType)->PrintFmt;

			b8& textInputActiveLastFrame = *Gui::GetStateStorage()->GetBoolRef(Gui::GetID("TextInputActiveLastFrame"), false);
			const b8 showMixedValues = (in.HasMixedValues && !textInputActiveLastFrame);

			Gui::SetNextItemWidth(-1.0f);
			MultiEditDataUnion v = in.Value;
			Gui::InputScalarWithButtonsExData exData {}; exData.TextColor = showMixedValues ? 0 : (in.TextColorOverride != nullptr) ? *in.TextColorOverride : Gui::GetColorU32(ImGuiCol_Text); exData.SpinButtons = in.UseSpinButtonsInstead;
			Gui::InputScalarWithButtonsResult result = Gui::InputScalarN_WithExtraStuff("##Input", in.DataType, &v, in.Components, in.EnableStepButtons ? &in.ButtonStep : nullptr, in.EnableStepButtons ? &in.ButtonStepFast : nullptr, formatString, ImGuiInputTextFlags_None, &exData);
			if (result.ValueChanged)
			{
				for (i32 c = 0; c < in.Components; c++)
				{
					if (result.ButtonClicked & (1 << c))
						out.HasValueIncrement |= ((result.ValueChanged & (1 << c)) ? 1 : 0) << c;
					else
						out.HasValueExact |= ((result.ValueChanged & (1 << c)) ? 1 : 0) << c;

					if (in.EnableClamp)
					{
						switch (in.DataType)
						{
						case ImGuiDataType_Float: { v.F32_V[c] = Clamp(v.F32_V[c], in.ValueClampMin.F32_V[c], in.ValueClampMax.F32_V[c]); } break;
						case ImGuiDataType_S32: { v.I32_V[c] = Clamp(v.I32_V[c], in.ValueClampMin.I32_V[c], in.ValueClampMax.I32_V[c]); } break;
						case ImGuiDataType_S16: { v.I16_V[c] = Clamp(v.I16_V[c], in.ValueClampMin.I16_V[c], in.ValueClampMax.I16_V[c]); } break;
						default: assert(false); break;
						}
					}

					switch (in.DataType)
					{
					case ImGuiDataType_Float: { out.ValueExact.F32_V[c] = v.F32_V[c]; out.ValueIncrement.F32_V[c] = (v.F32_V[c] - in.Value.F32_V[c]); } break;
					case ImGuiDataType_S32: { out.ValueExact.I32_V[c] = v.I32_V[c]; out.ValueIncrement.I32_V[c] = (v.I32_V[c] - in.Value.I32_V[c]); } break;
					case ImGuiDataType_S16: { out.ValueExact.I16_V[c] = v.I16_V[c]; out.ValueIncrement.I16_V[c] = (v.I16_V[c] - in.Value.I16_V[c]); } break;
					default: assert(false); break;
					}
				}
			}
			textInputActiveLastFrame = result.IsTextItemActive;

			if (showMixedValues)
			{
				if (in.TextColorOverride != nullptr) Gui::PushStyleColor(ImGuiCol_Text, *in.TextColorOverride);
				for (i32 c = 0; c < in.Components; c++)
				{
					if (in.HasMixedValues & (1 << c))
					{
						char min[64], max[64];
						switch (in.DataType)
						{
						case ImGuiDataType_Float: { sprintf_s(min, formatString, in.MixedValuesMin.F32_V[c]); sprintf_s(max, formatString, in.MixedValuesMax.F32_V[c]); } break;
						case ImGuiDataType_S32: { sprintf_s(min, formatString, in.MixedValuesMin.I32_V[c]); sprintf_s(max, formatString, in.MixedValuesMax.I32_V[c]); } break;
						case ImGuiDataType_S16: { sprintf_s(min, formatString, in.MixedValuesMin.I16_V[c]); sprintf_s(max, formatString, in.MixedValuesMax.I16_V[c]); } break;
						default: assert(false); break;
						}
						char multiSelectionPreview[128]; sprintf_s(multiSelectionPreview, "(%s ... %s)", min, max);

						Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, 0.6f));
						Gui::RenderTextClipped(result.TextItemRect[c].TL + vec2(style.FramePadding.x, 0.0f), result.TextItemRect[c].BR, multiSelectionPreview, nullptr, nullptr, { 0.0f, 0.5f });
						Gui::PopStyleColor();
					}
					else
					{
						char preview[64];
						switch (in.DataType)
						{
						case ImGuiDataType_Float: { sprintf_s(preview, formatString, in.MixedValuesMin.F32_V[c]); } break;
						case ImGuiDataType_S32: { sprintf_s(preview, formatString, in.MixedValuesMin.I32_V[c]); } break;
						case ImGuiDataType_S16: { sprintf_s(preview, formatString, in.MixedValuesMin.I16_V[c]); } break;
						default: assert(false); break;
						}
						Gui::RenderTextClipped(result.TextItemRect[c].TL + vec2(style.FramePadding.x, 0.0f), result.TextItemRect[c].BR, preview, nullptr, nullptr, { 0.0f, 0.5f });
					}
				}
				if (in.TextColorOverride != nullptr) Gui::PopStyleColor();
			}
		});
		Gui::PopID();
		return out;
	}

	static b8 GuiPropertyRangeInterpolationEditWidget(std::string_view label, f32 inOutStartEnd[2], f32 step, f32 stepFast, f32 minValue, f32 maxValue, cstr format, const cstr previewStrings[2])
	{
		b8 wasValueChanged = false;
		Gui::PushID(Gui::StringViewStart(label), Gui::StringViewEnd(label));
		Gui::Property::PropertyTextValueFunc(label, [&]
		{
			static constexpr i32 components = 2; // NOTE: Unicode "Rightwards Arrow" U+2192
			static constexpr std::string_view divisionText = u8"  →  "; // "  ->  "; // " < > ";
			const f32 divisionLabelWidth = Gui::CalcTextSize(Gui::StringViewStart(divisionText), Gui::StringViewEnd(divisionText)).x;
			const f32 perComponentInputFloatWidth = Floor(((Gui::GetContentRegionAvail().x - divisionLabelWidth) / static_cast<f32>(components)));

			for (i32 component = 0; component < components; component++)
			{
				Gui::PushID(&inOutStartEnd[component]);
				b8& textInputActiveLastFrame = *Gui::GetStateStorage()->GetBoolRef(Gui::GetID("TextInputActiveLastFrame"), false);
				const b8 showPreviewStrings = ((previewStrings[component] != nullptr) && !textInputActiveLastFrame);
				const b8 isLastComponent = ((component + 1) == components);

				Gui::SetNextItemWidth(isLastComponent ? (Gui::GetContentRegionAvail().x - 1.0f) : perComponentInputFloatWidth);
				Gui::InputScalarWithButtonsExData exData {}; exData.TextColor = Gui::GetColorU32(ImGuiCol_Text, showPreviewStrings ? 0.0f : 1.0f); exData.SpinButtons = true;
				Gui::InputScalarWithButtonsResult result = Gui::InputScalar_WithExtraStuff("##Component", ImGuiDataType_Float, &inOutStartEnd[component], &step, &stepFast, format, ImGuiInputTextFlags_None, &exData);
				if (result.ValueChanged)
				{
					inOutStartEnd[component] = Clamp(inOutStartEnd[component], minValue, maxValue);
					wasValueChanged = true;
				}
				textInputActiveLastFrame = result.IsTextItemActive;

				if (showPreviewStrings)
				{
					Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, 0.6f));
					Gui::RenderTextClipped(result.TextItemRect[0].TL + vec2(Gui::GetStyle().FramePadding.x, 0.0f), result.TextItemRect[0].BR, previewStrings[component], nullptr, nullptr, { 0.0f, 0.5f });
					Gui::PopStyleColor();
				}

				if (!isLastComponent)
				{
					Gui::SameLine(0.0f, 0.0f);
					Gui::TextUnformatted(divisionText);
					Gui::SameLine(0.0f, 0.0f);
				}
				Gui::PopID();
			}
		});
		Gui::PopID();
		return wasValueChanged;
	}
}

namespace PeepoDrumKit
{
	// NOTE: Soft clamp for sliders but hard limit to allow *typing in* values higher, even if it can cause clipping
	static constexpr f32 MinVolume = 0.0f;
	static constexpr f32 MaxVolumeSoftLimit = 1.0f;
	static constexpr f32 MaxVolumeHardLimit = 4.0f;

	static constexpr i32 MinTimeSignatureValue = -Beat::TicksPerBeat * 4;
	static constexpr i32 MaxTimeSignatureValue = Beat::TicksPerBeat * 4;
	static constexpr i16 MinBalloonCount = 0;
	static constexpr i16 MaxBalloonCount = 999;

	// TODO: Turn these into user settings
	// "allowed_tempo_bpm_range_min" = 30
	// "allowed_tempo_bpm_range_max" = 960
	// "allowed_scroll_speed_range_min" = -100
	// "allowed_scroll_speed_range_max" = +100
	// "allowed_note_time_offset_range_min" = -35
	// "allowed_note_time_offset_range_max" = +35
	static constexpr f32 MinBPM = -10000.0f;//30.0f;
	static constexpr f32 MaxBPM = 10000.0f;//960.0f;
	static constexpr f32 MinScrollSpeed = -100.0f;
	static constexpr f32 MaxScrollSpeed = +100.0f;
	static constexpr f32 MaxJPOSScrollMove = +9999.0f;
	static constexpr f32 MinJPOSScrollMove = -9999.0f;
	static constexpr f32 MaxJPOSScrollDuration = +3600.0f;
	static constexpr f32 MinJPOSScrollDuration = -3600.0f;
	static constexpr Time MinNoteTimeOffset = Time::FromMS(-35.0);
	static constexpr Time MaxNoteTimeOffset = Time::FromMS(+35.0);

	cstr LoadingTextAnimation::UpdateFrameAndGetText(b8 isLoadingThisFrame, f32 deltaTimeSec)
	{
		static constexpr cstr TextFrames[] = { "Loading o....", "Loading .o...", "Loading ..o..", "Loading ...o.", "Loading ....o", "Loading ...o.", "Loading ..o..", "Loading .o..." };
		static constexpr f32 FrameIntervalSec = (1.0f / 12.0f);

		if (!WasLoadingLastFrame && isLoadingThisFrame)
			RingIndex = 0;

		WasLoadingLastFrame = isLoadingThisFrame;
		if (isLoadingThisFrame)
			AccumulatedTimeSec += deltaTimeSec;

		cstr loadingText = TextFrames[RingIndex];
		if (AccumulatedTimeSec >= FrameIntervalSec)
		{
			AccumulatedTimeSec -= FrameIntervalSec;
			if (++RingIndex >= ArrayCountI32(TextFrames))
				RingIndex = 0;
		}
		return loadingText;
	}

	void ChartUpdateNotesWindow::DrawGui(ChartContext& context)
	{
		static struct
		{
			u32 GreenDark = 0xFFACF7DC;
			u32 GreenBright = 0xFF95CCB8;
			u32 RedDark = 0xFF9BBAEF;
			u32 RedBright = 0xFF93B2E7;
			u32 WhiteDark = 0xFF95DCDC;
			u32 WhiteBright = 0xFFBBD3D3;
			b8 Show = false;
		} colors;

		Gui::PushStyleColor(ImGuiCol_Border, Gui::GetColorU32(ImGuiCol_Separator));
		Gui::BeginChild("BackgroundChild", Gui::GetContentRegionAvail(), true);
		Gui::PopStyleColor(1);
		defer{ Gui::EndChild(); };

		Gui::UpdateSmoothScrollWindow();

		Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
		{
			// Header with current version
			{
				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::TextUnformatted("Update Notes");
				Gui::PopFont();
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenBright);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::Text("Current version: %s", BuildInfo::CurrentVersion());
				Gui::Separator();
				Gui::PopFont();
				Gui::PopStyleColor();
			}

			// Update Log wrapper
			{
				Gui::PushStyleColor(ImGuiCol_Text, colors.RedDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::TextUnformatted("Update Logs:");
				Gui::PopFont();
				Gui::PopStyleColor();

				// v1.2
				{
					Gui::PushStyleColor(ImGuiCol_Text, colors.RedBright);
					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
					Gui::TextUnformatted("v1.2");
					Gui::PopFont();

					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
					Gui::TextUnformatted(u8"- Add the possibility to reorder difficulties by dragging difficulty tabs");
					Gui::TextUnformatted(u8"- Add support for STYLE: and #START P<n>");
					Gui::TextUnformatted(u8"- Add comparison mode to compare difficulties side-by-side");
					Gui::TextUnformatted(u8"- Add current #JPOSSCROLL position display on the judge mark");
					Gui::TextUnformatted(u8"- The gameplay lane border now show the visible region in simulators when in wide view");
					Gui::TextUnformatted(u8"- Add sound volume limiter and remove sound effects' play frequency limits");
					Gui::TextUnformatted(u8"- AdLibs are now shown semi-transparent instead of hidden");
					Gui::TextUnformatted(u8"- KaDon are now played with Don + Ka sounds instead of just Don");
					Gui::TextUnformatted(u8"- Balloon-type notes' pop count is now shown when they are being popped");
					Gui::TextUnformatted(u8"- Add “Buffer Frame Size” option for manually fixing audio distortion due to insufficient buffer size (in Settings → Audio Settings)");
					Gui::TextUnformatted(u8"- Add proper SENote assignment and the コ (Ko) SENote");
					Gui::TextUnformatted(u8"- Add Go-go time effect");
					Gui::TextUnformatted(u8"- Add support for editing any localized TITLE: and SUBTITLE: with custom locales");
					Gui::TextUnformatted(u8"- Fix #JPOSSCROLL's overlapping behavior (was not stopped by next #JPOSSCROLL as in simulators)");
					Gui::TextUnformatted(u8"- Add support for the Handed Don (A) and Handed Katsu (B) notes");
					Gui::TextUnformatted(u8"- Remove restriction of creating existing difficulties, for creating multiplayer charts and using comparison mode");
					Gui::TextUnformatted(u8"- Fix inaccurate time interval of balloon-type note popping sound");
					Gui::TextUnformatted(u8"- Add support for editing any unknown TJA headers");
					Gui::TextUnformatted(u8"- Add support for number-less NOTESDESIGNER: and file-scope usage of numbered NOTESDESIGNER headers");
					Gui::TextUnformatted(u8"- Fix notes in negative BPM were wrongly flipped horizontally and SENotes in positive BPM were wrongly rotated 180 degree");
					Gui::TextUnformatted(u8"- (the following are hotfixes)");
					Gui::TextUnformatted(u8"- Fix crash when player side and count numbers are too long");
					Gui::TextUnformatted(u8"- Fix compared lanes wrongly used selected lane's beat and time position if their timing commands differ");
					Gui::TextUnformatted(u8"- Fix flying notes moved with on-going #JPOSSCROLLs again due to reworking of #JPOSSCROLL in v1.2");
					Gui::TextUnformatted(u8"- Fix undefined default Chart Stats tab docking position since v1.1.1");
					Gui::TextUnformatted(u8"- (for the full change list, please refer to the commit history)");
					Gui::TextUnformatted("");
					Gui::PopFont();
					Gui::PopStyleColor();
				}

				// v1.1.1
				{
					Gui::PushStyleColor(ImGuiCol_Text, colors.RedBright);
					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
					Gui::TextUnformatted("v1.1.1");
					Gui::PopFont();

					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
					Gui::TextUnformatted(u8"- Fix notes' and bar lines' position in #HBSCROLL and #BMSCROLL (when past judgement or around #BPMCHANGEs)");
					Gui::TextUnformatted(u8"- Fix #JPOSSCROLL distance (was 720px/lane instead of simulators' 948px/lane), direction 0 mode (failed to flip vertically)");
					Gui::TextUnformatted(u8"- Fix could not set #JPOSSCROLL duration in Chart Inspector");
					Gui::TextUnformatted(u8"- Improve compatibility and performance of chart importing and exporting");
					Gui::TextUnformatted(u8"- Widen allowed BPM's and time signature's input and effective range (negative allowed)");
					Gui::TextUnformatted(u8"- Add .ini localization; add zh-CN and zh-TW localizations");
					Gui::TextUnformatted(u8"- Fix displayed position of balloons and flying notes with non-positive #SCROLL");
					Gui::TextUnformatted(u8"- Add TaikoJiro2-like note display supporting complex-valued #SCROLL and stretching rolls with bar");
					Gui::TextUnformatted(u8"- Add the possibility to edit notes' and long events' end position by dragging their end when selected");
					Gui::TextUnformatted(u8"- #JPOSSCROLL is now visualized and editable as long event");
					Gui::TextUnformatted(u8"- Widen playback speed range to 10%–200%");
					Gui::TextUnformatted(u8"- Add Chart Stats tab");
					Gui::TextUnformatted(u8"- Tweak difficulty number display and remove star view for decimal");
					Gui::TextUnformatted(u8"- Add support of editing Tower charts and view Dan charts");
					Gui::TextUnformatted(u8"- Add “Insert at Selected Items”, the successor of “Selection to Scroll Changes” which applies to all chart events");
					Gui::TextUnformatted(u8"- Migrate to Dear ImGui 1.92.0-docking and solve missing font glyph issues");
					Gui::TextUnformatted(u8"- Add “Select to End of Chart”");
					Gui::TextUnformatted(u8"- Add advanced chart scale options, fix “missing notes after undo” problem when scaling");
					Gui::TextUnformatted(u8"- (for the full change list, please refer to the commit history)");
					Gui::TextUnformatted("");
					Gui::PopFont();
					Gui::PopStyleColor();
				}

				// v1.1
				{
					Gui::PushStyleColor(ImGuiCol_Text, colors.RedBright);
					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
					Gui::TextUnformatted("v1.1");
					Gui::PopFont();

					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
					Gui::TextUnformatted("- Add support for #HBSCROLL and #BMSCROLL methods");
					Gui::TextUnformatted("- Add support for the #JPOSSCROLL gimmick");
					Gui::TextUnformatted("- Add support for complex #SCROLL changes (y axis)");
					Gui::TextUnformatted("- Add support for the PREIMAGE metadata");
					Gui::TextUnformatted("- Add support for more tuplet subdivisions (Quintuplets, Septuplets, Nonuplets)");
					Gui::TextUnformatted("- Add the possibility to add courses real time (Only regular difficulties for the moment)");
					Gui::TextUnformatted("- Automatically convert #DIRECTION tags to complex #SCROLL values");
					Gui::TextUnformatted("- Fix display for the Fuseroll (D) tails");
					Gui::TextUnformatted("");
					Gui::PopFont();
					Gui::PopStyleColor();
				}

				// v1.0
				{
					Gui::PushStyleColor(ImGuiCol_Text, colors.RedBright);
					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
					Gui::TextUnformatted("v1.0");
					Gui::PopFont();

					Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
					Gui::TextUnformatted("- Add support for the Bomb (C) note");
					Gui::TextUnformatted("- Add support for the Fuseroll (D) note");
					Gui::TextUnformatted("- Add support for the ADLib (F) note");
					Gui::TextUnformatted("- Add support for the Swap (G) note");
					Gui::TextUnformatted("");
					Gui::PopFont();
					Gui::PopStyleColor();
				}
			}


		}
		Gui::PopFont();
	}

	void ChartChartStatsWindow::DrawGui(ChartContext& context)
	{
		auto trimPrefix = [](std::string str) -> std::string {
			if (str.rfind("--", 0) == 0 || str.rfind("++", 0) == 0) return str.substr(2);
			return str;
		};

		static struct
		{
			u32 GreenDark = 0xFFACF7DC;
			u32 GreenBright = 0xFF95CCB8;
			u32 RedDark = 0xFF9BBAEF;
			u32 RedBright = 0xFF93B2E7;
			u32 WhiteDark = 0xFF95DCDC;
			u32 WhiteBright = 0xFFBBD3D3;
			b8 Show = false;
		} colors;

		assert(context.ChartSelectedCourse != nullptr);
		auto& chart = context.Chart;
		auto& course = *context.ChartSelectedCourse;

		Gui::PushStyleColor(ImGuiCol_Border, Gui::GetColorU32(ImGuiCol_Separator));
		Gui::BeginChild("BackgroundChild", Gui::GetContentRegionAvail(), true);
		Gui::PopStyleColor(1);
		defer{ Gui::EndChild(); };

		Gui::UpdateSmoothScrollWindow();

		Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
		{
			// Header with chart main information
			{
				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::TextUnformatted("Chart Stats");
				Gui::PopFont();
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenBright);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::Text("%s", chart.ChartTitle.c_str());
				Gui::PopFont();
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
				Gui::Text("%s", trimPrefix(chart.ChartSubtitle).c_str());
				Gui::Text("Charter: %s", course.CourseCreator.c_str());
				Gui::Text("%s Lv.%d %s", UI_StrRuntime(DifficultyTypeNames[(int)course.Type]), course.Level, GetStyleName(course.Style, course.PlayerSide).c_str());
				Gui::Separator();
				Gui::PopFont();
				Gui::PopStyleColor();
			}

			// Details
			{
				SortedNotesList& notes = context.ChartSelectedCourse->GetNotes(context.ChartSelectedBranch);

				int _donCount = notes.CountIf([](Note N) {return IsDonNote(N.Type);});
				int _kaCount = notes.CountIf([](Note N) {return IsKaNote(N.Type);});
				int _kaDonCount = notes.CountIf([](Note N) {return IsKaDonNote(N.Type);});
				int _adLibCount = notes.CountIf([](Note N) {return IsAdlibNote(N.Type);});
				int _bombCount = notes.CountIf([](Note N) {return IsBombNote(N.Type);});
				int _maxCombo = _donCount + _kaCount + _kaDonCount;

				f64 _density = _maxCombo / chart.ChartDuration.Seconds;

				Gui::PushStyleColor(ImGuiCol_Text, colors.RedDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::Text("Max Combo: %d", _maxCombo);
				Gui::PopFont();
				Gui::PopStyleColor();

				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));

				Gui::PushStyleColor(ImGuiCol_Text, colors.RedDark);
				Gui::Text("Density: %.3f hit/s", _density);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 122, 122, 255));
				Gui::Text("Don: %d", _donCount);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, IM_COL32(122, 122, 255, 255));
				Gui::Text("Ka: %d", _kaCount);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 122, 255, 255));
				Gui::Text("KaDon: %d", _kaDonCount);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
				Gui::Text("Adlib: %d", _adLibCount);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, IM_COL32(122, 122, 122, 255));
				Gui::Text("Bomb: %d", _bombCount);
				Gui::PopStyleColor();
				
				Gui::PopFont();

			}


		}
		Gui::PopFont();
	}

	void ChartHelpWindow::DrawGui(ChartContext& context)
	{
		static struct
		{
			u32 GreenDark = 0xFFACF7DC; // 0xFF60BE9C;
			u32 GreenBright = 0xFF95CCB8; // 0xFF60BE9C;
			u32 RedDark = 0xFF9BBAEF; // 0xFF72A0ED; // 0xFF71A4FA; // 0xFF7474EF; // 0xFF6363DE;
			u32 RedBright = 0xFF93B2E7; // 0xFFBAC2E4; // 0xFF6363DE;
			u32 WhiteDark = 0xFF95DCDC; // 0xFFEAEAEA;
			u32 WhiteBright = 0xFFBBD3D3; // 0xFFEAEAEA;
			b8 Show = false;
		} colors;
#if PEEPO_DEBUG
		if (Gui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && Gui::IsKeyPressed(ImGuiKey_GraveAccent))
			colors.Show ^= true;
		if (colors.Show)
		{
			Gui::ColorEdit4_U32("Green Dark", &colors.GreenDark);
			Gui::ColorEdit4_U32("Green Bright", &colors.GreenBright);
			Gui::ColorEdit4_U32("Red Dark", &colors.RedDark);
			Gui::ColorEdit4_U32("Red Bright", &colors.RedBright);
			Gui::ColorEdit4_U32("White Dark", &colors.WhiteDark);
			Gui::ColorEdit4_U32("White Bright", &colors.WhiteBright);
		}
#endif

		Gui::PushStyleColor(ImGuiCol_Border, Gui::GetColorU32(ImGuiCol_Separator));
		Gui::BeginChild("BackgroundChild", Gui::GetContentRegionAvail(), true);
		Gui::PopStyleColor(1);
		defer { Gui::EndChild(); };

		Gui::UpdateSmoothScrollWindow();

		// TODO: Maybe use .md markup language to write instructions (?) (how feasable would it be to integrate peepo emotes inbetween text..?)
		Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
		{
			{
				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::TextUnformatted("Welcome to Peepo Drum Kit (Unofficial fork)");
				Gui::PopFont();
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, colors.GreenBright);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::TextWrapped("Things are still very much WIP and subject to change with some features still missing " UTF8_FeelsOkayMan);
				Gui::Separator();
				Gui::TextUnformatted("");
				Gui::PopFont();
				Gui::PopStyleColor();
			}

			{
				Gui::PushStyleColor(ImGuiCol_Text, colors.RedDark);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::TextUnformatted("Basic Controls:");
				Gui::PopFont();
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, colors.RedBright);
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Small));
				if (Gui::BeginTable("ControlsTable", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_NoSavedSettings))
				{
					static constexpr auto row = [&](auto funcLeft, auto funcRight)
					{
						Gui::TableNextRow();
						Gui::TableSetColumnIndex(0); funcLeft();
						Gui::TableSetColumnIndex(1); funcRight();
					};
					static constexpr auto rowSeparator = []() { row([] { Gui::Text(""); }, [] {}); };

					row([] { Gui::Text("Move cursor"); }, [] { Gui::Text("Mouse Left"); });
					row([] { Gui::Text("Select items"); }, [] { Gui::Text("Mouse Right"); });
					row([] { Gui::Text("Scroll"); }, [] { Gui::Text("Mouse Wheel"); });
					row([] { Gui::Text("Scroll panning"); }, [] { Gui::Text("Mouse Middle"); });
					row([] { Gui::Text("Zoom"); }, [] { Gui::Text("Alt + Mouse Wheel"); });
					row([] { Gui::Text("Fast (Action)"); }, [] { Gui::Text("Shift + { Action }"); });
					row([] { Gui::Text("Slow (Action)"); }, [] { Gui::Text("Alt + { Action }"); });
					rowSeparator();
					row([] { Gui::Text("Play / pause"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_TogglePlayback).Data); });
					row([] { Gui::Text("Add / remove note"); }, []
					{
						Gui::Text("%s / %s / %s / %s",
							ToShortcutString(Settings.Input.Timeline_PlaceNoteKa->Slots[0]).Data, ToShortcutString(Settings.Input.Timeline_PlaceNoteDon->Slots[0]).Data,
							ToShortcutString(Settings.Input.Timeline_PlaceNoteDon->Slots[1]).Data, ToShortcutString(Settings.Input.Timeline_PlaceNoteKa->Slots[1]).Data);
					});
					row([] { Gui::Text("Add long note"); }, []
					{
						Gui::Text("%s / %s / %s / %s",
							ToShortcutString(Settings.Input.Timeline_PlaceNoteBalloon->Slots[0]).Data, ToShortcutString(Settings.Input.Timeline_PlaceNoteDrumroll->Slots[0]).Data,
							ToShortcutString(Settings.Input.Timeline_PlaceNoteDrumroll->Slots[1]).Data, ToShortcutString(Settings.Input.Timeline_PlaceNoteBalloon->Slots[1]).Data);
					});
					row([] { Gui::Text("Add big note"); }, [] { Gui::Text("Alt + { Note }"); });
					row([] { Gui::Text("Fill range-selection with notes"); }, [] { Gui::Text("Shift + { Note }"); });
					row([] { Gui::Text("Start / end range-selection"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_StartEndRangeSelection).Data); });
					row([] { Gui::Text("Grid division"); }, [] { Gui::Text("Mouse [X1/X2] / [%s/%s]", ToShortcutString(*Settings.Input.Timeline_IncreaseGridDivision).Data, ToShortcutString(*Settings.Input.Timeline_DecreaseGridDivision).Data); });
					row([] { Gui::Text("Step cursor"); }, [] { Gui::Text("%s / %s", ToShortcutString(*Settings.Input.Timeline_StepCursorLeft).Data, ToShortcutString(*Settings.Input.Timeline_StepCursorRight).Data); });
					rowSeparator();
					row([] { Gui::Text("Move selected items"); }, [] { Gui::Text("Mouse Left (Hover)"); });
					row([] { Gui::Text("Cut selected items"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_Cut).Data); });
					row([] { Gui::Text("Copy selected items"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_Copy).Data); });
					row([] { Gui::Text("Paste selected items"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_Paste).Data); });
					row([] { Gui::Text("Delete selected items"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_DeleteSelection).Data); });
					rowSeparator();
					row([] { Gui::Text("Playback speed"); }, [] { Gui::Text("%s / %s", ToShortcutString(*Settings.Input.Timeline_DecreasePlaybackSpeed).Data, ToShortcutString(*Settings.Input.Timeline_IncreasePlaybackSpeed).Data); });
					row([] { Gui::Text("Toggle metronome"); }, [] { Gui::Text(ToShortcutString(*Settings.Input.Timeline_ToggleMetronome).Data); });

					Gui::EndTable();
				}
				Gui::PopFont();
				Gui::PopStyleColor();
			}

			{
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
				Gui::PushStyleColor(ImGuiCol_Text, colors.WhiteDark);
				Gui::TextUnformatted("");
				Gui::TextUnformatted("About reading and writing TJAs:");
				Gui::PopStyleColor();
				Gui::Separator();

				Gui::PushStyleColor(ImGuiCol_Text, colors.WhiteBright);
				Gui::TextWrapped(
					"NOTE: This is an altered, unofficial version of Peepo Drum Kit aimed to support wider gimmicks and to be a best fit with OpenTaiko.\n"
					"Some main components are altered, like the 1/192nd grid being extended to be able to support odd tuplets (Quintuplets, Septuplets, Nonuplets.)\n"
					"The following description is from the official version of Peepo Drum Kit.\n"
					"\n"
					"Peepo Drum Kit is not really a TJA editor in the same sense that a text editor is one.\n"
					"It's a Taiko chart editor that transparently converts to and from the TJA format,"
					" however its internal data representation of a chart differs significantly.\n"
					"\n"
					"As such, potential data-loss is the convertion process (to some extend) is to be expected.\n"
					"This may either take the form of general formatting differences (removing comments, white space, etc.),\n"
					"issues with (yet) unimplemented features (such as branches and other commands)\n"
					"or general \"rounding errors\" due to notes and other commands being quantized onto a fixed 1/192nd grid (as is a common rhythm game convention).\n"
					"\n"
					"To prevent the user from accidentally overwriting existing TJAs, that have not originally been created via this program (as marked by a \"PeepoDrumKit\" comment),\n"
					"edited TJAs will therefore have a \".bak\" backup file created in-place before being overwritten.\n"
					"\n"
					"With that said, none of this should be a problem when creating new charts from inside the program itself\n"
					"as the goal is for the user to not have to interact with the \".tja\" in text form at all " UTF8_PeepoCoffeeBlonket);
				Gui::PopStyleColor();
				Gui::PopFont();
			}
		}
		Gui::PopFont();
	}

	void ChartUndoHistoryWindow::DrawGui(ChartContext& context)
	{
		const auto& undoStack = context.Undo.UndoStack;
		const auto& redoStack = context.Undo.RedoStack;
		i32 undoClickedIndex = -1, redoClickedIndex = -1;

		const auto& style = Gui::GetStyle();
		// NOTE: Desperate attempt for the table row selectables to not having any spacing between them
		Gui::PushStyleVar(ImGuiStyleVar_CellPadding, { style.CellPadding.x, GuiScale(4.0f) });
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { style.ItemSpacing.x, GuiScale(8.0f) });
		Gui::PushStyleColor(ImGuiCol_Header, Gui::GetColorU32(ImGuiCol_Header, 0.5f));
		Gui::PushStyleColor(ImGuiCol_HeaderHovered, Gui::GetColorU32(ImGuiCol_HeaderHovered, 0.5f));
		defer { Gui::PopStyleColor(2); Gui::PopStyleVar(2); };

		if (Gui::BeginTable("UndoHistoryTable", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, Gui::GetContentRegionAvail()))
		{
			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
			Gui::TableSetupScrollFreeze(0, 1);
			Gui::TableSetupColumn(UI_Str("UNDO_HISTORY_DESCRIPTION"), ImGuiTableColumnFlags_None);
			Gui::TableSetupColumn(UI_Str("UNDO_HISTORY_TIME"), ImGuiTableColumnFlags_None);
			Gui::TableHeadersRow();
			Gui::PopFont();

			static constexpr auto undoCommandRow = [](Undo::CommandInfo commandInfo, CPUTime creationTime, const void* id, b8 isSelected)
			{
				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0);
				char labelBuffer[256]; sprintf_s(labelBuffer, "%.*s###%p", FmtStrViewArgs(commandInfo.Description), id);
				const b8 clicked = Gui::Selectable(labelBuffer, isSelected, ImGuiSelectableFlags_SpanAllColumns);

				// TODO: Display as formatted local time instead of time relative to program startup (?)
				Gui::TableSetColumnIndex(1);
				Gui::TextDisabled("%s", CPUTime::DeltaTime(CPUTime {}, creationTime).ToString().Data);
				return clicked;
			};

			if (undoCommandRow(Undo::CommandInfo { UI_Str("UNDO_HISTORY_INITIAL_STATE") }, CPUTime {}, nullptr, undoStack.empty()))
				context.Undo.Undo(undoStack.size());

			if (!undoStack.empty())
			{
				ImGuiListClipper clipper; clipper.Begin(static_cast<i32>(undoStack.size()));
				while (clipper.Step())
				{
					for (i32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						if (undoCommandRow(undoStack[i]->GetInfo(), undoStack[i]->CreationTime, undoStack[i].get(), ((i + 1) == undoStack.size())))
							undoClickedIndex = i;
					}
				}
			}

			if (!redoStack.empty())
			{
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				ImGuiListClipper clipper; clipper.Begin(static_cast<i32>(redoStack.size()));
				while (clipper.Step())
				{
					for (i32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						const i32 redoIndex = ((static_cast<i32>(redoStack.size()) - 1) - i);
						if (undoCommandRow(redoStack[redoIndex]->GetInfo(), redoStack[redoIndex]->CreationTime, redoStack[redoIndex].get(), false))
							redoClickedIndex = redoIndex;
					}
				}
				Gui::PopStyleColor();
			}

			// NOTE: Auto scroll if not already at bottom
			if (Gui::GetScrollY() >= Gui::GetScrollMaxY())
				Gui::SetScrollHereY(1.0f);

			Gui::EndTable();
		}

		if (InBounds(undoClickedIndex, undoStack))
			context.Undo.Undo(undoStack.size() - undoClickedIndex - 1);
		else if (InBounds(redoClickedIndex, redoStack))
			context.Undo.Redo(redoStack.size() - redoClickedIndex);
	}

	void TempoCalculatorWindow::DrawGui(ChartContext& context)
	{
		Gui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(ImGuiCol_Header));
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, Gui::GetStyleColorVec4(ImGuiCol_HeaderHovered));
		Gui::PushStyleColor(ImGuiCol_ButtonActive, Gui::GetStyleColorVec4(ImGuiCol_HeaderActive));
		{
			const b8 windowFocused = Gui::IsWindowFocused() && (Gui::GetActiveID() == 0);
			const b8 tapPressed = windowFocused && Gui::IsAnyPressed(*Settings.Input.TempoCalculator_Tap, false);
			const b8 resetPressed = windowFocused && Gui::IsAnyPressed(*Settings.Input.TempoCalculator_Reset, false);
			const b8 resetDown = windowFocused && Gui::IsAnyDown(*Settings.Input.TempoCalculator_Reset);

			Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
			{
				const Time lastBeatDuration = Time::FromSec(60.0 / Round(Calculator.LastTempo.BPM));
				const f32 tapBeatLerpT = ImSaturate((Calculator.TapCount == 0) ? 1.0f : static_cast<f32>(Calculator.LastTap.GetElapsed() / lastBeatDuration));
				const ImVec4 animatedButtonColor = ImLerp(Gui::GetStyleColorVec4(ImGuiCol_ButtonActive), Gui::GetStyleColorVec4(ImGuiCol_Button), tapBeatLerpT);

				const b8 hasTimedOut = Calculator.HasTimedOut();
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, (Calculator.TapCount > 0 && hasTimedOut) ? 0.8f : 1.0f));
				Gui::PushStyleColor(ImGuiCol_Button, animatedButtonColor);
				Gui::PushStyleColor(ImGuiCol_ButtonHovered, (Calculator.TapCount > 0 && !hasTimedOut) ? animatedButtonColor : Gui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
				Gui::PushStyleColor(ImGuiCol_ButtonActive, (Calculator.TapCount > 0 && !hasTimedOut) ? animatedButtonColor : Gui::GetStyleColorVec4(ImGuiCol_ButtonActive));

				char buttonName[32]; sprintf_s(buttonName, (Calculator.TapCount == 0) ? UI_Str("ACT_TEMPO_CALCULATOR_TAP") : (Calculator.TapCount == 1) ? UI_Str("INFO_TEMPO_CALCULATOR_TAP_FIRST_BEAT") : "%.2f BPM", Calculator.LastTempo.BPM);
				if (tapPressed | Gui::ButtonEx(buttonName, vec2(-1.0f, Gui::GetFrameHeightWithSpacing() * 3.0f), ImGuiButtonFlags_PressedOnClick))
				{
					context.SfxVoicePool.PlaySound(SoundEffectType::MetronomeBeat);
					Calculator.Tap();
				}

				Gui::PopStyleColor(4);
			}
			Gui::PopFont();

			Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4((Calculator.TapCount > 0) ? ImGuiCol_Text : ImGuiCol_TextDisabled));
			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetStyleColorVec4(resetDown ? ImGuiCol_ButtonActive : ImGuiCol_Button));
			if (resetPressed | Gui::Button(UI_Str("ACT_TEMPO_CALCULATOR_RESET"), vec2(-1.0f, Gui::GetFrameHeightWithSpacing() * 1.0f)))
			{
				if (Calculator.TapCount > 0)
					context.SfxVoicePool.PlaySound(SoundEffectType::MetronomeBar);
				Calculator.Reset();
			}
			Gui::PopStyleColor(2);
		}
		Gui::PopStyleColor(3);
		Gui::PopStyleVar(1);

		Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Medium));
		if (Gui::BeginTable("Table", 2, ImGuiTableFlags_BordersInner | ImGuiTableFlags_NoSavedSettings, Gui::GetContentRegionAvail()))
		{
			cstr formatStrBPM_g = (Calculator.TapCount > 1) ? "%g BPM" : "--.-- BPM";
			cstr formatStrBPM_2f = (Calculator.TapCount > 1) ? "%.2f BPM" : "--.-- BPM";
			static constexpr auto row = [&](auto funcLeft, auto funcRight)
			{
				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); funcLeft();
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); Gui::SetNextItemWidth(-1.0f); funcRight();
			};
			row([&] { Gui::TextUnformatted(UI_Str("LABEL_TEMPO_CALCULATOR_NEAREST_WHOLE")); }, [&]
			{
				f32 v = Round(Calculator.LastTempo.BPM);
				Gui::InputFloat("##NearestWhole", &v, 0.0f, 0.0f, formatStrBPM_g, ImGuiInputTextFlags_ReadOnly);
			});
			row([&] { Gui::TextUnformatted(UI_Str("LABEL_TEMPO_CALCULATOR_NEAREST")); }, [&]
			{
				f32 v = Calculator.LastTempo.BPM;
				Gui::InputFloat("##Nearest", &v, 0.0f, 0.0f, formatStrBPM_2f, ImGuiInputTextFlags_ReadOnly);
			});
			row([&] { Gui::TextUnformatted(UI_Str("LABEL_TEMPO_CALCULATOR_MIN_AND_MAX")); }, [&]
			{
				f32 v[2] = { Calculator.LastTempoMin.BPM, Calculator.LastTempoMax.BPM };
				Gui::InputFloat2("##MinMax", v, formatStrBPM_2f, ImGuiInputTextFlags_ReadOnly);
			});
			row([&] { Gui::TextUnformatted(UI_Str("LABEL_TEMPO_CALCULATOR_TAPS")); }, [&]
			{
				Gui::Text((Calculator.TapCount == 1) ? UI_Str("INFO_TEMPO_CALCULATOR_TAPS_FIRST_BEAT") : UI_Str("INFO_TEMPO_CALCULATOR_TAPS_FMT_%d_TAPS"), Calculator.TapCount);
			});
			// NOTE: ReadOnly InputFloats specifically to allow easy copy and paste
			Gui::EndTable();
		}
		Gui::PopFont();
	}

	void ChartInspectorWindow::DrawGui(ChartContext& context)
	{
		Gui::UpdateSmoothScrollWindow();

		assert(context.ChartSelectedCourse != nullptr);
		auto& chart = context.Chart;
		auto& course = *context.ChartSelectedCourse;

		if (Gui::CollapsingHeader(UI_Str("DETAILS_INSPECTOR_SELECTED_ITEMS"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			defer { SelectedItems.clear(); };
			BeatSortedForwardIterator<TempoChange> scrollTempoChangeIt {};
			ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it)
			{
				TempChartItem& out = SelectedItems.emplace_back();
				out.List = it.List;
				out.Index = it.Index;
				out.AvailableMembers = GetAvailableMemberFlags(it.List);

				for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
				{
					if (out.AvailableMembers & EnumToFlag(member))
					{
						const b8 success = TryGet(course, it.List, it.Index, member, out.MemberValues[member]);
						assert(success);
					}
				}

				if (it.List == GenericList::ScrollChanges)
					out.BaseScrollTempo = TempoOrDefault(scrollTempoChangeIt.Next(course.TempoMap.Tempo.Sorted, out.MemberValues.BeatStart()));
			});

			GenericMemberFlags commonAvailableMemberFlags = SelectedItems.empty() ? GenericMemberFlags_None : GenericMemberFlags_All;
			GenericList commonListType = SelectedItems.empty() ? GenericList::Count : SelectedItems[0].List;
			size_t perListSelectionCounts[EnumCount<GenericList>] = {};
			{

				for (const TempChartItem& item : SelectedItems)
				{
					commonAvailableMemberFlags &= item.AvailableMembers;
					perListSelectionCounts[EnumToIndex(item.List)]++;
					if (item.List != commonListType)
						commonListType = GenericList::Count;
				}
			}

			GenericMemberFlags commonEqualMemberFlags = commonAvailableMemberFlags;
			AllGenericMembersUnionArray sharedValues = {};
			AllGenericMembersUnionArray mixedValuesMin = {};
			AllGenericMembersUnionArray mixedValuesMax = {};
			if (!SelectedItems.empty())
			{
				for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
				{
					sharedValues[member] = SelectedItems[0].MemberValues[member];
					mixedValuesMin[member] = sharedValues[member];
					mixedValuesMax[member] = sharedValues[member];
				}

				for (const TempChartItem& item : SelectedItems)
				{
					for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
					{
						if ((commonAvailableMemberFlags & EnumToFlag(member)) && item.MemberValues[member] != sharedValues[member])
							commonEqualMemberFlags &= ~EnumToFlag(member);

						const auto& v = item.MemberValues[member];
						auto& min = mixedValuesMin[member];
						auto& max = mixedValuesMax[member];
						switch (member)
						{
						case GenericMember::B8_IsSelected: { /* ... */ } break;
						case GenericMember::B8_BarLineVisible: { /* ... */ } break;
						case GenericMember::I16_BalloonPopCount: { min.I16 = Min(min.I16, v.I16); max.I16 = Max(max.I16, v.I16); } break;
						case GenericMember::F32_ScrollSpeed: {
							min.CPX.SetRealPart(Min(min.CPX.GetRealPart(), v.CPX.GetRealPart()));
							min.CPX.SetImaginaryPart(Min(min.CPX.GetImaginaryPart(), v.CPX.GetImaginaryPart()));
							max.CPX.SetRealPart(Max(max.CPX.GetRealPart(), v.CPX.GetRealPart()));
							max.CPX.SetImaginaryPart(Max(max.CPX.GetImaginaryPart(), v.CPX.GetImaginaryPart()));
						} break;
						case GenericMember::Beat_Start: { min.Beat = Min(min.Beat, v.Beat); max.Beat = Max(max.Beat, v.Beat); } break;
						case GenericMember::Beat_Duration: { min.Beat = Min(min.Beat, v.Beat); max.Beat = Max(max.Beat, v.Beat); } break;
						case GenericMember::Time_Offset: { min.Time = Min(min.Time, v.Time); max.Time = Max(max.Time, v.Time); } break;
						case GenericMember::NoteType_V:
						{
							min.NoteType = static_cast<NoteType>(Min(EnumToIndex(min.NoteType), EnumToIndex(v.NoteType)));
							max.NoteType = static_cast<NoteType>(Max(EnumToIndex(max.NoteType), EnumToIndex(v.NoteType)));
						} break;
						case GenericMember::Tempo_V: { min.Tempo.BPM = Min(min.Tempo.BPM, v.Tempo.BPM); max.Tempo.BPM = Max(max.Tempo.BPM, v.Tempo.BPM); } break;
						case GenericMember::TimeSignature_V:
						{
							min.TimeSignature.Numerator = Min(min.TimeSignature.Numerator, v.TimeSignature.Numerator);
							max.TimeSignature.Numerator = Max(max.TimeSignature.Numerator, v.TimeSignature.Numerator);
							min.TimeSignature.Denominator = Min(min.TimeSignature.Denominator, v.TimeSignature.Denominator);
							max.TimeSignature.Denominator = Max(max.TimeSignature.Denominator, v.TimeSignature.Denominator);
						} break;
						case GenericMember::CStr_Lyric: { /* ... */ } break;
						case GenericMember::I8_ScrollType: { /* ... */ } break;
						case GenericMember::F32_JPOSScroll: { /* ... */ } break;
						case GenericMember::F32_JPOSScrollDuration: { /* ... */ } break;
						default: assert(false); break;
						}
					}
				}
			}

			if (SelectedItems.empty())
			{
				Gui::PushFont(FontMain, GuiScaleI32_AtTarget(FontBaseSizes::Large));
				Gui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_TextDisabled));
				Gui::PushStyleColor(ImGuiCol_Button, 0x00000000);
				Gui::Button(UI_Str("INFO_INSPECTOR_NOTHING_SELECTED"), { Gui::GetContentRegionAvail().x, Gui::GetFrameHeight() * 2.0f });
				Gui::PopStyleColor(2);
				Gui::PopItemFlag();
				Gui::PopFont();
			}
			else
			{
				if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner))
				{
					const cstr listTypeNames[] = { UI_Str("SELECTED_EVENTS_TEMPOS"), UI_Str("SELECTED_EVENTS_TIME_SIGNATURES"), UI_Str("EVENT_NOTES"), UI_Str("EVENT_NOTES"), UI_Str("EVENT_NOTES"), UI_Str("SELECTED_EVENTS_SCROLL_SPEEDS"), UI_Str("SELECTED_EVENTS_BAR_LINE_VISIBILITIES"), UI_Str("SELECTED_EVENTS_GO_GO_RANGES"), UI_Str("EVENT_LYRICS"), UI_Str("SELECTED_EVENTS_SCROLL_TYPES"), UI_Str("SELECTED_EVENTS_JPOS_SCROLLS"), UI_Str("EVENT_DELAY"), };
					static_assert(ArrayCount(listTypeNames) == EnumCount<GenericList>);

					Gui::Property::Property([&]
					{
						std::string_view selectedListName = (commonListType < GenericList::Count) ? listTypeNames[EnumToIndex(commonListType)] : UI_Str("SELECTED_EVENTS_ITEMS");
						if (SelectedItems.size() == 1 && ASCII::EndsWith(selectedListName, 's'))
							selectedListName = selectedListName.substr(0, selectedListName.size() - sizeof('s'));

						Gui::AlignTextToFramePadding();
						Gui::Text("%s%.*s: %zu", UI_Str("INFO_INSPECTOR_SELECTED_ITEM"), FmtStrViewArgs(selectedListName), SelectedItems.size());
					});
					Gui::Property::Value([&]()
					{
						const TimeSpace displaySpace = *Settings.General.DisplayTimeInSongSpace ? TimeSpace::Song : TimeSpace::Chart;
						auto chartToDisplaySpace = [&](Time v) -> Time { return ConvertTimeSpace(v, TimeSpace::Chart, displaySpace, context.Chart); };

						// TODO: Also account for + Beat_Duration (?)
						Gui::AlignTextToFramePadding();
						if (SelectedItems.size() == 1)
							Gui::TextDisabled("%s",
								chartToDisplaySpace(context.BeatToTime(mixedValuesMin.BeatStart())).ToString().Data);
						else
							Gui::TextDisabled("(%s ... %s)",
								chartToDisplaySpace(context.BeatToTime(mixedValuesMin.BeatStart())).ToString().Data,
								chartToDisplaySpace(context.BeatToTime(mixedValuesMax.BeatStart())).ToString().Data);
					});

					if (commonListType >= GenericList::Count)
					{
						Gui::Property::PropertyTextValueFunc(UI_Str("ACT_SELECTION_REFINE"), [&]
						{
							for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
							{
								if (perListSelectionCounts[EnumToIndex(list)] == 0)
									continue;

								char buttonName[64];
								sprintf_s(buttonName, "[ %s ]  x%zu", listTypeNames[EnumToIndex(list)], perListSelectionCounts[EnumToIndex(list)]);
								if (list == GenericList::Notes_Master) { strcat_s(buttonName, " (Master)"); }
								if (list == GenericList::Notes_Expert) { strcat_s(buttonName, " (Expert)"); }

								Gui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.0f, 0.0f });
								Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_FrameBg));
								Gui::PushStyleColor(ImGuiCol_ButtonHovered, Gui::GetColorU32(ImGuiCol_FrameBgHovered));
								Gui::PushStyleColor(ImGuiCol_ButtonActive, Gui::GetColorU32(ImGuiCol_FrameBgActive));
								if (Gui::Button(buttonName, vec2(-1.0f, 0.0f)))
								{
									for (const auto& selectedItem : SelectedItems)
									{
										if (selectedItem.List != list)
											TrySet<GenericMember::B8_IsSelected>(course, selectedItem.List, selectedItem.Index, false);
									}
								}
								Gui::PopStyleColor(3);
								Gui::PopStyleVar(1);
							}
						});
					}

					b8 disableChangePropertiesCommandMerge = false;
					GenericMemberFlags outModifiedMembers = GenericMemberFlags_None;
					for (const GenericMember member : { GenericMember::NoteType_V, GenericMember::I16_BalloonPopCount, GenericMember::Time_Offset,
						GenericMember::Tempo_V, GenericMember::TimeSignature_V, GenericMember::F32_ScrollSpeed, GenericMember::B8_BarLineVisible,
						GenericMember::I8_ScrollType, GenericMember::F32_JPOSScroll, GenericMember::F32_JPOSScrollDuration})
					{
						if (!(commonAvailableMemberFlags & EnumToFlag(member)))
							continue;

						b8 valueWasChanged = false;
						switch (member)
						{
						case GenericMember::B8_BarLineVisible:
						{
							Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_PROP_BAR_LINE_VISIBLE"), [&]
							{
								enum class VisibilityType { Visible, Hidden, Count };
								const cstr visibilityTypeNames[] = { UI_Str("BAR_LINE_VISIBILITY_VISIBLE"), UI_Str("BAR_LINE_VISIBILITY_HIDDEN"), };
								auto v = !(commonEqualMemberFlags & EnumToFlag(member)) ? VisibilityType::Count : sharedValues.BarLineVisible() ? VisibilityType::Visible : VisibilityType::Hidden;

								Gui::PushItemWidth(-1.0f);
								if (Gui::ComboEnum("##BarLineVisible", &v, visibilityTypeNames, ImGuiComboFlags_None))
								{
									for (auto& selectedItem : SelectedItems)
									{
										auto& isVisible = selectedItem.MemberValues.BarLineVisible();
										isVisible = (v == VisibilityType::Visible) ? true : (v == VisibilityType::Hidden) ? false : isVisible;
									}
									valueWasChanged = true;
									disableChangePropertiesCommandMerge = true;
								}
							});
						} break;
						case GenericMember::I16_BalloonPopCount:
						{
							b8 isAnyBalloonNoteSelected = false;
							for (const auto& selectedItem : SelectedItems)
								isAnyBalloonNoteSelected |= IsBalloonNote(selectedItem.MemberValues.NoteType());

							MultiEditWidgetParam widgetIn = {};
							widgetIn.DataType = ImGuiDataType_S16;
							widgetIn.Value.I16 = sharedValues.BalloonPopCount();
							widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
							widgetIn.MixedValuesMin.I16 = mixedValuesMin.BalloonPopCount();
							widgetIn.MixedValuesMax.I16 = mixedValuesMax.BalloonPopCount();
							widgetIn.EnableStepButtons = true;
							widgetIn.ButtonStep.I16 = 1;
							widgetIn.ButtonStepFast.I16 = 4;
							widgetIn.EnableDragLabel = false;
							widgetIn.FormatString = "%d";
							widgetIn.EnableDragLabel = true;
							widgetIn.DragLabelSpeed = 0.05f;
							Gui::BeginDisabled(!isAnyBalloonNoteSelected);
							const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(UI_Str("EVENT_PROP_BALLOON_POP_COUNT"), widgetIn);
							Gui::EndDisabled();

							if (widgetOut.HasValueExact)
							{
								for (auto& selectedItem : SelectedItems)
								{
									if (IsBalloonNote(selectedItem.MemberValues.NoteType()))
										selectedItem.MemberValues.BalloonPopCount() = Clamp(widgetOut.ValueExact.I16, MinBalloonCount, MaxBalloonCount);
								}
								valueWasChanged = true;
							}
							else if (widgetOut.HasValueIncrement)
							{
								for (auto& selectedItem : SelectedItems)
								{
									if (IsBalloonNote(selectedItem.MemberValues.NoteType()))
										selectedItem.MemberValues.BalloonPopCount() = Clamp(static_cast<i16>(selectedItem.MemberValues.BalloonPopCount() + widgetOut.ValueIncrement.I16), MinBalloonCount, MaxBalloonCount);
								}
								valueWasChanged = true;
							}
						} break;
						case GenericMember::F32_JPOSScroll:
						{
							b8 areAllJPOSScrollMovesTheSame = (commonEqualMemberFlags & EnumToFlag(member));

							for (size_t i = 0; i < 2; i++)
							{
								MultiEditWidgetParam widgetIn = {};
								widgetIn.EnableStepButtons = true;
								if (i == 0)
								{
									widgetIn.Value.F32 = sharedValues.JPOSScrollMove().GetRealPart();
									widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
									widgetIn.MixedValuesMin.F32 = mixedValuesMin.JPOSScrollMove().GetRealPart();
									widgetIn.MixedValuesMax.F32 = mixedValuesMax.JPOSScrollMove().GetRealPart();
									widgetIn.ButtonStep.F32 = 1.f;
									widgetIn.ButtonStepFast.F32 = 5.f;
									widgetIn.DragLabelSpeed = 0.5f;
									widgetIn.FormatString = "%gpx";
									widgetIn.EnableClamp = true;
									widgetIn.ValueClampMin.F32 = MinJPOSScrollMove;
									widgetIn.ValueClampMax.F32 = MaxJPOSScrollMove;
								}
								else {
									widgetIn.Value.F32 = sharedValues.JPOSScrollMove().GetImaginaryPart();
									widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
									widgetIn.MixedValuesMin.F32 = mixedValuesMin.JPOSScrollMove().GetImaginaryPart();
									widgetIn.MixedValuesMax.F32 = mixedValuesMax.JPOSScrollMove().GetImaginaryPart();
									widgetIn.ButtonStep.F32 = 1.f;
									widgetIn.ButtonStepFast.F32 = 5.f;
									widgetIn.DragLabelSpeed = 0.5f;
									widgetIn.FormatString = "%gipx";
									widgetIn.EnableClamp = true;
									widgetIn.ValueClampMin.F32 = MinJPOSScrollMove;
									widgetIn.ValueClampMax.F32 = MaxJPOSScrollMove;
								}

								const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(
									(i == 0)
									? UI_Str("EVENT_PROP_JPOS_SCROLL_MOVE")
									: UI_Str("EVENT_PROP_VERTICAL_JPOS_SCROLL_MOVE")
									, widgetIn);
								if (widgetOut.HasValueExact)
								{
									for (auto& selectedItem : SelectedItems)
									{
										if (i == 0)
											selectedItem.MemberValues.JPOSScrollMove().SetRealPart(widgetOut.ValueExact.F32);
										else
											selectedItem.MemberValues.JPOSScrollMove().SetImaginaryPart(widgetOut.ValueExact.F32);
									}
									valueWasChanged = true;
								}
								else if (widgetOut.HasValueIncrement)
								{
									for (auto& selectedItem : SelectedItems)
									{
										if (i == 0)
											selectedItem.MemberValues.JPOSScrollMove().SetRealPart(Clamp(selectedItem.MemberValues.JPOSScrollMove().GetRealPart() + widgetOut.ValueIncrement.F32, MinJPOSScrollMove, MaxJPOSScrollMove));
										else
											selectedItem.MemberValues.JPOSScrollMove().SetImaginaryPart(Clamp(selectedItem.MemberValues.JPOSScrollMove().GetImaginaryPart() + widgetOut.ValueIncrement.F32, MinJPOSScrollMove, MaxJPOSScrollMove));
									}
									valueWasChanged = true;
								}
							}
						} break;
						case GenericMember::F32_JPOSScrollDuration:
						{
							b8 areAllJPOSScrollDurationsTheSame = true;
							f32 commonDuration = 0.f, minDuration = 0.f, maxDuration = 0.f;
							for (const auto& selectedItem : SelectedItems)
							{
								const f32 duration = selectedItem.MemberValues.JPOSScrollDuration();
								if (&selectedItem == &SelectedItems[0])
								{
									commonDuration = minDuration = maxDuration = duration;
								}
								else
								{
									minDuration = Min(minDuration, duration);
									maxDuration = Max(maxDuration, duration);
									areAllJPOSScrollDurationsTheSame &= ApproxmiatelySame(duration, commonDuration, 0.001f);
								}
							}

							{
								MultiEditWidgetParam widgetIn = {};
								widgetIn.EnableStepButtons = true;
								widgetIn.Value.F32 = commonDuration;
								widgetIn.HasMixedValues = !areAllJPOSScrollDurationsTheSame;
								widgetIn.MixedValuesMin.F32 = minDuration;
								widgetIn.MixedValuesMax.F32 = maxDuration;
								widgetIn.ButtonStep.F32 = 0.1f;
								widgetIn.ButtonStepFast.F32 = 0.5f;
								widgetIn.DragLabelSpeed = 0.005f;
								widgetIn.FormatString = "%gs";
								widgetIn.EnableClamp = true;
								widgetIn.ValueClampMin.F32 = MinJPOSScrollDuration;
								widgetIn.ValueClampMax.F32 = MaxJPOSScrollDuration;

								const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(
									UI_Str("EVENT_PROP_JPOS_SCROLL_DURATION"),
									widgetIn);
								if (widgetOut.HasValueExact)
								{
									for (auto& selectedItem : SelectedItems)
									{
										selectedItem.MemberValues.JPOSScrollDuration() = widgetOut.ValueExact.F32;
									}
									valueWasChanged = true;
								}
								else if (widgetOut.HasValueIncrement)
								{
									for (auto& selectedItem : SelectedItems)
									{
										selectedItem.MemberValues.JPOSScrollDuration() = Clamp(selectedItem.MemberValues.JPOSScrollDuration() + widgetOut.ValueIncrement.F32, MinJPOSScrollDuration, MaxJPOSScrollDuration);
									}
									valueWasChanged = true;
								}
							}
						} break;
						case GenericMember::F32_ScrollSpeed:
						{
							b8 areAllScrollSpeedsTheSame = (commonEqualMemberFlags & EnumToFlag(member));
							b8 areAllScrollTemposTheSame = true;
							Tempo commonScrollTempo = {}, minScrollTempo {}, maxScrollTempo {};
							for (const auto& selectedItem : SelectedItems)
							{
								const Tempo scrollTempo = ScrollSpeedToTempo(selectedItem.MemberValues.ScrollSpeed().GetRealPart(), selectedItem.BaseScrollTempo);
								if (&selectedItem == &SelectedItems[0])
								{
									commonScrollTempo = minScrollTempo = maxScrollTempo = scrollTempo;
								}
								else
								{
									minScrollTempo.BPM = Min(minScrollTempo.BPM, scrollTempo.BPM);
									maxScrollTempo.BPM = Max(maxScrollTempo.BPM, scrollTempo.BPM);
									areAllScrollTemposTheSame &= ApproxmiatelySame(scrollTempo.BPM, commonScrollTempo.BPM, 0.001f);
								}
							}

							for (size_t i = 0; i < 3; i++)
							{
								MultiEditWidgetParam widgetIn = {};
								widgetIn.EnableStepButtons = true;
								if (i == 0)
								{
									widgetIn.Value.F32 = sharedValues.ScrollSpeed().GetRealPart();
									widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
									widgetIn.MixedValuesMin.F32 = mixedValuesMin.ScrollSpeed().GetRealPart();
									widgetIn.MixedValuesMax.F32 = mixedValuesMax.ScrollSpeed().GetRealPart();
									widgetIn.ButtonStep.F32 = 0.1f;
									widgetIn.ButtonStepFast.F32 = 0.5f;
									widgetIn.DragLabelSpeed = 0.005f;
									widgetIn.FormatString = "%gx";
									widgetIn.EnableClamp = true;
									widgetIn.ValueClampMin.F32 = MinScrollSpeed;
									widgetIn.ValueClampMax.F32 = MaxScrollSpeed;
								}
								else if (i == 1) {
									widgetIn.Value.F32 = sharedValues.ScrollSpeed().GetImaginaryPart();
									widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
									widgetIn.MixedValuesMin.F32 = mixedValuesMin.ScrollSpeed().GetImaginaryPart();
									widgetIn.MixedValuesMax.F32 = mixedValuesMax.ScrollSpeed().GetImaginaryPart();
									widgetIn.ButtonStep.F32 = 0.1f;
									widgetIn.ButtonStepFast.F32 = 0.5f;
									widgetIn.DragLabelSpeed = 0.005f;
									widgetIn.FormatString = "%gix";
									widgetIn.EnableClamp = true;
									widgetIn.ValueClampMin.F32 = MinScrollSpeed;
									widgetIn.ValueClampMax.F32 = MaxScrollSpeed;
								}
								else
								{
									widgetIn.Value.F32 = commonScrollTempo.BPM;
									widgetIn.HasMixedValues = !areAllScrollTemposTheSame;
									widgetIn.MixedValuesMin.F32 = minScrollTempo.BPM;
									widgetIn.MixedValuesMax.F32 = maxScrollTempo.BPM;
									widgetIn.ButtonStep.F32 = 1.0f;
									widgetIn.ButtonStepFast.F32 = 10.0f;
									widgetIn.DragLabelSpeed = 1.0f;
									widgetIn.FormatString = "%g BPM";
									widgetIn.EnableClamp = true;
									widgetIn.ValueClampMin.F32 = MinBPM;
									widgetIn.ValueClampMax.F32 = MaxBPM;
								}
								const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(
									(i == 0) 
										? UI_Str("EVENT_SCROLL_SPEED") 
										: (i == 1)
											? UI_Str("EVENT_PROP_VERTICAL_SCROLL_SPEED")
											: UI_Str("EVENT_PROP_SCROLL_SPEED_TEMPO")
									, widgetIn);
								if (widgetOut.HasValueExact)
								{
									for (auto& selectedItem : SelectedItems)
									{
										if (i == 0)
											selectedItem.MemberValues.ScrollSpeed().SetRealPart(widgetOut.ValueExact.F32);
										else if (i == 1)
											selectedItem.MemberValues.ScrollSpeed().SetImaginaryPart(widgetOut.ValueExact.F32);
										else
											selectedItem.MemberValues.ScrollSpeed().SetRealPart(ScrollTempoToSpeed(Tempo(widgetOut.ValueExact.F32), selectedItem.BaseScrollTempo));
									}
									valueWasChanged = true;
								}
								else if (widgetOut.HasValueIncrement)
								{
									for (auto& selectedItem : SelectedItems)
									{
										if (i == 0)
											selectedItem.MemberValues.ScrollSpeed().SetRealPart(Clamp(selectedItem.MemberValues.ScrollSpeed().GetRealPart() + widgetOut.ValueIncrement.F32, MinScrollSpeed, MaxScrollSpeed));
										else if (i == 1)
											selectedItem.MemberValues.ScrollSpeed().SetImaginaryPart(Clamp(selectedItem.MemberValues.ScrollSpeed().GetImaginaryPart() + widgetOut.ValueIncrement.F32, MinScrollSpeed, MaxScrollSpeed));
										else if (selectedItem.BaseScrollTempo.BPM != 0.0f)
										{
											const Tempo oldScrollTempo = ScrollSpeedToTempo(selectedItem.MemberValues.ScrollSpeed().GetRealPart(), selectedItem.BaseScrollTempo);
											const Tempo newScrollTempo = Tempo(oldScrollTempo.BPM + widgetOut.ValueIncrement.F32);
											selectedItem.MemberValues.ScrollSpeed().SetRealPart(ScrollTempoToSpeed(Tempo(Clamp(newScrollTempo.BPM, MinBPM, MaxBPM)), selectedItem.BaseScrollTempo));
										}
									}
									valueWasChanged = true;
								}
							}

							for (size_t i = 0; i < 3; i++)
							{
								// TODO: Maybe option to switch between Beat/Time interpolation modes (?)
								static constexpr auto getT = [](const TempChartItem& item) -> f64 { return static_cast<f64>(item.MemberValues.BeatStart().Ticks); };
								static constexpr auto getInterpolatedScrollSpeed = [](const TempChartItem& startItem, const TempChartItem& endItem, const TempChartItem& thisItem, f32 startValue, f32 endValue) -> f32
								{
									return static_cast<f32>(ConvertRange<f64>(getT(startItem), getT(endItem), startValue, endValue, getT(thisItem)));
								};

								TempChartItem* startItem = !SelectedItems.empty() ? &SelectedItems.front() : nullptr;
								TempChartItem* endItem = !SelectedItems.empty() ? &SelectedItems.back() : nullptr;
								f32 inOutStartEnd[2] =
								{
									(i == 0)
										? startItem->MemberValues.ScrollSpeed().GetRealPart()
										: (i == 1)
											? startItem->MemberValues.ScrollSpeed().GetImaginaryPart()
											: ScrollSpeedToTempo(startItem->MemberValues.ScrollSpeed().GetRealPart(), startItem->BaseScrollTempo).BPM,
									(i == 0)
										? endItem->MemberValues.ScrollSpeed().GetRealPart()
										: (i == 1)
											? endItem->MemberValues.ScrollSpeed().GetImaginaryPart()
											: ScrollSpeedToTempo(endItem->MemberValues.ScrollSpeed().GetRealPart(), endItem->BaseScrollTempo).BPM
								};

								const b8 isSelectionTooSmall = (SelectedItems.size() < 2);
								b8 isSelectionAlreadyInterpolated = true;
								if (!isSelectionTooSmall)
								{
									for (const auto& thisItem : SelectedItems)
									{
										const f32 thisValue = getInterpolatedScrollSpeed(*startItem, *endItem, thisItem, inOutStartEnd[0], inOutStartEnd[1]);
										if (!(
											(i == 0 || i == 2)
											? ApproxmiatelySame(thisItem.MemberValues.ScrollSpeed().GetRealPart(), 
												(i == 0) 
													? thisValue 
													: ScrollTempoToSpeed(Tempo(thisValue), thisItem.BaseScrollTempo))
											: ApproxmiatelySame(thisItem.MemberValues.ScrollSpeed().GetImaginaryPart(), thisValue)
											))
											isSelectionAlreadyInterpolated = false;
									}
								}

								// NOTE: Invalid selection		-> "..." dummied out
								//		 All the same			-> "=" marker
								//		 Mixed values			-> "()" values
								//		 Already interpolated	-> regular values
								char previewBuffersStartEnd[2][64]; cstr previewStrings[2] = { nullptr, nullptr };
								if (isSelectionTooSmall)
								{
									previewStrings[0] = "...";
									previewStrings[1] = "...";
								}
								else if ((i != 2) ? areAllScrollSpeedsTheSame : areAllScrollTemposTheSame)
								{
									previewStrings[1] = "=";
								}
								else if (!isSelectionAlreadyInterpolated)
								{
									previewStrings[0] = previewBuffersStartEnd[0]; sprintf_s(previewBuffersStartEnd[0], 
										(i == 0) 
										? "(%gx)" 
										: (i == 1) 
										? "(%gix)" 
										:"(%g BPM)"
										, inOutStartEnd[0]);
									previewStrings[1] = previewBuffersStartEnd[1]; sprintf_s(previewBuffersStartEnd[1], 
										(i == 0)
										? "(%gx)"
										: (i == 1)
										? "(%gix)"
										: "(%g BPM)"
										, inOutStartEnd[1]);
								}

								Gui::BeginDisabled(isSelectionTooSmall);
								if ((i == 0) 
									? GuiPropertyRangeInterpolationEditWidget(UI_Str("EVENT_PROP_INTERPOLATE_SCROLL_SPEED"), inOutStartEnd, 0.1f, 0.5f, MinScrollSpeed, MaxScrollSpeed, "%gx", previewStrings)
									: (i == 1)
									? GuiPropertyRangeInterpolationEditWidget(UI_Str("EVENT_PROP_INTERPOLATE_VERTICAL_SCROLL_SPEED"), inOutStartEnd, 0.1f, 0.5f, MinScrollSpeed, MaxScrollSpeed, "%gix", previewStrings)
									: GuiPropertyRangeInterpolationEditWidget(UI_Str("EVENT_PROP_INTERPOLATE_SCROLL_SPEED_TEMPO"), inOutStartEnd, 1.0f, 10.0f, MinBPM, MaxBPM, "%g BPM", previewStrings))
								{
									for (auto& thisItem : SelectedItems)
									{
										const f32 thisValue = getInterpolatedScrollSpeed(*startItem, *endItem, thisItem, inOutStartEnd[0], inOutStartEnd[1]);
										if (i == 0 || i == 2)
											thisItem.MemberValues.ScrollSpeed().SetRealPart((i == 0) ? thisValue : ScrollTempoToSpeed(Tempo(thisValue), thisItem.BaseScrollTempo));
										else
											thisItem.MemberValues.ScrollSpeed().SetImaginaryPart(thisValue);
									}
									valueWasChanged = true;
								}
								Gui::EndDisabled();
							}
						} break;
						case GenericMember::Time_Offset:
						{
							const b8 isDelayEdit = (commonListType == GenericList::DelayChanges);
							MultiEditWidgetParam widgetIn = {};
							widgetIn.EnableStepButtons = true;
							widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
							widgetIn.EnableDragLabel = true;
							if (isDelayEdit)
							{
								widgetIn.Value.F32 = sharedValues.TimeOffset().ToSec_F32();
								widgetIn.MixedValuesMin.F32 = mixedValuesMin.TimeOffset().ToSec_F32();
								widgetIn.MixedValuesMax.F32 = mixedValuesMax.TimeOffset().ToSec_F32();
								widgetIn.ButtonStep.F32 = 0.01f;
								widgetIn.ButtonStepFast.F32 = 0.1f;
								widgetIn.DragLabelSpeed = 0.01f;
								widgetIn.FormatString = "%g sec";
							}
							else
							{
								widgetIn.Value.F32 = sharedValues.TimeOffset().ToMS_F32();
								widgetIn.MixedValuesMin.F32 = mixedValuesMin.TimeOffset().ToMS_F32();
								widgetIn.MixedValuesMax.F32 = mixedValuesMax.TimeOffset().ToMS_F32();
								widgetIn.ButtonStep.F32 = 1.0f;
								widgetIn.ButtonStepFast.F32 = 5.0f;
								widgetIn.DragLabelSpeed = 1.0f;
								widgetIn.FormatString = "%g ms";
							}
							const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(isDelayEdit ? UI_Str("EVENT_DELAY") : UI_Str("EVENT_PROP_TIME_OFFSET"), widgetIn);
							if (widgetOut.HasValueExact || widgetOut.HasValueIncrement)
							{
								if (widgetOut.HasValueExact)
								{
									const Time valueExact = isDelayEdit
										? Time::FromSec(widgetOut.ValueExact.F32)
										: Clamp(Time::FromMS(widgetOut.ValueExact.F32), MinNoteTimeOffset, MaxNoteTimeOffset);
									for (auto& selectedItem : SelectedItems)
										selectedItem.MemberValues.TimeOffset() = valueExact;
									valueWasChanged = true;
								}
								else if (widgetOut.HasValueIncrement)
								{
									const Time valueIncrement = isDelayEdit
										? Time::FromSec(widgetOut.ValueIncrement.F32)
										: Time::FromMS(widgetOut.ValueIncrement.F32);
									for (auto& selectedItem : SelectedItems)
									{
										Time newValue = selectedItem.MemberValues.TimeOffset() + valueIncrement;
										if (!isDelayEdit)
											newValue = Clamp(newValue, MinNoteTimeOffset, MaxNoteTimeOffset);
										selectedItem.MemberValues.TimeOffset() = newValue;
									}
									valueWasChanged = true;
								}

								for (auto& selectedItem : SelectedItems)
								{
									if (ApproxmiatelySame(selectedItem.MemberValues.TimeOffset().Seconds, 0.0))
										selectedItem.MemberValues.TimeOffset() = Time::Zero();
								}
							}
						} break;
						case GenericMember::NoteType_V:
						{
							b8 isAnyRegularNoteSelected = false, isAnyDrumrollNoteSelected = false, isAnyBalloonNoteSelected = false;
							b8 areAllSelectedNotesSmall = true, areAllSelectedNotesBig = true, areAllSelectedNotesHand = true;
							b8 perNoteTypeHasAtLeastOneSelected[EnumCount<NoteType>] = {};
							for (const auto& selectedItem : SelectedItems)
							{
								const auto noteType = selectedItem.MemberValues.NoteType();
								isAnyRegularNoteSelected |= IsRegularNote(noteType);
								isAnyDrumrollNoteSelected |= IsDrumrollNote(noteType);
								isAnyBalloonNoteSelected |= IsBalloonNote(noteType);
								areAllSelectedNotesSmall &= IsSmallNote(noteType);
								areAllSelectedNotesBig &= IsBigNote(noteType);
								areAllSelectedNotesHand &= IsHandNote(noteType);
								perNoteTypeHasAtLeastOneSelected[EnumToIndex(noteType)] = true;
							}

							Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_PROP_NOTE_TYPE"), [&]
							{
								const cstr noteTypeNames[] = { UI_Str("NOTE_TYPE_DON"), UI_Str("NOTE_TYPE_DON_BIG"), UI_Str("NOTE_TYPE_KA"), UI_Str("NOTE_TYPE_KA_BIG"),
									UI_Str("NOTE_TYPE_DRUMROLL"), UI_Str("NOTE_TYPE_DRUMROLL_BIG"), UI_Str("NOTE_TYPE_BALLOON"), UI_Str("NOTE_TYPE_BALLOON_EX"),
									UI_Str("NOTE_TYPE_DON_HAND"), UI_Str("NOTE_TYPE_KA_HAND"),
									UI_Str("NOTE_TYPE_KADON"), UI_Str("NOTE_TYPE_BOMB"), UI_Str("NOTE_TYPE_ADLIB"), UI_Str("NOTE_TYPE_FUSEROLL"),
								};

								static_assert(ArrayCount(noteTypeNames) == EnumCount<NoteType>);

								const b8 isSingleNoteType = (commonEqualMemberFlags & EnumToFlag(member));
								char previewValue[256] {}; if (!isSingleNoteType) { previewValue[0] = '('; }
								for (i32 i = 0; i < EnumCountI32<NoteType>; i++) { if (perNoteTypeHasAtLeastOneSelected[i]) { strcat_s(previewValue, noteTypeNames[i]); strcat_s(previewValue, ", "); } }
								for (i32 i = ArrayCountI32(previewValue) - 1; i >= 0; i--) { if (previewValue[i] == ',') { if (!isSingleNoteType) { previewValue[i++] = ')'; } previewValue[i] = '\0'; break; } }

								Gui::SetNextItemWidth(-1.0f);
								Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_Text, (commonEqualMemberFlags & EnumToFlag(member)) ? 1.0f : 0.6f));
								const b8 beginCombo = Gui::BeginCombo("##NoteType", previewValue, ImGuiComboFlags_None);
								Gui::PopStyleColor();
								if (beginCombo)
								{
									for (NoteType i = {}; i < NoteType::Count; IncrementEnum(i))
									{
										const b8 isSelected = perNoteTypeHasAtLeastOneSelected[EnumToIndex(i)];

										b8 isEnabled = false;
										isEnabled |= IsRegularNote(i) && isAnyRegularNoteSelected;
										isEnabled |= IsDrumrollNote(i) && isAnyDrumrollNoteSelected;
										isEnabled |= IsBalloonNote(i) && isAnyBalloonNoteSelected;

										if (Gui::Selectable(noteTypeNames[EnumToIndex(i)], isSelected, !isEnabled ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
										{
											for (auto& selectedItem : SelectedItems)
											{
												// NOTE: Don't allow changing long notes and balloons for now at that would also require updating BeatDuration / BalloonPopCount
												auto& inOutNoteType = selectedItem.MemberValues.NoteType();
												if (IsLongNote(i) == IsLongNote(inOutNoteType) && IsBalloonNote(i) == IsBalloonNote(inOutNoteType))
													inOutNoteType = i;
											}
											valueWasChanged = true;
											disableChangePropertiesCommandMerge = true;
										}

										if (isSelected)
											Gui::SetItemDefaultFocus();
									}
									Gui::EndCombo();
								}
							});

							Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_PROP_NOTE_TYPE_SIZE"), [&]
							{
								enum class NoteSizeType { Small, Big, Hand, Count };
								const cstr noteSizeTypeNames[] = { UI_Str("NOTE_TYPE_SIZE_SMALL"), UI_Str("NOTE_TYPE_SIZE_BIG"), UI_Str("NOTE_TYPE_SIZE_HAND"), };
								auto v = !(areAllSelectedNotesSmall || areAllSelectedNotesBig || areAllSelectedNotesHand) ? NoteSizeType::Count
									: IsHandNote(sharedValues.NoteType()) ? NoteSizeType::Hand
									: IsBigNote(sharedValues.NoteType()) ? NoteSizeType::Big
									: NoteSizeType::Small;

								Gui::PushItemWidth(-1.0f);
								if (Gui::ComboEnum("##NoteTypeIsBig", &v, noteSizeTypeNames, ImGuiComboFlags_None))
								{
									for (auto& selectedItem : SelectedItems)
									{
										auto& inOutNoteType = selectedItem.MemberValues.NoteType();
										inOutNoteType = (v == NoteSizeType::Hand) ? ToHandNote(inOutNoteType)
											: (v == NoteSizeType::Big) ? ToBigNote(inOutNoteType)
											: (v == NoteSizeType::Small) ? ToSmallNote(inOutNoteType)
											: inOutNoteType;
									}
									valueWasChanged = true;
									disableChangePropertiesCommandMerge = true;
								}
							});
						} break;
						case GenericMember::Tempo_V:
						{
							// TODO: X0.5/x2.0 buttons (?)
							MultiEditWidgetParam widgetIn = {};
							widgetIn.Value.F32 = sharedValues.Tempo().BPM;
							widgetIn.HasMixedValues = !(commonEqualMemberFlags & EnumToFlag(member));
							widgetIn.MixedValuesMin.F32 = mixedValuesMin.Tempo().BPM;
							widgetIn.MixedValuesMax.F32 = mixedValuesMax.Tempo().BPM;
							widgetIn.EnableStepButtons = true;
							widgetIn.ButtonStep.F32 = 1.0f;
							widgetIn.ButtonStepFast.F32 = 10.0f;
							widgetIn.DragLabelSpeed = 1.0f;
							widgetIn.FormatString = "%g BPM";
							widgetIn.EnableClamp = true;
							widgetIn.ValueClampMin.F32 = MinBPM;
							widgetIn.ValueClampMax.F32 = MaxBPM;
							const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(UI_Str("EVENT_TEMPO"), widgetIn);

							if (widgetOut.HasValueExact)
							{
								for (auto& selectedItem : SelectedItems)
									selectedItem.MemberValues.Tempo() = Tempo(widgetOut.ValueExact.F32);
								valueWasChanged = true;
							}
							else if (widgetOut.HasValueIncrement)
							{
								for (auto& selectedItem : SelectedItems)
									selectedItem.MemberValues.Tempo().BPM = Clamp(selectedItem.MemberValues.Tempo().BPM + widgetOut.ValueIncrement.F32, MinBPM, MaxBPM);
								valueWasChanged = true;
							}
						} break;
						case GenericMember::TimeSignature_V:
						{
							static constexpr i32 components = 2;

							b8 isAnyTimeSignatureInvalid = false;
							for (const auto& selectedItem : SelectedItems)
								isAnyTimeSignatureInvalid |= !IsTimeSignatureSupported(selectedItem.MemberValues.TimeSignature());

							MultiEditWidgetParam widgetIn = {};
							widgetIn.DataType = ImGuiDataType_S32;
							widgetIn.Components = components;
							widgetIn.EnableClamp = true;
							widgetIn.EnableStepButtons = true;
							widgetIn.UseSpinButtonsInstead = true;
							for (i32 c = 0; c < components; c++)
							{
								widgetIn.Value.I32_V[c] = sharedValues.TimeSignature()[c];
								widgetIn.HasMixedValues |= (static_cast<u32>(mixedValuesMin.TimeSignature()[c] != mixedValuesMax.TimeSignature()[c]) << c);
								widgetIn.MixedValuesMin.I32_V[c] = mixedValuesMin.TimeSignature()[c];
								widgetIn.MixedValuesMax.I32_V[c] = mixedValuesMax.TimeSignature()[c];
								widgetIn.ValueClampMin.I32_V[c] = MinTimeSignatureValue;
								widgetIn.ValueClampMax.I32_V[c] = MaxTimeSignatureValue;
								widgetIn.ButtonStep.I32_V[c] = 1;
								widgetIn.ButtonStepFast.I32_V[c] = 4;
							}
							widgetIn.EnableDragLabel = false;
							widgetIn.FormatString = "%d";
							widgetIn.TextColorOverride = isAnyTimeSignatureInvalid ? &InputTextWarningTextColor : nullptr;
							const MultiEditWidgetResult widgetOut = GuiPropertyMultiSelectionEditWidget(UI_Str("EVENT_TIME_SIGNATURE"), widgetIn);

							if (widgetOut.HasValueExact || widgetOut.HasValueIncrement)
							{
								for (i32 c = 0; c < components; c++)
								{
									if (widgetOut.HasValueExact & (1 << c))
									{
										for (auto& selectedItem : SelectedItems)
											selectedItem.MemberValues.TimeSignature()[c] = widgetOut.ValueExact.I32_V[c];
										valueWasChanged = true;
									}
									else if (widgetOut.HasValueIncrement & (1 << c))
									{
										for (auto& selectedItem : SelectedItems)
											selectedItem.MemberValues.TimeSignature()[c] = Clamp(selectedItem.MemberValues.TimeSignature()[c] + widgetOut.ValueIncrement.I32_V[c], MinTimeSignatureValue, MaxTimeSignatureValue);
										valueWasChanged = true;
									}
								}
							}
						} break;
						case GenericMember::I8_ScrollType:
						{
							Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_SCROLL_TYPE"), [&]
								{
									enum class ScrollMethods { NMSCROLL, HBSCROLL, BMSCROLL, Count };
									const cstr scrollTypeNames[] = { UI_Str("SCROLL_TYPE_NMSCROLL"), UI_Str("SCROLL_TYPE_HBSCROLL"), UI_Str("SCROLL_TYPE_BMSCROLL"), };
									const i16 selectedType = sharedValues.ScrollType();

									auto v = !(commonEqualMemberFlags & EnumToFlag(member))
										? ScrollMethods::Count
										: static_cast<ScrollMethods>(selectedType);

									Gui::PushItemWidth(-1.0f);
									if (Gui::ComboEnum("##ScrollType", &v, scrollTypeNames, ImGuiComboFlags_None))
									{
										for (auto& selectedItem : SelectedItems)
										{
											auto& scrollType = selectedItem.MemberValues.ScrollType();
												selectedItem.MemberValues.BarLineVisible();
											scrollType = static_cast<i16>(v);
										}
										valueWasChanged = true;
										disableChangePropertiesCommandMerge = true;
									}
								});
						} break;
						default: { assert(false); } break;
						}

						if (valueWasChanged)
							outModifiedMembers |= EnumToFlag(member);
					}

					if (outModifiedMembers != GenericMemberFlags_None)
					{
						std::vector<Commands::ChangeMultipleGenericProperties::Data> propertiesToChange;
						propertiesToChange.reserve(SelectedItems.size());

						for (const TempChartItem& selectedItem : SelectedItems)
						{
							for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
							{
								if (outModifiedMembers & EnumToFlag(member))
								{
									auto& out = propertiesToChange.emplace_back();
									out.Index = selectedItem.Index;
									out.List = selectedItem.List;
									out.Member = member;
									out.NewValue = selectedItem.MemberValues[member];
								}
							}
						}

						if (disableChangePropertiesCommandMerge)
							context.Undo.DisallowMergeForLastCommand();
						context.Undo.Execute<Commands::ChangeMultipleGenericProperties>(&course, std::move(propertiesToChange));
					}

					Gui::Property::EndTable();
				}
			}
		}
	}


	// ImGui #3565
	static f32 TableFullRowBegin()
	{
		ImGuiTable* table = Gui::GetCurrentTable();
		Gui::TableSetColumnIndex((table->LeftMostEnabledColumn >= 0) ? table->LeftMostEnabledColumn : 0);
		ImRect* workRect = &Gui::GetCurrentWindow()->WorkRect;
		f32 restore_x = workRect->Max.x;
		ImRect bgClipRect = table->BgClipRect;
		Gui::PushClipRect(bgClipRect.Min, bgClipRect.Max, false);
		workRect->Max.x = bgClipRect.Max.x;
		return restore_x;
	}

	static void TableFullRowEnd(f32 restore_x)
	{
		Gui::GetCurrentWindow()->WorkRect.Max.x = restore_x;
		Gui::PopClipRect();
	}

	static float getInsertButtonWidth()
	{
		return std::max(1.0f, ImGui::GetContentRegionAvail().x - 1 - Gui::GetStyle().ItemInnerSpacing.x - Gui::GetFrameHeight());
	}

	template <typename FCharFilter, typename FKeyFilter>
	static void PropertyMapCollapsingHeader(ChartContext& context, const char* label, std::map<std::string, std::string>& properties,
		const char* labelAdd, std::string* pNewKey, FCharFilter&& charFilter, FKeyFilter&& keyFilter, const std::string& newDefault)
	{
		ImGuiTable* table = Gui::GetCurrentTable();
		Gui::TableNextRow();
		f32 restoreX = TableFullRowBegin();
		if (Gui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
			if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner)) {
				const std::string* localeToRemove = nullptr;
				for (auto&& [key, val] : properties) {
					Gui::Property::PropertyTextValueFunc(key, [&]
					{
						Gui::SetNextItemWidth(getInsertButtonWidth());
						if (Gui::InputTextWithHint(("##" + key).c_str(), "n/a", &val))
							context.Undo.NotifyChangesWereMade();
						Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
						if (Gui::Button(("-##" + key).c_str(), {Gui::GetFrameHeight(), Gui::GetFrameHeight()}))
							localeToRemove = &key;
						Gui::SetItemTooltip(UI_Str("ACT_EVENT_REMOVE"));
					});
				}
				if (localeToRemove != nullptr) {
					properties.erase(*localeToRemove);
					context.Undo.NotifyChangesWereMade();
				}
				// add new entry
				Gui::Property::PropertyTextValueFunc(labelAdd, [&]
				{
					b8* pIsValid = Gui::GetStateStorage()->GetBoolRef(reinterpret_cast<ImGuiID>(pNewKey), keyFilter(newDefault));

					Gui::SetNextItemWidth(getInsertButtonWidth());
					Gui::PushStyleColor(ImGuiCol_Text, *pIsValid ? Gui::GetColorU32(ImGuiCol_Text) : InputTextWarningTextColor);
					b8 shouldAdd = Gui::InputTextWithHint("##new", newDefault.c_str(), pNewKey, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter, charFilter);
					if (Gui::IsItemEdited())
						*pIsValid = keyFilter(!pNewKey->empty() ? *pNewKey : newDefault);
					Gui::PopStyleColor();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!*pIsValid);
					if ((Gui::Button("+", { Gui::GetFrameHeight(), Gui::GetFrameHeight() }) || shouldAdd) && *pIsValid) {
						if (!pNewKey->empty()) {
							properties.try_emplace(std::move(*pNewKey), "");
							*pNewKey = "";
						}
						else if (!newDefault.empty())
							properties.try_emplace(newDefault, "");
						context.Undo.NotifyChangesWereMade();
					}
					Gui::SetItemTooltip(UI_Str("ACT_EVENT_ADD"));
					Gui::EndDisabled();
				});
				Gui::EndTable();
			}
		}
		TableFullRowEnd(restoreX);
	}

	static void LocalizedPropertyCollapsingHeader(ChartContext& context, const char* label, std::map<std::string, std::string>& propertyLocalized, std::string* pNewLocale)
	{
		static constexpr auto charFilter = [](ImGuiInputTextCallbackData* data) -> int
		{
			data->EventChar = ASCII::IETFLangTagToTJALangTag(data->EventChar);
			return 0;
		};
		static constexpr auto keyFilter = [](std::string_view in) { return std::regex_match(std::begin(in), std::end(in), ASCII::PatIETFLangTagForTJA); };
		PropertyMapCollapsingHeader(context, label, propertyLocalized, UI_Str("ACT_ADD_NEW_LOCALE"), pNewLocale, charFilter, keyFilter, SelectedGuiLanguageTJA);
	}

	static void OtherMetadataCollapsingHeader(ChartContext& context, const char* label, std::map<std::string, std::string>& otherMetadata, std::string* pNewMetadataKey)
	{
		static constexpr auto charFilter = [](ImGuiInputTextCallbackData* data) -> int
		{
			data->EventChar = ASCII::IETFLangTagToTJALangTag(data->EventChar); // TJA-ize
			return 0;
		};
		static constexpr auto keyFilter = [](std::string_view in) { return !in.empty() && (TJA::GetKeyColonValueTokenKey(in) == TJA::Key::Course_Unknown); };
		PropertyMapCollapsingHeader(context, label, otherMetadata, UI_Str("ACT_ADD_NEW_METADATA"), pNewMetadataKey, charFilter, keyFilter, "");
	}

	void ChartPropertiesWindow::DrawGui(ChartContext& context, const ChartPropertiesWindowIn& in, ChartPropertiesWindowOut& out)
	{
		Gui::UpdateSmoothScrollWindow();

		assert(context.ChartSelectedCourse != nullptr);
		auto& chart = context.Chart;
		auto& course = *context.ChartSelectedCourse;

		if (Gui::CollapsingHeader(UI_Str("DETAILS_CHART_PROPERTIES_CHART"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner))
			{

				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_TITLE"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (Gui::InputTextWithHint("##ChartTitle", "n/a", &chart.ChartTitle))
						context.Undo.NotifyChangesWereMade();
				});
				static std::string newTitleLocale = "";
				LocalizedPropertyCollapsingHeader(context, UI_Str("DETAILS_CHART_PROP_TITLE_LOCALIZED"), chart.ChartTitleLocalized, &newTitleLocale);
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_SUBTITLE"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (Gui::InputTextWithHint("##ChartSubtitle", "n/a", &chart.ChartSubtitle))
						context.Undo.NotifyChangesWereMade();
				});
				static std::string newSubtitleLocale = "";
				LocalizedPropertyCollapsingHeader(context, UI_Str("DETAILS_CHART_PROP_SUBTITLE_LOCALIZED"), chart.ChartSubtitleLocalized, &newSubtitleLocale);
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_CREATOR"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (Gui::InputTextWithHint("##ChartCreator", "n/a", &chart.ChartCreator))
						context.Undo.NotifyChangesWereMade();
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_SONG_FILE_NAME"), [&]
				{
					const b8 songIsLoading = in.IsSongAsyncLoading;
					cstr loadingText = SongLoadingTextAnimation.UpdateFrameAndGetText(songIsLoading, Gui::DeltaTime());
					SongFileNameInputBuffer = songIsLoading ? "" : chart.SongFileName;

					Gui::BeginDisabled(songIsLoading);
					const auto result = Gui::PathInputTextWithHintAndBrowserDialogButton("##SongFileName", "...##SongFileName",
						songIsLoading ? loadingText : "song.ogg", &SongFileNameInputBuffer, songIsLoading ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
					Gui::EndDisabled();

					if (result.InputTextEdited)
					{
						out.LoadNewSong = true;
						out.NewSongFilePath = SongFileNameInputBuffer;
					}
					else if (result.BrowseButtonClicked)
					{
						out.BrowseOpenSong = true;
					}
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_JACKET_FILE_NAME"), [&]
					{
						
						const b8 jacketIsLoading = in.IsJacketAsyncLoading;
						cstr loadingText = JacketLoadingTextAnimation.UpdateFrameAndGetText(jacketIsLoading, Gui::DeltaTime());
						JacketFileNameInputBuffer = jacketIsLoading ? "" : chart.SongJacket;

						Gui::BeginDisabled(jacketIsLoading);
						const auto result = Gui::PathInputTextWithHintAndBrowserDialogButton("##JacketFileName", "...##JacketFileName",
							jacketIsLoading ? loadingText : "jacket.png", &JacketFileNameInputBuffer, jacketIsLoading ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
						Gui::EndDisabled();

						if (result.InputTextEdited)
						{
							out.LoadNewJacket = true;
							out.NewJacketFilePath = JacketFileNameInputBuffer;
						}
						else if (result.BrowseButtonClicked)
						{
							out.BrowseOpenJacket = true;
						}
					});
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_SONG_VOLUME"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = ToPercent(chart.SongVolume); Gui::SliderFloat("##SongVolume", &v, ToPercent(MinVolume), ToPercent(MaxVolumeSoftLimit), "%.0f%%"))
					{
						chart.SongVolume = Clamp(FromPercent(v), MinVolume, MaxVolumeHardLimit);
						context.Undo.NotifyChangesWereMade();
					}
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("CHART_PROP_SOUND_EFFECT_VOLUME"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = ToPercent(chart.SoundEffectVolume); Gui::SliderFloat("##SoundEffectVolume", &v, ToPercent(MinVolume), ToPercent(MaxVolumeSoftLimit), "%.0f%%"))
					{
						chart.SoundEffectVolume = Clamp(FromPercent(v), MinVolume, MaxVolumeHardLimit);
						context.Undo.NotifyChangesWereMade();
					}
				});
				static std::string newChartMetadataKey = "";
				OtherMetadataCollapsingHeader(context, UI_Str("DETAILS_CHART_PROP_OTHER_METADATA"), chart.OtherMetadata, &newChartMetadataKey);
				Gui::Property::EndTable();
			}
		}

		if (Gui::CollapsingHeader(UI_Str("DETAILS_CHART_PROPERTIES_COURSE"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner))
			{
				Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_DIFFICULTY_TYPE"), [&]
				{
					cstr difficultyTypeNames[ArrayCount(DifficultyTypeNames)];
					for (size_t i = 0; i < ArrayCount(DifficultyTypeNames); i++)
						difficultyTypeNames[i] = UI_StrRuntime(DifficultyTypeNames[i]);

					Gui::SetNextItemWidth(-1.0f);
					if (Gui::ComboEnum("##DifficultyType", &course.Type, difficultyTypeNames))
						context.Undo.NotifyChangesWereMade();
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_DIFFICULTY_LEVEL"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (GuiDifficultyLevelStarSliderWidget("##DifficultyLevel", &course.Level, DifficultySliderStarsFitOnScreenLastFrame, DifficultySliderStarsWasHoveredLastFrame))
						context.Undo.NotifyChangesWereMade();
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_DIFFICULTY_LEVEL_DECIMAL"), [&]
					{
						Gui::SetNextItemWidth(-1.0f);
						if (GuiDifficultyDecimalLevelStarSliderWidget("##DifficultyLevelDecimal", &course.Decimal, DifficultySliderStarsFitOnScreenLastFrame, DifficultySliderStarsWasHoveredLastFrame))
							context.Undo.NotifyChangesWereMade();
				});
				Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_PLAYER_SIDE_COUNT"), [&]
				{
					Gui::SetNextItemWidth(-1.0f);

					if (ivec2 v = { course.PlayerSide, course.Style };
						GuiInputFraction("##PlayerSideCount", &v, {}, 1, 5, nullptr)
						) {
						course.Style = std::max(v[1], 1);
						course.PlayerSide = std::clamp(v[0], 1, course.Style);
						context.Undo.NotifyChangesWereMade();
					}
				});

				// Tower
				if (course.Type == DifficultyType::Tower) {
					
					Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_TOWER_LIFE"), [&]
						{
							Gui::SetNextItemWidth(-1.0f);
							if (Gui::SpinInt("##TowerLife", (i32*)(&course.Life), 1, 5))
								context.Undo.NotifyChangesWereMade();
					});
					
					Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_TOWER_SIDE"), [&]
						{
							cstr sideNames[ArrayCount(TowerSideNames)];
							for (size_t i = 0; i < ArrayCount(TowerSideNames); i++)
								sideNames[i] = UI_StrRuntime(TowerSideNames[i]);

							Gui::SetNextItemWidth(-1.0f);
							if (Gui::ComboEnum("##TowerSide", &course.Side, sideNames))
								context.Undo.NotifyChangesWereMade();
					});
				}

#if 0 // TODO:
				Gui::Property::PropertyTextValueFunc("Score Init (TODO)", [&] { Gui::Text("%d", course.ScoreInit); });
				Gui::Property::PropertyTextValueFunc("Score Diff (TODO)", [&] { Gui::Text("%d", course.ScoreDiff); });
#endif
				Gui::Property::PropertyTextValueFunc(UI_Str("COURSE_PROP_CREATOR"), [&]
				{
					const cstr hint = (course.CourseCreator.empty() && !chart.ChartCreator.empty()) ? chart.ChartCreator.c_str() : "n/a";
					Gui::SetNextItemWidth(-1.0f);
					if (Gui::InputTextWithHint("##CourseCreator", hint, &course.CourseCreator))
						context.Undo.NotifyChangesWereMade();
				});

				static std::string newCourseMetadataKey = "";
				OtherMetadataCollapsingHeader(context, UI_Str("DETAILS_COURSE_PROP_OTHER_METADATA"), course.OtherMetadata, &newCourseMetadataKey);

				Gui::Property::EndTable();
			}
		}
	}

	template <typename TValue, typename... TLables>
	static b8 GuiEnumLikeButtons(cstr labelGroup, TValue* currentValue, TLables... labelNames)
	{
		b8 valueChanged = false;

		cstr labels[] = { labelNames... };

		Gui::PushID(labelGroup);
		Gui::SameLineMultiWidget(std::size(labels), [&](const Gui::MultiWidgetIt& i)
		{
			const f32 alphaFactor = (i.Index == static_cast<i32>(*currentValue)) ? 1.0f : 0.5f;
			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_Button, alphaFactor));
			Gui::PushStyleColor(ImGuiCol_ButtonHovered, Gui::GetColorU32(ImGuiCol_ButtonHovered, alphaFactor));
			Gui::PushStyleColor(ImGuiCol_ButtonActive, Gui::GetColorU32(ImGuiCol_ButtonActive, alphaFactor));
			if (Gui::Button(labels[i.Index], { Gui::CalcItemWidth(), 0.0f })) {
				*currentValue = static_cast<TValue>(i.Index);
				valueChanged = true;
			}
			Gui::PopStyleColor(3);
			return false;
		});
		Gui::PopID();
		return valueChanged;
	}

	static std::map<std::tuple<SprID, u32, float, float, float, float>, ImImageQuad> sprImageQuadCache = {};
	// [{sprite_id, tint_col, uv0.x, uv0.y, uv1.x, uv1.y}] = quad;
	// Cannot use std::unordered_map because std::hash<std::tuple<...>> is not defined.
	static i32 sprImageGuiScaleCache = GuiScaleFactorCurrent;

	static bool SpriteButton(const char* tooltip_key, ChartContext& context, SprID sprite_id, const ImVec2& button_size,
		const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1),
		const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
	{
		if (GuiScaleFactorCurrent != sprImageGuiScaleCache) {
			sprImageQuadCache.clear(); // remove invalidated textures
		}

		u32 u32_tint_col = Gui::ColorConvertFloat4ToU32(tint_col);
		ImImageQuad& quad = sprImageQuadCache[{sprite_id, u32_tint_col, uv0.x, uv0.y, uv1.x, uv1.y}];
		ImTextureID tex_id = quad.TexID;
		if (tex_id == 0) {
			SprUV quadUV = SprUV::FromRect(uv0, uv1);
			if (context.Gfx.GetImageQuad(quad, sprite_id, { {0, 0}, {0, 0}, {1, 1,}, 0 }, u32_tint_col, &quadUV)) {
				tex_id = quad.TexID;
			} else {
				sprImageQuadCache.erase({ sprite_id, u32_tint_col, uv0.x, uv0.y, uv1.x, uv1.y });
				tex_id = 0;
			}
		}

		bool res;
		if (tex_id == 0) {
			res = Gui::Button(tooltip_key, button_size);
		} else {
			ImVec2 image_size = { button_size.x - 2 * Gui::GetStyle().FramePadding.x, button_size.y - 2 * Gui::GetStyle().FramePadding.y };
			res = Gui::ImageButton(tooltip_key, tex_id, image_size, uv0, uv1, bg_col, tint_col);
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_ForTooltip)) {
			ImGui::SetTooltip(tooltip_key);
		}
		return res;
	}

	void ChartTempoWindow::DrawGui(ChartContext& context, ChartTimeline& timeline)
	{
		Gui::UpdateSmoothScrollWindow();

		assert(context.ChartSelectedCourse != nullptr);
		auto& chart = context.Chart;
		auto& course = *context.ChartSelectedCourse;

		if (Gui::CollapsingHeader(UI_Str("DETAILS_EVENTS_SYNC"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner))
			{
				static constexpr auto guiPropertyDragTimeAndSetCursorTimeButtonWidgets = [](ChartContext& context, const TimelineCamera& camera, cstr label, Time* inOutValue, TimeSpace storageSpace) -> b8
				{
					const TimeSpace displaySpace = *Settings.General.DisplayTimeInSongSpace ? TimeSpace::Song : TimeSpace::Chart;
					const Time min = ConvertTimeSpace(Time::Zero(), storageSpace, displaySpace, context.Chart), max = Time::FromSec(F64Max);

					b8 valueChanged = false;
					Gui::Property::Property([&]
					{
						Gui::SetNextItemWidth(-1.0f);
						f32 v = ConvertTimeSpace(*inOutValue, storageSpace, displaySpace, context.Chart).ToSec_F32();
						if (GuiDragLabelFloat(label, &v, TimelineDragScalarSpeedAtZoomSec(camera), min.ToSec_F32(), max.ToSec_F32()))
						{
							*inOutValue = ConvertTimeSpace(Time::FromSec(v), displaySpace, storageSpace, context.Chart);
							valueChanged = true;
						}
					});
					Gui::Property::Value([&]
					{
						Gui::SetNextItemWidth(-1.0f);
						Gui::PushID(label);
						f32 v = ConvertTimeSpace(*inOutValue, storageSpace, displaySpace, context.Chart).ToSec_F32();
						Gui::SameLineMultiWidget(2, [&](const Gui::MultiWidgetIt& i)
						{
							if (i.Index == 0 && Gui::DragFloat("##DragTime", &v, TimelineDragScalarSpeedAtZoomSec(camera), min.ToSec_F32(), max.ToSec_F32(),
								(*inOutValue <= Time::Zero()) ? "n/a" : Time::FromSec(v).ToString().Data, ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoRoundToFormat | ImGuiSliderFlags_NoInput))
							{
								*inOutValue = ConvertTimeSpace(Time::FromSec(v), displaySpace, storageSpace, context.Chart);
								valueChanged = true;
							}
							else if (i.Index == 1 && Gui::Button(UI_Str("ACT_SYNC_SET_CURSOR"), { Gui::CalcItemWidth(), 0.0f }))
							{
								*inOutValue = ClampBot(ConvertTimeSpace(context.GetCursorTime(), TimeSpace::Chart, storageSpace, context.Chart), Time::Zero());
								context.Undo.DisallowMergeForLastCommand();
								valueChanged = true;
							}
							return false;
						});
						Gui::PopID();
					});
					return valueChanged;
				};

				if (Time v = chart.ChartDuration; guiPropertyDragTimeAndSetCursorTimeButtonWidgets(context, timeline.Camera, UI_Str("SYNC_CHART_DURATION"), &v, TimeSpace::Chart))
					context.Undo.Execute<Commands::ChangeChartDuration>(&chart, v);

				if (Time v = chart.SongDemoStartTime; guiPropertyDragTimeAndSetCursorTimeButtonWidgets(context, timeline.Camera, UI_Str("SYNC_SONG_DEMO_START"), &v, TimeSpace::Song))
					context.Undo.Execute<Commands::ChangeSongDemoStartTime>(&chart, v);

				Gui::Property::Property([&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = chart.SongOffset.ToMS_F32(); GuiDragLabelFloat(UI_Str("SYNC_SONG_OFFSET"), &v, TimelineDragScalarSpeedAtZoomMS(timeline.Camera)))
						context.Undo.Execute<Commands::ChangeSongOffset>(&chart, Time::FromMS(v));
				});
				Gui::Property::Value([&]
				{
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = chart.SongOffset.ToMS_F32(); Gui::SpinFloat("##SongOffset", &v, 1.0f, 10.0f, "%.2f ms", ImGuiInputTextFlags_None))
						context.Undo.Execute<Commands::ChangeSongOffset>(&chart, Time::FromMS(v));
				});
				// TODO: Disable merge if made inactive this frame (?)
				// if (Gui::IsItemDeactivatedAfterEdit()) {}

				Gui::Property::EndTable();
			}
		}

		if (Gui::CollapsingHeader(UI_Str("DETAILS_CHART_EVENT_EVENTS"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			b8 isAnyItemOtherThanNotesSelected = false;
			b8 isAnyItemNotInListSelected[EnumCount<GenericList>] = {}; // all false
			ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it)
			{
				if (!IsNotesList(it.List)) isAnyItemOtherThanNotesSelected = true;
				for (GenericList list = {}; list != GenericList::Count; IncrementEnum(list)) {
					if (it.List != list) isAnyItemNotInListSelected[EnumToIndex(list)] = true;
				}
			});

			if (Gui::Property::BeginTable(ImGuiTableFlags_BordersInner))
			{
				// NOTE: This might seem like a strange design decision at first (hence it at least being optional)
				//		 but the widgets here can only be used for inserting / editing (single) changes directly underneath the cursor
				//		 which in case of an active selection (in the properties window) can become confusing
				//		 since there then are multiple edit widgets shown to the user with similar labels yet that differ in functionality.
				//		 To make it clear that selected items can only be edited via the properties window, these regular widgets here will therefore be disabled.
				b8 disableWidgetsBeacuseOfSelection = false;
				if (*Settings.General.DisableTempoWindowWidgetsIfHasSelection && isAnyItemOtherThanNotesSelected)
					disableWidgetsBeacuseOfSelection = true;

				const Beat cursorBeat = FloorBeatToGrid(context.GetCursorBeat(), GetGridBeatSnap(timeline.CurrentGridBarDivision));
				// NOTE: Specifically to prevent ugly "flashing" between add/remove labels during playback
				const b8 disallowRemoveButton = (cursorBeat.Ticks < 0) || context.GetIsPlayback();
				const b8 disableEditingAtPlayCursor = disableWidgetsBeacuseOfSelection || (cursorBeat.Ticks < 0);

				const TempoChange* tempoChangeAtCursor = course.TempoMap.Tempo.TryFindLastAtBeat(cursorBeat);
				const Tempo tempoAtCursor = (tempoChangeAtCursor != nullptr) ? tempoChangeAtCursor->Tempo : FallbackEvent<TempoChange>.Tempo;
				auto insertOrUpdateCursorTempoChange = [&](Tempo newTempo)
				{
					if (tempoChangeAtCursor == nullptr || tempoChangeAtCursor->Beat != cursorBeat)
						context.Undo.Execute<Commands::AddTempoChange>(&course, &course.TempoMap, TempoChange(cursorBeat, newTempo));
					else
						context.Undo.Execute<Commands::UpdateTempoChange>(&course, &course.TempoMap, TempoChange(cursorBeat, newTempo));
				};

				Gui::Property::Property([&]
				{
					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = tempoAtCursor.BPM; GuiDragLabelFloat(UI_Str("EVENT_TEMPO"), &v, 1.0f, MinBPM, MaxBPM, ImGuiSliderFlags_AlwaysClamp))
						insertOrUpdateCursorTempoChange(Tempo(v));
					Gui::EndDisabled();
				});
				Gui::Property::Value([&]
				{
					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f);
					if (f32 v = tempoAtCursor.BPM; Gui::SpinFloat("##TempoAtCursor", &v, 1.0f, 10.0f, "%g BPM", ImGuiInputTextFlags_None))
						insertOrUpdateCursorTempoChange(Tempo(Clamp(v, MinBPM, MaxBPM)));

					Gui::PushID(&course.TempoMap.Tempo);
					if (!disallowRemoveButton && tempoChangeAtCursor != nullptr && tempoChangeAtCursor->Beat == cursorBeat)
					{
						if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
							context.Undo.Execute<Commands::RemoveTempoChange>(&course, &course.TempoMap, cursorBeat);
					}
					else
					{
						if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
							insertOrUpdateCursorTempoChange(tempoAtCursor);
					}
					Gui::EndDisabled();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::TempoChanges)]);
					if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
						timeline.ExecuteConvertSelectionToEvents<GenericList::TempoChanges>(context);
					Gui::EndDisabled();

					Gui::PopID();
				});

				Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_TIME_SIGNATURE"), [&]
				{
					const TimeSignatureChange* signatureChangeAtCursor = course.TempoMap.Signature.TryFindLastAtBeat(cursorBeat);
					const TimeSignature signatureAtCursor = (signatureChangeAtCursor != nullptr) ? signatureChangeAtCursor->Signature : FallbackEvent<TimeSignatureChange>.Signature;
					auto insertOrUpdateCursorSignatureChange = [&](TimeSignature newSignature)
					{
						// TODO: Also floor cursor beat to next whole bar (?)
						if (signatureChangeAtCursor == nullptr || signatureChangeAtCursor->Beat != cursorBeat)
							context.Undo.Execute<Commands::AddTimeSignatureChange>(&course, &course.TempoMap, TimeSignatureChange(cursorBeat, newSignature));
						else
							context.Undo.Execute<Commands::UpdateTimeSignatureChange>(&course, &course.TempoMap, TimeSignatureChange(cursorBeat, newSignature));
					};

					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f);
					if (ivec2 v = { signatureAtCursor.Numerator, signatureAtCursor.Denominator };
						GuiInputFraction("##SignatureAtCursor", &v, ivec2(MinTimeSignatureValue, MaxTimeSignatureValue), 1, 4, !IsTimeSignatureSupported(signatureAtCursor) ? &InputTextWarningTextColor : nullptr))
						insertOrUpdateCursorSignatureChange(TimeSignature(v[0], v[1]));

					Gui::PushID(&course.TempoMap.Signature);
					if (!disallowRemoveButton && signatureChangeAtCursor != nullptr && signatureChangeAtCursor->Beat == cursorBeat)
					{
						if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
							context.Undo.Execute<Commands::RemoveTimeSignatureChange>(&course, &course.TempoMap, cursorBeat);
					}
					else
					{
						if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
							insertOrUpdateCursorSignatureChange(signatureAtCursor);
					}
					Gui::EndDisabled();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::SignatureChanges)]);
					if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
						timeline.ExecuteConvertSelectionToEvents<GenericList::SignatureChanges>(context);
					Gui::EndDisabled();

					Gui::PopID();
				});

				const ScrollChange* scrollChangeChangeAtCursor = course.ScrollChanges.TryFindLastAtBeat(cursorBeat);
				const Complex scrollSpeedAtCursor = (scrollChangeChangeAtCursor != nullptr) ? scrollChangeChangeAtCursor->ScrollSpeed : FallbackEvent<ScrollChange>.ScrollSpeed;
				auto insertOrUpdateCursorScrollSpeedChange = [&](Complex newScrollSpeed)
				{
					if (scrollChangeChangeAtCursor == nullptr || scrollChangeChangeAtCursor->BeatTime != cursorBeat)
						context.Undo.Execute<Commands::AddScrollChange>(&course, &course.ScrollChanges, ScrollChange { cursorBeat, newScrollSpeed });
					else
						context.Undo.Execute<Commands::UpdateScrollChange>(&course, &course.ScrollChanges, ScrollChange { cursorBeat, newScrollSpeed });
				};

				Gui::Property::Property([&]
				{
					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f);
					if (Complex v = scrollSpeedAtCursor; GuiDragLabelFloat(UI_Str("EVENT_SCROLL_SPEED"), &reinterpret_cast<f32*>(&(v.cpx))[0], 0.005f, MinScrollSpeed, MaxScrollSpeed, ImGuiSliderFlags_AlwaysClamp))
						insertOrUpdateCursorScrollSpeedChange(v);
					Gui::EndDisabled();
				});
				Gui::Property::Value([&]
				{
					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f); Gui::SameLineMultiWidget(3, [&](const Gui::MultiWidgetIt& i)
					{
						if (i.Index == 0)
						{
							if (Complex v = scrollSpeedAtCursor;
								Gui::SpinFloat("##ScrollSpeedAtCursor", &reinterpret_cast<f32*>(&(v.cpx))[0], 0.1f, 0.5f, "%gx"))
								insertOrUpdateCursorScrollSpeedChange(Complex(Clamp(v.GetRealPart(), MinScrollSpeed, MaxScrollSpeed), Clamp(v.GetImaginaryPart(), MinScrollSpeed, MaxScrollSpeed)));
						}
						else if (i.Index == 1)
						{
							if (Complex v = scrollSpeedAtCursor;
								Gui::SpinFloat("##ScrollSpeedAtCursorImag", &reinterpret_cast<f32*>(&(v.cpx))[1], 0.1f, 0.5f, "%gix"))
								insertOrUpdateCursorScrollSpeedChange(Complex(Clamp(v.GetRealPart(), MinScrollSpeed, MaxScrollSpeed), Clamp(v.GetImaginaryPart(), MinScrollSpeed, MaxScrollSpeed)));
						}
						else
						{
							if (f32 v = ScrollSpeedToTempo(scrollSpeedAtCursor.GetRealPart(), tempoAtCursor).BPM;
								Gui::SpinFloat("##ScrollTempoAtCursor", &v, 1.0f, 10.0f, "%g BPM"))
								insertOrUpdateCursorScrollSpeedChange(Complex(ScrollTempoToSpeed(Tempo(Clamp(v, MinBPM, MaxBPM)), tempoAtCursor), 0.0f));
						}
						return false;
					});

					Gui::PushID(&course.ScrollChanges);
					if (!disallowRemoveButton && scrollChangeChangeAtCursor != nullptr && scrollChangeChangeAtCursor->BeatTime == cursorBeat)
					{
						if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
							context.Undo.Execute<Commands::RemoveScrollChange>(&course, &course.ScrollChanges, cursorBeat);
					}
					else
					{
						if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
							insertOrUpdateCursorScrollSpeedChange(scrollSpeedAtCursor);
					}
					Gui::EndDisabled();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::ScrollChanges)]);
					if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
						timeline.ExecuteConvertSelectionToEvents<GenericList::ScrollChanges>(context);
					Gui::EndDisabled();

					Gui::PopID();
				});

				Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_BAR_LINE_VISIBILITY"), [&]
				{
					const BarLineChange* barLineChangeAtCursor = course.BarLineChanges.TryFindLastAtBeat(cursorBeat);
					auto insertOrUpdateCursorBarLineChange = [&](i8 newVisibility)
					{
						if (barLineChangeAtCursor == nullptr || barLineChangeAtCursor->BeatTime != cursorBeat)
							context.Undo.Execute<Commands::AddBarLineChange>(&course, &course.BarLineChanges, BarLineChange { cursorBeat, newVisibility == 0 });
						else
							context.Undo.Execute<Commands::UpdateBarLineChange>(&course, &course.BarLineChanges, BarLineChange { cursorBeat, newVisibility == 0 });
					};

					Gui::BeginDisabled(disableEditingAtPlayCursor);
					Gui::SetNextItemWidth(-1.0f);
					b8 isVisible = (barLineChangeAtCursor != nullptr) ? barLineChangeAtCursor->IsVisible : FallbackEvent<BarLineChange>.IsVisible;
					if (i8 v = isVisible ? 0 : 1; GuiEnumLikeButtons("##OnOffBarLineAtCursor", &v, UI_Str("BAR_LINE_VISIBILITY_VISIBLE"), UI_Str("BAR_LINE_VISIBILITY_HIDDEN")))
						insertOrUpdateCursorBarLineChange(v);

					Gui::PushID(&course.BarLineChanges);
					if (!disallowRemoveButton && barLineChangeAtCursor != nullptr && barLineChangeAtCursor->BeatTime == cursorBeat)
					{
						if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
							context.Undo.Execute<Commands::RemoveBarLineChange>(&course, &course.BarLineChanges, cursorBeat);
					}
					else
					{
						if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
							insertOrUpdateCursorBarLineChange(isVisible);
					}
					Gui::EndDisabled();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::BarLineChanges)]);
					if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
						timeline.ExecuteConvertSelectionToEvents<GenericList::BarLineChanges>(context);
					Gui::EndDisabled();

					Gui::PopID();
				});

				Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_SCROLL_TYPE"), [&]
				{
						const ScrollType* ScrollTypeAtCursor = course.ScrollTypes.TryFindLastAtBeat(cursorBeat);
						auto insertOrUpdateCursorScrollType = [&](ScrollMethod newMethod)
						{
							if (ScrollTypeAtCursor == nullptr || ScrollTypeAtCursor->BeatTime != cursorBeat)
								context.Undo.Execute<Commands::AddScrollType>(&course, &course.ScrollTypes, ScrollType{ cursorBeat, newMethod });
							else
								context.Undo.Execute<Commands::UpdateScrollType>(&course, &course.ScrollTypes, ScrollType{ cursorBeat, newMethod });
						};

						Gui::BeginDisabled(disableEditingAtPlayCursor);
						Gui::SetNextItemWidth(-1.0f);
						if (ScrollMethod v = (ScrollTypeAtCursor != nullptr) ? ScrollTypeAtCursor->Method : FallbackEvent<ScrollType>.Method; GuiEnumLikeButtons("##ScrollTypeAtCursor", &v, UI_Str("SCROLL_TYPE_NMSCROLL"), UI_Str("SCROLL_TYPE_HBSCROLL"), UI_Str("SCROLL_TYPE_BMSCROLL")))
							insertOrUpdateCursorScrollType(v);

						Gui::PushID(&course.ScrollTypes);
						if (!disallowRemoveButton && ScrollTypeAtCursor != nullptr && ScrollTypeAtCursor->BeatTime == cursorBeat)
						{
							if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
								context.Undo.Execute<Commands::RemoveScrollType>(&course, &course.ScrollTypes, cursorBeat);
						}
						else
						{
							if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
								insertOrUpdateCursorScrollType((ScrollTypeAtCursor != nullptr) ? ScrollTypeAtCursor->Method : FallbackEvent<ScrollType>.Method);
						}
						Gui::EndDisabled();

						Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
						Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::ScrollType)]);
						if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
							timeline.ExecuteConvertSelectionToEvents<GenericList::ScrollType>(context);
						Gui::EndDisabled();

						Gui::PopID();
				});

				const JPOSScrollChange* JPOSScrollChangeAtCursor = course.JPOSScrollChanges.TryFindLastAtBeat(cursorBeat);
				const Complex JPOSScrollMoveAtCursor = (JPOSScrollChangeAtCursor != nullptr) ? JPOSScrollChangeAtCursor->Move : FallbackEvent<JPOSScrollChange>.Move;
				const f32 JPOSScrollDurationAtCursor = (JPOSScrollChangeAtCursor != nullptr) ? JPOSScrollChangeAtCursor->Duration : FallbackEvent<JPOSScrollChange>.Duration;
				auto insertOrUpdateCursorJPOSScrollChange = [&](Complex newMove, f32 newDuration)
				{
					if (JPOSScrollChangeAtCursor == nullptr || JPOSScrollChangeAtCursor->BeatTime != cursorBeat)
						context.Undo.Execute<Commands::AddJPOSScroll>(&course, &course.JPOSScrollChanges, JPOSScrollChange{ cursorBeat, newMove, newDuration });
					else
						context.Undo.Execute<Commands::UpdateJPOSScroll>(&course, &course.JPOSScrollChanges, JPOSScrollChange{ cursorBeat, newMove, newDuration });
				};

				Gui::Property::Property([&]
					{
						Gui::BeginDisabled(disableEditingAtPlayCursor);
						Gui::SetNextItemWidth(-1.0f);
						if (Complex v = JPOSScrollMoveAtCursor;
							GuiDragLabelFloat(UI_Str("EVENT_JPOS_SCROLL"), &reinterpret_cast<f32*>(&(v.cpx))[0], 0.5f, MinJPOSScrollMove, MaxJPOSScrollMove, ImGuiSliderFlags_AlwaysClamp))
							insertOrUpdateCursorJPOSScrollChange(v, JPOSScrollDurationAtCursor);
						Gui::EndDisabled();
					});
				Gui::Property::Value([&]
					{
						Gui::BeginDisabled(disableEditingAtPlayCursor);
						Gui::SetNextItemWidth(-1.0f); Gui::SameLineMultiWidget(2, [&](const Gui::MultiWidgetIt& i)
							{
								if (i.Index == 0)
								{
									if (Complex v = JPOSScrollMoveAtCursor;
										Gui::SpinFloat("##JPOSScrollMoveAtCursor", &reinterpret_cast<f32*>(&(v.cpx))[0], 1.f, 5.f, "%gpx"))
										insertOrUpdateCursorJPOSScrollChange(
											Complex(Clamp(v.GetRealPart(), MinJPOSScrollMove, MaxJPOSScrollMove), Clamp(v.GetImaginaryPart(), MinJPOSScrollMove, MaxJPOSScrollMove)),
											JPOSScrollDurationAtCursor);
								}
								else
								{
									if (Complex v = JPOSScrollMoveAtCursor;
										Gui::SpinFloat("##JPOSScrollMoveCursorImag", &reinterpret_cast<f32*>(&(v.cpx))[1], 1.f, 5.f, "%gipx"))
										insertOrUpdateCursorJPOSScrollChange(
											Complex(Clamp(v.GetRealPart(), MinJPOSScrollMove, MaxJPOSScrollMove), Clamp(v.GetImaginaryPart(), MinJPOSScrollMove, MaxJPOSScrollMove)),
											JPOSScrollDurationAtCursor);
								}
								return false;
							});

						Gui::SetNextItemWidth(-1.0f);
						if (f32 v = JPOSScrollDurationAtCursor;
							Gui::SpinFloat("##JPOSScrollDurationAtCursor", &v, .1f, .5f, "%gs"))
							insertOrUpdateCursorJPOSScrollChange(
								JPOSScrollMoveAtCursor, Clamp(v, MinJPOSScrollDuration, MaxJPOSScrollDuration)
							);

						Gui::PushID(&course.JPOSScrollChanges);
						if (!disallowRemoveButton && JPOSScrollChangeAtCursor != nullptr && JPOSScrollChangeAtCursor->BeatTime == cursorBeat)
						{
							if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
								context.Undo.Execute<Commands::RemoveJPOSScroll>(&course, &course.JPOSScrollChanges, cursorBeat);
						}
						else
						{
							if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
								insertOrUpdateCursorJPOSScrollChange(
									JPOSScrollMoveAtCursor,
									JPOSScrollDurationAtCursor);
						}
						Gui::EndDisabled();

						Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
						Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::JPOSScroll)]);
						if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
							timeline.ExecuteConvertSelectionToEvents<GenericList::JPOSScroll>(context);
						Gui::EndDisabled();

						Gui::PopID();
					});

					const DelayChange* delayChangeAtCursor = course.DelayChanges.TryFindLastAtBeat(cursorBeat);
					// Show the delta of the change exactly at cursor, or 0 if none (so the user edits this specific change's delta)
					const Time delayAtCursor = (delayChangeAtCursor != nullptr && delayChangeAtCursor->BeatTime == cursorBeat) ? delayChangeAtCursor->Delay : FallbackEvent<DelayChange>.Delay;
					auto insertOrUpdateCursorDelayChange = [&](Time newDelay)
					{
						if (delayChangeAtCursor == nullptr || delayChangeAtCursor->BeatTime != cursorBeat)
							context.Undo.Execute<Commands::AddDelayChange>(&course, &course.DelayChanges, DelayChange{ cursorBeat, newDelay });
						else
							context.Undo.Execute<Commands::UpdateDelayChange>(&course, &course.DelayChanges, DelayChange{ cursorBeat, newDelay });
					};

					Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_DELAY"), [&]
					{
						Gui::BeginDisabled(disableEditingAtPlayCursor);

						Gui::PushID(&course.DelayChanges);
						f32 delaySec = delayAtCursor.ToSec_F32();
						Gui::SetNextItemWidth(-1.0f);
						if (Gui::SpinFloat("##DelayAtCursor", &delaySec, 0.01f, 0.1f, "%g sec"))
							insertOrUpdateCursorDelayChange(Time::FromSec(delaySec));

						if (!disallowRemoveButton && delayChangeAtCursor != nullptr && delayChangeAtCursor->BeatTime == cursorBeat)
						{
							if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), { getInsertButtonWidth(), 0.0f }))
								context.Undo.Execute<Commands::RemoveDelayChange>(&course, &course.DelayChanges, cursorBeat);
						}
						else
						{
							if (Gui::Button(UI_Str("ACT_EVENT_ADD"), { getInsertButtonWidth(), 0.0f }))
								insertOrUpdateCursorDelayChange(delayAtCursor);
						}

						Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
						Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::DelayChanges)]);
						if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
							timeline.ExecuteConvertSelectionToEvents<GenericList::DelayChanges>(context);
						Gui::EndDisabled();

						Gui::PopID();
						Gui::EndDisabled();
					});

				Gui::Property::PropertyTextValueFunc(UI_Str("EVENT_GO_GO_TIME"), [&]
				{
					const GoGoRange* gogoRangeAtCursor = course.GoGoRanges.TryFindOverlappingBeat(cursorBeat, cursorBeat);
					const b8 hasRangeSelection = timeline.RangeSelection.IsActiveAndHasEnd();

					Gui::PushID(&course.GoGoRanges);
					Gui::BeginDisabled(!hasRangeSelection);
					if (Gui::Button(UI_Str("ACT_EVENT_SET_FROM_RANGE_SELECTION"), { getInsertButtonWidth(), 0.0f }))
					{
						const Beat rangeSelectionMin = timeline.RoundBeatToCurrentGrid(timeline.RangeSelection.GetMin());
						const Beat rangeSelectionMax = timeline.RoundBeatToCurrentGrid(timeline.RangeSelection.GetMax());
						auto gogoIntersectsSelection = [&](const GoGoRange& gogo) { return (gogo.GetStart() < rangeSelectionMax) && (gogo.GetEnd() > rangeSelectionMin); };

						// TODO: Try to shorten/move intersecting gogo ranges instead of removing them outright
						SortedGoGoRangesList newGoGoRanges = course.GoGoRanges;
						erase_remove_if(newGoGoRanges.Sorted, gogoIntersectsSelection);
						newGoGoRanges.InsertOrUpdate(GoGoRange { rangeSelectionMin, (rangeSelectionMax - rangeSelectionMin) });

						context.Undo.Execute<Commands::AddGoGoRange>(&course, &course.GoGoRanges, std::move(newGoGoRanges));
					}
					Gui::EndDisabled();

					Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
					Gui::BeginDisabled(!isAnyItemNotInListSelected[EnumToIndex(GenericList::GoGoRanges)]);
					if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
						timeline.ExecuteConvertSelectionToEvents<GenericList::GoGoRanges>(context);
					Gui::EndDisabled();

					Gui::BeginDisabled(gogoRangeAtCursor == nullptr);
					if (Gui::Button(UI_Str("ACT_EVENT_REMOVE"), vec2(-1.0f, 0.0f)))
					{
						if (gogoRangeAtCursor != nullptr)
							context.Undo.Execute<Commands::RemoveGoGoRange>(&course, &course.GoGoRanges, gogoRangeAtCursor->BeatTime);
					}
					Gui::EndDisabled();

					Gui::PopID();
				});

				Gui::Property::EndTable();
			}
		}
	}

	static void ConvertAllLyricsToString(TimeSpace timeSpace, Time songOffset, const SortedTempoMap& tempoMap, const SortedLyricsList& in, std::string& out)
	{
		// TODO: Maybe format as some existing subtitle format and visually show syntax errors somehow (?)
		for (const auto& lyricChange : in.Sorted)
		{
			out += ConvertTimeSpace(tempoMap.BeatToTime(lyricChange.BeatTime), TimeSpace::Chart, timeSpace, songOffset).ToString().Data;
			out += " > ";
			ConvertToEscapeSequences(lyricChange.Lyric, out, EscapeSequenceFlags::NewLines);
			out += "\n";
		}
	};

	static void ConvertAllLyricsFromString(TimeSpace timeSpace, Time songOffset, const SortedTempoMap& tempoMap, std::string_view in, SortedLyricsList& out)
	{
		ASCII::ForEachLineInMultiLineString(in, false, [&](std::string_view line)
		{
			if (line.size() < ArrayCount("00:00.000"))
				return;

			const size_t timeLyricSplitIndex = in.find_first_of('>');
			if (timeLyricSplitIndex == std::string_view::npos || timeLyricSplitIndex + 1 >= line.size())
				return;

			const std::string_view timeSubStr = ASCII::Trim(line.substr(0, timeLyricSplitIndex));
			const std::string_view lyricSubStr = ASCII::TrimSuffix(ASCII::Trim(line.substr(timeLyricSplitIndex + 1)), "\n");

			// BUG: Time round-trip conversion not lossless
			Time::FormatBuffer zeroTerminatedTimeBuffer; CopyStringViewIntoFixedBuffer(zeroTerminatedTimeBuffer.Data, timeSubStr);
			const Time parsedTime = ConvertTimeSpace(Time::FromString(zeroTerminatedTimeBuffer.Data), timeSpace, TimeSpace::Chart, songOffset);
			const Beat parsedBeat = tempoMap.TimeToBeat(parsedTime);

			b8 isOnlyWhitespace = true;
			for (const char c : lyricSubStr)
				isOnlyWhitespace &= ASCII::IsWhitespace(c);

			// TODO: Handle inside the tja format code instead (?)
			std::string parsedLyrc;
			if (!isOnlyWhitespace)
				ResolveEscapeSequences(lyricSubStr, parsedLyrc, EscapeSequenceFlags::NewLines);

			out.InsertOrUpdate(LyricChange { parsedBeat, std::move(parsedLyrc) });
		});
	};

	void ChartLyricsWindow::DrawGui(ChartContext& context, ChartTimeline& timeline)
	{
		Gui::UpdateSmoothScrollWindow();

		assert(context.ChartSelectedCourse != nullptr);
		auto& chart = context.Chart;
		auto& course = *context.ChartSelectedCourse;
		const TimeSpace timeSpace = *Settings.General.DisplayTimeInSongSpace ? TimeSpace::Song : TimeSpace::Chart;

		if (Gui::CollapsingHeader(UI_Str("DETAILS_LYRICS_OVERVIEW"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!IsAllLyricsInputActiveThisFrame)
			{
				AllLyricsBuffer.clear();
				ConvertAllLyricsToString(timeSpace, chart.SongOffset, context.ChartSelectedCourse->TempoMap, context.ChartSelectedCourse->Lyrics, AllLyricsBuffer);
			}

			// BUG: Made ImGuiInputTextFlags_ReadOnly for now to avoid round-trip time conversion error
			Gui::SetNextItemWidth(-1.0f);
			if (Gui::InputTextMultilineWithHint("##AllLyrics", UI_Str("INFO_LYRICS_NO_LYRICS"), &AllLyricsBuffer, { -1.0f, Gui::GetContentRegionAvail().y * 0.65f }, ImGuiInputTextFlags_ReadOnly))
			{
				SortedLyricsList newLyrics;
				ConvertAllLyricsFromString(timeSpace, chart.SongOffset, context.ChartSelectedCourse->TempoMap, AllLyricsBuffer, newLyrics);
				// HACK: Full conversion every time a letter is typed, not great but seeing as lyrics is relatively rare and typically small in size this might be fine
				context.Undo.Execute<Commands::ReplaceAllLyricChanges>(&course, &context.ChartSelectedCourse->Lyrics, std::move(newLyrics));
			}

			IsAllLyricsInputActiveLastFrame = IsAllLyricsInputActiveThisFrame;
			IsAllLyricsInputActiveThisFrame = Gui::IsItemActive();

			if (IsAllLyricsInputActiveThisFrame) context.Undo.ResetMergeTimeThresholdStopwatch();
			if (!IsAllLyricsInputActiveThisFrame && IsAllLyricsInputActiveLastFrame) context.Undo.DisallowMergeForLastCommand();
			if (IsAllLyricsInputActiveThisFrame && !IsAllLyricsInputActiveLastFrame) AllLyricsCopyOnMadeActive = AllLyricsBuffer;
		}

		if (Gui::CollapsingHeader(UI_Str("DETAILS_LYRICS_EDIT_LINE"), ImGuiTreeNodeFlags_DefaultOpen))
		{
			b8 isAnyItemOtherThanLyricsSelected = false;
			ForEachSelectedChartItem(course, [&](const ForEachChartItemData& it)
			{
				if (it.List != GenericList::Lyrics) isAnyItemOtherThanLyricsSelected = true;
			});

			const Beat cursorBeat = FloorBeatToGrid(context.GetCursorBeat(), GetGridBeatSnap(timeline.CurrentGridBarDivision));
			const LyricChange* lyricChangeAtCursor = context.ChartSelectedCourse->Lyrics.TryFindLastAtBeat(cursorBeat);
			Gui::BeginDisabled(cursorBeat.Ticks < 0);

			static constexpr auto guiDoubleButton = [](cstr labelA, cstr labelB) -> i32
			{
				return Gui::SameLineMultiWidget(2, [&](const Gui::MultiWidgetIt& i) { return Gui::Button((i.Index == 0) ? labelA : labelB, { Gui::CalcItemWidth(), 0.0f }); }).ChangedIndex;
			};

			if (!IsLyricInputActiveThisFrame)
			{
				// TODO: Handle inside the tja format code instead (?)
				LyricInputBuffer.clear();
				if (lyricChangeAtCursor != nullptr)
					ResolveEscapeSequences(lyricChangeAtCursor->Lyric, LyricInputBuffer, EscapeSequenceFlags::NewLines);
			}

			const f32 textInputHeight = ClampBot(Gui::GetContentRegionAvail().y - Gui::GetFrameHeightWithSpacing(), Gui::GetFrameHeightWithSpacing());
			Gui::SetNextItemWidth(-1.0f);
			if (Gui::InputTextMultilineWithHint("##LyricAtCursor", "n/a", &LyricInputBuffer, { -1.0f, textInputHeight }, ImGuiInputTextFlags_AutoSelectAll))
			{
				std::string newLyricLine;
				ConvertToEscapeSequences(LyricInputBuffer, newLyricLine, EscapeSequenceFlags::NewLines);

				if (lyricChangeAtCursor == nullptr || lyricChangeAtCursor->BeatTime != cursorBeat)
					context.Undo.Execute<Commands::AddLyricChange>(&course, &course.Lyrics, LyricChange { cursorBeat, std::move(newLyricLine) });
				else
					context.Undo.Execute<Commands::UpdateLyricChange>(&course, &course.Lyrics, LyricChange { cursorBeat, std::move(newLyricLine) });
			}

			// HACK: Workaround for ImGuiInputTextFlags_AutoSelectAll being ignored for multiline text inputs
			if (auto* inputTextState = Gui::IsItemActivated() ? Gui::GetInputTextState(Gui::GetActiveID()) : nullptr; inputTextState != nullptr)
				inputTextState->SelectAll();

			IsLyricInputActiveLastFrame = IsLyricInputActiveThisFrame;
			IsLyricInputActiveThisFrame = Gui::IsItemActive();

			if (IsLyricInputActiveThisFrame) context.Undo.ResetMergeTimeThresholdStopwatch();
			if (!IsLyricInputActiveThisFrame && IsLyricInputActiveLastFrame) context.Undo.DisallowMergeForLastCommand();

			Gui::PushID(&course.Lyrics);
			Gui::SetNextItemWidth(getInsertButtonWidth());
			if (i32 clicked = guiDoubleButton(UI_Str("ACT_EVENT_CLEAR"), UI_Str("ACT_EVENT_REMOVE")); clicked > -1)
			{
				if (clicked == 0)
				{
					if (lyricChangeAtCursor == nullptr || lyricChangeAtCursor->BeatTime != cursorBeat)
						context.Undo.Execute<Commands::AddLyricChange>(&course, &course.Lyrics, LyricChange { cursorBeat, "" });
					else
						context.Undo.Execute<Commands::UpdateLyricChange>(&course, &course.Lyrics, LyricChange { cursorBeat, "" });
				}
				else if (clicked == 1)
				{
					if (lyricChangeAtCursor != nullptr && lyricChangeAtCursor->BeatTime == cursorBeat)
						context.Undo.Execute<Commands::RemoveLyricChange>(&course, &course.Lyrics, cursorBeat);
				}
			}
			Gui::EndDisabled();

			Gui::SetNextItemWidth(-1.0f);
			Gui::SameLine(0, Gui::GetStyle().ItemInnerSpacing.x);
			Gui::BeginDisabled(!isAnyItemOtherThanLyricsSelected);
			if (SpriteButton(UI_Str("ACT_EVENT_INSERT_AT_SELECTED_ITEMS"), context, SprID::Timeline_Icon_InsertAtSelectedItems, { Gui::GetFrameHeight(), Gui::GetFrameHeight() }))
				timeline.ExecuteConvertSelectionToEvents<GenericList::Lyrics>(context);
			Gui::EndDisabled();

			Gui::PopID();
		}
	}
}
