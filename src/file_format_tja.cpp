#include "file_format_tja.h"
#include <algorithm>
#include <array>
#include <numeric>

namespace TJA
{
	// NOTE: Comment marker stored in the format: "// PeepoDrumKit yyyy/MM/dd"
	static constexpr std::string_view PeepoDrumKitCommentMarkerPrefix = "PeepoDrumKit";

	static constexpr std::string_view KeyStrings[] =
	{
		"",

		// NOTE: Main_
		"",
		"TITLE",
		"TITLE", // prefix
		"SUBTITLE",
		"SUBTITLE", // prefix
		"BPM",
		"WAVE",
		"PREIMAGE",
		"OFFSET",
		"DEMOSTART",
		"GENRE",
		"SCOREMODE",
		"MAKER",
		"LYRICS",
		"SONGVOL",
		"SEVOL",
		"GAME",
		"HEADSCROLL",
		"BGIMAGE",
		"BGMOVIE",
		"MOVIEOFFSET",
		"",

		// NOTE: Course_
		"",
		"COURSE",
		"LEVEL",
		"BALLOON",
		"SCOREINIT",
		"SCOREDIFF",
		"BALLOONNOR",
		"BALLOONEXP",
		"BALLOONMAS",
		"STYLE",
		"EXPLICIT",
		"NOTESDESIGNER", // prefix
		"EXAM", // prefix
		"GAUGEINCR",
		"TOTAL",
		"HIDDENBRANCH",
		"LIFE",
		"SIDE",
		"",

		// NOTE: Chart_
		"",
		"START",
		"END",
		"MEASURE",
		"BPMCHANGE",
		"DELAY",
		"SCROLL",
		"GOGOSTART",
		"GOGOEND",
		"BARLINEOFF",
		"BARLINEON",
		"BRANCHSTART",
		"N",
		"E",
		"M",
		"BRANCHEND",
		"SECTION",
		"LYRIC",
		"LEVELHOLD",
		"BMSCROLL",
		"HBSCROLL",
		"NMSCROLL",
		"BARLINE",
		"GAMETYPE",
		"SENOTECHANGE",
		"NEXTSONG",
		"DIRECTION",
		"SUDDEN",
		"JPOSSCROLL",
		"",
	};

	static_assert(ArrayCount(KeyStrings) == EnumCount<Key>);

	static const std::regex PatTJAHeader = std::regex("^\\.?[A-Z][A-Z0-9_]*$");
	static const std::regex PatTJACommand = std::regex("^[A-Z]+$");

	static bool MatchKeyString(Key key, std::string_view str)
	{
		switch (key) {
		case Key::Main_Invalid:
		case Key::Course_Invalid:
			return !std::regex_match(begin(str), end(str), PatTJAHeader);
		case Key::Chart_Invalid:
			return !std::regex_match(begin(str), end(str), PatTJACommand);

		case Key::Main_TITLE_localized:
		case Key::Main_SUBTITLE_localized:
		case Key::Course_NOTESDESIGNERs:
		case Key::Course_EXAMs:
			return ASCII::StartsWith(str, KeyStrings[EnumToIndex(key)]);

		case Key::Unknown:
		case Key::Course_Unknown: // ambiguous between Main and Course scope, reassigned later
		case Key::Chart_Unknown:
			return true;

		case Key::Main_Unknown:
			return false; // ambiguous between Main and Course scope, assigned separately

		default:
			return (str == KeyStrings[EnumToIndex(key)]);
		}
	}

	Key GetKeyColonValueTokenKey(std::string_view str)
	{
		for (Key i = Key::KeyColonValue_First; i <= Key::KeyColonValue_Last; IncrementEnum(i))
			if (MatchKeyString(i, str)) { return i; }
		return Key::Course_Unknown; // already matched; should never be reached
	}

	Key GetHashCommandTokenKey(std::string_view str)
	{
		for (Key i = Key::HashCommand_First; i <= Key::HashCommand_Last; IncrementEnum(i))
			if (MatchKeyString(i, str)) { return i; }
		return Key::Chart_Unknown; // already matched; should never be reached
	}

	struct LinePrefixCommentSuffixSplit { std::string_view LinePrefix, CommentSuffix; };

	static constexpr LinePrefixCommentSuffixSplit SplitLineIntoPrefixAndCommentSuffix(std::string_view line)
	{
		if (line.size() >= (sizeof('/') * 2))
		{
			for (size_t i = 0; i < line.size() - 1; i++)
			{
				if (line[i] == '/' && line[i + 1] == '/')
					return { line.substr(0, i), line.substr(i) };
			}
		}

		return { line, line.substr(0, 0) };
	}

	std::vector<std::string_view> SplitLines(std::string_view fileContent)
	{
		i32 lineCount = 0;
		ASCII::ForEachLineInMultiLineString(fileContent, false, [&](std::string_view) { lineCount++; });

		std::vector<std::string_view> outLines;
		outLines.reserve(lineCount);
		ASCII::ForEachLineInMultiLineString(fileContent, false, [&](std::string_view line) { outLines.push_back(line); });

		return outLines;
	}

	std::vector<Token> TokenizeLines(const std::vector<std::string_view>& lines)
	{
		std::vector<Token> outTokens;
		outTokens.reserve(lines.size());

		b8 currentlyBetweenChartStartAndEnd = false;
		b8 currentlyAfterFirstCourse = false;

		for (size_t lineIndex = 0; lineIndex < lines.size(); lineIndex++)
		{
			const std::string_view lineFull = lines[lineIndex];
			std::string_view lineTrimmed = ASCII::Trim(lineFull);

			if (lineTrimmed.empty() || ASCII::IsAllWhitespace(lineTrimmed))
			{
				Token& newToken = outTokens.emplace_back();
				newToken.Type = TokenType::EmptyLine;
				newToken.LineIndex = static_cast<i16>(lineIndex);
				newToken.Line = lineTrimmed;
			}
			else
			{
				const LinePrefixCommentSuffixSplit lineCommentSplit = SplitLineIntoPrefixAndCommentSuffix(lineTrimmed);
				if (!lineCommentSplit.CommentSuffix.empty())
					lineTrimmed = ASCII::Trim(lineCommentSplit.LinePrefix);

				if (!lineTrimmed.empty())
				{
					Token& newToken = outTokens.emplace_back();
					newToken.Type = TokenType::Unknown;
					newToken.LineIndex = static_cast<i16>(lineIndex);
					newToken.Line = lineTrimmed;

					if (lineTrimmed[0] == '#')
					{
						newToken.Type = TokenType::HashChartCommand;
						if (const size_t spaceSeparator = lineTrimmed.find_first_of(' '); spaceSeparator != std::string_view::npos)
						{
							newToken.KeyString = lineTrimmed.substr(sizeof('#'), spaceSeparator - sizeof('#'));
							newToken.ValueString = lineTrimmed.substr(spaceSeparator + sizeof(' '));
						}
						else
						{
							newToken.KeyString = lineTrimmed.substr(sizeof('#'), lineTrimmed.size() - sizeof('#'));
							newToken.ValueString = {};
						}

						newToken.Key = GetHashCommandTokenKey(newToken.KeyString);
						if (newToken.Key == Key::Chart_START)
							currentlyBetweenChartStartAndEnd = true;
						else if (newToken.Key == Key::Chart_END)
							currentlyBetweenChartStartAndEnd = false;
					}
					else if (const size_t colonSeparator = lineTrimmed.find_first_of(':'); colonSeparator != std::string_view::npos)
					{
						newToken.Type = TokenType::KeyColonValue;
						newToken.KeyString = lineTrimmed.substr(0, colonSeparator);
						newToken.ValueString = lineTrimmed.substr(colonSeparator + sizeof(':'));

						newToken.Key = GetKeyColonValueTokenKey(newToken.KeyString);
						if (newToken.Key == Key::Course_COURSE) {
							currentlyAfterFirstCourse = true;
						} else if (currentlyAfterFirstCourse) {
							// treat unknown headers after first COURSE: as course-scope header
							if (newToken.Key == Key::Main_Invalid)
								newToken.Key = Key::Course_Invalid;
						} else {
							// treat unknown headers before first COURSE: as file-scope header
							if (newToken.Key == Key::Course_Unknown)
								newToken.Key = Key::Main_Unknown;
						}
					}
					else
					{
						newToken.Type = currentlyBetweenChartStartAndEnd ? TokenType::ChartData : TokenType::Unknown;
						newToken.KeyString = {};
						newToken.ValueString = lineTrimmed;
					}
				}

				if (!lineCommentSplit.CommentSuffix.empty())
				{
					Token& newCommentToken = outTokens.emplace_back();
					newCommentToken.Type = TokenType::Comment;
					newCommentToken.LineIndex = static_cast<i16>(lineIndex);
					newCommentToken.Line = lineTrimmed;
					newCommentToken.ValueString = ASCII::Trim(lineCommentSplit.CommentSuffix.substr(sizeof('/') * 2));
				}
			}
		}

		// end-of-file token as implicit `#END`
		if (!lines.empty())
			outTokens.push_back({ TokenType::HashChartCommand, Key::Chart_END, static_cast<i16>(size(lines) - 1) });

		return outTokens;
	}

	ParsedTJA ParseTokens(const std::vector<Token>& tokens, ErrorList& outErrors)
	{
		static constexpr auto tryParseDefaultForEmpty = [](std::string_view in, auto* out, auto dflt) -> b8 { if (in.empty()) { *out = dflt; return true; } else { return ASCII::TryParse(in, *out); } };
		static constexpr auto tryParseCommaSeparatedValues = [](std::string_view in, auto* out) -> b8
		{
			i32 count = 0;
			ASCII::ForEachInCommaSeparatedList(in, [&](std::string_view) { count++; });
			std::remove_reference_t<decltype(*out)> tmp = {};
			tmp.reserve(count);
			b8 allSuccessful = true;
			ASCII::ForEachInCommaSeparatedList(in, [&](std::string_view valueString)
			{
				i32 v = 0;
				allSuccessful &= ASCII::TryParse(ASCII::Trim(valueString), v);
				tmp.push_back(v);
			});
			if (allSuccessful)
				*out = std::move(tmp);
			return allSuccessful;
		};
		static constexpr auto tryParseDifficultyType = [](std::string_view in, DifficultyType* out) -> b8
		{
			if (i32 v; ASCII::TryParse(in, v)) {
				if (v >= 0 && v < EnumCount<DifficultyType>) { *out = DifficultyType(v); return true; }
				return false;
			}
			if (ASCII::MatchesInsensitive(in, "easy")) { *out = DifficultyType::Easy; return true; }
			if (ASCII::MatchesInsensitive(in, "normal")) { *out = DifficultyType::Normal; return true; }
			if (ASCII::MatchesInsensitive(in, "hard")) { *out = DifficultyType::Hard; return true; }
			if (ASCII::MatchesInsensitive(in, "oni")) { *out = DifficultyType::Oni; return true; }
			if (ASCII::MatchesInsensitive(in, "edit")) { *out = DifficultyType::OniUra; return true; }
			if (ASCII::MatchesInsensitive(in, "tower")) { *out = DifficultyType::Tower; return true; }
			if (ASCII::MatchesInsensitive(in, "dan")) { *out = DifficultyType::Dan; return true; }
			if (ASCII::MatchesInsensitive(in, "ura")) { *out = DifficultyType::OniUra; return true; }
			return false;
		};
		static constexpr auto tryParseTime = [](std::string_view in, Time* out) -> b8
		{
			if (f32 v; ASCII::TryParse(in, v)) { *out = Time::FromSec(v); return true; }
			return false;
		};
		static constexpr auto tryParseTempo = [](std::string_view in, Tempo* out) -> b8
		{
			if (f32 v; ASCII::TryParse(in, v)) { *out = Tempo(v); return true; }
			return false;
		};
		static constexpr auto tryParsePercent = [](std::string_view in, f32* out) -> b8
		{
			if (f32 v; ASCII::TryParse(in, v)) { *out = FromPercent(v); return true; }
			return false;
		};
		static constexpr auto tryParseTimeSignature = [](std::string_view in, TimeSignature* out) -> b8
		{
			const size_t splitIndex = in.find_first_of("/");
			if (splitIndex == std::string_view::npos)
				return false;

			const std::string_view inNum = ASCII::Trim(in.substr(0, splitIndex));
			const std::string_view inDen = ASCII::Trim(in.substr(splitIndex + 1));

			f32 numF, denF;
			if (!ASCII::TryParse(inNum, numF) || !ASCII::TryParse(inDen, denF))
				return false;

			// Find smallest integer scale factor to represent both values as exact integers.
			// Handles common TJA fractional time signatures like 3.5/4 (-> 7/8) or 1.5/4 (-> 3/8).
			i32 scale = 1;
			for (i32 s : {1, 2, 4, 8, 10, 16, 100, 1000})
			{
				if (fabsf(numF * s - roundf(numF * s)) < 1e-4f && fabsf(denF * s - roundf(denF * s)) < 1e-4f)
				{
					scale = s;
					break;
				}
			}

			const i32 outNum = static_cast<i32>(roundf(numF * scale));
			const i32 outDen = static_cast<i32>(roundf(denF * scale));
			if (outDen == 0)
				return false;

			*out = TimeSignature(outNum, outDen);
			return true;
		};
		static constexpr auto tryParseNoteTypeChar = [](char in, NoteType* out) -> b8
		{
			switch (in)
			{
			case '0': *out = NoteType::None; return true;
			case '1': *out = NoteType::Don; return true;
			case '2': *out = NoteType::Ka; return true;
			case '3': *out = NoteType::DonBig; return true;
			case '4': *out = NoteType::KaBig; return true;
			case '5': *out = NoteType::Start_Drumroll; return true;
			case '6': *out = NoteType::Start_DrumrollBig; return true;
			case '7': *out = NoteType::Start_Balloon; return true;
			case '8': *out = NoteType::End_BalloonOrDrumroll; return true;
			case '9': *out = NoteType::Start_BaloonSpecial; return true;
			case 'A': *out = NoteType::DonBigBoth; return true;
			case 'B': *out = NoteType::KaBigBoth; return true;
			case 'C': *out = NoteType::Bomb; return true;
			case 'D': *out = NoteType::Fuse; return true;
			case 'F': *out = NoteType::Hidden; return true;
			case 'G': *out = NoteType::KaDon; return true;
			default: return false;
			}
		};
		static constexpr auto tryParseScoreMode = [](std::string_view in, ScoreMode* out) -> b8
		{
			if (in == "0") { *out = ScoreMode::AC2_To_AC7_Oni; return true; }
			if (in == "1") { *out = ScoreMode::AC1_To_AC14; return true; }
			if (in == "2") { *out = ScoreMode::AC15; return true; }
			return false;
		};
		static constexpr auto tryParseSongSelectSide = [](std::string_view in, SongSelectSide* out) -> b8
		{
			if (ASCII::MatchesInsensitive(in, "Normal") || in == "1") { *out = SongSelectSide::Normal; return true; }
			if (ASCII::MatchesInsensitive(in, "Ex") || in == "2") { *out = SongSelectSide::Ex; return true; }
			if (ASCII::MatchesInsensitive(in, "Both") || in == "3") { *out = SongSelectSide::Both; return true; }
			return false;
		};
		static constexpr auto tryParseGameType = [](std::string_view in, GameType* out) -> b8
		{
			if (ASCII::MatchesInsensitive(in, "Taiko")) { *out = GameType::Taiko; return true; }
			if (ASCII::MatchesInsensitive(in, "Konga")) { *out = GameType::Konga; return true; }
			if (ASCII::MatchesInsensitive(in, "Bongo")) { *out = GameType::Konga; return true; } // Alias for Konga
			return false;
		};
		static constexpr auto tryParseScrollDirection = [](std::string_view in, ScrollDirection* out) -> b8
		{
			if (i32 v; ASCII::TryParse(in, v)) { *out = static_cast<ScrollDirection>(v); return true; }
			return false;
		};
		static constexpr auto tryParseBranchCondition = [](std::string_view in, BranchCondition* out) -> b8
		{
			if (ASCII::MatchesInsensitive(in, "r")) { *out = BranchCondition::Roll; return true; }
			if (ASCII::MatchesInsensitive(in, "p")) { *out = BranchCondition::Precise; return true; }
			if (ASCII::MatchesInsensitive(in, "s")) { *out = BranchCondition::Score; return true; }
			return false;
		};
		static constexpr auto tryParseStyleMode = [](std::string_view in, i32* out) -> b8
		{
			if (ASCII::MatchesInsensitive(in, "Single")) { *out = 1; return true; }
			if (ASCII::MatchesInsensitive(in, "Double") || ASCII::MatchesInsensitive(in, "Couple")) { *out = 2; return true; }
			if (ASCII::TryParse(in, *out) && *out > 0) { return true; }
			return false;
		};
		static constexpr auto tryParsePlayerSide = [](std::string_view in, i32* out) -> b8
		{
			if (in.empty()) { *out = 0; return true; }
			if (ASCII::MatchesInsensitive(in.substr(0, 1), "P") && ASCII::TryParse(in.substr(1), *out) && *out > 0) { return true; }
			return false;
		};
		static constexpr auto tryParseGaugeIncrementMethod = [](std::string_view in, GaugeIncrementMethod* out) -> b8
		{
			if (ASCII::MatchesInsensitive(in, "NORMAL")) { *out = GaugeIncrementMethod::Normal; return true; }
			if (ASCII::MatchesInsensitive(in, "FLOOR")) { *out = GaugeIncrementMethod::Floor; return true; }
			if (ASCII::MatchesInsensitive(in, "ROUND")) { *out = GaugeIncrementMethod::Round; return true; }
			if (ASCII::MatchesInsensitive(in, "NOTFIX")) { *out = GaugeIncrementMethod::NotFix; return true; }
			if (ASCII::MatchesInsensitive(in, "CEILING")) { *out = GaugeIncrementMethod::Ceiling; return true; }
			return false;
		};
		static constexpr auto tryParseLocaleSuffix = [](std::string_view in, std::string_view prefix, std::string_view* out) -> b8
		{
			*out = ASCII::TrimPrefix(in, prefix);
			return std::regex_match(begin(*out), end(*out), ASCII::PatIETFLangTagForTJA);
		};
		static constexpr auto tryParseNotesDesignerSuffix = [](std::string_view in, std::string_view prefix, std::string* outString, DifficultyType* out) -> b8
		{
			*outString = ASCII::TrimPrefix(in, prefix);
			if (outString->empty()) { *out = DifficultyType::Count; return true; }
			if (i32 v; ASCII::TryParse(*outString, v)) { *out = static_cast<DifficultyType>(v); return (*out >= DifficultyType{ 0 } && *out < DifficultyType::Count); }
			return false;
		};
		static constexpr auto tryParseExamSuffix = [](std::string_view in, std::string_view prefix, std::string* outString, i32* out) -> b8
		{
			*outString = ASCII::TrimPrefix(in, prefix);
			if (i32 v; ASCII::TryParse(*outString, v) && v > 0) { *out = v; return true; }
			if (*outString == "GAUGE") { *out = 0; return true; }
			return false;
		};

		static constexpr auto validateEndOfMeasureNoteCount = [](i32 noteCountAtEndOfMeasure, i32 lineIndex, ErrorList& outErrors)
		{
			if (noteCountAtEndOfMeasure <= 1)
				return;

#if 0 // TODO: JUMP HERE
			static constexpr i32 supported4By4BarDivisions[] = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64, 96, 192 };
			for (const i32 supportedDivision : supported4By4BarDivisions)
			{
				if (noteCountAtEndOfMeasure == supportedDivision)
					return;
			}

			// TODO: Also needs to take time signature into account..?
			// TODO: Report error if measure note count not nicely divisible assuming max 1/192nd (?)
			outErrors.Push(lineIndex, "Unusual note count measure division (?)");
#endif
		};

		ParsedTJA outTJA = {};

		i32 currentMeasureNoteCount = 0;
		b8 currentlyBetweenFirstCommandAndEnd = false;
		b8 currentlyBetweenChartStartAndEnd = false;
		b8 currentlyBetweenGoGoStartAndEnd = false;
		b8 currentlyInBetweenMeasure = false;
		Complex cachedScrollSpeed = Complex(1.f, 0.f);

		// chart scope handler
		DifficultyType currentCourseScope = DifficultyType::Count; // default course scope
		auto idxLastCourses = InitializedArray<i32, EnumCount<DifficultyType> + 1>(-1);
		std::string notesDesigners[EnumCount<DifficultyType>] = {};

		ParsedCourse* currentCourse = nullptr;

		auto initCurrentCourse = [&]()
		{
			for (auto courseScope : { currentCourseScope, DifficultyType::Count }) {
				if (auto idx = idxLastCourses[EnumToIndex(courseScope)]; idx >= 0) {
					currentCourse = &outTJA.Courses.emplace_back();
					currentCourse->Metadata = outTJA.Courses[idx].Metadata; // inherit last (need to explicitly copy)
					goto set_up_course;
				}
			}
			currentCourse = &outTJA.Courses.emplace_back(); // create from scratch;
		set_up_course:
			currentCourse->HasChart = false;
			b8 fromScratch = (idxLastCourses[EnumToIndex(currentCourseScope)] == -1);
			idxLastCourses[EnumToIndex(currentCourseScope)] = outTJA.Courses.size() - 1;
			return fromScratch;
		};
		auto getCurrentCourse = [&]()
		{
			if (currentCourse == nullptr)
				initCurrentCourse();
			return currentCourse;
		};

		auto pushChartCommand = [&](ParsedChartCommandType type) -> ParsedChartCommand&
		{
			auto& newCommand = getCurrentCourse()->ChartCommands.emplace_back();
			newCommand.Type = type;
			return newCommand;
		};

		for (const Token& token : tokens)
		{
			const i16 lineIndex = token.LineIndex;
			switch (token.Type)
			{
			case TokenType::Unknown:
			{
				outErrors.Push(lineIndex, "Unknown data '%.*s'", FmtStrViewArgs(token.Line));
			} break;

			case TokenType::EmptyLine:
			{
				// ...
			} break;

			case TokenType::Comment:
			{
				// ...
				if (!outTJA.HasPeepoDrumKitComment && ASCII::StartsWith(token.ValueString, PeepoDrumKitCommentMarkerPrefix))
				{
					outTJA.HasPeepoDrumKitComment = true;
					outTJA.PeepoDrumKitCommentDate = Date::Zero();

					std::string_view comment = token.ValueString.substr(PeepoDrumKitCommentMarkerPrefix.size());
					comment = ASCII::Trim(comment);
					comment = ASCII::TrimSuffix(ASCII::TrimPrefix(comment, "("), ")");
					if (!comment.empty())
					{
						Date::FormatBuffer dateBuffer {};
						CopyStringViewIntoFixedBuffer(dateBuffer.Data, comment);
						outTJA.PeepoDrumKitCommentDate = Date::FromString(dateBuffer.Data, '/');
					}
				}
			} break;

			case TokenType::KeyColonValue:
			{
				if (currentlyBetweenFirstCommandAndEnd && token.Key != Key::Course_EXAMs) {
					if (currentlyBetweenChartStartAndEnd)
						outErrors.Push(lineIndex, "This property should not be placed between #START and #END");
					else
						outErrors.Push(lineIndex, "This property should be placed before the first chart command");
				}

				if (token.Key >= Key::Main_First && token.Key <= Key::Main_Last)
				{
					if (currentCourseScope != DifficultyType::Count)
						outErrors.Push(lineIndex, "This property is per-file in most simulators and should be defined before the first COURSE:");

					const std::string_view in = ASCII::Trim(token.ValueString);
					ParsedMainMetadata& out = outTJA.Metadata;
					switch (token.Key)
					{
					case Key::Main_TITLE: { out.TITLE = in; } break;
					case Key::Main_TITLE_localized:
					{
						std::string_view key;
						if (!tryParseLocaleSuffix(token.KeyString, "TITLE", &key)) { outErrors.Push(lineIndex, "Invalid language tag '%.*s'", FmtStrViewArgs(key)); }
						out.TITLE_localized.insert_or_assign(std::string{ key }, in);
					}
					break;
					case Key::Main_SUBTITLE: { out.SUBTITLE = in; } break;
					case Key::Main_SUBTITLE_localized:
					{
						std::string_view key;
						if (!tryParseLocaleSuffix(token.KeyString, "SUBTITLE", &key)) { outErrors.Push(lineIndex, "Invalid language tag '%.*s'", FmtStrViewArgs(key)); }
						out.SUBTITLE_localized.insert_or_assign(std::string{ key }, in);
					}
					break;
					case Key::Main_BPM: { if (!tryParseTempo(in, &out.BPM)) { outErrors.Push(lineIndex, "Invalid tempo '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_WAVE: { out.WAVE = in; } break;
					case Key::Main_PREIMAGE: { out.PREIMAGE = in; } break;
					case Key::Main_OFFSET: { if (!tryParseTime(in, &out.OFFSET)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_DEMOSTART: { if (!tryParseTime(in, &out.DEMOSTART)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_GENRE: { out.GENRE = in; } break;
					case Key::Main_SCOREMODE: { if (!tryParseScoreMode(in, &out.SCOREMODE)) { outErrors.Push(lineIndex, "Unknown score mode '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_MAKER: { out.MAKER = in; } break;
					case Key::Main_LYRICS: { out.LYRICS = in; } break;
					case Key::Main_SONGVOL: { if (!tryParsePercent(in, &out.SONGVOL)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_SEVOL: { if (!tryParsePercent(in, &out.SEVOL)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_GAME: { if (!tryParseGameType(in, &out.GAME)) { outErrors.Push(lineIndex, "Unknown game type '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_HEADSCROLL: { if (!ASCII::TryParse(in, out.HEADSCROLL)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_BGIMAGE: { out.BGIMAGE = in; } break;
					case Key::Main_BGMOVIE: { out.BGMOVIE = in; } break;
					case Key::Main_MOVIEOFFSET: { if (!tryParseTime(in, &out.MOVIEOFFSET)) { outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Main_Unknown: { out.Others.emplace(token.KeyString, token.ValueString); outErrors.Push(lineIndex, "Unknown file-scoped (?) header '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					case Key::Main_Invalid: { out.Others.emplace(token.KeyString, token.ValueString); outErrors.Push(lineIndex, "Invalid header '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					default: { assert(!"Unhandled Key::Main_ switch case despite (Key::Main_First to Key::Main_Last) range check"); } break;
					}
				}
				else if (token.Key == Key::Course_COURSE) { // special case of Key::Course_
					const std::string_view in = ASCII::Trim(token.ValueString);
					if (DifficultyType course; !tryParseDifficultyType(in, &course)) {
						outErrors.Push(lineIndex, "Invalid difficulty '%.*s'", FmtStrViewArgs(in));
					} else { // change course scope
						currentCourseScope = course;
						b8 fromScratch = initCurrentCourse();
						auto& metadata = getCurrentCourse()->Metadata;
						metadata.COURSE = course;
						if (fromScratch)
							metadata.NOTESDESIGNER = notesDesigners[EnumToIndex(course)];
					}
				}
				else if (token.Key >= Key::Course_First && token.Key <= Key::Course_Last)
				{
					if (currentCourseScope == DifficultyType::Count)
						outErrors.Push(lineIndex, "This property has course scope, defining this before the first COURSE: is not supported on some simulators");

					const std::string_view in = ASCII::Trim(token.ValueString);
					ParsedCourseMetadata& out = getCurrentCourse()->Metadata;
					switch (token.Key)
					{
					case Key::Course_LEVEL: 
					{ 
						f32 _level;
						bool containsDot = (in.find('.') != std::string_view::npos);
						if (!ASCII::TryParse(in, _level))
						{ 
							outErrors.Push(lineIndex, "Invalid float '%.*s'", FmtStrViewArgs(in));
							break;
						} 
						out.LEVEL = static_cast<int>(_level);
						if (containsDot) {
							std::string_view devpart = in.substr(in.find('.') + 1, 1);
							if (!tryParseDefaultForEmpty(devpart, &out.LEVEL_DECIMALTAG, 0))
							{
								outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(devpart));
								break;
							}
						}
						break;
					} 
					case Key::Course_BALLOON: { if (!tryParseCommaSeparatedValues(in, &out.BALLOON)) { outErrors.Push(lineIndex, "Invalid int in comma separated list '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_SCOREINIT: { if (!tryParseDefaultForEmpty(in, &out.SCOREINIT, 0)) { outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_SCOREDIFF: { if (!tryParseDefaultForEmpty(in, &out.SCOREDIFF, 0)) { outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_BALLOONNOR: { if (!tryParseCommaSeparatedValues(in, &out.BALLOON_Normal)) { outErrors.Push(lineIndex, "Invalid int in comma separated list '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_BALLOONEXP: { if (!tryParseCommaSeparatedValues(in, &out.BALLOON_Expert)) { outErrors.Push(lineIndex, "Invalid int in comma separated list '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_BALLOONMAS: { if (!tryParseCommaSeparatedValues(in, &out.BALLOON_Master)) { outErrors.Push(lineIndex, "Invalid int in comma separated list '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_STYLE: { if (!tryParseStyleMode(in, &out.STYLE)) { outErrors.Push(lineIndex, "Unknown or invalid style mode '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_EXPLICIT: { if (!ASCII::TryParse(in, out.EXPLICIT)) { outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_NOTESDESIGNERs:
					{
						DifficultyType diff;
						if (std::string key; !tryParseNotesDesignerSuffix(token.KeyString, KeyStrings[EnumToIndex(token.Key)], &key, &diff))
							outErrors.Push(lineIndex, "Invalid difficulty number '%.*s', expected '0' to '%d' or (empty)", FmtStrViewArgs(key), EnumCountI32<DifficultyType> - 1);
						else if (currentCourseScope == DifficultyType::Count) { // before first `COURSE:` -> as file-scope header -> apply to specified difficulty
							if (diff == DifficultyType::Count)
								outErrors.Push(lineIndex, "Empty difficulty number is invalid for file-scope usage");
							else
								notesDesigners[EnumToIndex(diff)] = in;
						} else { // otherwise -> as course-scope header -> ignore suffix
							if (diff != DifficultyType::Count && diff != currentCourse->Metadata.COURSE)
								outErrors.Push(lineIndex, "Difficulty number '%.*s' does not match current difficulty's number %d", FmtStrViewArgs(key), EnumToIndex(currentCourse->Metadata.COURSE));
							out.NOTESDESIGNER = in;
						}
					}
					break;
					case Key::Course_EXAMs:
					{
						i32 index;
						if (std::string key; !tryParseExamSuffix(token.KeyString, KeyStrings[EnumToIndex(token.Key)], &key, &index))
							outErrors.Push(lineIndex, "Invalid exam key '%.*s', expected 'n' ('1', '2', ...) or 'GAUGE'", FmtStrViewArgs(key));
						out.EXAMs.insert_or_assign(index, in);
					}
					break;
					case Key::Course_GAUGEINCR: { if (!tryParseGaugeIncrementMethod(in, &out.GAUGEINCR)) { outErrors.Push(lineIndex, "Unknown gauge increment method '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_TOTAL: { out.TOTAL; } break;
					case Key::Course_HIDDENBRANCH: { if (!ASCII::TryParse(in, out.HIDDENBRANCH)) { outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_LIFE: { if (!ASCII::TryParse(in, out.LIFE)) { outErrors.Push(lineIndex, "Invalid int '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_SIDE: { if (!tryParseSongSelectSide(in, &out.SIDE)) { outErrors.Push(lineIndex, "Invalid SIDE '%.*s'", FmtStrViewArgs(in)); } } break;
					case Key::Course_Unknown: { out.Others.emplace(token.KeyString, token.ValueString); outErrors.Push(lineIndex, "Unknown course-scoped (?) header '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					case Key::Course_Invalid: { out.Others.emplace(token.KeyString, token.ValueString); outErrors.Push(lineIndex, "Invalid header '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					default: { assert(!"Unhandled Key::Course_ switch case despite (Key::Course_First to Key::Course_Last) range check"); } break;
					}
				}
				else
				{
					outErrors.Push(lineIndex, "Unknown property '%.*s'", FmtStrViewArgs(token.KeyString));
				}
			} break;

			case TokenType::HashChartCommand:
			{
				if (!currentlyBetweenFirstCommandAndEnd) {
					if (currentCourseScope == DifficultyType::Count && !(token.Key == Key::Chart_END && token.KeyString.empty())) // exclude implicit `#END` from end-of-file
						outErrors.Push(lineIndex, "Defining chart body before the first COURSE: is not supported on some simulators");
					currentlyBetweenFirstCommandAndEnd = true;
				}

				if (!currentlyBetweenChartStartAndEnd) {
					if (token.Key != Key::Chart_START && token.Key != Key::Chart_END
						&& token.Key != Key::Chart_BMSCROLL && token.Key != Key::Chart_HBSCROLL && token.Key != Key::Chart_NMSCROLL
						) {
						outErrors.Push(lineIndex, "This chart command should be placed between #START and #END");
					}
				}

				if (token.Key >= Key::Chart_First && token.Key <= Key::Chart_Last)
				{
					const std::string_view in = ASCII::Trim(token.ValueString);
					switch (token.Key)
					{
					case Key::Chart_START:
					{
						if (currentlyBetweenChartStartAndEnd)
							outErrors.Push(lineIndex, "Missing #END command");

						ParsedCourse* course = getCurrentCourse();
						ParsedCourseMetadata& out = course->Metadata;
						if (!tryParsePlayerSide(in, &out.START_PLAYERSIDE) || out.START_PLAYERSIDE < 0)
							outErrors.Push(lineIndex, "Invalid player side '%.*s', expected '' or 'Pn' ('P1', 'P2', ...)", FmtStrViewArgs(in));
						else if (out.START_PLAYERSIDE == 0 && out.STYLE > 1)
							outErrors.Push(lineIndex, "Empty player side '%.*s' is invalid for multi-player modes", FmtStrViewArgs(in));
						else if (out.START_PLAYERSIDE > 0 && out.STYLE == 1)
							outErrors.Push(lineIndex, "Player side ('%.*s') is invalid in single-player mode", FmtStrViewArgs(in));
						else if (out.START_PLAYERSIDE > out.STYLE)
							outErrors.Push(lineIndex, "The player side '%.*s' is invalid in %d-player mode", FmtStrViewArgs(in), out.STYLE);

						course->HasChart = true;
						currentlyBetweenChartStartAndEnd = true;
					} break;
					case Key::Chart_END:
					{
						b8 isEof = token.KeyString.empty(); // implicit `#END` from end-of-file
						if (!currentlyBetweenChartStartAndEnd) {
							if (isEof)
								break;
							outErrors.Push(lineIndex, "Missing #START command");
						}

						if (currentlyBetweenGoGoStartAndEnd)
						{
							outErrors.Push(lineIndex, "Missing #GOGOEND command");
							pushChartCommand(ParsedChartCommandType::GoGoEnd);
							currentlyBetweenGoGoStartAndEnd = false;
						}

						if (currentlyInBetweenMeasure)
						{
#if 0 // NOTE: Ending on an open measure feels like it should be supported..? Otherwise it forces any chart to have at least one full empty measure at the end (?)
							outErrors.Error(lineIndex, "Missing end-of-measure comma");
							pushChartCommand(ParsedChartCommandType::MeasureEnd);
#endif
							validateEndOfMeasureNoteCount(currentMeasureNoteCount, lineIndex, outErrors);
							currentMeasureNoteCount = 0;
							currentlyInBetweenMeasure = false;
						}

						if (isEof && currentlyBetweenChartStartAndEnd)
							outErrors.Push(lineIndex, "Missing #END command");

						currentlyBetweenFirstCommandAndEnd = currentlyBetweenChartStartAndEnd = false;
						currentCourse = nullptr;
					} break;
					case Key::Chart_MEASURE:
					{
						if (currentlyInBetweenMeasure)
							outErrors.Push(lineIndex, "Time signature changes in the middle of a measure takes effects at unspecified measure");
						if (TimeSignature sig; !tryParseTimeSignature(in, &sig)) {
							outErrors.Push(lineIndex, "Invalid time signature '%.*s'", FmtStrViewArgs(in));
						} else {
							pushChartCommand(ParsedChartCommandType::ChangeTimeSignature).Param.ChangeTimeSignature.Value = sig;

							// NOTE: Just a limitation of the fixed point Beat implementation, arguably not a problem with the file itself..?
							if (!IsTimeSignatureSupported(sig))
								outErrors.Push(lineIndex, "Unsupported time signature denominator '%.*s'", FmtStrViewArgs(in));
						}
					} break;
					case Key::Chart_BPMCHANGE:
					{
						if (!tryParseTempo(in, &pushChartCommand(ParsedChartCommandType::ChangeTempo).Param.ChangeTempo.Value))
							outErrors.Push(lineIndex, "Invalid tempo '%.*s'", FmtStrViewArgs(in));
					} break;
					case Key::Chart_DELAY:
					{
						if (!tryParseTime(in, &pushChartCommand(ParsedChartCommandType::ChangeDelay).Param.ChangeDelay.Value))
							outErrors.Push(lineIndex, "Invalid delay '%.*s'", FmtStrViewArgs(in));
					} break;
					case Key::Chart_SCROLL:
					{
						if (!ASCII::TryParse(in, cachedScrollSpeed))
							outErrors.Push(lineIndex, "Invalid scroll speed '%.*s'", FmtStrViewArgs(in));
						pushChartCommand(ParsedChartCommandType::ChangeScrollSpeed).Param.ChangeScrollSpeed.Value = cachedScrollSpeed;
					} break;
					case Key::Chart_GOGOSTART:
					{
						if (currentlyBetweenGoGoStartAndEnd) {
							outErrors.Push(lineIndex, "Missing #GOGOEND command; inserting one");
							pushChartCommand(ParsedChartCommandType::GoGoEnd);
						}

						pushChartCommand(ParsedChartCommandType::GoGoStart);
						currentlyBetweenGoGoStartAndEnd = true;
					} break;
					case Key::Chart_GOGOEND:
					{
						if (!currentlyBetweenGoGoStartAndEnd)
							outErrors.Push(lineIndex, "Missing #GOGOSTART command; ignored");
						else
							pushChartCommand(ParsedChartCommandType::GoGoEnd);
						currentlyBetweenGoGoStartAndEnd = false;
					} break;
					case Key::Chart_BARLINEOFF:
					{
						pushChartCommand(ParsedChartCommandType::ChangeBarLine).Param.ChangeBarLine.Visible = false;
					} break;
					case Key::Chart_BARLINEON:
					{
						pushChartCommand(ParsedChartCommandType::ChangeBarLine).Param.ChangeBarLine.Visible = true;
					} break;
					case Key::Chart_BRANCHSTART:
					{
						ParsedChartCommand& newCommand = pushChartCommand(ParsedChartCommandType::BranchStart);
						auto& param = newCommand.Param.BranchStart;
						param = { BranchCondition::Precise, 101, 101 };
						i32 valueIndex = 0;
						ASCII::ForEachInCommaSeparatedList(in, [&](std::string_view value)
						{
							value = ASCII::Trim(value);
							if (valueIndex == 0) {
								if (!tryParseBranchCondition(value, &param.Condition))
									outErrors.Push(lineIndex, "Invalid branch condition '%.*s'", FmtStrViewArgs(value));
							} else if (valueIndex == 1) {
								if (!ASCII::TryParse(value, param.RequirementExpert))
									outErrors.Push(lineIndex, "Invalid branch requirement '%.*s'", FmtStrViewArgs(value));
							} else if (valueIndex == 2) {
								if (!ASCII::TryParse(value, param.RequirementMaster))
									outErrors.Push(lineIndex, "Invalid branch requirement '%.*s'", FmtStrViewArgs(value));
							} else {
								outErrors.Push(lineIndex, "Exceeded 3 arguments");
							}
							valueIndex++;
						});
						if (valueIndex < 3)
							outErrors.Push(lineIndex, "Less than 3 required arguments");
					} break;
					case Key::Chart_N:
					{
						// TODO: Bunch of branch validation etc.
						pushChartCommand(ParsedChartCommandType::BranchNormal);
					} break;
					case Key::Chart_E:
					{
						pushChartCommand(ParsedChartCommandType::BranchExpert);
					} break;
					case Key::Chart_M:
					{
						pushChartCommand(ParsedChartCommandType::BranchMaster);
					} break;
					case Key::Chart_BRANCHEND:
					{
						pushChartCommand(ParsedChartCommandType::BranchEnd);
					} break;
					case Key::Chart_SECTION: { pushChartCommand(ParsedChartCommandType::ResetAccuracyValues); } break;
					case Key::Chart_LYRIC: { pushChartCommand(ParsedChartCommandType::SetLyricLine).Param.SetLyricLine.Value = in; } break;
					case Key::Chart_LEVELHOLD: { pushChartCommand(ParsedChartCommandType::BranchLevelHold); } break;
					case Key::Chart_BMSCROLL: { pushChartCommand(ParsedChartCommandType::BMScroll); } break;
					case Key::Chart_HBSCROLL: { pushChartCommand(ParsedChartCommandType::HBScroll); } break;
					case Key::Chart_NMSCROLL: { pushChartCommand(ParsedChartCommandType::NMScroll); } break;
					case Key::Chart_SENOTECHANGE:
						if (i32 v; !ASCII::TryParse(in, v))
							outErrors.Push(lineIndex, "Invalid SENote type integer '%.*s'", FmtStrViewArgs(in));
						else
							pushChartCommand(ParsedChartCommandType::SENoteChange).Param.SENoteChange.Type = v;
						break;
					case Key::Chart_NEXTSONG: { pushChartCommand(ParsedChartCommandType::SetNextSong).Param.SetNextSong.CommaSeparatedList = in; } break;
					case Key::Chart_DIRECTION: 
					{
						f32 scrollspeed = cachedScrollSpeed.GetRealPart();
						auto tryParseDirection = [&](std::string_view in, Complex* out)
						{
							i32 direction = 0;
							if (!ASCII::TryParse(in, direction))
								return false;
							switch (direction)
							{
							case 0: *out = Complex(scrollspeed, 0.f); return true;
							case 1: *out = Complex(0.f, -scrollspeed); return true;
							case 2: *out = Complex(0.f, scrollspeed); return true;
							case 3: *out = Complex(scrollspeed, -scrollspeed); return true;
							case 4: *out = Complex(scrollspeed, scrollspeed); return true;
							case 5: *out = Complex(-scrollspeed, 0.f); return true;
							case 6: *out = Complex(-scrollspeed, -scrollspeed); return true;
							case 7: *out = Complex(-scrollspeed, scrollspeed); return true;
							default: return false;
							}
						};
						if (Complex v; !tryParseDirection(in, &v))
							outErrors.Push(lineIndex, "Invalid direction integer '%.*s'", FmtStrViewArgs(in));
						else
							pushChartCommand(ParsedChartCommandType::ChangeScrollSpeed).Param.ChangeScrollSpeed.Value = v;
					} break;
					case Key::Chart_SUDDEN:
					{
						decltype(ParsedChartCommand::ParamData::SetSudden) param = { Time::Zero(), Time::Zero() };
						i32 valueIndex = 0;
						b8 valid = true;

						ASCII::ForEachInSpaceSeparatedList(in, [&](std::string_view value)
						{
							value = ASCII::Trim(value);
							if (value.empty())
								return;
							if (valueIndex == 0) {
								if (!tryParseTime(value, &param.AppearanceOffset)) {
									valid = false;
									outErrors.Push(lineIndex, "Invalid appearance offset number '%.*s'", FmtStrViewArgs(value));
								}
							} else if (valueIndex == 1) {
								if (!tryParseTime(value, &param.MovementWaitDelay)) {
									valid = false;
									outErrors.Push(lineIndex, "Invalid movement offset number '%.*s'", FmtStrViewArgs(value));
								}
							} else {
								outErrors.Push(lineIndex, "Exceeded 2 arguments");
							}
							valueIndex++;
						});
						if (valueIndex < 2) {
							outErrors.Push(lineIndex, "Invalid lacking of 2 required arguments");
							valid = false;
						}
						if (valid)
							pushChartCommand(ParsedChartCommandType::SetSudden).Param.SetSudden = param;
					} break;
					case Key::Chart_JPOSSCROLL:
					{
						decltype(ParsedChartCommand::ParamData::ChangeJPOSScroll) param = { Time::Zero(), Complex(0, 0) };
						i32 valueIndex = 0;
						b8 valid = true;
						i32 direction = 0;
						b8 splitComplex = true;
						// 3-arg form: `#JPOSSCROLL 0.017 3+2i 0`
						// 4-arg form (splitComplex; TJAP3 1.6.x): `#JPOSSCROLL 3 100 100i 0`
						ASCII::ForEachInSpaceSeparatedList(in, [&](std::string_view value)
						{
							value = ASCII::Trim(value);
							if (value.empty())
								return;
							if (valueIndex == 0) {
								if (!tryParseTime(value, &param.Duration)) {
									outErrors.Push(lineIndex, "Invalid duration time '%.*s'", FmtStrViewArgs(value));
									valid = false;
								}
							} else if (valueIndex == 1) {
								if (!ASCII::TryParse(value, param.Move)) {
									outErrors.Push(lineIndex, "Invalid complex number '%.*s'", FmtStrViewArgs(value));
									valid = false;
								}
								if (ASCII::ToLowerCase(value.back()) == 'i')
									splitComplex = false;
							}
							else if (valueIndex == 2) {
								if (ASCII::ToLowerCase(value.back()) == 'i') {
									// arg 2 is move y
									if (Complex val; splitComplex && std::regex_match(begin(value), end(value), Complex::PatPureImaginary) && ASCII::TryParse(value, val))
										param.Move.SetImaginaryPart(val.cpx.imag());
									else
										outErrors.Push(lineIndex, "Invalid split complex number in '%.*s'", FmtStrViewArgs(token.ValueString));
								} else {
									++valueIndex; // arg 2 is direction
								}
							}
							if (valueIndex == 3) {
								if (!ASCII::TryParse(value, direction)) {
									outErrors.Push(lineIndex, "Invalid direction integer '%.*s'", FmtStrViewArgs(value));
									valid = false;
								}
							} else if (valueIndex > 3) {
								outErrors.Push(lineIndex, "Exceeded 3 arguments");
							}
							valueIndex++;
						});
						if (direction == 0) {
							param.Move.SetRealPart(-param.Move.GetRealPart());
							param.Move.SetImaginaryPart(-param.Move.GetImaginaryPart());
						}
						if (valueIndex < 2) {
							outErrors.Push(lineIndex, "Invalid lacking of 2 required arguments");
							valid = false;
						}
						if (valid)
							pushChartCommand(ParsedChartCommandType::SetJPOSScroll).Param.ChangeJPOSScroll = param;
					} break;
					case Key::Chart_Unknown: { outErrors.Push(lineIndex, "Unknown command '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					case Key::Chart_Invalid: { outErrors.Push(lineIndex, "Invalid command '%.*s'", FmtStrViewArgs(token.KeyString)); } break;
					default: { assert(!"Unhandled Key::Chart_ switch case despite (Key::Chart_First to Key::Chart_Last) range check"); } break;
					}
				}
				else
				{
					outErrors.Push(lineIndex, "Unknown chart command '%.*s'", FmtStrViewArgs(token.KeyString));
				}
			} break;

			case TokenType::ChartData:
			{
				if (currentlyBetweenChartStartAndEnd)
				{
					b8 hasSpaces = false;
					currentlyInBetweenMeasure = true;

					ParsedChartCommand& newCommand = pushChartCommand(ParsedChartCommandType::MeasureNotes);
					newCommand.Param.MeasureNotes.Notes.reserve(token.ValueString.size());
					for (const char& c : token.ValueString)
					{
						if (c == ',')
						{
							pushChartCommand(ParsedChartCommandType::MeasureEnd);
							if (&c != &token.ValueString.back())
								outErrors.Push(lineIndex, "Unexpected trailing data after end-of-measure comma");

							validateEndOfMeasureNoteCount(currentMeasureNoteCount, lineIndex, outErrors);

							currentMeasureNoteCount = 0;
							currentlyInBetweenMeasure = false;
							break;
						} else if (ASCII::IsWhitespace(c)) { // Whitespaces are ignore in TaikoJiro, especially must be ignored in GAME:Jube
							if (!hasSpaces) {
								outErrors.Push(lineIndex, "Whitespaces in the middle of note data is not supported on some simulators");
								hasSpaces = true;
							}
						} else {
							// NOTE: Treating unknown note types as empty spaces makes the most sense because that way timing won't be messed up 
							//		 in case it's somehow a genuine note that just happens to be unsupported by this parser (?)
							NoteType parsedNoteTypeOrNone = NoteType::None;
							if (!tryParseNoteTypeChar(c, &parsedNoteTypeOrNone))
								outErrors.Push(lineIndex, "Unknown note type '%c'", c);

							newCommand.Param.MeasureNotes.Notes.push_back(parsedNoteTypeOrNone);
							currentMeasureNoteCount++;
						}
					}
				}
				else
				{
					outErrors.Push(lineIndex, "Chart note data can only be added between #START and #END");
				}
			} break;

			default:
			{
				assert(!"OutOfBounds TokenType enum. TokenizeLines() should have output TokenType::Unknown in case of invalid input data");
			} break;
			}
		}

#if 1 // DEBUG: Always add at least one course for now so the debug gui has something to display
		if (outTJA.Courses.empty())
			outTJA.Courses.emplace_back();
#endif

		return outTJA;
	}

	static const ParsedMainMetadata DefaultMainMetadata = {};
	static const ParsedCourseMetadata DefaultCourseMetadata = {};

	void ConvertParsedToText(const ParsedTJA& inContent, std::string& out, Encoding encoding)
	{
		// TODO: ... or maybe tokenize first instead of going right to text..?
		out.reserve(out.size() + 0x4000);
		if (encoding == Encoding::UTF8)
			out += std::string_view(UTF8::BOM_UTF8, sizeof(UTF8::BOM_UTF8));

		static constexpr auto appendLine = [](std::string& out, std::string_view line) { out += line; out += '\n'; };
		static constexpr auto appendProperyLine = [](std::string& out, Key key, std::string_view value) { out += KeyStrings[EnumToIndex(key)]; out += ':'; out += value; out += '\n'; };
		static constexpr auto appendSuffixedPropertyLine = [](std::string& out, Key key, std::string_view suffix, std::string_view value)
		{ out += KeyStrings[EnumToIndex(key)]; out += suffix; out += ':'; out += value; out += '\n'; };
		static constexpr auto appendCommandLine = [](std::string& out, Key key, std::string_view value) { out += '#'; out += KeyStrings[EnumToIndex(key)]; if (!value.empty()) { out += ' '; out += value; }out += '\n'; };
		static constexpr auto appendBalloonProperyLine = [](std::string& out, Key key, const std::vector<i32>& popCounts)
		{
			out += KeyStrings[EnumToIndex(key)];
			out += ':';
			char buffer[16];
			for (size_t i = 0; i < popCounts.size(); i++) { if (i != 0) { out += ','; } out += std::string_view(buffer, sprintf_s(buffer, "%d", popCounts[i])); }
			out += '\n';
		};
		char buffer[512];

		static constexpr auto noteTypeToChar = [](NoteType in) -> char
		{
			switch (in)
			{
			case NoteType::None: return '0';
			case NoteType::Don: return '1';
			case NoteType::Ka: return '2';
			case NoteType::DonBig: return '3';
			case NoteType::KaBig: return '4';
			case NoteType::Start_Drumroll: return '5';
			case NoteType::Start_DrumrollBig: return '6';
			case NoteType::Start_Balloon: return '7';
			case NoteType::End_BalloonOrDrumroll: return '8';
			case NoteType::Start_BaloonSpecial: return '9';
			case NoteType::DonBigBoth: return 'A';
			case NoteType::KaBigBoth: return 'B';
			case NoteType::Bomb: return 'C';
			case NoteType::Fuse: return 'D';
			case NoteType::Hidden: return 'F';
			case NoteType::KaDon: return 'G';
			default: return ' ';
			}
		};
		static constexpr auto difficultyTypeToString = [](DifficultyType in) -> cstr
		{
			switch (in)
			{
			case DifficultyType::Easy: return "Easy";
			case DifficultyType::Normal: return "Normal";
			case DifficultyType::Hard: return "Hard";
			case DifficultyType::Oni: return "Oni";
			case DifficultyType::OniUra: return "Edit";
			case DifficultyType::Tower: return "Tower";
			case DifficultyType::Dan: return "Dan";
			default: return "";
			}
		};
		static auto styleModeToString = [&](i32 in) -> std::string
		{
			switch (in)
			{
			case 1: return "Single";
			case 2: return "Double";
			default: return std::to_string(in);
			}
		};
		static constexpr auto sideToString = [](SongSelectSide in) -> cstr
		{
			switch (in)
			{
			case SongSelectSide::Normal: return "Normal";
			case SongSelectSide::Ex: return "Ex";
			case SongSelectSide::Both: return "Both";
			default: return "";
			}
		};


		if (inContent.HasPeepoDrumKitComment)
		{
			out += "// ";
			out += PeepoDrumKitCommentMarkerPrefix;
			if (inContent.PeepoDrumKitCommentDate != Date::Zero())
			{
				out += " ";
				out += inContent.PeepoDrumKitCommentDate.ToString().Data;
			}
			out += '\n';
		}

		DifficultyType currentCourseScope = DifficultyType::Count; // default course scope

		auto shouldEmitMainMetadata = [&, &inContent = inContent](auto ParsedMainMetadata::*... membs) // MSVC++ bug?: cannot implicitly capture variables when used in folder expression
		{
			return (... || (inContent.Metadata.*membs != DefaultMainMetadata.*membs));
		};

		appendProperyLine(out, Key::Main_TITLE, inContent.Metadata.TITLE); // Required for TaikoJiro
		if (shouldEmitMainMetadata(&ParsedMainMetadata::TITLE_localized)) {
			for (const auto& [locale, val] : inContent.Metadata.TITLE_localized)
				appendSuffixedPropertyLine(out, Key::Main_TITLE_localized, locale, val);
		}
		if (shouldEmitMainMetadata(&ParsedMainMetadata::SUBTITLE, &ParsedMainMetadata::SUBTITLE_localized))
			appendProperyLine(out, Key::Main_SUBTITLE, inContent.Metadata.SUBTITLE); // Better to be explicit if localized
		if (shouldEmitMainMetadata(&ParsedMainMetadata::SUBTITLE_localized)) {
			for (const auto& [locale, val] : inContent.Metadata.SUBTITLE_localized)
				appendSuffixedPropertyLine(out, Key::Main_SUBTITLE_localized, locale, val);
		}
		appendProperyLine(out, Key::Main_BPM, std::string_view(buffer, sprintf_s(buffer, "%g", inContent.Metadata.BPM.BPM))); // Better to be explicit
		if (shouldEmitMainMetadata(&ParsedMainMetadata::WAVE))
			appendProperyLine(out, Key::Main_WAVE, inContent.Metadata.WAVE);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::PREIMAGE))
			appendProperyLine(out, Key::Main_PREIMAGE, inContent.Metadata.PREIMAGE);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::WAVE, &ParsedMainMetadata::OFFSET)) // Better to be explicit if `WAVE:` is given
			appendProperyLine(out, Key::Main_OFFSET, std::string_view(buffer, sprintf_s(buffer, "%g", inContent.Metadata.OFFSET.Seconds)));
		if (shouldEmitMainMetadata(&ParsedMainMetadata::DEMOSTART))
			appendProperyLine(out, Key::Main_DEMOSTART, std::string_view(buffer, sprintf_s(buffer, "%g", inContent.Metadata.DEMOSTART.Seconds)));
		if (shouldEmitMainMetadata(&ParsedMainMetadata::GENRE))
			appendProperyLine(out, Key::Main_GENRE, inContent.Metadata.GENRE);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::SCOREMODE))
			appendProperyLine(out, Key::Main_SCOREMODE, std::string_view(buffer, sprintf_s(buffer, "%d", static_cast<i32>(inContent.Metadata.SCOREMODE))));
		if (shouldEmitMainMetadata(&ParsedMainMetadata::MAKER))
			appendProperyLine(out, Key::Main_MAKER, inContent.Metadata.MAKER);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::LYRICS))
			appendProperyLine(out, Key::Main_LYRICS, inContent.Metadata.LYRICS);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::SONGVOL))
			appendProperyLine(out, Key::Main_SONGVOL, std::string_view(buffer, sprintf_s(buffer, "%g", ToPercent(inContent.Metadata.SONGVOL))));
		if (shouldEmitMainMetadata(&ParsedMainMetadata::SEVOL))
			appendProperyLine(out, Key::Main_SEVOL, std::string_view(buffer, sprintf_s(buffer, "%g", ToPercent(inContent.Metadata.SEVOL))));
		// TODO: Key::Main_SIDE;
		// TODO: Key::Main_GAME;
		if (shouldEmitMainMetadata(&ParsedMainMetadata::HEADSCROLL))
			appendProperyLine(out, Key::Main_HEADSCROLL, std::string_view(buffer, sprintf_s(buffer, "%g", inContent.Metadata.HEADSCROLL)));
		if (shouldEmitMainMetadata(&ParsedMainMetadata::BGIMAGE))
			appendProperyLine(out, Key::Main_BGIMAGE, inContent.Metadata.BGIMAGE);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::BGMOVIE))
			appendProperyLine(out, Key::Main_BGMOVIE, inContent.Metadata.BGMOVIE);
		if (shouldEmitMainMetadata(&ParsedMainMetadata::BGIMAGE, &ParsedMainMetadata::BGMOVIE, &ParsedMainMetadata::MOVIEOFFSET)) // Better to be explicit if bg is given
			appendProperyLine(out, Key::Main_MOVIEOFFSET, std::string_view(buffer, sprintf_s(buffer, "%g", inContent.Metadata.MOVIEOFFSET.Seconds)));

		if (shouldEmitMainMetadata(&ParsedMainMetadata::Others)) {
			for (const auto& [header, val] : inContent.Metadata.Others)
				appendSuffixedPropertyLine(out, Key::Main_Unknown, header, val);
		}

		appendLine(out, "");

		// group difficulties by course scope
		using CourseIter = decltype(inContent.Courses)::const_iterator;
		std::vector<std::pair<CourseIter, CourseIter>> courseScopes = {};

		for (auto it = begin(inContent.Courses); it != end(inContent.Courses); ++it) {
			const auto& course = *it;
			if (course.Metadata.COURSE == currentCourseScope)
				continue;
			// change course scope
			if (!courseScopes.empty())
				courseScopes.back().second = it;
			courseScopes.emplace_back(it, end(inContent.Courses));
			currentCourseScope = course.Metadata.COURSE;
		}
		// revert course scope
		currentCourseScope = DifficultyType::Count;

		static constexpr auto courseMetadataDifferWithin = [](CourseIter it, CourseIter itBeg, CourseIter itEnd, auto ParsedCourseMetadata::*... membs)
		{
			for (CourseIter itI = itBeg; itI != itEnd; ++itI) {
				if ((... || (it->Metadata.*membs != itI->Metadata.*membs)))
					return true;
			}
			return false;
		};

		auto convertCourse = [&](CourseIter it, CourseIter itBeg, CourseIter itEnd)
		{
			auto shouldEmitCourseMetadata = [&, &inContent = inContent](auto ParsedCourseMetadata::*... membs) // MSVC++ bug?: cannot implicitly capture variables when used in folder expression
			{
				if (it == itBeg) {
					return (... || (it->Metadata.*membs != DefaultCourseMetadata.*membs)) // group-initial, non-default
						|| courseMetadataDifferWithin(it, begin(inContent.Courses), end(inContent.Courses), membs...); // differ globally, better to be explicit
				}
				return courseMetadataDifferWithin(it, itBeg, itEnd, membs...); // differ in group, better to be explicit
			};

			const ParsedCourse& course = *it;
			if (&course != &inContent.Courses[0])
				appendLine(out, "");

			b8 firstInGroup = (it == itBeg);
			if (firstInGroup) { // change course scope
				appendProperyLine(out, Key::Course_COURSE, difficultyTypeToString(course.Metadata.COURSE));
				currentCourseScope = course.Metadata.COURSE;
			}

			// scope-like, omit mid-group when possible
			if (firstInGroup ? shouldEmitCourseMetadata(&ParsedCourseMetadata::STYLE) : course.Metadata.STYLE != (it - 1)->Metadata.STYLE) {
				if (firstInGroup)
					appendLine(out, "");
				appendProperyLine(out, Key::Course_STYLE, styleModeToString(course.Metadata.STYLE));
				appendLine(out, "");
			}

			// Unspecified default value
			if (firstInGroup || shouldEmitCourseMetadata(&ParsedCourseMetadata::LEVEL, &ParsedCourseMetadata::LEVEL_DECIMALTAG)) {
				if (course.Metadata.LEVEL_DECIMALTAG == -1)
					appendProperyLine(out, Key::Course_LEVEL, std::string_view(buffer, sprintf_s(buffer, "%d", course.Metadata.LEVEL)));
				else
					appendProperyLine(out, Key::Course_LEVEL, std::string_view(buffer, sprintf_s(buffer, "%.1f", course.Metadata.LEVEL + static_cast<float>(course.Metadata.LEVEL_DECIMALTAG) / 10.)));
			}

			// Better to be explicit
			if (course.Metadata.COURSE == DifficultyType::Tower) {
				appendProperyLine(out, Key::Course_LIFE, std::string_view(buffer, sprintf_s(buffer, "%d", course.Metadata.LIFE)));
				appendProperyLine(out, Key::Course_SIDE, sideToString(course.Metadata.SIDE));
			}

			// Better to be explicit
			if (!course.Metadata.BALLOON.empty() || !course.Metadata.BALLOON_Normal.empty() || !course.Metadata.BALLOON_Expert.empty() || !course.Metadata.BALLOON_Master.empty())
				appendBalloonProperyLine(out, Key::Course_BALLOON, course.Metadata.BALLOON); // necessary for branched charts as branched BALLOON headers are not handled consistently across all simulators
			if (!course.Metadata.BALLOON_Normal.empty() || !course.Metadata.BALLOON_Expert.empty() || !course.Metadata.BALLOON_Master.empty())
			{
				appendBalloonProperyLine(out, Key::Course_BALLOONNOR, course.Metadata.BALLOON_Normal);
				appendBalloonProperyLine(out, Key::Course_BALLOONEXP, course.Metadata.BALLOON_Expert);
				appendBalloonProperyLine(out, Key::Course_BALLOONMAS, course.Metadata.BALLOON_Master);
			}

			if (shouldEmitCourseMetadata(&ParsedCourseMetadata::SCOREINIT, &ParsedCourseMetadata::SCOREDIFF)) {
				appendProperyLine(out, Key::Course_SCOREINIT, (course.Metadata.SCOREINIT == 0) ? "" : std::string_view(buffer, sprintf_s(buffer, "%d", course.Metadata.SCOREINIT)));
				appendProperyLine(out, Key::Course_SCOREDIFF, (course.Metadata.SCOREDIFF == 0) ? "" : std::string_view(buffer, sprintf_s(buffer, "%d", course.Metadata.SCOREDIFF)));
			}

			if (shouldEmitCourseMetadata(&ParsedCourseMetadata::NOTESDESIGNER))
				appendSuffixedPropertyLine(out, Key::Course_NOTESDESIGNERs, std::to_string(EnumToIndex(course.Metadata.COURSE)), course.Metadata.NOTESDESIGNER);

			// TODO: Key::Course_EXAM1;
			// TODO: Key::Course_EXAM2;
			// TODO: Key::Course_EXAM3;
			// TODO: Key::Course_GAUGEINCR;
			// TODO: Key::Course_TOTAL;
			// TODO: Key::Course_HIDDENBRANCH;

			if (shouldEmitCourseMetadata(&ParsedCourseMetadata::Others)) {
				for (const auto& [header, val] : course.Metadata.Others)
					appendSuffixedPropertyLine(out, Key::Course_Unknown, header, val);
			}

			appendLine(out, "");

			if (course.Metadata.STYLE <= 1)
				appendCommandLine(out, Key::Chart_START, "");
			else
				appendCommandLine(out, Key::Chart_START, "P" + std::to_string(course.Metadata.START_PLAYERSIDE));

			for (const ParsedChartCommand& command : course.ChartCommands)
			{
				switch (command.Type)
				{
				case ParsedChartCommandType::MeasureNotes:
				{
					for (const NoteType note : command.Param.MeasureNotes.Notes)
						out += noteTypeToChar(note);

					if (ArrayItToIndex(&command, &course.ChartCommands[0]) + 1 < course.ChartCommands.size())
					{
						if ((&command + 1)->Type != ParsedChartCommandType::MeasureEnd)
							appendLine(out, "");
					}
				} break;
				case ParsedChartCommandType::MeasureEnd: { appendLine(out, ","); } break;
				case ParsedChartCommandType::ChangeTimeSignature:
				{
					appendCommandLine(out, Key::Chart_MEASURE, std::string_view(buffer, sprintf_s(buffer, "%d/%d", command.Param.ChangeTimeSignature.Value.Numerator, command.Param.ChangeTimeSignature.Value.Denominator)));
				} break;
				case ParsedChartCommandType::ChangeTempo:
				{
					appendCommandLine(out, Key::Chart_BPMCHANGE, std::string_view(buffer, sprintf_s(buffer, "%g", command.Param.ChangeTempo.Value.BPM)));
				} break;
				case ParsedChartCommandType::ChangeDelay:
				{
					appendCommandLine(out, Key::Chart_DELAY, std::string_view(buffer, sprintf_s(buffer, "%g", command.Param.ChangeDelay.Value.ToSec())));
				} break;
				case ParsedChartCommandType::ChangeScrollSpeed:
				{
					appendCommandLine(out, Key::Chart_SCROLL, std::string_view(buffer, sprintf_s(buffer, "%s", command.Param.ChangeScrollSpeed.Value.toStringCompat().c_str())));
				} break;
				case ParsedChartCommandType::ChangeBarLine:
				{
					appendCommandLine(out, command.Param.ChangeBarLine.Visible ? Key::Chart_BARLINEON : Key::Chart_BARLINEOFF, "");
				} break;
				case ParsedChartCommandType::GoGoStart:
				{
					appendCommandLine(out, Key::Chart_GOGOSTART, "");
				} break;
				case ParsedChartCommandType::GoGoEnd:
				{
					appendCommandLine(out, Key::Chart_GOGOEND, "");
				} break;
				case ParsedChartCommandType::BranchStart:
				{
					appendCommandLine(out, Key::Chart_BRANCHSTART, std::string_view(buffer, sprintf_s(buffer, "%c,%d,%d", BranchConditionToChar(command.Param.BranchStart.Condition), command.Param.BranchStart.RequirementExpert, command.Param.BranchStart.RequirementMaster)));
				} break;
				case ParsedChartCommandType::BranchNormal:
				{
					appendCommandLine(out, Key::Chart_N, "");
				} break;
				case ParsedChartCommandType::BranchExpert:
				{
					appendCommandLine(out, Key::Chart_E, "");
				} break;
				case ParsedChartCommandType::BranchMaster:
				{
					appendCommandLine(out, Key::Chart_M, "");
				} break;
				case ParsedChartCommandType::BranchEnd:
				{
					appendCommandLine(out, Key::Chart_BRANCHEND, "");
				} break;
				case ParsedChartCommandType::BranchLevelHold:
				{
					// TODO:
				} break;
				case ParsedChartCommandType::ResetAccuracyValues:
				{
					// TODO:
				} break;
				case ParsedChartCommandType::SetLyricLine:
				{
					// TODO: Handle escape characters, most importantly "\n"
					appendCommandLine(out, Key::Chart_LYRIC, command.Param.SetLyricLine.Value);
				} break;
				case ParsedChartCommandType::NMScroll:
				{
					appendCommandLine(out, Key::Chart_NMSCROLL, "");
				} break;
				case ParsedChartCommandType::BMScroll:
				{
					appendCommandLine(out, Key::Chart_BMSCROLL, "");
				} break;
				case ParsedChartCommandType::HBScroll:
				{
					appendCommandLine(out, Key::Chart_HBSCROLL, "");
				} break;
				case ParsedChartCommandType::SENoteChange:
				{
					// TODO: DEPRECATED (?)
				} break;
				case ParsedChartCommandType::SetNextSong:
				{
					// TODO:
				} break;
				case ParsedChartCommandType::ChangeDirection:
				{
					// TODO: DEPRECATED
				} break;
				case ParsedChartCommandType::SetSudden:
				{
					appendCommandLine(out, Key::Chart_SUDDEN, std::string_view(buffer, sprintf_s(buffer, "%g %g", command.Param.SetSudden.AppearanceOffset.ToSec(), command.Param.SetSudden.MovementWaitDelay.ToSec())));
				} break;
				case ParsedChartCommandType::SetJPOSScroll:
				{
					appendCommandLine(out, Key::Chart_JPOSSCROLL, std::string_view(buffer, sprintf_s(buffer, "%g %s 1", command.Param.ChangeJPOSScroll.Duration.ToSec(), command.Param.ChangeJPOSScroll.Move.toStringCompat().c_str())));
				} break;
				default: { assert(!"Unhandled ParsedChartCommandType switch case"); } break;
				}
			}
			appendCommandLine(out, Key::Chart_END, "");
		};

		for (const auto& [itBeg, itEnd] : courseScopes) {
			for (CourseIter it = itBeg; it != itEnd; ++it)
				convertCourse(it, itBeg, itEnd);
		}
	}

	void ConvertConvertedMeasuresToParsedCommands(const std::vector<ConvertedMeasure>& inMeasures, std::vector<ParsedChartCommand>& outCommands)
	{
		struct TempCommand { Beat TimeWithinMeasure; ParsedChartCommand ParsedCommand; };
		std::vector<TempCommand> tempBuffer;
		tempBuffer.reserve(64);

		outCommands.reserve(inMeasures.size() * 4);

		TimeSignature lastSignature = DefaultTimeSignature;
		for (const ConvertedMeasure& inMeasure : inMeasures)
		{
			if (inMeasure.TimeSignature != lastSignature)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { Beat::Zero() }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::ChangeTimeSignature;
				tempCommand.Param.ChangeTimeSignature.Value = inMeasure.TimeSignature;
				lastSignature = inMeasure.TimeSignature;
			}

			for (const ConvertedGoGoChange& gogoChange : inMeasure.GoGoChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand{ gogoChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = (gogoChange.IsGogo) ? ParsedChartCommandType::GoGoStart : ParsedChartCommandType::GoGoEnd;
			}

			for (const ConvertedBarLineChange& barLineChange : inMeasure.BarLineChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { barLineChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::ChangeBarLine;
				tempCommand.Param.ChangeBarLine.Visible = barLineChange.Visibile;
			}

			for (const ConvertedTempoChange& tempoChange : inMeasure.TempoChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { tempoChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::ChangeTempo;
				tempCommand.Param.ChangeTempo.Value = tempoChange.Tempo;
			}

			for (const ConvertedScrollChange& scrollChange : inMeasure.ScrollChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { scrollChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::ChangeScrollSpeed;
				tempCommand.Param.ChangeScrollSpeed.Value = scrollChange.ScrollSpeed;
			}

			for (const ConvertedScrollType& scrollType : inMeasure.ScrollTypes)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand{ scrollType.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = (scrollType.Method == 0)
					? ParsedChartCommandType::NMScroll
					: (scrollType.Method == 1)
					? ParsedChartCommandType::HBScroll
					: ParsedChartCommandType::BMScroll;
				tempCommand.Param.ChangeScrollType.Method = scrollType.Method;
			}

			for (const ConvertedJPOSScroll& JPOSscrollChange : inMeasure.JPOSScrollChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand{ JPOSscrollChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::SetJPOSScroll;
				tempCommand.Param.ChangeJPOSScroll.Duration = Time(JPOSscrollChange.Duration);
				tempCommand.Param.ChangeJPOSScroll.Move = JPOSscrollChange.Move;
			}

			for (const ConvertedLyricChange& lyricChange : inMeasure.LyricChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { lyricChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::SetLyricLine;
				tempCommand.Param.SetLyricLine.Value = lyricChange.Lyric;
			}

			for (const ConvertedDelayChange& delayChange : inMeasure.DelayChanges)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { delayChange.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::ChangeDelay;
				tempCommand.Param.ChangeDelay.Value = delayChange.Delay;
			}

			// inMeasure.Notes should already be ordered by beat position
			size_t noteCommandStart = tempBuffer.size();
			i32 actualNotesInThisMeasure = 0;
			for (const ConvertedNote& note : inMeasure.Notes)
			{
				ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { note.TimeWithinMeasure }).ParsedCommand;
				tempCommand.Type = ParsedChartCommandType::MeasureNotes;
				tempCommand.Param.MeasureNotes.Notes.push_back(note.Type);
				actualNotesInThisMeasure++;
			}
			size_t noteCommandEnd = tempBuffer.size();

			if (!tempBuffer.empty())
			{
				const Beat measureBarDuration = abs(lastSignature.GetDurationPerBar());

				// NOTE: Find smallest bar division to fit in all the commands then add into temp
				i32 tickPerNoteInThisMeasure = measureBarDuration.Ticks;
				for (const TempCommand& c : tempBuffer) {
					if (c.TimeWithinMeasure.Ticks <= 0)
						continue;
					tickPerNoteInThisMeasure = std::gcd(tickPerNoteInThisMeasure, c.TimeWithinMeasure.Ticks);
					if (tickPerNoteInThisMeasure <= 1)
						break;
				}

				const Beat beatPerNoteInThisMeasure = Beat::FromTicks(tickPerNoteInThisMeasure);
				const i32 noteCommandsInThisMeasure = (beatPerNoteInThisMeasure == Beat::Zero()) ? 0 : (measureBarDuration.Ticks / beatPerNoteInThisMeasure.Ticks);

				static constexpr auto isLessTick = [](const TempCommand& a, const TempCommand& b) { return (a.TimeWithinMeasure < b.TimeWithinMeasure); };

				// NOTE: Insert empty notes based on smallestBarDivision
				if (noteCommandsInThisMeasure > 0)
				{
					// Cached searched beat range of non-blank note commands
					size_t commandIndex = noteCommandStart;
					i32 alreadyExistNoteIndexes = 0;

					for (i32 noteIndex = 0; noteIndex < noteCommandsInThisMeasure; noteIndex++)
					{
						const Beat noteBeat = Beat::FromTicks(noteIndex * beatPerNoteInThisMeasure.Ticks);
						auto tempCommand = TempCommand{ noteBeat };

						b8 noteAlreadyExists = false;
						auto noteNonAfterBeat = std::lower_bound(tempBuffer.begin() + commandIndex, tempBuffer.begin() + noteCommandEnd, tempCommand, isLessTick);
						if (noteNonAfterBeat != tempBuffer.begin() + noteCommandEnd
							&& noteNonAfterBeat->TimeWithinMeasure == noteBeat
							) {
							noteAlreadyExists = true;
							++alreadyExistNoteIndexes;
						}

						if (!noteAlreadyExists)
						{
							ParsedChartCommand& tempCommand = tempBuffer.emplace_back(TempCommand { noteBeat }).ParsedCommand;
							tempCommand.Type = ParsedChartCommandType::MeasureNotes;
							tempCommand.Param.MeasureNotes.Notes.push_back(NoteType::None);
						}
					}

					// Sort non-blank notes/commands first: O(nlogn)
					std::stable_sort(tempBuffer.begin(), tempBuffer.begin() + noteCommandEnd, isLessTick);
					// Then include blank notes (assumed to be much more than non-blanks),
					// which are already ordered: O(n)
					std::inplace_merge(tempBuffer.begin(), tempBuffer.begin() + noteCommandEnd, tempBuffer.end(), isLessTick);

					if (actualNotesInThisMeasure != alreadyExistNoteIndexes)
					{
						// BUG: Loss of precision due to integer division or overlapping notes (?)
						// assert(false);
					}
				}

				ParsedChartCommand* lastNoteCommand = nullptr;
				for (size_t i = 0; i < tempBuffer.size(); i++)
				{
					TempCommand& thisCommand = tempBuffer[i];
					// NOTE: Merge adjacent single-note MeasureNotes commands
					if (lastNoteCommand != nullptr && (thisCommand.ParsedCommand.Type == ParsedChartCommandType::MeasureNotes))
					{
						for (NoteType note : thisCommand.ParsedCommand.Param.MeasureNotes.Notes)
							lastNoteCommand->Param.MeasureNotes.Notes.push_back(note);
					}
					else {
						// Push first, modify later
						outCommands.push_back(std::move(thisCommand.ParsedCommand));
						lastNoteCommand = (outCommands.back().Type == ParsedChartCommandType::MeasureNotes) ?
							&outCommands.back()
							: nullptr;
					}
				}
				tempBuffer.clear();
			}

			outCommands.push_back(ParsedChartCommand { ParsedChartCommandType::MeasureEnd });
		}
	}

	ConvertedCourse ConvertParsedToConvertedCourse(const ParsedTJA& inContent, const ParsedCourse& inCourse)
	{
		ConvertedCourse out = {};
		out.MainMetadata = inContent.Metadata;
		out.CourseMetadata = inCourse.Metadata;

		{
			const size_t measureCount = std::count_if(inCourse.ChartCommands.begin(), inCourse.ChartCommands.end(), [](auto& c) { return c.Type == ParsedChartCommandType::MeasureEnd; });
			out.Measures.reserve(measureCount + 1);
		}

		// NOTE: Add measures with time signatures and notes (including "empty" ones as needed for time calculations)
		{
			ConvertedMeasure* currentMeasure = &out.Measures.emplace_back();
			currentMeasure->TimeSignature = DefaultTimeSignature;

			for (const ParsedChartCommand& command : inCourse.ChartCommands)
			{
				if (command.Type == ParsedChartCommandType::MeasureNotes)
				{
					for (const NoteType note : command.Param.MeasureNotes.Notes)
						currentMeasure->Notes.push_back(ConvertedNote { Beat::Zero(), note });
				}
				else if (command.Type == ParsedChartCommandType::MeasureEnd)
				{
					currentMeasure = &out.Measures.emplace_back();
					currentMeasure->TimeSignature = out.Measures[out.Measures.size() - 2].TimeSignature;
				}
				else if (command.Type == ParsedChartCommandType::ChangeTimeSignature)
				{
					currentMeasure->TimeSignature = command.Param.ChangeTimeSignature.Value;
				}
			}

			Beat currentMeasureTime = Beat::Zero();
			for (ConvertedMeasure& measure : out.Measures)
			{
				measure.StartTime = currentMeasureTime;
				currentMeasureTime += abs(measure.TimeSignature.GetDurationPerBar());

				Beat currentTimeWithinMeasure = Beat::Zero();
				for (ConvertedNote& note : measure.Notes)
				{
					note.TimeWithinMeasure = currentTimeWithinMeasure;
					currentTimeWithinMeasure += (abs(measure.TimeSignature.GetDurationPerBar()) / static_cast<i32>(measure.Notes.size()));
				}
			}
		}

		// NOTE: Insert additional commands
		{
			ConvertedMeasure* currentMeasure = &out.Measures[0];
			Beat currentTimeWithinMeasure = Beat::Zero();
			i32 currentNotesInMeasure = 0;

			for (const ParsedChartCommand& command : inCourse.ChartCommands)
			{
				if (command.Type == ParsedChartCommandType::MeasureNotes)
				{
					currentNotesInMeasure += static_cast<i32>(command.Param.MeasureNotes.Notes.size());

					if (!currentMeasure->Notes.empty() && currentNotesInMeasure > 0)
						currentTimeWithinMeasure = currentMeasure->Notes[currentNotesInMeasure - 1].TimeWithinMeasure +
						// TODO: WHAT TO DO HERE?
						(currentMeasure->Notes.size() > 1 ? currentMeasure->Notes[1].TimeWithinMeasure : Beat {});
				}
				if (command.Type == ParsedChartCommandType::MeasureEnd)
				{
					currentMeasure++;
					currentTimeWithinMeasure = Beat::Zero();
					currentNotesInMeasure = 0;
				}
				else if (command.Type == ParsedChartCommandType::ChangeTempo)
				{
					currentMeasure->TempoChanges.push_back(ConvertedTempoChange { currentTimeWithinMeasure, command.Param.ChangeTempo.Value });
				}
				else if (command.Type == ParsedChartCommandType::ChangeDelay)
				{
					currentMeasure->DelayChanges.push_back(ConvertedDelayChange { currentTimeWithinMeasure, command.Param.ChangeDelay.Value });
				}
				else if (command.Type == ParsedChartCommandType::ChangeScrollSpeed)
				{
					currentMeasure->ScrollChanges.push_back(ConvertedScrollChange { currentTimeWithinMeasure, command.Param.ChangeScrollSpeed.Value });
				}
				else if (command.Type == ParsedChartCommandType::GoGoStart)
				{
					const Beat startTime = currentMeasure->StartTime + currentTimeWithinMeasure;
					out.GoGoRanges.push_back(ConvertedGoGoRange { startTime, startTime });
				}
				else if (command.Type == ParsedChartCommandType::GoGoEnd)
				{
					if (!out.GoGoRanges.empty())
						out.GoGoRanges.back().EndTime = currentMeasure->StartTime + currentTimeWithinMeasure;
				}
				else if (command.Type == ParsedChartCommandType::ChangeBarLine)
				{
					currentMeasure->BarLineChanges.push_back(ConvertedBarLineChange { currentTimeWithinMeasure, command.Param.ChangeBarLine.Visible });
				}
				else if (command.Type == ParsedChartCommandType::SetLyricLine)
				{
					currentMeasure->LyricChanges.push_back(ConvertedLyricChange { currentTimeWithinMeasure, command.Param.SetLyricLine.Value });
				}
				else if (command.Type == ParsedChartCommandType::NMScroll || command.Type == ParsedChartCommandType::HBScroll || command.Type == ParsedChartCommandType::BMScroll) 
				{
					currentMeasure->ScrollTypes.push_back(ConvertedScrollType{ currentTimeWithinMeasure,
						static_cast<i8>((command.Type == ParsedChartCommandType::NMScroll)
						? 0
						: (command.Type == ParsedChartCommandType::HBScroll)
						? 1
						: 2
						)});
				}
				else if (command.Type == ParsedChartCommandType::SetJPOSScroll) 
				{
					currentMeasure->JPOSScrollChanges.push_back(ConvertedJPOSScroll{
						currentTimeWithinMeasure,
						command.Param.ChangeJPOSScroll.Move,
						command.Param.ChangeJPOSScroll.Duration.ToSec_F32()
						});
				}
			}

			ConvertedMeasure& firstMeasure = out.Measures[0];
			if (firstMeasure.TempoChanges.empty())
				firstMeasure.TempoChanges.push_back(ConvertedTempoChange { Beat::Zero(), inContent.Metadata.BPM });
		}

		// NOTE: Final cleanup
		{
			for (ConvertedMeasure& measure : out.Measures)
			{
				if (!measure.Notes.empty())
					erase_remove_if(measure.Notes, [](const ConvertedNote& note) { return (note.Type == NoteType::None); });
			}
		}

		return out;
	}
}
