#pragma once
#include "core_types.h"
#include "core_string.h"
#include "core_beat.h"
#include "file_format_tja.h"
#include <unordered_map>
#include "chart_editor_i18n.h"

namespace PeepoDrumKit
{
	enum class NoteType : u8
	{
		// NOTE: Regular notes
		Don,
		DonBig,
		Ka,
		KaBig,
		// NOTE: Long notes
		Drumroll,
		DrumrollBig,
		Balloon,
		BalloonSpecial,
		// NOTE: TJAP2fPC
		DonBigHand,
		KaBigHand,
		// NOTE: OpenTaiko notes
		KaDon,
		Bomb,
		Adlib, // from TJAP2fPC
		Fuse,
		// ...
		Count
	};

	enum class NoteSEType : u8
	{
		Do, Ko, Don, DonBig, DonHand,
		Ka, Katsu, KatsuBig, KatsuHand,
		Drumroll, DrumrollBig,
		Balloon, BalloonSpecial,
		Count
	};

	constexpr b8 IsDonNote(NoteType v) { return (v == NoteType::Don) || (v == NoteType::DonBig) || (v == NoteType::DonBigHand); }
	constexpr b8 IsKaNote(NoteType v) { return (v == NoteType::Ka) || (v == NoteType::KaBig) || (v == NoteType::KaBigHand); }
	constexpr b8 IsKaDonNote(NoteType v) { return (v == NoteType::KaDon); }
	constexpr b8 IsAdlibNote(NoteType v) { return (v == NoteType::Adlib); }
	constexpr b8 IsBombNote(NoteType v) { return (v == NoteType::Bomb); }
	constexpr b8 IsSmallNote(NoteType v) { return (v == NoteType::Don) || (v == NoteType::Ka) || (v == NoteType::Drumroll) || (v == NoteType::Balloon) || (v == NoteType::Fuse); }
	constexpr b8 IsBigNote(NoteType v) { return !IsSmallNote(v); }
	constexpr b8 IsHandNote(NoteType v) { return (v == NoteType::DonBigHand) || (v == NoteType::KaBigHand); }
	constexpr b8 IsDrumrollNote(NoteType v) { return (v == NoteType::Drumroll) || (v == NoteType::DrumrollBig); }
	constexpr b8 IsBalloonNote(NoteType v) { return (v == NoteType::Balloon) || (v == NoteType::BalloonSpecial) || (v == NoteType::Fuse); }
	constexpr b8 IsLongNote(NoteType v) { return IsDrumrollNote(v) || IsBalloonNote(v); }
	constexpr b8 IsRegularNote(NoteType v) { return !IsLongNote(v); }
	constexpr b8 IsFuseRoll(NoteType v) { return (v == NoteType::Fuse); }
	constexpr NoteType ToSmallNote(NoteType v)
	{
		switch (v)
		{
		case NoteType::Don: return NoteType::Don;
		case NoteType::DonBig: return NoteType::Don;
		case NoteType::Ka: return NoteType::Ka;
		case NoteType::KaBig: return NoteType::Ka;
		case NoteType::Drumroll: return NoteType::Drumroll;
		case NoteType::DrumrollBig: return NoteType::Drumroll;
		case NoteType::Balloon: return NoteType::Balloon;
		case NoteType::BalloonSpecial: return NoteType::Balloon;
		case NoteType::DonBigHand: return NoteType::Don;
		case NoteType::KaBigHand: return NoteType::Ka;
		case NoteType::KaDon: return NoteType::KaDon;
		case NoteType::Bomb: return NoteType::Bomb;
		case NoteType::Adlib: return NoteType::Adlib;
		case NoteType::Fuse: return NoteType::Fuse;
		default: return v;
		}
	}
	constexpr NoteType ToBigNote(NoteType v)
	{
		switch (v)
		{
		case NoteType::Don: return NoteType::DonBig;
		case NoteType::DonBig: return NoteType::DonBig;
		case NoteType::Ka: return NoteType::KaBig;
		case NoteType::KaBig: return NoteType::KaBig;
		case NoteType::Drumroll: return NoteType::DrumrollBig;
		case NoteType::DrumrollBig: return NoteType::DrumrollBig;
		case NoteType::Balloon: return NoteType::BalloonSpecial;
		case NoteType::BalloonSpecial: return NoteType::BalloonSpecial;
		case NoteType::DonBigHand: return NoteType::DonBigHand;
		case NoteType::KaBigHand: return NoteType::KaBigHand;
		case NoteType::KaDon: return NoteType::KaDon;
		case NoteType::Bomb: return NoteType::Bomb;
		case NoteType::Adlib: return NoteType::Adlib;
		case NoteType::Fuse: return NoteType::Fuse;
		default: return v;
		}
	}
	constexpr NoteType ToHandNote(NoteType v)
	{
		switch (v)
		{
		case NoteType::Don: return NoteType::DonBigHand;
		case NoteType::DonBig: return NoteType::DonBigHand;
		case NoteType::Ka: return NoteType::KaBigHand;
		case NoteType::KaBig: return NoteType::KaBigHand;
		case NoteType::DonBigHand: return NoteType::DonBigHand;
		case NoteType::KaBigHand: return NoteType::KaBigHand;
		default: return v;
		}
	}
	constexpr NoteType ToggleNoteSize(NoteType v) { return IsSmallNote(v) ? ToBigNote(v) : ToSmallNote(v); }
	constexpr NoteType ToSmallNoteIf(NoteType v, b8 condition) { return condition ? ToSmallNote(v) : v; }
	constexpr NoteType ToBigNoteIf(NoteType v, b8 condition) { return condition ? ToBigNote(v) : v; }
	constexpr NoteType ToHandNoteIf(NoteType v, b8 condition) { return condition ? ToHandNote(v) : v; }
	constexpr NoteType FlipNote(NoteType v)
	{
		switch (v)
		{
		case NoteType::Don: return NoteType::Ka;
		case NoteType::DonBig: return NoteType::KaBig;
		case NoteType::DonBigHand: return NoteType::KaBigHand;
		case NoteType::Ka: return NoteType::Don;
		case NoteType::KaBig: return NoteType::DonBig;
		case NoteType::KaBigHand: return NoteType::DonBigHand;
		default: return v;
		}
	}
	constexpr bool IsNoteFlippable(NoteType v) { return FlipNote(v) != v; }

	constexpr i32 DefaultBalloonPopCount(Beat beatDuration, i32 gridBarDivision) { return (beatDuration.Ticks / GetGridBeatSnap(gridBarDivision).Ticks); }

	enum class DifficultyType : u8
	{
		Easy,
		Normal,
		Hard,
		Oni,
		OniUra,
		Tower,
		Dan,
		Count
	};

	enum class Side : u8
	{
		Normal,
		Ex,
		Both,
		Count
	};

	constexpr cstr DifficultyTypeNames[EnumCount<DifficultyType>] =
	{
		"DIFFICULTY_TYPE_EASY",
		"DIFFICULTY_TYPE_NORMAL",
		"DIFFICULTY_TYPE_HARD",
		"DIFFICULTY_TYPE_ONI",
		"DIFFICULTY_TYPE_ONI_URA",
		"DIFFICULTY_TYPE_TOWER",
		"DIFFICULTY_TYPE_DAN",
	};

	static inline std::string GetStyleName(i32 style, i32 playerSide)
	{
		if (style == 1)
			return UI_Str("PLAYER_SIDE_STYLE_SINGLE");
		char buf[32];
		std::string res = (style == 2) ? UI_Str("PLAYER_SIDE_STYLE_DOUBLE")
			: std::string(buf, sprintf_s(buf, UI_Str("PLAYER_SIDE_STYLE_FMT_%d_STYLE"), style));
		std::string_view strPlaySide (buf, sprintf_s(buf, UI_Str("PLAYER_SIDE_PLAYER_FMT_%d_PLAYER"), playerSide));
		res += " ("; res += strPlaySide; res += ")";
		return res;
	}

	constexpr cstr TowerSideNames[EnumCount<Side>] =
	{
		"TOWER_SIDE_NORMAL",
		"TOWER_SIDE_EX",
		"TOWER_SIDE_BOTH",
	};

	enum class DifficultyLevel : u8
	{
		Min = 1,
		Max = 20
	};

	enum class DifficultyLevelDecimal : i8
	{
		None = -1,
		Min = 0,
		PlusThreshold = 5,
		Max = 9
	};

	enum class TowerLives : i32
	{
		Min = 0,
		Max = 99999
	};

	enum class BranchType : u8
	{
		Normal,
		Expert,
		Master,
		Count
	};

	enum class ScrollMethod : u8 
	{
		NMSCROLL,
		HBSCROLL,
		BMSCROLL,
		Count
	};

	constexpr std::string_view PluralSuffixDefault = "s"; // unfortunately cannot just pass the string literal for now

	template <typename TEvent>
	constexpr std::string_view DisplayNameOfChartEvent = std::declval<std::string_view>(); // Forbid usage unless specialized
	template <typename TEvent>
	constexpr std::string_view DisplayNameOfLongChartEvent = DisplayNameOfChartEvent<TEvent>;
	template <typename TEvent>
	constexpr std::string_view DisplayNameOfChartEvents = ConstevalStrJoined<DisplayNameOfChartEvent<TEvent>, PluralSuffixDefault>;
	template <typename TEvent>
	constexpr std::string_view DisplayNameOfLongChartEvents = DisplayNameOfChartEvents<TEvent>;

	template <> constexpr std::string_view DisplayNameOfChartEvent<TempoChange> = "Tempo Change";
	template <> constexpr std::string_view DisplayNameOfChartEvent<TimeSignatureChange> = "Time Signature Change";

	// TODO: Animations for create / delete AND for moving left / right (?)
	struct Note
	{
		Beat BeatTime;
		Beat BeatDuration;
		Time TimeOffset;
		NoteType Type;
		b8 IsSelected;
		i16 BalloonPopCount;
		f32 ClickAnimationTimeRemaining;
		f32 ClickAnimationTimeDuration;
		// NOTE: Temp inline storage for rendering
		mutable NoteSEType TempSEType;

		constexpr Beat GetStart() const { return BeatTime; }
		constexpr Beat GetEnd() const { return BeatTime + BeatDuration; }
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<Note> = "Note";
	template <> constexpr std::string_view DisplayNameOfLongChartEvent<Note> = "Long Note";

	static_assert(sizeof(Note) == 32, "Accidentally introduced padding to Note struct (?)");

	template <typename TEvent>
	TEvent FallbackEvent = std::declval<TEvent>(); // Forbid usage unless specialized

	template <>
	constexpr TempoChange FallbackEvent<TempoChange> = {Beat::Zero(), FallbackTempo};
	template <>
	constexpr TimeSignatureChange FallbackEvent<TimeSignatureChange> = {Beat::Zero(), FallbackTimeSignature};

	struct ScrollChange
	{
		Beat BeatTime;
		Complex ScrollSpeed;
		b8 IsSelected;
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<ScrollChange> = "Scroll Changes";

	template <>
	constexpr ScrollChange FallbackEvent<ScrollChange> = {Beat::Zero(), Complex(1.0f, 0.0f)};

	struct ScrollType
	{
		Beat BeatTime;
		ScrollMethod Method;
		b8 IsSelected;

		std::string Method_ToString() const {
			switch (Method)
			{
				case ScrollMethod::HBSCROLL:
					return "HBSCROLL";
				case ScrollMethod::BMSCROLL:
					return "BMSCROLL";
				case ScrollMethod::NMSCROLL:
				default:
					return "Normal";
			}
		}
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<ScrollType> = "Scroll Type";

	template <>
	constexpr ScrollType FallbackEvent<ScrollType> = {Beat::Zero(), ScrollMethod::NMSCROLL};

	struct JPOSScrollChange
	{
		Beat BeatTime;
		Complex Move;
		f32 Duration;
		b8 IsSelected;
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<JPOSScrollChange> = "JPOSScroll";

	template <>
	constexpr JPOSScrollChange FallbackEvent<JPOSScrollChange> = {Beat::Zero(), Complex(100.0f, 0.0f), 0.f};

	struct BarLineChange
	{
		Beat BeatTime;
		b8 IsVisible;
		b8 IsSelected;
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<BarLineChange> = "Bar Line Change";

	template <>
	constexpr BarLineChange FallbackEvent<BarLineChange> = {Beat::Zero(), true};

	struct GoGoRange
	{
		Beat BeatTime;
		Beat BeatDuration;
		b8 IsSelected;
		f32 ExpansionAnimationCurrent = 0.0f;
		f32 ExpansionAnimationTarget = 1.0f;

		constexpr Beat GetStart() const { return BeatTime; }
		constexpr Beat GetEnd() const { return BeatTime + BeatDuration; }
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<GoGoRange> = "Go-Go Range";

	template <>
	constexpr GoGoRange FallbackEvent<GoGoRange> = {};

	struct LyricChange
	{
		Beat BeatTime;
		std::string Lyric;
		b8 IsSelected;
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<LyricChange> = "Lyric Change";

	template <>
	inline LyricChange FallbackEvent<LyricChange> = {};

	struct DelayChange
	{
		Beat BeatTime;
		Time Delay;
		b8 IsSelected;
	};
	template <> constexpr std::string_view DisplayNameOfChartEvent<DelayChange> = "Delay Change";

	template <>
	constexpr DelayChange FallbackEvent<DelayChange> = { Beat::Zero(), Time::Zero() };

	using SortedNotesList = BeatSortedList<Note>;
	using SortedScrollChangesList = BeatSortedList<ScrollChange>;
	using SortedBarLineChangesList = BeatSortedList<BarLineChange>;
	using SortedGoGoRangesList = BeatSortedList<GoGoRange>;
	using SortedLyricsList = BeatSortedList<LyricChange>;
	using SortedJPOSScrollChangesList = BeatSortedList<JPOSScrollChange>;
	using SortedScrollTypesList = BeatSortedList<ScrollType>;
	using SortedDelayChangesList = BeatSortedList<DelayChange>;

	// DelayChange.Delay stores the delta added by this specific #DELAY command.
	// These functions compute the accumulated total by summing all previous deltas.
	inline Time GetDelayAtBeat(const SortedDelayChangesList& delayChanges, Beat beat)
	{
		Time accumulated = Time::Zero();
		for (const DelayChange& dc : delayChanges.Sorted)
		{
			if (dc.BeatTime > beat) break;
			accumulated.Seconds += dc.Delay.Seconds;
		}
		return accumulated;
	}

	// Returns the total accumulated delay strictly BEFORE the given beat (exclusive).
	inline Time GetDelayBeforeBeat(const SortedDelayChangesList& delayChanges, Beat beat)
	{
		Time accumulated = Time::Zero();
		for (const DelayChange& dc : delayChanges.Sorted)
		{
			if (dc.BeatTime >= beat) break;
			accumulated.Seconds += dc.Delay.Seconds;
		}
		return accumulated;
	}

	// Converts an effective (delay-adjusted) time to a beat.
	// Returns false if the time falls inside a dead zone (gap created by a delay increase).
	// When in a dead zone, outBeat is set to the beat of that delay change boundary (so callers can clamp to it).
	inline b8 EffectiveTimeToBeat(const SortedTempoMap& tempoMap, const SortedDelayChangesList& delayChanges, Time effectiveTime, Beat& outBeat)
	{
		Time prevAccumulated = Time::Zero();
		Time accumulated = Time::Zero();
		for (const DelayChange& dc : delayChanges.Sorted)
		{
			accumulated.Seconds += dc.Delay.Seconds;
			const Time gapStart = Time::FromSec(tempoMap.BeatToTime(dc.BeatTime).Seconds + prevAccumulated.Seconds);
			const Time gapEnd   = Time::FromSec(tempoMap.BeatToTime(dc.BeatTime).Seconds + accumulated.Seconds);

			if (effectiveTime.Seconds < gapStart.Seconds)
			{
				outBeat = tempoMap.TimeToBeat(Time::FromSec(Max(0.0, effectiveTime.Seconds - prevAccumulated.Seconds)));
				if (outBeat.Ticks < 0) outBeat = Beat::Zero(); // guard: i32 overflow in ultra-high-BPM sections
				return true;
			}
			if (dc.Delay.Seconds > 0.0 && effectiveTime.Seconds < gapEnd.Seconds)
			{
				outBeat = dc.BeatTime;
				return false;
			}
			prevAccumulated = accumulated;
		}
		outBeat = tempoMap.TimeToBeat(Time::FromSec(Max(0.0, effectiveTime.Seconds - prevAccumulated.Seconds)));
		if (outBeat.Ticks < 0) outBeat = Beat::Zero(); // guard: i32 overflow in ultra-high-BPM sections
		return true;
	}

	constexpr Tempo ScrollSpeedToTempo(f32 scrollSpeed, Tempo baseTempo) { return Tempo(scrollSpeed * baseTempo.BPM); }
	constexpr f32 ScrollTempoToSpeed(Tempo scrollTempo, Tempo baseTempo) { return (baseTempo.BPM == 0.0f) ? 0.0f : (scrollTempo.BPM / baseTempo.BPM); }

	constexpr b8 VisibleOrDefault(const BarLineChange* v) { return (v == nullptr) ? true : v->IsVisible; }
	constexpr ScrollMethod ScrollTypeOrDefault(const ScrollType* v) { return (v == nullptr) ? ScrollMethod::NMSCROLL : v->Method; }
	constexpr Complex ScrollOrDefault(const ScrollChange* v) { return (v == nullptr) ? Complex(1.0f, 0.0f) : v->ScrollSpeed; }
	constexpr Tempo TempoOrDefault(const TempoChange* v) { return (v == nullptr) ? FallbackTempo : v->Tempo; }

	struct ChartCourse
	{
		DifficultyType Type = DifficultyType::Oni;
		DifficultyLevel Level = DifficultyLevel { 1 };
		DifficultyLevelDecimal Decimal = DifficultyLevelDecimal::None;
		i32 Style = 1;
		i32 PlayerSide = 1;

		std::string CourseCreator;

		SortedTempoMap TempoMap;

		// TODO: Have per-branch scroll speed changes (?)
		SortedNotesList Notes_Normal;
		SortedNotesList Notes_Expert;
		SortedNotesList Notes_Master;

		SortedScrollChangesList ScrollChanges;
		SortedBarLineChangesList BarLineChanges;
		SortedGoGoRangesList GoGoRanges;
		SortedLyricsList Lyrics;

		SortedScrollTypesList ScrollTypes;
		SortedJPOSScrollChangesList JPOSScrollChanges;
		SortedDelayChangesList DelayChanges;

		i32 ScoreInit = 0;
		i32 ScoreDiff = 0;

		// Tower specific
		TowerLives Life = TowerLives{ 5 };
		Side Side = Side::Normal;

		std::map<std::string, std::string> OtherMetadata;

		inline auto& GetNotes(BranchType branch) { assert(branch < BranchType::Count); return (&Notes_Normal)[EnumToIndex(branch)]; }
		inline auto& GetNotes(BranchType branch) const { assert(branch < BranchType::Count); return (&Notes_Normal)[EnumToIndex(branch)]; }

		void RecalculateSENotes() const
		{
			for (BranchType branch = BranchType::Normal; branch < BranchType::Count; IncrementEnum(branch))
				RecalculateSENotes(branch);
		}

		void RecalculateSENotes(BranchType branch) const; // implemented in chart_editor_widgets_game.cpp
	};

	// NOTE: Internal representation of a chart. Can then be imported / exported as .tja (and maybe as the native fumen binary format too eventually?)
	struct ChartProject
	{
		std::vector<std::unique_ptr<ChartCourse>> Courses;

		Time ChartDuration = {};
		std::string ChartTitle;
		std::map<std::string, std::string> ChartTitleLocalized; // (alphabetically) ordered
		std::string ChartSubtitle;
		std::map<std::string, std::string> ChartSubtitleLocalized;
		std::string ChartCreator;
		std::string ChartGenre;
		std::string ChartLyricsFileName;

		Time SongOffset = {};
		Time SongDemoStartTime = {};
		std::string SongFileName;
		std::string SongJacket;

		f32 SongVolume = 1.0f;
		f32 SoundEffectVolume = 1.0f;

		std::string BackgroundImageFileName;
		std::string BackgroundMovieFileName;
		Time MovieOffset = {};

		std::map<std::string, std::string> OtherMetadata;

		// TODO: Maybe change to GetDurationOr(Time defaultDuration) and always pass in context.SongDuration (?)
		inline Time GetDurationOrDefault() const { return (ChartDuration.Seconds <= 0.0) ? Time::FromMin(1.0) : ChartDuration; }
	};

	template <auto ChartProject::* Attr>
	extern constexpr std::string_view DisplayNameOfChartProjectAttr; // defined later

	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartDuration> = "Chart Duration";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartTitle> = "Chart Title";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartTitleLocalized> = "Chart Title Localized";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartSubtitle> = "Chart Subtitle";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartSubtitleLocalized> = "Chart Subtitle Localized";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartCreator> = "Chart Creator";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartGenre> = "Chart Genre";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::ChartLyricsFileName> = "Chart Lyrics File";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SongOffset> = "Song Offset";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SongDemoStartTime> = "Song Demo Start";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SongFileName> = "Song File";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SongJacket> = "Song Jacket";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SongVolume> = "Song Volume";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::SoundEffectVolume> = "Sound Effect Volume";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::BackgroundImageFileName> = "Background Image";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::BackgroundMovieFileName> = "Background Movie";
	template <> constexpr std::string_view DisplayNameOfChartProjectAttr<&ChartProject::MovieOffset> = "Movie Offset";

	// NOTE: Chart Space -> Starting at 00:00.000 (as most internal calculations are done in)
	//		  Song Space -> Starting relative to Song Offset (sometimes useful for displaying to the user)
	enum class TimeSpace : u8 { Chart, Song };
	constexpr Time ChartToSongTimeSpace(Time inTime, Time songOffset) { return (inTime - songOffset); }
	constexpr Time SongToChartTimeSpace(Time inTime, Time songOffset) { return (inTime + songOffset); }
	constexpr Time ConvertTimeSpace(Time v, TimeSpace in, TimeSpace out, Time songOffset) { v = (in == out) ? v : (in == TimeSpace::Chart) ? (v - songOffset) : (v + songOffset); return (v == Time { -0.0 }) ? Time {} : v; }
	constexpr Time ConvertTimeSpace(Time v, TimeSpace in, TimeSpace out, const ChartProject& chart) { return ConvertTimeSpace(v, in, out, chart.SongOffset); }

	using DebugCompareChartsOnMessageFunc = void(*)(std::string_view message, void* userData);
	void DebugCompareCharts(const ChartProject& chartA, const ChartProject& chartB, DebugCompareChartsOnMessageFunc onMessageFunc, void* userData = nullptr);

	Beat FindCourseMaxUsedBeat(const ChartCourse& course);
	b8 CreateChartProjectFromTJA(const TJA::ParsedTJA& inTJA, ChartProject& out);
	b8 ConvertChartProjectToTJA(const ChartProject& in, TJA::ParsedTJA& out, b8 includePeepoDrumKitComment = true);
}

namespace PeepoDrumKit
{
	enum class GenericList : u8
	{
		TempoChanges,
		SignatureChanges,
		Notes_Normal,
		Notes_Expert,
		Notes_Master,
		ScrollChanges,
		BarLineChanges,
		GoGoRanges,
		Lyrics,
		ScrollType,
		JPOSScroll,
		DelayChanges,
		Count
	};

	enum class GenericMember : u8
	{
		B8_IsSelected,
		B8_BarLineVisible,
		I16_BalloonPopCount,
		F32_ScrollSpeed,
		Beat_Start,
		Beat_Duration,
		Time_Offset,
		NoteType_V,
		Tempo_V,
		TimeSignature_V,
		CStr_Lyric,
		I8_ScrollType,
		F32_JPOSScroll,
		F32_JPOSScrollDuration,
		Count
	};

	using GenericMemberFlags = u32;
	constexpr GenericMemberFlags EnumToFlag(GenericMember type) { return (1u << static_cast<u32>(type)); }
	enum GenericMemberFlagsEnum : GenericMemberFlags
	{
		GenericMemberFlags_None = 0,
		GenericMemberFlags_IsSelected = EnumToFlag(GenericMember::B8_IsSelected),
		GenericMemberFlags_BarLineVisible = EnumToFlag(GenericMember::B8_BarLineVisible),
		GenericMemberFlags_BalloonPopCount = EnumToFlag(GenericMember::I16_BalloonPopCount),
		GenericMemberFlags_ScrollSpeed = EnumToFlag(GenericMember::F32_ScrollSpeed),
		GenericMemberFlags_Start = EnumToFlag(GenericMember::Beat_Start),
		GenericMemberFlags_Duration = EnumToFlag(GenericMember::Beat_Duration),
		GenericMemberFlags_Offset = EnumToFlag(GenericMember::Time_Offset),
		GenericMemberFlags_NoteType = EnumToFlag(GenericMember::NoteType_V),
		GenericMemberFlags_Tempo = EnumToFlag(GenericMember::Tempo_V),
		GenericMemberFlags_TimeSignature = EnumToFlag(GenericMember::TimeSignature_V),
		GenericMemberFlags_Lyric = EnumToFlag(GenericMember::CStr_Lyric),
		GenericMemberFlags_ScrollType = EnumToFlag(GenericMember::I8_ScrollType),
		GenericMemberFlags_JPOSScroll = EnumToFlag(GenericMember::F32_JPOSScroll),
		GenericMemberFlags_JPOSScrollDuration = EnumToFlag(GenericMember::F32_JPOSScrollDuration),
		GenericMemberFlags_All = 0b11111111111111,
	};

	static_assert(GenericMemberFlags_All & (1u << (static_cast<u32>(GenericMember::Count) - 1)));
	static_assert(!(GenericMemberFlags_All & (1u << static_cast<u32>(GenericMember::Count))));

	constexpr cstr GenericListNames[] = { "TempoChanges", "SignatureChanges", "Notes_Normal", "Notes_Expert", "Notes_Master", "ScrollChanges", "BarLineChanges", "GoGoRanges", "Lyrics", "ScrollType", "JPOSScroll", "DelayChanges", };
	constexpr cstr GenericMemberNames[] = { "IsSelected", "BarLineVisible", "BalloonPopCount", "ScrollSpeed", "Start", "Duration", "Offset", "NoteType", "Tempo", "TimeSignature", "Lyric", "ScrollType", "JPOSScroll", "JPOSScrollDuration", };

	// Member availability queries
	template <typename T, GenericMember Member>
	extern constexpr b8 IsMemberAvailable; // defined later

	template <typename T, GenericMember... Members>
	constexpr GenericMemberFlags GetAvailableMemberFlags(enum_sequence<GenericMember, Members...>) {
		return (GenericMemberFlags_None | ... | (IsMemberAvailable<T, Members> ? EnumToFlag(Members) : 0));
	}

	template <typename T>
	constexpr GenericMemberFlags AvailableMemberFlags = ForceConsteval<GetAvailableMemberFlags<T>(make_enum_sequence<GenericMember>())>;

	union GenericMemberUnion
	{
		b8 B8;
		i16 I16;
		f32 F32;
		Beat Beat;
		Time Time;
		NoteType NoteType;
		Tempo Tempo;
		TimeSignature TimeSignature;
		cstr CStr;
		Complex CPX;

		inline GenericMemberUnion() { ::memset(this, 0, sizeof(*this)); }
		inline b8 operator==(const GenericMemberUnion& other) const { return (::memcmp(this, &other, sizeof(*this)) == 0); }
		inline b8 operator!=(const GenericMemberUnion& other) const { return !(*this == other); }
	};

	static_assert(sizeof(GenericMemberUnion) == 8);

	/// tuple-like GenericMember access definition

	// types with all members available

	template <GenericMember Member, typename GenericMemberUnionT, expect_type_t<GenericMemberUnionT, GenericMemberUnion> = true>
	constexpr decltype(auto) get(GenericMemberUnionT&& values)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<GenericMemberUnionT>(values).B8);
		else if constexpr (Member == GenericMember::B8_BarLineVisible) return (std::forward<GenericMemberUnionT>(values).B8);
		else if constexpr (Member == GenericMember::I16_BalloonPopCount) return (std::forward<GenericMemberUnionT>(values).I16);
		else if constexpr (Member == GenericMember::F32_ScrollSpeed) return (std::forward<GenericMemberUnionT>(values).CPX);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<GenericMemberUnionT>(values).Beat);
		else if constexpr (Member == GenericMember::Beat_Duration) return (std::forward<GenericMemberUnionT>(values).Beat);
		else if constexpr (Member == GenericMember::Time_Offset) return (std::forward<GenericMemberUnionT>(values).Time);
		else if constexpr (Member == GenericMember::NoteType_V) return (std::forward<GenericMemberUnionT>(values).NoteType);
		else if constexpr (Member == GenericMember::Tempo_V) return (std::forward<GenericMemberUnionT>(values).Tempo);
		else if constexpr (Member == GenericMember::TimeSignature_V) return (std::forward<GenericMemberUnionT>(values).TimeSignature);
		else if constexpr (Member == GenericMember::CStr_Lyric) return (std::forward<GenericMemberUnionT>(values).CStr);
		else if constexpr (Member == GenericMember::I8_ScrollType) return (std::forward<GenericMemberUnionT>(values).I16);
		else if constexpr (Member == GenericMember::F32_JPOSScroll) return (std::forward<GenericMemberUnionT>(values).CPX);
		else if constexpr (Member == GenericMember::F32_JPOSScrollDuration) return (std::forward<GenericMemberUnionT>(values).F32);
		else static_assert(false, "unhandled or invalid GenericMember value");
	}

	template <GenericMember Member>
	using GenericMemberType = std::remove_cv_t<std::remove_reference_t<decltype(get<Member>(std::declval<GenericMemberUnion>()))>>;

	template <GenericMember Member, typename AllGenericMembersUnionArrayT, expect_type_t<AllGenericMembersUnionArrayT, struct AllGenericMembersUnionArray> = true>
	constexpr decltype(auto) get(AllGenericMembersUnionArrayT&& values)
	{
		return get<Member>(std::forward<AllGenericMembersUnionArrayT>(values)[Member]);
	}

	// defined here due to dependency
	struct AllGenericMembersUnionArray
	{
		GenericMemberUnion V[EnumCount<GenericMember>];

		constexpr GenericMemberUnion& operator[](GenericMember member) { return V[EnumToIndex(member)]; }
		constexpr const GenericMemberUnion& operator[](GenericMember member) const { return V[EnumToIndex(member)]; }

		constexpr auto& IsSelected() { return get<GenericMember::B8_IsSelected>(*this); }
		constexpr auto& BarLineVisible() { return get<GenericMember::B8_BarLineVisible>(*this); }
		constexpr auto& BalloonPopCount() { return get<GenericMember::I16_BalloonPopCount>(*this); }
		constexpr auto& ScrollSpeed() { return get<GenericMember::F32_ScrollSpeed>(*this); }
		constexpr auto& BeatStart() { return get<GenericMember::Beat_Start>(*this); }
		constexpr auto& BeatDuration() { return get<GenericMember::Beat_Duration>(*this); }
		constexpr auto& TimeOffset() { return get<GenericMember::Time_Offset>(*this); }
		constexpr auto& NoteType() { return get<GenericMember::NoteType_V>(*this); }
		constexpr auto& Tempo() { return get<GenericMember::Tempo_V>(*this); }
		constexpr auto& TimeSignature() { return get<GenericMember::TimeSignature_V>(*this); }
		constexpr auto& Lyric() { return get<GenericMember::CStr_Lyric>(*this); }
		constexpr auto& ScrollType() { return get<GenericMember::I8_ScrollType>(*this); }
		constexpr auto& JPOSScrollMove() { return get<GenericMember::F32_JPOSScroll>(*this); }
		constexpr auto& JPOSScrollDuration() { return get<GenericMember::F32_JPOSScrollDuration>(*this); }
		constexpr const auto& IsSelected() const { return get<GenericMember::B8_IsSelected>(*this); }
		constexpr const auto& BarLineVisible() const { return get<GenericMember::B8_BarLineVisible>(*this); }
		constexpr const auto& BalloonPopCount() const { return get<GenericMember::I16_BalloonPopCount>(*this); }
		constexpr const auto& ScrollSpeed() const { return get<GenericMember::F32_ScrollSpeed>(*this); }
		constexpr const auto& BeatStart() const { return get<GenericMember::Beat_Start>(*this); }
		constexpr const auto& BeatDuration() const { return get<GenericMember::Beat_Duration>(*this); }
		constexpr const auto& TimeOffset() const { return get<GenericMember::Time_Offset>(*this); }
		constexpr const auto& NoteType() const { return get<GenericMember::NoteType_V>(*this); }
		constexpr const auto& Tempo() const { return get<GenericMember::Tempo_V>(*this); }
		constexpr const auto& TimeSignature() const { return get<GenericMember::TimeSignature_V>(*this); }
		constexpr const auto& Lyric() const { return get<GenericMember::CStr_Lyric>(*this); }
		constexpr const auto& ScrollType() const { return get<GenericMember::I8_ScrollType>(*this); }
		constexpr const auto& JPOSScrollMove() const { return get<GenericMember::F32_JPOSScroll>(*this); }
		constexpr const auto& JPOSScrollDuration() const { return get<GenericMember::F32_JPOSScrollDuration>(*this); }
	};

	// types with subset members, return `void` for unavailable members

	// member accessing in decltype(), enclose by () for returning a reference
	template <GenericMember Member, typename TempoChangeT, expect_type_t<TempoChangeT, TempoChange> = true>
	constexpr decltype(auto) get(TempoChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<TempoChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<TempoChangeT>(event).Beat);
		else if constexpr (Member == GenericMember::Tempo_V) return (std::forward<TempoChangeT>(event).Tempo);
	}

	template <GenericMember Member, typename TimeSignatureChangeT, expect_type_t<TimeSignatureChangeT, TimeSignatureChange> = true>
	constexpr decltype(auto) get(TimeSignatureChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<TimeSignatureChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<TimeSignatureChangeT>(event).Beat);
		else if constexpr (Member == GenericMember::TimeSignature_V) return (std::forward<TimeSignatureChangeT>(event).Signature);
	}

	template <GenericMember Member, typename NoteT, expect_type_t<NoteT, Note> = true>
	constexpr decltype(auto) get(NoteT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<NoteT>(event).IsSelected);
		else if constexpr (Member == GenericMember::I16_BalloonPopCount) return (std::forward<NoteT>(event).BalloonPopCount);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<NoteT>(event).BeatTime);
		else if constexpr (Member == GenericMember::Beat_Duration) return (std::forward<NoteT>(event).BeatDuration);
		else if constexpr (Member == GenericMember::Time_Offset) return (std::forward<NoteT>(event).TimeOffset);
		else if constexpr (Member == GenericMember::NoteType_V) return (std::forward<NoteT>(event).Type);
	}

	template <GenericMember Member, typename ScrollChangeT, expect_type_t<ScrollChangeT, ScrollChange> = true>
	constexpr decltype(auto) get(ScrollChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<ScrollChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::F32_ScrollSpeed) return (std::forward<ScrollChangeT>(event).ScrollSpeed);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<ScrollChangeT>(event).BeatTime);
	}

	template <GenericMember Member, typename BarLineChangeT, expect_type_t<BarLineChangeT, BarLineChange> = true>
	constexpr decltype(auto) get(BarLineChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<BarLineChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::B8_BarLineVisible) return (std::forward<BarLineChangeT>(event).IsVisible);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<BarLineChangeT>(event).BeatTime);
	}

	template <GenericMember Member, typename GoGoRangeT, expect_type_t<GoGoRangeT, GoGoRange> = true>
	constexpr decltype(auto) get(GoGoRangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<GoGoRangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<GoGoRangeT>(event).BeatTime);
		else if constexpr (Member == GenericMember::Beat_Duration) return (std::forward<GoGoRangeT>(event).BeatDuration);
	}

	template <GenericMember Member, typename LyricChangeT, expect_type_t<LyricChangeT, LyricChange> = true>
	constexpr decltype(auto) get(LyricChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<LyricChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<LyricChangeT>(event).BeatTime);
		else if constexpr (Member == GenericMember::CStr_Lyric) return (std::forward<LyricChangeT>(event).Lyric);
	}

	template <GenericMember Member, typename ScrollTypeT, expect_type_t<ScrollTypeT, ScrollType> = true>
	constexpr decltype(auto) get(ScrollTypeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<ScrollTypeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::I8_ScrollType) return (std::forward<ScrollTypeT>(event).Method);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<ScrollTypeT>(event).BeatTime);
	}

	template <GenericMember Member, typename JPOSScrollChangeT, expect_type_t<JPOSScrollChangeT, JPOSScrollChange> = true>
	constexpr decltype(auto) get(JPOSScrollChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<JPOSScrollChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::F32_JPOSScroll) return (std::forward<JPOSScrollChangeT>(event).Move);
		else if constexpr (Member == GenericMember::F32_JPOSScrollDuration) return (std::forward<JPOSScrollChangeT>(event).Duration);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<JPOSScrollChangeT>(event).BeatTime);
	}

	template <GenericMember Member, typename DelayChangeT, expect_type_t<DelayChangeT, DelayChange> = true>
	constexpr decltype(auto) get(DelayChangeT&& event)
	{
		if constexpr (Member == GenericMember::B8_IsSelected) return (std::forward<DelayChangeT>(event).IsSelected);
		else if constexpr (Member == GenericMember::Beat_Start) return (std::forward<DelayChangeT>(event).BeatTime);
		else if constexpr (Member == GenericMember::Time_Offset) return (std::forward<DelayChangeT>(event).Delay);
	}

	// Member availability queries
	template <typename T, auto Tag, typename = void>
	struct has_get_t : std::false_type {};

	template <typename T, auto Tag>
	struct has_get_t<T, Tag, std::void_t<decltype(get<Tag>(std::forward<T>(std::declval<T&&>())))>> : std::true_type {};

	template <typename T, auto Tag>
	constexpr b8 has_get_v = has_get_t<T, Tag>::value;

	template <auto Tag, typename T>
	constexpr decltype(auto) get_or_forward(T&& value)
	{
		if constexpr (has_get_v<T, Tag>)
			return get<Tag>(std::forward<T>(value));
		else
			return std::forward<T>(value);
	}

	template <typename T, GenericMember Member>
	constexpr b8 IsMemberAvailable = has_get_v<T, Member> && !std::is_void_v<decltype(get_or_forward<Member>(std::declval<T>()))>;

	// Apply `action` on `args` resolved by `member` if available, otherwise return `vDefault` on nothing if valid, otherwise return `vError`
	// If `TRet` is not specified, all of `action`'s possible return values, `vDefault`, and `vError` must have the same type
	template <typename TRet = keep_deduced_t, typename FAction, typename TDefault, typename TError, typename... TCastedArgs >
	constexpr decltype(auto) ApplySingleGenericMember(GenericMember member, FAction&& action, TDefault&& vDefault, TError&& vError, TCastedArgs&&... args)
	{
		// unfortunately, as for C++20, there are no ways to make a switch-like lookup reliably without typing out all the cases
		switch (member) {
#define X(_Member) { \
		case (_Member): \
			if constexpr ((... && IsMemberAvailable<TCastedArgs, (_Member)>)) \
				return keep_or_static_cast<TRet>(action(get_or_forward<(_Member)>(std::forward<TCastedArgs>(args))...)); \
			else \
				return keep_or_static_cast<TRet>(vDefault); \
		}
		X(GenericMember::B8_IsSelected)
		X(GenericMember::B8_BarLineVisible)
		X(GenericMember::I16_BalloonPopCount)
		X(GenericMember::F32_ScrollSpeed)
		X(GenericMember::Beat_Start)
		X(GenericMember::Beat_Duration)
		X(GenericMember::Time_Offset)
		X(GenericMember::NoteType_V)
		X(GenericMember::Tempo_V)
		X(GenericMember::TimeSignature_V)
		X(GenericMember::CStr_Lyric)
		X(GenericMember::I8_ScrollType)
		X(GenericMember::F32_JPOSScroll)
		X(GenericMember::F32_JPOSScrollDuration)
#undef X
		default: assert(false); return keep_or_static_cast<TRet>(vError);
		}
	}

	template <GenericMember Member, typename FAction, typename T, typename... Args>
	constexpr b8 TryDoImpl(FAction&& action, T&& event, Args&&... args)
	{
		if constexpr (IsMemberAvailable<T, Member>) {
			action(get<Member>(std::forward<T>(event)), get_or_forward<Member>(std::forward<Args>(args))...);
			return true;
		}
		else {
			return false;
		}
	}

	template <typename FAction, typename T, typename... Args>
	constexpr b8 TryDoImpl(FAction&& action, T&& event, GenericMember member, Args&&... args)
	{
		return ApplySingleGenericMember(member,
			[&](auto&& typedMember, auto&&... typedArgs)
			{
				action(std::forward<decltype(typedMember)>(typedMember), std::forward<decltype(typedArgs)>(typedArgs)...);
				return true;
			}, false, false,
			std::forward<decltype(event)>(event), std::forward<Args>(args)...);
	}

	template <GenericMember Member, typename GenericListStructT, expect_type_t<GenericListStructT, struct GenericListStruct> = true, typename FAction, typename... Args>
	constexpr b8 TryDo(FAction&& action, GenericListStructT&& in, GenericList list, Args&&...args)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedIn) -> bool
			{
				return TryDoImpl<Member>(std::forward<FAction>(action), std::forward<decltype(typedIn)>(typedIn), get_or_forward<Member>(std::forward<Args>(args)...));
			}, false,
			in);
	}

	template <typename GenericListStructT, expect_type_t<GenericListStructT, struct GenericListStruct> = true, typename FAction, typename... Args>
	constexpr b8 TryDo(FAction&& action, GenericListStructT&& in, GenericList list, GenericMember member, Args&&...args)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedIn)
			{
				return TryDoImpl(std::forward<FAction>(action), std::forward<decltype(typedIn)>(typedIn), member, std::forward<Args>(args)...);
			}, false,
			in);
	}

	// need to be lambdas to be used as arguments with to-be-deduced parameter types (not needed since C++20)
	constexpr auto GetGeneric = [&](auto&& typedMember, auto& typedOutValue)
	{
		if constexpr (expect_type_v<decltype(typedMember), std::string> && !expect_type_v<decltype(typedOutValue), std::string>) // for GenericMember::CStr_Lyric
			typedOutValue = typedMember.data();
		else
			typedOutValue = static_cast<std::remove_reference_t<decltype(typedOutValue)>>(typedMember);
	};

	constexpr auto SetGeneric = [&](auto& typedMember, auto&& typedInValue)
	{
		typedMember = static_cast<std::remove_reference_t<decltype(typedMember)>>(typedInValue);
	};

	// generic adapters
	// TryGet/Set(..., member, obj), where obj is a GenericMemberUnion object
	// TryGet/Set<Member>(..., obj), where obj is either a GenericMemberUnion object or a concrete type object
	template <auto... Tags, typename... Args>
	constexpr __forceinline decltype(auto) TryGet(Args&&... args) { return TryDo<Tags...>(GetGeneric, std::forward<Args>(args)...); }
	template <auto... Tags, typename... Args>
	constexpr __forceinline decltype(auto) TrySet(Args&&... args) { return TryDo<Tags...>(SetGeneric, std::forward<Args>(args)...); }

	// unfortunately the parameter order has to be changed to make function overloading works
	template <GenericMember Member, typename TDefault, typename... Args>
	constexpr __forceinline decltype(auto) GetOrDefault(TDefault&& vDefault, Args&&... args)
	{
		auto v = vDefault;
		TryGet<Member>(std::forward<Args>(args)..., v);
		return v;
	}

	template <GenericMember Member, typename TDefault = GenericMemberType<Member>, typename... Args>
	constexpr __forceinline decltype(auto) GetOrEmpty(Args&&... args)
	{
		return GetOrDefault<Member>(TDefault{}, std::forward<Args>(args)...);
	}

	// NOTE: Little helpers here just for convenience
	template <typename... Args>
	constexpr b8 GetIsSelected(Args&&... args) { return GetOrEmpty<GenericMember::B8_IsSelected>(std::forward<Args>(args)...); }
	template <typename... Args>
	constexpr Beat GetBeat(Args&&... args) { return GetOrEmpty<GenericMember::Beat_Start>(std::forward<Args>(args)...); }
	template <typename... Args>
	constexpr Beat GetBeatDuration(Args&&... args) { return GetOrEmpty<GenericMember::Beat_Duration>(std::forward<Args>(args)...); }
	template <typename... Args>
	constexpr std::tuple<bool, Time> GetTimeDuration(Args&&... args) { f32 v{}; return { TryGet<GenericMember::F32_JPOSScrollDuration>(std::forward<Args>(args)..., v), Time::FromSec(v) }; }

	// unfortunately the parameter order has to be changed to make function overloading works
	template <typename... Args>
	constexpr void SetIsSelected(b8 isSelected, Args&&... args) { TrySet<GenericMember::B8_IsSelected>(std::forward<Args>(args)..., isSelected); }
	template <typename... Args>
	constexpr void SetBeat(Beat beat, Args&&... args) { TrySet<GenericMember::Beat_Start>(std::forward<Args>(args)..., beat); }
	template <typename... Args>
	constexpr void SetBeatDuration(Beat beatDuration, Args&&... args) { TrySet<GenericMember::Beat_Duration>(std::forward<Args>(args)..., beatDuration); }
	template <typename... Args>
	constexpr void SetTimeDuration(Time timeDuration, Args&&... args) { TrySet<GenericMember::F32_JPOSScrollDuration>(std::forward<Args>(args)..., timeDuration.Seconds); }

	struct GenericListStruct
	{
		union PODData
		{
			TempoChange Tempo;
			TimeSignatureChange Signature;
			Note Note;
			ScrollChange Scroll;
			BarLineChange BarLine;
			GoGoRange GoGo;
			ScrollType ScrollType;
			JPOSScrollChange JPOSScroll;
			DelayChange Delay;

			inline PODData() { ::memset(this, 0, sizeof(*this)); }
		} POD;

		// NOTE: Handle separately due to constructor / destructor requirement
		struct NonTrivialData
		{
			LyricChange Lyric;
		} NonTrivial {};

		GenericListStruct(const GenericListStruct& other) {
			// Perform a deep copy of data within the union and other members
			::memcpy(&POD, &other.POD, sizeof(POD));
			NonTrivial = other.NonTrivial; // Copy the non-trivial data
		}

		GenericListStruct() {};

		template <typename TEvent, std::enable_if_t<!expect_type_v<TEvent, GenericListStruct>, bool> = true>
		constexpr GenericListStruct(TEvent&& event);
	};

	template <auto... Tags, typename FAction, typename GenericListStructWithTypeT, typename... Args,
		expect_type_t<GenericListStructWithTypeT, struct GenericListStructWithType> = true>
	constexpr b8 TryDo(FAction&& action, GenericListStructWithTypeT&& data, Args&&... args)
	{
		return TryDo<Tags...>(std::forward<FAction>(action), std::forward<GenericListStructWithTypeT>(data).Value, std::forward<GenericListStructWithTypeT>(data).List, std::forward<Args>(args)...);
	}

	struct GenericListStructWithType
	{
		GenericList List;
		GenericListStruct Value;

		// Default constructor: empty value
		GenericListStructWithType() : List(GenericList::Count), Value() {}

		// Constructor with parameters
		GenericListStructWithType(GenericList list, const GenericListStruct& value)
			: List(list), Value(value) {}

		template <typename TEvent, std::enable_if_t<!expect_type_v<TEvent, GenericListStruct>, bool> = true>
		constexpr GenericListStructWithType(GenericList list, TEvent&& event);
	};

	/// tuple-like GenericList access definition

	// member accessing in decltype(), enclose by () for returning a reference
	template <GenericList List, typename ChartCourseT, expect_type_t<ChartCourseT, ChartCourse> = true>
	constexpr decltype(auto) get(ChartCourseT&& course)
	{
		if constexpr (List == GenericList::TempoChanges) return (std::forward<ChartCourseT>(course).TempoMap.Tempo);
		else if constexpr (List == GenericList::SignatureChanges) return (std::forward<ChartCourseT>(course).TempoMap.Signature);
		else if constexpr (List == GenericList::Notes_Normal) return (std::forward<ChartCourseT>(course).Notes_Normal);
		else if constexpr (List == GenericList::Notes_Expert) return (std::forward<ChartCourseT>(course).Notes_Expert);
		else if constexpr (List == GenericList::Notes_Master) return (std::forward<ChartCourseT>(course).Notes_Master);
		else if constexpr (List == GenericList::ScrollChanges) return (std::forward<ChartCourseT>(course).ScrollChanges);
		else if constexpr (List == GenericList::BarLineChanges) return (std::forward<ChartCourseT>(course).BarLineChanges);
		else if constexpr (List == GenericList::GoGoRanges) return (std::forward<ChartCourseT>(course).GoGoRanges);
		else if constexpr (List == GenericList::Lyrics) return (std::forward<ChartCourseT>(course).Lyrics);
		else if constexpr (List == GenericList::ScrollType) return (std::forward<ChartCourseT>(course).ScrollTypes);
		else if constexpr (List == GenericList::JPOSScroll) return (std::forward<ChartCourseT>(course).JPOSScrollChanges);
		else if constexpr (List == GenericList::DelayChanges) return (std::forward<ChartCourseT>(course).DelayChanges);
		else static_assert(false, "unhandled or invalid GenericList value");
	}

	template <GenericList List>
	using GenericListType = std::remove_cv_t<std::remove_reference_t<decltype(get<List>(std::declval<ChartCourse>()))>>;

	template <GenericList List, typename GenericListStructT, expect_type_t<GenericListStructT, GenericListStruct> = true>
	constexpr decltype(auto) get(GenericListStructT&& inValue)
	{
		if constexpr (List == GenericList::TempoChanges) return (std::forward<GenericListStructT>(inValue).POD.Tempo);
		else if constexpr (List == GenericList::SignatureChanges) return (std::forward<GenericListStructT>(inValue).POD.Signature);
		else if constexpr (List == GenericList::Notes_Normal) return (std::forward<GenericListStructT>(inValue).POD.Note);
		else if constexpr (List == GenericList::Notes_Expert) return (std::forward<GenericListStructT>(inValue).POD.Note);
		else if constexpr (List == GenericList::Notes_Master) return (std::forward<GenericListStructT>(inValue).POD.Note);
		else if constexpr (List == GenericList::ScrollChanges) return (std::forward<GenericListStructT>(inValue).POD.Scroll);
		else if constexpr (List == GenericList::BarLineChanges) return (std::forward<GenericListStructT>(inValue).POD.BarLine);
		else if constexpr (List == GenericList::GoGoRanges) return (std::forward<GenericListStructT>(inValue).POD.GoGo);
		else if constexpr (List == GenericList::Lyrics) return (std::forward<GenericListStructT>(inValue).NonTrivial.Lyric);
		else if constexpr (List == GenericList::ScrollType) return (std::forward<GenericListStructT>(inValue).POD.ScrollType);
		else if constexpr (List == GenericList::JPOSScroll) return (std::forward<GenericListStructT>(inValue).POD.JPOSScroll);
		else if constexpr (List == GenericList::DelayChanges) return (std::forward<GenericListStructT>(inValue).POD.Delay);
		else static_assert(false, "unhandled or invalid GenericList value");
	}

	// Access functions for concrete GenericListStruct types
	template <GenericList List>
	using GenericListStructType = std::remove_cv_t<std::remove_reference_t<decltype(get<List>(std::declval<GenericListStruct>()))>>;

	template <typename T>
	struct IsNonListChartEventTrait : std::false_type { };

	template <typename T, typename = void>
	struct IsChartEventTypeHelper : std::false_type { };

	template <typename T, GenericList... Lists>
	struct IsChartEventTypeHelper<T, enum_sequence<GenericList, Lists...>> : std::bool_constant<(... || expect_type_v<T, GenericListStructType<Lists>>)> { };

	template <typename T>
	constexpr b8 IsChartEventType = IsNonListChartEventTrait<std::remove_cv_t<std::remove_reference_t<T>>>::value || IsChartEventTypeHelper<T, make_enum_sequence<GenericList>>::value;

	template <typename T, typename = void>
	struct ChartEventTypeToGenericListHelper : std::false_type {};

	template <typename T, GenericList... Lists>
	struct ChartEventTypeToGenericListHelper<T, enum_sequence<GenericList, Lists...>>
		: std::conditional_t<IsChartEventType<T>,
			std::integral_constant<GenericList, std::min({ (expect_type_v<T, GenericListStructType<Lists>> ? Lists : GenericList::Count)... })>,
			std::false_type>
		{};

	template <typename T>
	constexpr GenericList ChartEventTypeToGenericList = ChartEventTypeToGenericListHelper<T, make_enum_sequence<GenericList>>::value;

	template <typename TEvent, typename GenericListStructT, expect_type_t<GenericListStructT, GenericListStruct> = true>
	constexpr decltype(auto) get(GenericListStructT&& inValue)
	{
		return get<ChartEventTypeToGenericList<TEvent>>(std::forward<GenericListStructT>(inValue));
	}

	template <typename TEvent, std::enable_if_t<!expect_type_v<TEvent, GenericListStruct>, bool>>
	constexpr GenericListStruct::GenericListStruct(TEvent&& event) { get<TEvent>(*this) = event; };

	template <typename TEvent, std::enable_if_t<!expect_type_v<TEvent, GenericListStruct>, bool>>
	constexpr GenericListStructWithType::GenericListStructWithType(GenericList list, TEvent&& event) : List(list) { get<TEvent>(Value) = event; };

	template <GenericMember Member, typename FAction, typename T, std::enable_if_t<IsChartEventType<T>, bool> = true, typename... Args>
	constexpr __forceinline b8 TryDo(FAction&& action, T&& event, Args&&... args)
	{
		return TryDoImpl<Member>(std::forward<FAction>(action), std::forward<decltype(event)>(event), get_or_forward<Member>(std::forward<Args>(args)...));
	}

	template <typename FAction, typename T, std::enable_if_t<IsChartEventType<T>, bool> = true, typename... Args>
	constexpr __forceinline b8 TryDo(FAction&& action, T&& event, GenericMember member, Args&&... args)
	{
		return TryDoImpl(std::forward<FAction>(action), std::forward<decltype(event)>(event), member, std::forward<Args>(args)...);
	}

	// Apply `action` on `args` resolved by `list` if valid, otherwise return `vError`
	// If `TRet` is not specified, all of `action`'s possible return values and `vError` must have the same type
	template <typename TRet = keep_deduced_t, typename TDefault, typename FAction, typename... TCastedArgs>
	constexpr decltype(auto) ApplySingleGenericList(GenericList list, FAction&& action, TDefault&& vError, TCastedArgs&&... args)
	{
		// unfortunately, as for C++20, there are no ways to make a switch-like lookup reliably without typing out all the cases
		switch (list) {
#define X(_List) \
		{ case (_List): return keep_or_static_cast<TRet>(action(get_or_forward<(_List)>(std::forward<TCastedArgs>(args))...)); }
		X(GenericList::TempoChanges)
		X(GenericList::SignatureChanges)
		X(GenericList::Notes_Normal)
		X(GenericList::Notes_Expert)
		X(GenericList::Notes_Master)
		X(GenericList::ScrollChanges)
		X(GenericList::BarLineChanges)
		X(GenericList::GoGoRanges)
		X(GenericList::Lyrics)
		X(GenericList::ScrollType)
		X(GenericList::JPOSScroll)
		X(GenericList::DelayChanges)
#undef X
		default: assert(false); return keep_or_static_cast<TRet>(vError);
		}
	}

	// Apply `action` on `args` resolved by every valid `list` (unrolled in compile-time)
	// The returned value from `action` is ignored for simplicity.
	template <GenericList... Lists, typename FAction, typename... TCastedArgs>
	constexpr void ApplyForEachGenericList(enum_sequence<GenericList, Lists...>, FAction&& action, TCastedArgs&&... args)
	{
		([&] {
			auto getSingle = [](auto&& arg) { return get<Lists>(std::forward<decltype(arg)>(arg)); };
			action(Lists, getSingle(std::forward<TCastedArgs>(args))...);
		}(), ...);
	}

	template <typename FAction, typename... TCastedArgs>
	constexpr void ApplyForEachGenericList(FAction&& action, TCastedArgs&&... args)
	{
		ApplyForEachGenericList(make_enum_sequence<GenericList>(), std::forward<FAction>(action), std::forward<TCastedArgs>(args)...);
	}

	// course list attribute query helpers
	struct GetRawByteSize_T {};

	template <GenericMember Member>
	constexpr __forceinline size_t get(GetRawByteSize_T) { return sizeof(GenericMemberType<Member>); }

	struct GetListStructAvailableMemberFlags_T {};

	template <GenericList List>
	constexpr __forceinline GenericMemberFlags get(GetListStructAvailableMemberFlags_T) {
		return AvailableMemberFlags<GenericListStructType<List>>;
	}

	// course list attribute query functions
	constexpr b8 IsNotesList(GenericList list) { return (list == GenericList::Notes_Normal) || (list == GenericList::Notes_Expert) || (list == GenericList::Notes_Master); }
	constexpr b8 ListHasDurations(GenericList list) { return IsNotesList(list) || (list == GenericList::GoGoRanges); }
	constexpr b8 ListUsesInclusiveBeatCheck(GenericList list) { return IsNotesList(list) || (list != GenericList::GoGoRanges && list != GenericList::Lyrics); }
	constexpr b8 ListIsItemEndBounded(GenericList list) { return IsNotesList(list) || (list == GenericList::GoGoRanges) || (list == GenericList::JPOSScroll); }
	constexpr b8 ListHasNoteStaticEffects(GenericList list) { return (list == GenericList::TempoChanges) || (list == GenericList::ScrollChanges) || (list == GenericList::ScrollType); }
	constexpr b8 ListHasBarlineStaticEffects(GenericList list) { return ListHasNoteStaticEffects(list) || (list == GenericList::BarLineChanges); }

	constexpr size_t GetGenericMember_RawByteSize(GenericMember member)
	{
		return ApplySingleGenericMember<size_t>(member,
			[](size_t size) constexpr { return size; }, 0, 0,
			GetRawByteSize_T());
	}

	constexpr GenericMemberFlags GetAvailableMemberFlags(GenericList list)
	{
		return ApplySingleGenericList<GenericMemberFlags>(list,
			[](GenericMemberFlags flags) constexpr { return flags; }, GenericMemberFlags_None,
			GetListStructAvailableMemberFlags_T());
	}

	constexpr size_t GetGenericListCount(const ChartCourse& course, GenericList list)
	{
		return ApplySingleGenericList<size_t>(list,
			[](auto&& typedList) { return typedList.size(); }, 0,
			course);
	}

	// course list element access functions
	template <GenericMember Member, typename ChartCourseT, expect_type_t<ChartCourseT, ChartCourse> = true, typename FAction, typename... Args>
	constexpr b8 TryDo(FAction&& action, ChartCourseT&& course, GenericList list, size_t index, Args&&...args)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList)
			{
				if constexpr (IsMemberAvailable<decltype(typedList[index]), Member>) {
					if (!(index < typedList.size()))
						return false;
					action(get<Member>(std::forward<decltype(typedList)>(typedList)[index]), get_or_forward<Member>(std::forward<Args>(args))...);
					return true;
				} else {
					return false;
				}
			}, false,
			course);
	}

	template <typename ChartCourseT, expect_type_t<ChartCourseT, ChartCourse> = true, typename FAction, typename... Args>
	constexpr b8 TryDo(FAction&& action, ChartCourseT&& course, GenericList list, size_t index, GenericMember member, Args&&...args)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList)
			{
				return ApplySingleGenericMember(member,
					[&](auto&& typedMember, auto&&... typedArgs)
					{
						if (!(index < typedList.size()))
							return false;
						action(std::forward<decltype(typedMember)>(typedMember), std::forward<decltype(typedArgs)>(typedArgs)...);
						return true;
					}, false, false,
					std::forward<decltype(typedList)>(typedList)[index], std::forward<Args>(args)...);
			}, false,
			course);
	}

	constexpr b8 TryGetGenericStruct(const ChartCourse& course, GenericList list, size_t index, GenericListStruct& outValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedOutValue) { if (InBounds(index, typedList)) { typedOutValue = typedList[index]; return true; } return false; }, false,
			course, outValue);
	}

	constexpr b8 TrySetGenericStruct(ChartCourse& course, GenericList list, size_t index, const GenericListStruct& inValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedInValue) { if (InBounds(index, typedList)) { typedList[index] = typedInValue; return true; } return false; }, false,
			course, inValue);
	}

	template <typename Func>
	b8 TryAddOrFuncGenericStruct(ChartCourse& course, GenericList list, GenericListStruct inValue, Func funcExist)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedInValue) { typedList.InsertOrFunc(typedInValue, funcExist); return true; }, false,
			course, std::move(inValue)); // `std::move` makes no differences on POD
	}

	inline b8 TryAddOrReplaceGenericStruct(ChartCourse& course, GenericList list, GenericListStruct inValue)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList, auto&& typedInValue) { typedList.InsertOrUpdate(typedInValue); return true; }, false,
			course, std::move(inValue)); // `std::move` makes no differences on POD
	}

	constexpr b8 TryRemoveGenericStruct(ChartCourse& course, GenericList list, Beat beatToRemove)
	{
		return ApplySingleGenericList(list,
			[&](auto&& typedList) { typedList.RemoveAtBeat(beatToRemove); return true; }, false,
			course);
	}

	constexpr b8 TryRemoveGenericStruct(ChartCourse& course, GenericList list, const GenericListStruct& inValueToRemove)
	{
		return TryRemoveGenericStruct(course, list, GetBeat(inValueToRemove, list));
	}

	template <auto... Tags, typename FAction, typename ForEachChartItemDataT, typename ChartCourseT, typename... Args,
		expect_type_t<ForEachChartItemDataT, struct ForEachChartItemData> = true,
		expect_type_t<ChartCourseT, struct ChartCourse> = true>
	constexpr decltype(auto) TryDo(FAction&& action, ForEachChartItemDataT&& data, ChartCourseT&& c, Args&&... args)
	{
		return TryDo<Tags...>(std::forward<FAction>(action), std::forward<ChartCourseT>(c), std::forward<ForEachChartItemDataT>(data).List, std::forward<ForEachChartItemDataT>(data).Index, std::forward<Args>(args)...);
	}

	struct ForEachChartItemData
	{
		GenericList List;
		size_t Index;
	};

	template <typename Func>
	constexpr void ForEachChartItem(const ChartCourse& course, Func perItemFunc)
	{
		ApplyForEachGenericList([&](GenericList list, auto&& typedList) {
			for (size_t i = 0; i < typedList.size(); i++)
				perItemFunc(ForEachChartItemData{ list, i });
		}, course);
	}

	template <typename Func>
	constexpr void ForEachSelectedChartItem(const ChartCourse& course, Func perSelectedItemFunc)
	{
		ApplyForEachGenericList([&](GenericList list, auto&& typedList) {
			for (size_t i = 0; i < typedList.size(); i++)
				if (typedList[i].IsSelected)
					perSelectedItemFunc(ForEachChartItemData{ list, i });
		}, course);
	}

	// helpers for end-unbounded events
	template <b8 Inclusive>
	constexpr Beat GetLastEffectBeatBefore(const ChartCourse& course, GenericList list, Beat beat)
	{
		constexpr auto compare = [](auto&& a, auto&& b) constexpr { if constexpr (Inclusive) { return a <= b; } else { return a < b; } };
		assert(!ListIsItemEndBounded(list) && "Not for end-bounded events"); // not handled here
		if (!(ListHasNoteStaticEffects(list) || ListHasBarlineStaticEffects(list)) // can just end at note or barline
			|| list == GenericList::TempoChanges // need to be handled with scroll changes instead (TODO)
			) {
			return beat;
		}
		// do not end at note or barline if they are effect targets of the next event
		Beat lastEffectBeat = Beat::FromTicks(-1);
		if (ListHasNoteStaticEffects(list)) {
			for (const auto& note : course.Notes_Normal) { // no sorted-by-end lists => need linear search for handling overlapping notes
				if (!compare(note.BeatTime, beat))
					break;
				if (auto beatEnd = note.BeatTime + note.BeatDuration; compare(beatEnd, beat))
					lastEffectBeat = std::max(lastEffectBeat, beatEnd);
			}
		}
		if (ListHasBarlineStaticEffects(list)) {
			Beat beatLastBarline = Beat::Zero();
			course.TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
				{
					if (!it.IsBar)
						return ControlFlow::Continue;
					if (!compare(it.Beat, beat))
						return ControlFlow::Break;
					beatLastBarline = it.Beat;
					return ControlFlow::Continue;
				});
			lastEffectBeat = std::max(lastEffectBeat, beatLastBarline);
		}
		return lastEffectBeat;
	}

	constexpr Beat GetLastEffectBeatBefore(const ChartCourse& course, GenericList list, Beat beat, b8 inclusive)
	{
		return (inclusive) ? GetLastEffectBeatBefore<true>(course, list, beat) : GetLastEffectBeatBefore<false>(course, list, beat);
	}

	template <b8 Inclusive>
	constexpr Beat GetFirstEffectBeatAfter(const ChartCourse& course, GenericList list, Beat beat)
	{
		constexpr auto compare = [](auto&& a, auto&& b) constexpr { if constexpr (Inclusive) { return a >= b; } else { return a > b; } };
		assert(!ListIsItemEndBounded(list) && "Not for end-bounded events");
		if (!(ListHasNoteStaticEffects(list) || ListHasBarlineStaticEffects(list)) // can just end at note or barline
			|| list == GenericList::TempoChanges // need to be handled with scroll changes instead (TODO)
			) {
			return beat;
		}
		// end at note or barline if they are effect targets of the current event
		Beat firstEffectBeat = Beat::FromTicks(I32Max);
		if (ListHasNoteStaticEffects(list)) {
			for (const auto& note : course.Notes_Normal) { // no sorted-by-end lists => need linear search for handling overlapping notes
				if (auto beatEnd = note.BeatTime + note.BeatDuration; compare(beatEnd, beat))
					firstEffectBeat = std::min(firstEffectBeat, beatEnd);
				if (compare(note.BeatTime, beat))
					break;
			}
		}
		if (ListHasBarlineStaticEffects(list)) {
			Beat beatFirstBarline = Beat::FromTicks(I32Max);
			course.TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
				{
					if (!it.IsBar)
						return ControlFlow::Continue;
					if (compare(it.Beat, beat)) {
						beatFirstBarline = it.Beat;
						return ControlFlow::Break;
					}
					return ControlFlow::Continue;
				});
			firstEffectBeat = std::min(firstEffectBeat, beatFirstBarline);
		}
		return firstEffectBeat;
	}

	constexpr Beat GetFirstEffectBeatAfter(const ChartCourse& course, GenericList list, Beat beat, b8 inclusive)
	{
		return (inclusive) ? GetFirstEffectBeatAfter<true>(course, list, beat) : GetFirstEffectBeatAfter<false>(course, list, beat);
	}

	constexpr Beat GetLastEffectBeat(const ChartCourse& course, GenericList list, size_t index)
	{
		assert(!ListIsItemEndBounded(list) && "Not for end-bounded events");
		Beat beatStartNext = {};
		if (!TryGet<GenericMember::Beat_Start>(course, list, index + 1, beatStartNext))
			return Beat::FromTicks(I32Max);
		return GetLastEffectBeatBefore<false>(course, list, beatStartNext);
	}
}
