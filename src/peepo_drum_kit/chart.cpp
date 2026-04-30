#include "chart.h"
#include "core_build_info.h"
#include <algorithm>

namespace PeepoDrumKit
{
	void DebugCompareCharts(const ChartProject& chartA, const ChartProject& chartB, DebugCompareChartsOnMessageFunc onMessageFunc, void* userData)
	{
		auto logf = [onMessageFunc, userData](cstr fmt, ...)
		{
			char buffer[512];
			va_list args;
			va_start(args, fmt);
			onMessageFunc(std::string_view(buffer, _vsnprintf_s(buffer, ArrayCount(buffer), fmt, args)), userData);
			va_end(args);
		};

		if (chartA.Courses.size() != chartB.Courses.size()) { logf("Course count mismatch (%zu != %zu)", chartA.Courses.size(), chartB.Courses.size()); return; }

		for (size_t i = 0; i < chartA.Courses.size(); i++)
		{
			const ChartCourse& courseA = *chartA.Courses[i];
			const ChartCourse& courseB = *chartB.Courses[i];

			for (GenericList list = {}; list < GenericList::Count; IncrementEnum(list))
			{
				const size_t countA = GetGenericListCount(courseA, list);
				const size_t countB = GetGenericListCount(courseB, list);
				if (countA != countB) { logf("%s count mismatch (%zu != %zu)", GenericListNames[EnumToIndex(list)], countA, countB); continue; }

				for (size_t itemIndex = 0; itemIndex < countA; itemIndex++)
				{
					for (GenericMember member = {}; member < GenericMember::Count; IncrementEnum(member))
					{
						GenericMemberUnion valueA {}, valueB {};
						const b8 hasValueA = TryGet(courseA, list, itemIndex, member, valueA);
						const b8 hasValueB = TryGet(courseB, list, itemIndex, member, valueB);
						assert(hasValueA == hasValueB);
						if (!hasValueA || member == GenericMember::B8_IsSelected)
							continue;

						static constexpr auto safeCStrAreSame = [](cstr a, cstr b) -> b8 { if ((a == nullptr) || (b == nullptr) && a != b) return false; return (strcmp(a, b) == 0); };
						cstr memberName = ""; b8 isSame = false;
						switch (member)
						{
						case GenericMember::B8_IsSelected: { memberName = "IsSelected"; isSame = (valueA.B8 == valueB.B8); } break;
						case GenericMember::B8_BarLineVisible: { memberName = "IsVisible"; isSame = (valueA.B8 == valueB.B8); } break;
						case GenericMember::I16_BalloonPopCount: { memberName = "BalloonPopCount"; isSame = (valueA.I16 == valueB.I16); } break;
						case GenericMember::F32_ScrollSpeed: { memberName = "ScrollSpeed"; isSame = ApproxmiatelySame(valueA.CPX, valueB.CPX); } break;
						case GenericMember::Beat_Start: { memberName = "BeatStart"; isSame = (valueA.Beat == valueB.Beat); } break;
						case GenericMember::Beat_Duration: { memberName = "BeatDuration"; isSame = (valueA.Beat == valueB.Beat); } break;
						case GenericMember::Time_Offset: { memberName = "TimeOffset"; isSame = ApproxmiatelySame(valueA.Time.Seconds, valueB.Time.Seconds); } break;
						case GenericMember::NoteType_V: { memberName = "NoteType"; isSame = (valueA.NoteType == valueB.NoteType); } break;
						case GenericMember::Tempo_V: { memberName = "Tempo"; isSame = ApproxmiatelySame(valueA.Tempo.BPM, valueB.Tempo.BPM); } break;
						case GenericMember::TimeSignature_V: { memberName = "TimeSignature"; isSame = (valueA.TimeSignature == valueB.TimeSignature); } break;
						case GenericMember::CStr_Lyric: { memberName = "Lyric"; isSame = safeCStrAreSame(valueA.CStr, valueB.CStr); } break;
						case GenericMember::I8_ScrollType: { memberName = "ScrollType"; isSame = (valueA.I16 == valueB.I16); } break;
						}

						if (!isSame)
							logf("%s[%zu].%s value mismatch", GenericListNames[EnumToIndex(list)], itemIndex, memberName);
					}
				}
			}
		}
	}

	struct TempTimedDelayCommand { Beat Beat; Time Delay; };

	template <>
	struct IsNonListChartEventTrait<TempTimedDelayCommand> : std::true_type { };

	template <GenericMember Member, typename TempTimedDelayCommandT, expect_type_t<TempTimedDelayCommandT, TempTimedDelayCommand> = true>
	constexpr decltype(auto) get(TempTimedDelayCommandT&& event)
	{
		if constexpr (Member == GenericMember::Beat_Start) return (std::forward<TempTimedDelayCommandT>(event).Beat);
	}

	static constexpr NoteType ConvertTJANoteType(TJA::NoteType tjaNoteType)
	{
		switch (tjaNoteType)
		{
		case TJA::NoteType::None: return NoteType::Count;
		case TJA::NoteType::Don: return NoteType::Don;
		case TJA::NoteType::Ka: return NoteType::Ka;
		case TJA::NoteType::DonBig: return NoteType::DonBig;
		case TJA::NoteType::KaBig: return NoteType::KaBig;
		case TJA::NoteType::Start_Drumroll: return NoteType::Drumroll;
		case TJA::NoteType::Start_DrumrollBig: return NoteType::DrumrollBig;
		case TJA::NoteType::Start_Balloon: return NoteType::Balloon;
		case TJA::NoteType::End_BalloonOrDrumroll: return NoteType::Count;
		case TJA::NoteType::Start_BaloonSpecial: return NoteType::BalloonSpecial;
		case TJA::NoteType::DonBigBoth: return NoteType::DonBigHand;
		case TJA::NoteType::KaBigBoth: return NoteType::KaBigHand;
		case TJA::NoteType::Hidden: return NoteType::Adlib;
		case TJA::NoteType::Bomb: return NoteType::Bomb;
		case TJA::NoteType::KaDon: return NoteType::KaDon;
		case TJA::NoteType::Fuse: return NoteType::Fuse;
		default: return NoteType::Count;
		}
	}

	static constexpr TJA::NoteType ConvertTJANoteType(NoteType noteType)
	{
		switch (noteType)
		{
		case NoteType::Don: return TJA::NoteType::Don;
		case NoteType::DonBig: return TJA::NoteType::DonBig;
		case NoteType::Ka: return TJA::NoteType::Ka;
		case NoteType::KaBig: return TJA::NoteType::KaBig;
		case NoteType::Drumroll: return TJA::NoteType::Start_Drumroll;
		case NoteType::DrumrollBig: return TJA::NoteType::Start_DrumrollBig;
		case NoteType::Balloon: return TJA::NoteType::Start_Balloon;
		case NoteType::BalloonSpecial: return TJA::NoteType::Start_BaloonSpecial;
		case NoteType::DonBigHand: return TJA::NoteType::DonBigBoth;
		case NoteType::KaBigHand: return TJA::NoteType::KaBigBoth;
		case NoteType::KaDon: return TJA::NoteType::KaDon;
		case NoteType::Adlib: return TJA::NoteType::Hidden;
		case NoteType::Fuse: return TJA::NoteType::Fuse;
		case NoteType::Bomb: return TJA::NoteType::Bomb;
		default: return TJA::NoteType::None;
		}
	}

	Beat FindCourseMaxUsedBeat(const ChartCourse& course)
	{
		// NOTE: Technically only need to look at the last item of each sorted list **but just to be sure**, in case there is something wonky going on with out-of-order durations or something
		Beat maxBeat = Beat::Zero();
		for (const auto& v : course.TempoMap.Tempo) maxBeat = Max(maxBeat, v.Beat);
		for (const auto& v : course.TempoMap.Signature) maxBeat = Max(maxBeat, v.Beat);
		for (size_t i = 0; i < EnumCount<BranchType>; i++)
			for (const auto& v : course.GetNotes(static_cast<BranchType>(i))) maxBeat = Max(maxBeat, v.BeatTime + Max(Beat::Zero(), v.BeatDuration));
		for (const auto& v : course.GoGoRanges) maxBeat = Max(maxBeat, v.BeatTime + Max(Beat::Zero(), v.BeatDuration));
		for (const auto& v : course.BranchSections) maxBeat = Max(maxBeat, v.GetEnd());
		for (const auto& v : course.ScrollChanges) maxBeat = Max(maxBeat, v.BeatTime);
		for (const auto& v : course.BarLineChanges) maxBeat = Max(maxBeat, v.BeatTime);
		for (const auto& v : course.Lyrics) maxBeat = Max(maxBeat, v.BeatTime);
		return maxBeat;
	}

	b8 CreateChartProjectFromTJA(const TJA::ParsedTJA& inTJA, ChartProject& out)
	{
		out.ChartDuration = Time::Zero();
		out.ChartTitle = inTJA.Metadata.TITLE;
		out.ChartTitleLocalized = inTJA.Metadata.TITLE_localized;
		out.ChartSubtitle = inTJA.Metadata.SUBTITLE;
		out.ChartSubtitleLocalized = inTJA.Metadata.SUBTITLE_localized;
		out.ChartCreator = inTJA.Metadata.MAKER;
		out.ChartGenre = inTJA.Metadata.GENRE;
		out.ChartLyricsFileName = inTJA.Metadata.LYRICS;
		out.SongOffset = inTJA.Metadata.OFFSET;
		out.SongDemoStartTime = inTJA.Metadata.DEMOSTART;
		out.SongFileName = inTJA.Metadata.WAVE;
		out.SongJacket = inTJA.Metadata.PREIMAGE;
		out.SongVolume = inTJA.Metadata.SONGVOL;
		out.SoundEffectVolume = inTJA.Metadata.SEVOL;
		out.BackgroundImageFileName = inTJA.Metadata.BGIMAGE;
		out.BackgroundMovieFileName = inTJA.Metadata.BGMOVIE;
		out.MovieOffset = inTJA.Metadata.MOVIEOFFSET;
		out.OtherMetadata = inTJA.Metadata.Others;
		for (size_t i = 0; i < inTJA.Courses.size(); i++)
		{
			if (!inTJA.Courses[i].HasChart) // metadata-only TJA section
				continue;

			const TJA::ConvertedCourse& inCourse = TJA::ConvertParsedToConvertedCourse(inTJA, inTJA.Courses[i]);
			ChartCourse& outCourse = *out.Courses.emplace_back(std::make_unique<ChartCourse>());

			// HACK: Write proper enum conversion functions
			outCourse.Type = Clamp(static_cast<DifficultyType>(inCourse.CourseMetadata.COURSE), DifficultyType {}, DifficultyType::Count);
			outCourse.Level = Clamp(static_cast<DifficultyLevel>(inCourse.CourseMetadata.LEVEL), DifficultyLevel::Min, DifficultyLevel::Max);
			outCourse.Decimal = Clamp(static_cast<DifficultyLevelDecimal>(inCourse.CourseMetadata.LEVEL_DECIMALTAG), DifficultyLevelDecimal::None, DifficultyLevelDecimal::Max);
			outCourse.Style = std::max(inCourse.CourseMetadata.STYLE, 1);
			outCourse.PlayerSide = std::clamp(inCourse.CourseMetadata.START_PLAYERSIDE, 1, outCourse.Style);

			outCourse.CourseCreator = inCourse.CourseMetadata.NOTESDESIGNER;

			outCourse.Life = Clamp(static_cast<TowerLives>(inCourse.CourseMetadata.LIFE), TowerLives::Min, TowerLives::Max);
			outCourse.Side = Clamp(static_cast<Side>(inCourse.CourseMetadata.SIDE), Side{}, Side::Count);

			outCourse.TempoMap.Tempo.Sorted = { TempoChange(Beat::Zero(), inTJA.Metadata.BPM) };
			outCourse.TempoMap.Signature.Sorted = { TimeSignatureChange(Beat::Zero(), TimeSignature(4, 4)) };
			TimeSignature lastSignature = TimeSignature(4, 4);

			BeatSortedList<TempTimedDelayCommand> tempSortedDelayCommands;
			BeatSortedForwardIterator<TempTimedDelayCommand> tempDelayCommandsIt;
			for (const TJA::ConvertedMeasure& inMeasure : inCourse.Measures)
			{
				for (const TJA::ConvertedDelayChange& inDelayChange : inMeasure.DelayChanges)
					tempSortedDelayCommands.InsertOrUpdate(TempTimedDelayCommand { inMeasure.StartTime + inDelayChange.TimeWithinMeasure, inDelayChange.Delay });
			}

			// Check whether this course uses branches
			const b8 hasBranches = std::any_of(inTJA.Courses[i].ChartCommands.begin(), inTJA.Courses[i].ChartCommands.end(),
				[](const TJA::ParsedChartCommand& cmd) { return cmd.Type == TJA::ParsedChartCommandType::BranchStart; });

			// Import #SECTION markers from raw commands for both branched and non-branched charts
			// (non-branched: simple beat lookup by measure index; branched: handled below in the else block too)
			if (!hasBranches)
			{
				i32 smMeasureIdx = 0;
				for (const TJA::ParsedChartCommand& cmd : inTJA.Courses[i].ChartCommands)
				{
					if (cmd.Type == TJA::ParsedChartCommandType::ResetAccuracyValues && smMeasureIdx < (i32)inCourse.Measures.size())
					{
						SectionMarker sm; sm.BeatTime = inCourse.Measures[smMeasureIdx].StartTime;
						outCourse.SectionMarkers.InsertOrIgnore(std::move(sm));
					}
					else if (cmd.Type == TJA::ParsedChartCommandType::MeasureEnd) { smMeasureIdx++; }
				}
			}

			if (!hasBranches)
			{
				// No branches: standard single-pass note extraction (original behaviour)
				i32 currentBalloonIndex = 0;
				for (const TJA::ConvertedMeasure& inMeasure : inCourse.Measures)
				{
					for (const TJA::ConvertedNote& inNote : inMeasure.Notes)
					{
						if (inNote.Type == TJA::NoteType::End_BalloonOrDrumroll)
						{
							if (!outCourse.Notes_Normal.Sorted.empty())
								outCourse.Notes_Normal.Sorted.back().BeatDuration = (inMeasure.StartTime + inNote.TimeWithinMeasure) - outCourse.Notes_Normal.Sorted.back().BeatTime;
							continue;
						}

						const NoteType outNoteType = ConvertTJANoteType(inNote.Type);
						if (outNoteType == NoteType::Count)
							continue;

						Note& outNote = outCourse.Notes_Normal.Sorted.emplace_back();
						outNote.BeatTime = (inMeasure.StartTime + inNote.TimeWithinMeasure);
						outNote.Type = outNoteType;

						const TempTimedDelayCommand* delayCommandForThisNote = tempDelayCommandsIt.Next(tempSortedDelayCommands.Sorted, outNote.BeatTime);
						outNote.TimeOffset = (delayCommandForThisNote != nullptr) ? delayCommandForThisNote->Delay : Time::Zero();

						if (inNote.Type == TJA::NoteType::Start_Balloon || inNote.Type == TJA::NoteType::Start_BaloonSpecial || inNote.Type == TJA::NoteType::Fuse)
						{
							if (InBounds(currentBalloonIndex, inCourse.CourseMetadata.BALLOON))
								outNote.BalloonPopCount = inCourse.CourseMetadata.BALLOON[currentBalloonIndex];
							currentBalloonIndex++;
						}
					}
				}
			}
			else
			{
				// Branches present: per-branch note extraction.
				// ConvertParsedToConvertedCourse merges all branch notes into one list which gives
				// wrong beat positions for notes inside branch sections.  We need to re-derive beat
				// positions independently per branch by replaying the raw ParsedChartCommand stream.

				// Per-measure, per-branch note type buffer
				struct BranchMeasureNotes { std::vector<TJA::NoteType> notes[3]; };
				std::vector<BranchMeasureNotes> branchMeasures(inCourse.Measures.size());

				i32 measureIdx = 0;
				i32 branchSectionStartMeasure = 0;
				b8 inBranchBlock = false;
				BranchType currentBranch = BranchType::Count;
				TJA::BranchCondition currentCondition = TJA::BranchCondition::Precise;
				i32 currentReqExpert = 100, currentReqMaster = 100;
				Beat currentBranchStartBeat = Beat::Zero();

				auto finishBranchBlock = [&]()
				{
					const Beat endBeat = (measureIdx < (i32)inCourse.Measures.size())
						? inCourse.Measures[measureIdx].StartTime
						: Beat::FromTicks(I32Max);
					const Beat duration = Max(Beat::Zero(), endBeat - currentBranchStartBeat);
					outCourse.BranchSections.Sorted.push_back(BranchSection { currentBranchStartBeat, duration, currentCondition, currentReqExpert, currentReqMaster });
				};

				for (const TJA::ParsedChartCommand& cmd : inTJA.Courses[i].ChartCommands)
				{
					switch (cmd.Type)
					{
					case TJA::ParsedChartCommandType::BranchStart:
						if (inBranchBlock)
							finishBranchBlock(); // implicit end
						inBranchBlock = true;
						currentBranch = BranchType::Count;
						branchSectionStartMeasure = measureIdx;
						currentCondition = cmd.Param.BranchStart.Condition;
						currentReqExpert = cmd.Param.BranchStart.RequirementExpert;
						currentReqMaster = cmd.Param.BranchStart.RequirementMaster;
						currentBranchStartBeat = (measureIdx < (i32)inCourse.Measures.size())
							? inCourse.Measures[measureIdx].StartTime : Beat::Zero();
						break;
					case TJA::ParsedChartCommandType::BranchNormal:
						currentBranch = BranchType::Normal;
						measureIdx = branchSectionStartMeasure;
						break;
					case TJA::ParsedChartCommandType::BranchExpert:
						currentBranch = BranchType::Expert;
						measureIdx = branchSectionStartMeasure;
						break;
					case TJA::ParsedChartCommandType::BranchMaster:
						currentBranch = BranchType::Master;
						measureIdx = branchSectionStartMeasure;
						break;
					case TJA::ParsedChartCommandType::BranchEnd:
						if (inBranchBlock)
							finishBranchBlock();
						inBranchBlock = false;
						currentBranch = BranchType::Count;
						break;
					case TJA::ParsedChartCommandType::ResetAccuracyValues:
					{
						const Beat sectionBeat = (measureIdx < (i32)inCourse.Measures.size())
							? inCourse.Measures[measureIdx].StartTime : Beat::Zero();
						SectionMarker sm; sm.BeatTime = sectionBeat;
						outCourse.SectionMarkers.InsertOrIgnore(std::move(sm));
						break;
					}
					case TJA::ParsedChartCommandType::MeasureNotes:
						if (measureIdx >= 0 && measureIdx < (i32)branchMeasures.size())
						{
							if (!inBranchBlock)
							{
								for (const TJA::NoteType nt : cmd.Param.MeasureNotes.Notes)
									for (i32 b = 0; b < 3; b++)
										branchMeasures[measureIdx].notes[b].push_back(nt);
							}
							else if (currentBranch < BranchType::Count)
							{
								for (const TJA::NoteType nt : cmd.Param.MeasureNotes.Notes)
									branchMeasures[measureIdx].notes[EnumToIndex(currentBranch)].push_back(nt);
							}
						}
						break;
					case TJA::ParsedChartCommandType::ChangeScrollSpeed:
						if (inBranchBlock && currentBranch < BranchType::Count && currentBranch != BranchType::Normal
							&& measureIdx < (i32)inCourse.Measures.size())
						{
							const Beat beat = inCourse.Measures[measureIdx].StartTime;
							outCourse.GetScrollChanges(currentBranch).InsertOrUpdate(ScrollChange { beat, cmd.Param.ChangeScrollSpeed.Value });
						}
						break;
					case TJA::ParsedChartCommandType::NMScroll:
					case TJA::ParsedChartCommandType::HBScroll:
					case TJA::ParsedChartCommandType::BMScroll:
						if (inBranchBlock && currentBranch < BranchType::Count && currentBranch != BranchType::Normal
							&& measureIdx < (i32)inCourse.Measures.size())
						{
							const Beat beat = inCourse.Measures[measureIdx].StartTime;
							const ScrollMethod method = (cmd.Type == TJA::ParsedChartCommandType::NMScroll) ? ScrollMethod::NMSCROLL
								: (cmd.Type == TJA::ParsedChartCommandType::HBScroll) ? ScrollMethod::HBSCROLL : ScrollMethod::BMSCROLL;
							outCourse.GetScrollTypes(currentBranch).InsertOrUpdate(ScrollType { beat, method });
						}
						break;
					case TJA::ParsedChartCommandType::SetJPOSScroll:
						if (inBranchBlock && currentBranch < BranchType::Count && currentBranch != BranchType::Normal
							&& measureIdx < (i32)inCourse.Measures.size())
						{
							const Beat beat = inCourse.Measures[measureIdx].StartTime;
							outCourse.GetJPOSScrollChanges(currentBranch).InsertOrUpdate(
								JPOSScrollChange { beat, cmd.Param.ChangeJPOSScroll.Move, cmd.Param.ChangeJPOSScroll.Duration.ToSec_F32() });
						}
						break;
					case TJA::ParsedChartCommandType::MeasureEnd:
						measureIdx++;
						break;
					default:
						break;
					}
				}

				// Determine balloon lists per branch (prefer per-branch headers, fall back to combined)
				const std::vector<i32>* balloonLists[3] = {
					!inCourse.CourseMetadata.BALLOON_Normal.empty() ? &inCourse.CourseMetadata.BALLOON_Normal : &inCourse.CourseMetadata.BALLOON,
					!inCourse.CourseMetadata.BALLOON_Expert.empty() ? &inCourse.CourseMetadata.BALLOON_Expert : &inCourse.CourseMetadata.BALLOON,
					!inCourse.CourseMetadata.BALLOON_Master.empty() ? &inCourse.CourseMetadata.BALLOON_Master : &inCourse.CourseMetadata.BALLOON,
				};
				i32 balloonIndex[3] = { 0, 0, 0 };

				// Convert per-measure per-branch notes to timed note lists
				tempDelayCommandsIt = BeatSortedForwardIterator<TempTimedDelayCommand>(); // reset
				for (size_t mi = 0; mi < inCourse.Measures.size() && mi < branchMeasures.size(); mi++)
				{
					const TJA::ConvertedMeasure& measure = inCourse.Measures[mi];
					const Beat measureDuration = Beat { std::abs(measure.TimeSignature.GetDurationPerBar().Ticks) };

					for (i32 b = 0; b < 3; b++)
					{
						SortedNotesList& branchNotes = outCourse.GetNotes(static_cast<BranchType>(b));
						const auto& noteTypes = branchMeasures[mi].notes[b];
						const i32 noteCount = (i32)noteTypes.size();
						if (noteCount == 0) continue;

						const Beat beatPerNote = (noteCount > 0) ? Beat::FromTicks(measureDuration.Ticks / noteCount) : Beat::Zero();

						for (i32 ni = 0; ni < noteCount; ni++)
						{
							const TJA::NoteType nt = noteTypes[ni];
							if (nt == TJA::NoteType::End_BalloonOrDrumroll)
							{
								if (!branchNotes.Sorted.empty())
									branchNotes.Sorted.back().BeatDuration = (measure.StartTime + Beat::FromTicks(ni * beatPerNote.Ticks)) - branchNotes.Sorted.back().BeatTime;
								continue;
							}

							const NoteType outNoteType = ConvertTJANoteType(nt);
							if (outNoteType == NoteType::Count) continue;

							Note& outNote = branchNotes.Sorted.emplace_back();
							outNote.BeatTime = measure.StartTime + Beat::FromTicks(ni * beatPerNote.Ticks);
							outNote.Type = outNoteType;

							const TempTimedDelayCommand* dc = tempDelayCommandsIt.Next(tempSortedDelayCommands.Sorted, outNote.BeatTime);
							outNote.TimeOffset = (dc != nullptr) ? dc->Delay : Time::Zero();

							if (nt == TJA::NoteType::Start_Balloon || nt == TJA::NoteType::Start_BaloonSpecial || nt == TJA::NoteType::Fuse)
							{
								const std::vector<i32>& bl = *balloonLists[b];
								if (InBounds(balloonIndex[b], bl))
									outNote.BalloonPopCount = bl[balloonIndex[b]];
								balloonIndex[b]++;
							}
						}
					}
				}
			}

			// Extract time-signature, tempo, scroll, barline, lyric changes from the converted measures
			for (const TJA::ConvertedMeasure& inMeasure : inCourse.Measures)
			{
				if (inMeasure.TimeSignature != lastSignature)
				{
					outCourse.TempoMap.Signature.InsertOrUpdate(TimeSignatureChange(inMeasure.StartTime, inMeasure.TimeSignature));
					lastSignature = inMeasure.TimeSignature;
				}

				for (const TJA::ConvertedTempoChange& inTempoChange : inMeasure.TempoChanges)
					outCourse.TempoMap.Tempo.InsertOrUpdate(TempoChange(inMeasure.StartTime + inTempoChange.TimeWithinMeasure, inTempoChange.Tempo));

				for (const TJA::ConvertedScrollChange& inScrollChange : inMeasure.ScrollChanges)
					outCourse.ScrollChanges.Sorted.push_back(ScrollChange { (inMeasure.StartTime + inScrollChange.TimeWithinMeasure), inScrollChange.ScrollSpeed });

				for (const TJA::ConvertedScrollType& inScrollType : inMeasure.ScrollTypes)
					outCourse.ScrollTypes.Sorted.push_back(ScrollType{ (inMeasure.StartTime + inScrollType.TimeWithinMeasure), static_cast<ScrollMethod>(inScrollType.Method) });

				for (const TJA::ConvertedJPOSScroll& inJPOSScrollChange : inMeasure.JPOSScrollChanges)
					outCourse.JPOSScrollChanges.Sorted.push_back(JPOSScrollChange{ (inMeasure.StartTime + inJPOSScrollChange.TimeWithinMeasure), inJPOSScrollChange.Move, inJPOSScrollChange.Duration });

				for (const TJA::ConvertedBarLineChange& barLineChange : inMeasure.BarLineChanges)
					outCourse.BarLineChanges.Sorted.push_back(BarLineChange { (inMeasure.StartTime + barLineChange.TimeWithinMeasure), barLineChange.Visibile });

				for (const TJA::ConvertedLyricChange& lyricChange : inMeasure.LyricChanges)
					outCourse.Lyrics.Sorted.push_back(LyricChange { (inMeasure.StartTime + lyricChange.TimeWithinMeasure), lyricChange.Lyric });
			}

			for (const TJA::ConvertedGoGoRange& inGoGoRange : inCourse.GoGoRanges)
				outCourse.GoGoRanges.Sorted.push_back(GoGoRange { inGoGoRange.StartTime, (inGoGoRange.EndTime - inGoGoRange.StartTime) });

			//outCourse.TempoMap.SetTempoChange(TempoChange());
			//outCourse.TempoMap = inCourse.GoGoRanges;

			outCourse.ScoreInit = inCourse.CourseMetadata.SCOREINIT;
			outCourse.ScoreDiff = inCourse.CourseMetadata.SCOREDIFF;

			outCourse.OtherMetadata = inCourse.CourseMetadata.Others;

			outCourse.TempoMap.RebuildAccelerationStructure();
			outCourse.RecalculateSENotes();

			if (!inCourse.Measures.empty())
				out.ChartDuration = Max(out.ChartDuration, outCourse.TempoMap.BeatToTime(inCourse.Measures.back().StartTime /*+ inCourse.Measures.back().TimeSignature.GetDurationPerBar()*/));
		}

		return true;
	}

	b8 ConvertChartProjectToTJA(const ChartProject& in, TJA::ParsedTJA& out, b8 includePeepoDrumKitComment)
	{
		static constexpr cstr FallbackTJAChartTitle = "Untitled Chart";
		out.Metadata.TITLE = !in.ChartTitle.empty() ? in.ChartTitle : FallbackTJAChartTitle;
		out.Metadata.TITLE_localized = in.ChartTitleLocalized;
		out.Metadata.SUBTITLE = in.ChartSubtitle;
		out.Metadata.SUBTITLE_localized = in.ChartSubtitleLocalized;
		out.Metadata.MAKER = in.ChartCreator;
		out.Metadata.GENRE = in.ChartGenre;
		out.Metadata.LYRICS = in.ChartLyricsFileName;
		out.Metadata.OFFSET = in.SongOffset;
		out.Metadata.DEMOSTART = in.SongDemoStartTime;
		out.Metadata.WAVE = in.SongFileName;
		out.Metadata.PREIMAGE = in.SongJacket;
		out.Metadata.SONGVOL = in.SongVolume;
		out.Metadata.SEVOL = in.SoundEffectVolume;
		out.Metadata.BGIMAGE = in.BackgroundImageFileName;
		out.Metadata.BGMOVIE = in.BackgroundMovieFileName;
		out.Metadata.MOVIEOFFSET = in.MovieOffset;
		out.Metadata.Others = in.OtherMetadata;

		if (includePeepoDrumKitComment)
		{
			out.HasPeepoDrumKitComment = true;
			out.PeepoDrumKitCommentDate = BuildInfo::CompilationDateParsed;
		}

		if (!in.Courses.empty())
		{
			if (!in.Courses[0]->TempoMap.Tempo.empty())
			{
				const TempoChange* initialTempo = in.Courses[0]->TempoMap.Tempo.TryFindLastAtBeat(Beat::Zero());
				out.Metadata.BPM = (initialTempo != nullptr) ? initialTempo->Tempo : FallbackTempo;
			}
		}

		out.Courses.reserve(in.Courses.size());
		for (const std::unique_ptr<ChartCourse>& inCourseIt : in.Courses)
		{
			const ChartCourse& inCourse = *inCourseIt;
			TJA::ParsedCourse& outCourse = out.Courses.emplace_back();

			// HACK: Write proper enum conversion functions
			outCourse.Metadata.COURSE = static_cast<TJA::DifficultyType>(inCourse.Type);
			outCourse.Metadata.LEVEL = static_cast<i32>(inCourse.Level);
			outCourse.Metadata.LEVEL_DECIMALTAG = static_cast<i32>(inCourse.Decimal);
			outCourse.Metadata.STYLE = inCourse.Style;
			outCourse.Metadata.START_PLAYERSIDE = inCourse.PlayerSide;
			outCourse.Metadata.NOTESDESIGNER = inCourse.CourseCreator;
			if (inCourse.HasBranches())
			{
				for (const Note& n : inCourse.Notes_Normal) if (IsBalloonNote(n.Type)) { outCourse.Metadata.BALLOON.push_back(n.BalloonPopCount); outCourse.Metadata.BALLOON_Normal.push_back(n.BalloonPopCount); }
				for (const Note& n : inCourse.Notes_Expert) if (IsBalloonNote(n.Type)) outCourse.Metadata.BALLOON_Expert.push_back(n.BalloonPopCount);
				for (const Note& n : inCourse.Notes_Master) if (IsBalloonNote(n.Type)) outCourse.Metadata.BALLOON_Master.push_back(n.BalloonPopCount);
			}
			else
			{
				for (const Note& inNote : inCourse.Notes_Normal) if (IsBalloonNote(inNote.Type)) { outCourse.Metadata.BALLOON.push_back(inNote.BalloonPopCount); }
			}
			outCourse.Metadata.SCOREINIT = inCourse.ScoreInit;
			outCourse.Metadata.SCOREDIFF = inCourse.ScoreDiff;

			outCourse.Metadata.LIFE = static_cast<i32>(inCourse.Life);
			outCourse.Metadata.SIDE = static_cast<TJA::SongSelectSide>(inCourse.Side);

			outCourse.Metadata.Others = inCourse.OtherMetadata;

			// TODO: Is this implemented correctly..? Need to have enough measures to cover every note/command and pad with empty measures up to the chart duration
			// BUG: NOPE! "07 �Q�[���~���[�W�b�N/003D. MagiCatz/MagiCatz.tja" for example still gets rounded up and then increased by a measure each time it gets saved
			// ... and even so does "Heat Haze Shadow 2.tja" without any weird time signatures..??
			const Beat inChartMaxUsedBeat = FindCourseMaxUsedBeat(inCourse);
			const Beat inChartBeatDuration = inCourse.TempoMap.TimeToBeat(in.GetDurationOrDefault());
			std::vector<TJA::ConvertedMeasure> outConvertedMeasures;

			inCourse.TempoMap.ForEachBeatBar([&](const SortedTempoMap::ForEachBeatBarData& it)
			{
				if (inChartBeatDuration > inChartMaxUsedBeat && (it.Beat >= inChartBeatDuration))
					return ControlFlow::Break;
				if (it.IsBar)
				{
					TJA::ConvertedMeasure& outConvertedMeasure = outConvertedMeasures.emplace_back();
					outConvertedMeasure.StartTime = it.Beat;
					outConvertedMeasure.TimeSignature = it.Signature;
				}
				return (it.Beat >= Max(inChartBeatDuration, inChartMaxUsedBeat)) ? ControlFlow::Break : ControlFlow::Fallthrough;
			});

			if (outConvertedMeasures.empty())
				outConvertedMeasures.push_back(TJA::ConvertedMeasure { Beat::Zero(), TimeSignature(4, 4) });

			static constexpr auto tryFindMeasureForBeat = [](std::vector<TJA::ConvertedMeasure>& measures, Beat beatToFind) -> TJA::ConvertedMeasure*
			{
				static constexpr auto isMoreBeat = [](const TJA::ConvertedMeasure& lhs, const TJA::ConvertedMeasure& rhs)
				{
					return lhs.StartTime > rhs.StartTime;
				};
				// Binary search in descending (ascending but reversed) list
				// if found: `it` is the last element such that `beatToFind >= it->StartTime`
				auto it = std::lower_bound(measures.rbegin(), measures.rend(), TJA::ConvertedMeasure { beatToFind }, isMoreBeat);
				return (it == measures.rend()) ? nullptr : &*it;
			};

			for (const TempoChange& inTempoChange : inCourse.TempoMap.Tempo)
			{
				if (!(&inTempoChange == &inCourse.TempoMap.Tempo[0] && inTempoChange.Tempo.BPM == out.Metadata.BPM.BPM))
				{
					TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inTempoChange.Beat);
					if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
						outConvertedMeasure->TempoChanges.push_back(TJA::ConvertedTempoChange { (inTempoChange.Beat - outConvertedMeasure->StartTime), inTempoChange.Tempo });
				}
			}

			// Helper: place notes from a note list into a measure vector within an optional beat range
			auto placeNotesInMeasures = [&](const SortedNotesList& notes, std::vector<TJA::ConvertedMeasure>& measures,
				Beat rangeStart = Beat::Zero(), Beat rangeEnd = Beat::FromTicks(I32Max))
			{
				Time lastOffset = Time::Zero();
				for (const Note& inNote : notes)
				{
					if (inNote.BeatTime < rangeStart || inNote.BeatTime >= rangeEnd) continue;
					TJA::ConvertedMeasure* m = tryFindMeasureForBeat(measures, inNote.BeatTime);
					if (!m) continue;
					m->Notes.push_back(TJA::ConvertedNote { inNote.BeatTime - m->StartTime, ConvertTJANoteType(inNote.Type) });
					if (inNote.BeatDuration > Beat::Zero())
					{
						TJA::ConvertedMeasure* mEnd = tryFindMeasureForBeat(measures, inNote.BeatTime + inNote.BeatDuration);
						if (mEnd) mEnd->Notes.push_back(TJA::ConvertedNote { (inNote.BeatTime + inNote.BeatDuration) - mEnd->StartTime, TJA::NoteType::End_BalloonOrDrumroll });
					}
					const Time offset = ApproxmiatelySame(inNote.TimeOffset.Seconds, 0.0) ? Time::Zero() : inNote.TimeOffset;
					if (offset != lastOffset) { m->DelayChanges.push_back(TJA::ConvertedDelayChange { inNote.BeatTime - m->StartTime, offset }); lastOffset = offset; }
				}
			};

			// Helper: check whether a beat falls inside any branch section (inclusive on both ends)
			auto isInsideBranchSection = [&](Beat beat) -> b8 {
				for (const BranchSection& bs : inCourse.BranchSections)
					if (beat >= bs.BeatTime && beat <= bs.GetEnd()) return true;
				return false;
			};

			// Only export Normal scroll changes that are in shared (non-branched) areas
			for (const ScrollChange& inScroll : inCourse.ScrollChanges)
			{
				if (inCourse.HasBranches() && isInsideBranchSection(inScroll.BeatTime)) continue;
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inScroll.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->ScrollChanges.push_back(TJA::ConvertedScrollChange { (inScroll.BeatTime - outConvertedMeasure->StartTime), inScroll.ScrollSpeed });
			}

			for (const ScrollType& inScrollType : inCourse.ScrollTypes)
			{
				if (inCourse.HasBranches() && isInsideBranchSection(inScrollType.BeatTime)) continue;
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inScrollType.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->ScrollTypes.push_back(TJA::ConvertedScrollType { (inScrollType.BeatTime - outConvertedMeasure->StartTime), static_cast<i8>(inScrollType.Method) });
			}

			for (const JPOSScrollChange& JPOSScroll : inCourse.JPOSScrollChanges)
			{
				if (inCourse.HasBranches() && isInsideBranchSection(JPOSScroll.BeatTime)) continue;
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, JPOSScroll.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->JPOSScrollChanges.push_back(TJA::ConvertedJPOSScroll { (JPOSScroll.BeatTime - outConvertedMeasure->StartTime), JPOSScroll.Move, JPOSScroll.Duration });
			}

			for (const BarLineChange& barLineChange : inCourse.BarLineChanges)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, barLineChange.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->BarLineChanges.push_back(TJA::ConvertedBarLineChange { (barLineChange.BeatTime - outConvertedMeasure->StartTime), barLineChange.IsVisible });
			}

			for (const LyricChange& inLyric : inCourse.Lyrics)
			{
				TJA::ConvertedMeasure* outConvertedMeasure = tryFindMeasureForBeat(outConvertedMeasures, inLyric.BeatTime);
				if (assert(outConvertedMeasure != nullptr); outConvertedMeasure != nullptr)
					outConvertedMeasure->LyricChanges.push_back(TJA::ConvertedLyricChange { (inLyric.BeatTime - outConvertedMeasure->StartTime), inLyric.Lyric });
			}

			// For go-go time events, convert each range to a pair of start & end changes
			for (const GoGoRange& gogo : inCourse.GoGoRanges)
			{
				// start
				TJA::ConvertedMeasure* outConvertedMeasureStart = tryFindMeasureForBeat(outConvertedMeasures, gogo.BeatTime);
				if (assert(outConvertedMeasureStart != nullptr); outConvertedMeasureStart != nullptr)
					outConvertedMeasureStart->GoGoChanges.push_back(TJA::ConvertedGoGoChange{ (gogo.BeatTime - outConvertedMeasureStart->StartTime), true });
				// end
				const Beat endTime = gogo.BeatTime + Max(Beat::Zero(), gogo.BeatDuration);
				TJA::ConvertedMeasure* outConvertedMeasureEnd = tryFindMeasureForBeat(outConvertedMeasures, endTime);
				if (assert(outConvertedMeasureEnd != nullptr); outConvertedMeasureEnd != nullptr)
					outConvertedMeasureEnd->GoGoChanges.push_back(TJA::ConvertedGoGoChange{ (endTime - outConvertedMeasureEnd->StartTime), false });
			}

			// SectionMarkers not coinciding with a BranchSection start → mark measure with HasSectionMarker
			// (SectionMarkers at BranchSection starts are handled in the branch export loop below)
			for (const SectionMarker& sm : inCourse.SectionMarkers)
			{
				const b8 atBranchStart = (inCourse.BranchSections.TryFindExactAtBeat(sm.BeatTime) != nullptr);
				if (!atBranchStart)
				{
					TJA::ConvertedMeasure* m = tryFindMeasureForBeat(outConvertedMeasures, sm.BeatTime);
					if (m != nullptr) m->HasSectionMarker = true;
				}
			}

			if (!inCourse.HasBranches())
			{
				// Simple non-branched: place Normal notes and emit all measures at once
				placeNotesInMeasures(inCourse.Notes_Normal, outConvertedMeasures);
				TJA::ConvertConvertedMeasuresToParsedCommands(outConvertedMeasures, outCourse.ChartCommands);
			}
			else
			{
				// Branched: determine which measure indices fall in each branch section
				struct SectionMeasureRange { size_t start, end; };
				std::vector<SectionMeasureRange> sectionRanges(inCourse.BranchSections.size(), { SIZE_MAX, 0 });
				for (size_t si = 0; si < inCourse.BranchSections.size(); si++)
				{
					const Beat bsStart = inCourse.BranchSections[si].BeatTime;
					const Beat bsEnd   = inCourse.BranchSections[si].GetEnd();
					for (size_t mi = 0; mi < outConvertedMeasures.size(); mi++)
					{
						const Beat mt = outConvertedMeasures[mi].StartTime;
						if (mt >= bsStart && mt < bsEnd)
						{
							if (sectionRanges[si].start == SIZE_MAX) sectionRanges[si].start = mi;
							sectionRanges[si].end = mi + 1;
						}
					}
					if (sectionRanges[si].start == SIZE_MAX) sectionRanges[si].start = sectionRanges[si].end = 0;
				}

				// Place Notes_Normal into shared measures; branch-section notes placed per-branch below
				placeNotesInMeasures(inCourse.Notes_Normal, outConvertedMeasures);
				// Clear notes/delays from branch section measures (will be regenerated per-branch)
				for (const SectionMeasureRange& r : sectionRanges)
					for (size_t mi = r.start; mi < r.end; mi++)
						{ outConvertedMeasures[mi].Notes.clear(); outConvertedMeasures[mi].DelayChanges.clear(); }

				// Walk measures and emit commands, inserting branch blocks at section boundaries
				size_t nextSectionIdx = 0;
				size_t sharedSegmentStart = 0;

				auto flushSharedSegment = [&](size_t endExclusive)
				{
					if (sharedSegmentStart >= endExclusive) return;
					std::vector<TJA::ConvertedMeasure> seg(outConvertedMeasures.begin() + sharedSegmentStart, outConvertedMeasures.begin() + endExclusive);
					TJA::ConvertConvertedMeasuresToParsedCommands(seg, outCourse.ChartCommands);
				};

				for (size_t mi = 0; mi <= outConvertedMeasures.size(); )
				{
					// Check if current position is the start of a branch section
					if (nextSectionIdx < inCourse.BranchSections.size() && mi == sectionRanges[nextSectionIdx].start)
					{
						flushSharedSegment(mi);
						sharedSegmentStart = sectionRanges[nextSectionIdx].end;

						const BranchSection& bs = inCourse.BranchSections[nextSectionIdx];
						const size_t sStart = sectionRanges[nextSectionIdx].start;
						const size_t sEnd   = sectionRanges[nextSectionIdx].end;

						// Compute time signature for a partial-bar measure covering `partialTicks` ticks
						auto makePartialTimeSig = [](Beat partialDur) -> TimeSignature {
							const i32 barTicks = Beat::FromBars(1).Ticks; // 20160
							const i32 partTicks = partialDur.Ticks;
							const i32 g = std::gcd(partTicks, barTicks);
							return TimeSignature(partTicks / g, barTicks / g);
						};

						// Detect whether the branch ends mid-bar
						Beat branchLastMeasureFullEnd = bs.GetEnd(); // default: no tail needed
						b8 branchEndsMidBar = false;
						if (sStart < sEnd) {
							const TJA::ConvertedMeasure& lastM = outConvertedMeasures[sEnd - 1];
							const Beat fullBarEnd = lastM.StartTime + abs(lastM.TimeSignature.GetDurationPerBar());
							if (bs.GetEnd() < fullBarEnd) {
								branchLastMeasureFullEnd = fullBarEnd;
								branchEndsMidBar = true;
							}
						}

						// #BRANCHSTART
						{
							auto& cmd = outCourse.ChartCommands.emplace_back();
							cmd.Type = TJA::ParsedChartCommandType::BranchStart;
							cmd.Param.BranchStart.Condition = bs.Condition;
							cmd.Param.BranchStart.RequirementExpert = bs.RequirementExpert;
							cmd.Param.BranchStart.RequirementMaster = bs.RequirementMaster;
						}
						if (inCourse.SectionMarkers.TryFindExactAtBeat(bs.BeatTime) != nullptr)
						{
							auto& cmd = outCourse.ChartCommands.emplace_back();
							cmd.Type = TJA::ParsedChartCommandType::ResetAccuracyValues;
						}

						// Emit each branch (#N / #E / #M + measures)
						for (i32 b = 0; b < 3; b++)
						{
							{
								auto& cmd = outCourse.ChartCommands.emplace_back();
								cmd.Type = (b == 0) ? TJA::ParsedChartCommandType::BranchNormal :
								           (b == 1) ? TJA::ParsedChartCommandType::BranchExpert :
								           TJA::ParsedChartCommandType::BranchMaster;
							}
							if (sStart < sEnd)
							{
								std::vector<TJA::ConvertedMeasure> branchMeasures(outConvertedMeasures.begin() + sStart, outConvertedMeasures.begin() + sEnd);

								// If branch ends mid-bar, truncate the last measure
								if (branchEndsMidBar) {
									auto& last = branchMeasures.back();
									last.TimeSignature = makePartialTimeSig(bs.GetEnd() - last.StartTime);
								}

								// Replace shared scroll/scrolltype/jposscroll with branch-specific data
								for (auto& m : branchMeasures) { m.ScrollChanges.clear(); m.ScrollTypes.clear(); m.JPOSScrollChanges.clear(); }
								const BranchType branchType = static_cast<BranchType>(b);
								for (const ScrollChange& sc : inCourse.GetScrollChanges(branchType))
								{
									if (sc.BeatTime < bs.BeatTime || sc.BeatTime > bs.GetEnd()) continue;
									TJA::ConvertedMeasure* m = tryFindMeasureForBeat(branchMeasures, sc.BeatTime);
									if (m) m->ScrollChanges.push_back({ sc.BeatTime - m->StartTime, sc.ScrollSpeed });
								}
								for (const ScrollType& st : inCourse.GetScrollTypes(branchType))
								{
									if (st.BeatTime < bs.BeatTime || st.BeatTime > bs.GetEnd()) continue;
									TJA::ConvertedMeasure* m = tryFindMeasureForBeat(branchMeasures, st.BeatTime);
									if (m) m->ScrollTypes.push_back({ st.BeatTime - m->StartTime, static_cast<i8>(st.Method) });
								}
								for (const JPOSScrollChange& jps : inCourse.GetJPOSScrollChanges(branchType))
								{
									if (jps.BeatTime < bs.BeatTime || jps.BeatTime > bs.GetEnd()) continue;
									TJA::ConvertedMeasure* m = tryFindMeasureForBeat(branchMeasures, jps.BeatTime);
									if (m) m->JPOSScrollChanges.push_back({ jps.BeatTime - m->StartTime, jps.Move, jps.Duration });
								}

								placeNotesInMeasures(inCourse.GetNotes(branchType), branchMeasures, bs.BeatTime, bs.GetEnd());
								TJA::ConvertConvertedMeasuresToParsedCommands(branchMeasures, outCourse.ChartCommands);
							}
						}

						// #BRANCHEND
						{ auto& cmd = outCourse.ChartCommands.emplace_back(); cmd.Type = TJA::ParsedChartCommandType::BranchEnd; }

						// If branch ended mid-bar, output the tail (shared) partial measure covering [bs.GetEnd(), branchLastMeasureFullEnd)
						if (branchEndsMidBar && sStart < sEnd) {
							const TJA::ConvertedMeasure& fullLastM = outConvertedMeasures[sEnd - 1];
							const Beat tailStart_rel = bs.GetEnd() - fullLastM.StartTime;

							TJA::ConvertedMeasure tailMeasure;
							tailMeasure.StartTime = bs.GetEnd();
							tailMeasure.TimeSignature = makePartialTimeSig(branchLastMeasureFullEnd - bs.GetEnd());

							auto copyTailEvents = [&tailStart_rel](auto& dstVec, const auto& srcVec) {
								for (const auto& e : srcVec)
									if (e.TimeWithinMeasure >= tailStart_rel) {
										auto copy = e; copy.TimeWithinMeasure = copy.TimeWithinMeasure - tailStart_rel; dstVec.push_back(copy);
									}
							};
							copyTailEvents(tailMeasure.GoGoChanges,     fullLastM.GoGoChanges);
							copyTailEvents(tailMeasure.BarLineChanges,   fullLastM.BarLineChanges);
							copyTailEvents(tailMeasure.TempoChanges,     fullLastM.TempoChanges);
							copyTailEvents(tailMeasure.LyricChanges,     fullLastM.LyricChanges);
							copyTailEvents(tailMeasure.ScrollChanges,    fullLastM.ScrollChanges);
							copyTailEvents(tailMeasure.ScrollTypes,      fullLastM.ScrollTypes);
							copyTailEvents(tailMeasure.JPOSScrollChanges,fullLastM.JPOSScrollChanges);

							std::vector<TJA::ConvertedMeasure> tailVec = { tailMeasure };
							placeNotesInMeasures(inCourse.Notes_Normal, tailVec, bs.GetEnd(), branchLastMeasureFullEnd);
							TJA::ConvertConvertedMeasuresToParsedCommands(tailVec, outCourse.ChartCommands);
						}

						mi = sEnd;
						nextSectionIdx++;
					}
					else
					{
						mi++;
					}
				}
				// Flush any remaining shared measures after the last branch section
				flushSharedSegment(outConvertedMeasures.size());
			}
		}

		return true;
	}
}
