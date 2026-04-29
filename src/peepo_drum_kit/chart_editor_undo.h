#pragma once
#include "core_types.h"
#include "core_undo.h"
#include "chart.h"

namespace PeepoDrumKit
{
	// NOTE: General chart commands
	namespace Commands
	{
		// Generic definitions

		constexpr std::string_view ActionPrefixChange = "Change ";

		template <auto ChartProject::* Attr>
		struct ChangeSingleChartAttributeBase : Undo::Command
		{
			using TAttr = remove_member_pointer_t<decltype(Attr)>;
			ChangeSingleChartAttributeBase(ChartProject* chart, TAttr value) : Chart(chart), NewValue(value), OldValue(chart->*Attr) { }

			void Undo() override { Chart->*Attr = OldValue; }
			void Redo() override { Chart->*Attr = NewValue; }

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Chart != Chart)
					return Undo::MergeResult::Failed;

				NewValue = other->NewValue;
				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixChange, DisplayNameOfChartProjectAttr<Attr>> }; }

			ChartProject* Chart;
			Time NewValue, OldValue;
		};
		template <auto ChartProject::* Attr>
		struct ChangeSingleChartAttribute : ChangeSingleChartAttributeBase<Attr> { using ChangeSingleChartAttributeBase<Attr>::ChangeSingleChartAttributeBase; };

		// Specializations and aliases

		using ChangeSongOffset = ChangeSingleChartAttribute<&ChartProject::SongOffset>;
		using ChangeSongDemoStartTime = ChangeSingleChartAttribute<&ChartProject::SongDemoStartTime>;
		using ChangeChartDuration = ChangeSingleChartAttribute<&ChartProject::ChartDuration>;
	}

	// NOTE: Generic chart event commands
	namespace Commands
	{
		// ChartCourse member accessing helpers

		template <typename TEvent> struct ChartCourseListTypeHelper { using type = BeatSortedList<TEvent>; };
		template <> struct ChartCourseListTypeHelper<TempoChange> { using type = SortedTempoMap; };
		template <> struct ChartCourseListTypeHelper<TimeSignatureChange> { using type = SortedTempoMap; };

		template <typename TEvent> using ChartCourseListType = typename ChartCourseListTypeHelper<TEvent>::type;

		template <typename TEvent> struct TempoMapMemberPointerHelper { constexpr static auto value = nullptr; };
		template <> struct TempoMapMemberPointerHelper<TempoChange> { constexpr static auto value = &SortedTempoMap::Tempo; };
		template <> struct TempoMapMemberPointerHelper<TimeSignatureChange> { constexpr static auto value = &SortedTempoMap::Signature; };

		template <typename TEvent>
		constexpr BeatSortedList<TEvent> SortedTempoMap::* TempoMapMemberPointer = TempoMapMemberPointerHelper<TEvent>::value;

		template <auto SortedTempoMap::* EventList, typename TMap>
		constexpr __forceinline decltype(auto) GetEventList(TMap&& MapOrList)
		{
			if constexpr (EventList != nullptr)
				return (std::forward<TMap>(MapOrList).*EventList); // member access; ()-enclosed for returning reference
			else
				return std::forward<TMap>(MapOrList);
		}

		// Generic commands

		constexpr std::string_view ActionPrefixAdd = "Add ";
		constexpr std::string_view ActionPrefixRemove = "Remove ";
		constexpr std::string_view ActionPrefixUpdate = "Update ";
		constexpr std::string_view ActionPrefixUpdateAll = "Update All ";

		template <typename TEvent>
		static void RefreshChart(ChartCourse* Course, ChartCourseListType<TEvent>* Map)
		{
			if constexpr (TempoMapMemberPointer<TEvent> != nullptr) { Map->RebuildAccelerationStructure(); Course->RecalculateSENotes(); }
			else if constexpr (expect_type_v<TEvent, Note>) { Course->RecalculateSENotes(); }
		}

		template <typename TEvent>
		struct AddSingleChartEventBase : Undo::Command
		{;
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			AddSingleChartEventBase(ChartCourse* course, ChartCourseListType* map, TEvent newValue) : Course(course), Map(map), NewValue(newValue) {}

			void Undo() override
			{
				if (ReplacedValue.has_value())
					GetEventList<EventList>(*Map).InsertOrUpdate(ReplacedValue.value());
				else
					GetEventList<EventList>(*Map).RemoveAtBeat(GetBeat(NewValue));
				RefreshChart<TEvent>(Course, Map);
			}
			void Redo() override
			{
				GetEventList<EventList>(*Map).InsertOrFunc(NewValue, [&](TEvent& v, ...) { ReplacedValue = std::move(v); v = NewValue; }); // safe replace
				RefreshChart<TEvent>(Course, Map);
			}

			Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixAdd, DisplayNameOfChartEvent<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			TEvent NewValue;
			std::optional<TEvent> ReplacedValue;
		};
		template <typename TEvent>
		struct AddSingleChartEvent : AddSingleChartEventBase<TEvent> { using AddSingleChartEventBase<TEvent>::AddSingleChartEventBase; };

		template <typename TEvent>
		struct AddMultipleChartEventsBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			AddMultipleChartEventsBase(ChartCourse* course, ChartCourseListType* map, std::vector<TEvent> newValues) : Course(course), Map(map)
			{
				for (const auto& event : newValues)
					NewEvents.InsertOrUpdate(event); // merge new events
			}

			void Undo() override
			{
				for (const auto& event : NewEvents)
					GetEventList<EventList>(*Map).RemoveAtBeat(GetBeat(event));
				for (const auto& event : ReplacedEvents)
					GetEventList<EventList>(*Map).InsertOrUpdate(event);
				RefreshChart<TEvent>(Course, Map);
			}
			void Redo() override
			{
				ReplacedEvents.clear();
				for (const auto& event : NewEvents)
					GetEventList<EventList>(*Map).InsertOrFunc(event, [&](TEvent& v, ...) { ReplacedEvents.push_back(std::move(v)); v = event; }); // safe replace
				RefreshChart<TEvent>(Course, Map);
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixAdd, DisplayNameOfChartEvents<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			BeatSortedList<TEvent> NewEvents;
			std::vector<TEvent> ReplacedEvents;
		};
		template <typename TEvent>
		struct AddMultipleChartEvents : AddMultipleChartEventsBase<TEvent> { using AddMultipleChartEventsBase<TEvent>::AddMultipleChartEventsBase; };

		template <typename TEvent>
		struct RemoveSingleChartEventBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			RemoveSingleChartEventBase(ChartCourse* course, ChartCourseListType* map, TEvent oldValue) : Course(course), Map(map), OldValue(oldValue) { }
			RemoveSingleChartEventBase(ChartCourse* course, ChartCourseListType* map, Beat beat) : Course(course), Map(map), OldValue(*GetEventList<EventList>(*Map).TryFindExactAtBeat(beat)) { assert(GetBeat(OldValue) == beat); }

			void Undo() override { GetEventList<EventList>(*Map).InsertOrUpdate(OldValue); RefreshChart<TEvent>(Course, Map); }
			void Redo() override { GetEventList<EventList>(*Map).RemoveAtBeat(GetBeat(OldValue)); RefreshChart<TEvent>(Course, Map); }

			Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixRemove, DisplayNameOfChartEvent<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			TEvent OldValue;
		};
		template <typename TEvent>
		struct RemoveSingleChartEvent : RemoveSingleChartEventBase<TEvent> { using RemoveSingleChartEventBase<TEvent>::RemoveSingleChartEventBase; };

		template <typename TEvent>
		struct RemoveMultipleChartEventsBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			RemoveMultipleChartEventsBase(ChartCourse* course, ChartCourseListType* map, std::vector<TEvent> oldValues) : Course(course), Map(map), OldValues(std::move(oldValues)) { }

			void Undo() override
			{
				for (const auto& event : OldValues)
					GetEventList<EventList>(*Map).InsertOrUpdate(event);
				RefreshChart<TEvent>(Course, Map);
			}
			void Redo() override
			{
				for (const TEvent& event : OldValues) GetEventList<EventList>(*Map).RemoveAtBeat(GetBeat(event));
				RefreshChart<TEvent>(Course, Map);
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixRemove, DisplayNameOfChartEvents<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			std::vector<TEvent> OldValues;
		};
		template <typename TEvent>
		struct RemoveMultipleChartEvents : RemoveMultipleChartEventsBase<TEvent> { using RemoveMultipleChartEventsBase<TEvent>::RemoveMultipleChartEventsBase; };

		template <typename TEvent>
		struct AddSingleLongEventBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			AddSingleLongEventBase(ChartCourse* course, ChartCourseListType* map, TEvent newValue, std::vector<TEvent> eventsToRemove) : Course(course), Map(map), NewValue(newValue), EventsToRemove(course, map, std::move(eventsToRemove)) { }

			void Undo() override
			{
				GetEventList<EventList>(*Map).RemoveAtBeat(GetBeat(NewValue));
				EventsToRemove.Undo();
				RefreshChart<TEvent>(Course, Map);
			}
			void Redo() override
			{
				EventsToRemove.Redo();
				GetEventList<EventList>(*Map).InsertOrFunc(NewValue, [&](TEvent& v, ...) { EventsToRemove.OldValues.push_back(std::move(v)); v = NewValue; }); // safe replace
				RefreshChart<TEvent>(Course, Map);
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixAdd, DisplayNameOfLongChartEvent<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			TEvent NewValue;
			RemoveMultipleChartEvents<TEvent> EventsToRemove;
		};
		template <typename TEvent>
		struct AddSingleLongEvent : AddSingleLongEventBase<TEvent> { using AddSingleLongEventBase<TEvent>::AddSingleLongEventBase; };

		template <typename TEvent>
		struct UpdateSingleChartEventBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			UpdateSingleChartEventBase(ChartCourse* course, ChartCourseListType* map, TEvent newValue) : Course(course), Map(map), NewValue(newValue), OldValue(*GetEventList<EventList>(*Map).TryFindExactAtBeat(GetBeat(newValue))) { assert(GetBeat(newValue) == GetBeat(OldValue)); }

			void Undo() override { GetEventList<EventList>(*Map).InsertOrUpdate(OldValue); RefreshChart<TEvent>(Course, Map); }
			void Redo() override { GetEventList<EventList>(*Map).InsertOrUpdate(NewValue); RefreshChart<TEvent>(Course, Map); }

			Undo::MergeResult TryMerge(Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Map != Map || GetBeat(other->NewValue) != GetBeat(NewValue))
					return Undo::MergeResult::Failed;

				NewValue = other->NewValue;
				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixUpdate, DisplayNameOfChartEvent<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			TEvent NewValue, OldValue;
		};
		template <typename TEvent>
		struct UpdateSingleChartEvent : UpdateSingleChartEventBase<TEvent> { using UpdateSingleChartEventBase<TEvent>::UpdateSingleChartEventBase; };

		template <typename TEvent>
		struct ReplaceAllChartEventsBase : Undo::Command
		{
			using ChartCourseListType = ChartCourseListType<TEvent>;
			using SortedEventsList = BeatSortedList<TEvent>;
			constexpr static auto EventList = TempoMapMemberPointer<TEvent>;
			ReplaceAllChartEventsBase(ChartCourse* course, ChartCourseListType* map, SortedEventsList newValues) : Course(course), Map(map), NewValues(std::move(newValues)), OldValues(GetEventList<EventList>(*map)) { }

			void Undo() override { GetEventList<EventList>(*Map) = OldValues; RefreshChart<TEvent>(Course, Map); }
			void Redo() override { GetEventList<EventList>(*Map) = NewValues; RefreshChart<TEvent>(Course, Map); }

			Undo::MergeResult TryMerge(Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Map != Map)
					return Undo::MergeResult::Failed;

				NewValues = other->NewValues;
				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixUpdateAll, DisplayNameOfChartEvents<TEvent>> }; }

			ChartCourse* Course;
			ChartCourseListType* Map;
			SortedEventsList NewValues, OldValues;
		};
		template <typename TEvent>
		struct ReplaceAllChartEvents : ReplaceAllChartEventsBase<TEvent> { using ReplaceAllChartEventsBase<TEvent>::ReplaceAllChartEventsBase; };

		// Implemented as replace all
		template <typename TEvent>
		struct AddMultipleLongChartEvents : ReplaceAllChartEvents<TEvent> {
			using ReplaceAllChartEvents<TEvent>::ReplaceAllChartEvents;
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixAdd, DisplayNameOfChartEvents<TEvent>> }; }
		};
	}

	// NOTE: Chart change commands
	namespace Commands
	{
		// Specializations and aliases
		using AddTempoChange = AddSingleChartEvent<TempoChange>;
		using RemoveTempoChange = RemoveSingleChartEvent<TempoChange>;
		using UpdateTempoChange = UpdateSingleChartEvent<TempoChange>;
		using AddTimeSignatureChange = AddSingleChartEvent<TimeSignatureChange>;

		using RemoveTimeSignatureChange = RemoveSingleChartEvent<TimeSignatureChange>;
		using UpdateTimeSignatureChange = UpdateSingleChartEvent<TimeSignatureChange>;

		using AddJPOSScroll = AddSingleChartEvent<JPOSScrollChange>;
		using RemoveJPOSScroll = RemoveSingleChartEvent<JPOSScrollChange>;
		using UpdateJPOSScroll = UpdateSingleChartEvent<JPOSScrollChange>;

		using AddDelayChange = AddSingleChartEvent<DelayChange>;
		using RemoveDelayChange = RemoveSingleChartEvent<DelayChange>;
		using UpdateDelayChange = UpdateSingleChartEvent<DelayChange>;

		using AddScrollType = AddSingleChartEvent<ScrollType>;
		using RemoveScrollType = RemoveSingleChartEvent<ScrollType>;
		using UpdateScrollType = UpdateSingleChartEvent<ScrollType>;

		using AddScrollChange = AddSingleChartEvent<ScrollChange>;
		using AddMultipleScrollChanges = AddMultipleChartEvents<ScrollChange>;
		using RemoveScrollChange = RemoveSingleChartEvent<ScrollChange>;
		using UpdateScrollChange = UpdateSingleChartEvent<ScrollChange>;

		using AddBarLineChange = AddSingleChartEvent<BarLineChange>;
		using RemoveBarLineChange = RemoveSingleChartEvent<BarLineChange>;

		template <>
		struct UpdateSingleChartEvent<BarLineChange> : UpdateSingleChartEventBase<BarLineChange>
		{
			using UpdateSingleChartEventBase::UpdateSingleChartEventBase;

			Undo::MergeResult TryMerge(Command& commandToMerge) override
			{
#if 1 // NOTE: Merging here doesn't really make much sense..?
				return Undo::MergeResult::Failed;
#else
				return UpdateSingleChartEvent::TryMerge(commandToMerge);
#endif
			}
		};
		using UpdateBarLineChange = UpdateSingleChartEvent<BarLineChange>;

		// NOTE: Creating a full copy of the new/old state for now because it's easy and there typically are only a very few number of GoGoRanges
		template <>
		struct ReplaceAllChartEvents<GoGoRange> : ReplaceAllChartEventsBase<GoGoRange>
		{
			using ReplaceAllChartEventsBase::ReplaceAllChartEventsBase;
			Undo::MergeResult TryMerge(Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixAdd, DisplayNameOfChartEvent<GoGoRange>> }; }
		};
		using AddGoGoRange = ReplaceAllChartEvents<GoGoRange>;

		using RemoveGoGoRange = RemoveSingleChartEvent<GoGoRange>;

		using AddLyricChange = AddSingleChartEvent<LyricChange>;
		using RemoveLyricChange = RemoveSingleChartEvent<LyricChange>;
		using UpdateLyricChange = UpdateSingleChartEvent<LyricChange>;
		using ReplaceAllLyricChanges = ReplaceAllChartEvents<LyricChange>;
	}

	// NOTE: Note commands
	namespace Commands
	{
		// Specializations and aliases for Note events

		constexpr std::string_view ActionPrefixPaste = "Paste ";
		constexpr std::string_view ActionPrefixCut = "Cut ";

		using AddSingleNote = AddSingleChartEvent<Note>;
		using AddMultipleNotes = AddMultipleChartEvents<Note>;
		using RemoveSingleNote = RemoveSingleChartEvent<Note>;
		using RemoveMultipleNotes = RemoveMultipleChartEvents<Note>;
		using AddSingleLongNote = AddSingleLongEvent<Note>;

		struct AddMultipleNotes_Paste : AddMultipleNotes
		{
			using AddMultipleNotes::AddMultipleNotes;
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixPaste, DisplayNameOfChartEvents<Note>> }; }
		};

		struct RemoveMultipleNotes_Cut : RemoveMultipleNotes
		{
			using RemoveMultipleNotes::RemoveMultipleNotes;
			Undo::CommandInfo GetInfo() const override { return { ConstevalStrJoined<ActionPrefixCut, DisplayNameOfChartEvents<Note>> }; }
		};

		// Generic definitions for Note attributes

		template <typename TAttr>
		struct NoteAttributeData { size_t Index; TAttr NewValue, OldValue; };

		template <auto Note::* Attr>
		struct ChangeSingleNoteAttributeBase : Undo::Command
		{
			using TAttr = remove_member_pointer_t<decltype(Attr)>;
			using Data = NoteAttributeData<TAttr>;

			ChangeSingleNoteAttributeBase(ChartCourse* course, SortedNotesList* notes, Data newData) : Course(course), Notes(notes), NewData(std::move(newData)) { NewData.OldValue = (*Notes)[NewData.Index].*Attr; }

			void Undo() override { (*Notes)[NewData.Index].*Attr = NewData.OldValue; RefreshChart<Note>(Course, nullptr); }
			void Redo() override { (*Notes)[NewData.Index].*Attr = NewData.NewValue; RefreshChart<Note>(Course, nullptr); }

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Notes != Notes)
					return Undo::MergeResult::Failed;

				if (NewData.Index != other->NewData.Index)
					return Undo::MergeResult::Failed;

				NewData.NewValue = other->NewData.NewValue;

				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { "Change Note Attribute" }; }

			ChartCourse* Course;
			SortedNotesList* Notes;
			Data NewData;
		};
		template <auto Note::* Attr>
		struct ChangeSingleNoteAttribute : ChangeSingleNoteAttributeBase<Attr> {};

		template <auto Note::* Attr>
		struct ChangeMultipleNoteAttributesBase : Undo::Command
		{
			using TAttr = remove_member_pointer_t<decltype(Attr)>;
			using Data = NoteAttributeData<TAttr>;

			ChangeMultipleNoteAttributesBase(ChartCourse* course, SortedNotesList* notes, std::vector<Data> newData)
				: Course(course), Notes(notes), NewData(std::move(newData))
			{
				for (auto& newData : NewData)
					newData.OldValue = (*Notes)[newData.Index].*Attr;
			}

			void Undo() override
			{
				for (const auto& newData : NewData)
					(*Notes)[newData.Index].*Attr = newData.OldValue;
				RefreshChart<Note>(Course, nullptr);
			}

			void Redo() override
			{
				for (const auto& newData : NewData)
					(*Notes)[newData.Index].*Attr = newData.NewValue;
				RefreshChart<Note>(Course, nullptr);
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Notes != Notes)
					return Undo::MergeResult::Failed;

				if (other->NewData.size() != NewData.size())
					return Undo::MergeResult::Failed;

				for (size_t i = 0; i < NewData.size(); i++)
				{
					if (NewData[i].Index != other->NewData[i].Index)
						return Undo::MergeResult::Failed;
				}

				for (size_t i = 0; i < NewData.size(); i++)
					NewData[i].NewValue = other->NewData[i].NewValue;

				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { "Change Note Attributes" }; }

			ChartCourse* Course;
			SortedNotesList* Notes;
			std::vector<Data> NewData;
		};
		template <auto Note::* Attr>
		struct ChangeMultipleNoteAttributes : ChangeMultipleNoteAttributesBase<Attr> {};

		// Specializations and aliases for Note attributes

		template <>
		struct ChangeSingleNoteAttribute<&Note::Type> : ChangeSingleNoteAttributeBase<&Note::Type>
		{
			using ChangeSingleNoteAttributeBase::ChangeSingleNoteAttributeBase;
			Undo::CommandInfo GetInfo() const override { return { "Change Note Type" }; }
		};
		using ChangeSingleNoteType = ChangeSingleNoteAttribute<&Note::Type>;

		template <>
		struct ChangeMultipleNoteAttributes<&Note::Type> : ChangeMultipleNoteAttributesBase<&Note::Type>
		{
			using ChangeMultipleNoteAttributesBase::ChangeMultipleNoteAttributesBase;
			Undo::CommandInfo GetInfo() const override { return { "Change Note Types" }; }
		};
		using ChangeMultipleNoteTypes = ChangeMultipleNoteAttributes<&Note::Type>;

		struct ChangeMultipleNoteTypes_FlipTypes : ChangeMultipleNoteTypes
		{
			using ChangeMultipleNoteTypes::ChangeMultipleNoteTypes;
			Undo::CommandInfo GetInfo() const override { return { "Flip Notes" }; }
		};

		struct ChangeMultipleNoteTypes_ToggleSizes : ChangeMultipleNoteTypes
		{
			using ChangeMultipleNoteTypes::ChangeMultipleNoteTypes;
			Undo::CommandInfo GetInfo() const override { return { "Toggle Note Sizes" }; }
		};

		template <>
		struct ChangeMultipleNoteAttributes<&Note::BeatTime> : ChangeMultipleNoteAttributesBase<&Note::BeatTime>
		{
			using ChangeMultipleNoteAttributesBase::ChangeMultipleNoteAttributesBase;
			Undo::CommandInfo GetInfo() const override { return { "Change Note Beats" }; }
		};
		using ChangeMultipleNoteBeats = ChangeMultipleNoteAttributes<&Note::BeatTime>;

		struct ChangeMultipleNoteBeats_MoveNotes : ChangeMultipleNoteBeats
		{
			using ChangeMultipleNoteBeats::ChangeMultipleNoteBeats;
			Undo::CommandInfo GetInfo() const override { return { "Move Notes" }; }
		};

		template <>
		struct ChangeMultipleNoteAttributes<&Note::BeatDuration> : ChangeMultipleNoteAttributesBase<&Note::BeatDuration>
		{
			using ChangeMultipleNoteAttributesBase::ChangeMultipleNoteAttributesBase;
			Undo::CommandInfo GetInfo() const override { return { "Change Note Beat Durations" }; }
		};
		using ChangeMultipleNoteBeatDurations = ChangeMultipleNoteAttributes<&Note::BeatDuration>;

		struct ChangeMultipleNoteBeatDurations_AdjustRollNoteDurations : ChangeMultipleNoteBeatDurations
		{
			using ChangeMultipleNoteBeatDurations::ChangeMultipleNoteBeatDurations;
			Undo::CommandInfo GetInfo() const override { return { "Adjust Roll Note Durations" }; }
		};
	}

	// NOTE: Generic chart commands
	namespace Commands
	{
		struct AddMultipleGenericItems : Undo::Command
		{
			AddMultipleGenericItems(ChartCourse* course, std::vector<GenericListStructWithType> newData) : Course(course), UpdateTempoMap(false)
			{
				for (const auto& data : newData) {
					NewData[static_cast<size_t>(data.List)].InsertOrUpdate(data); // merge new data
					if (data.List == GenericList::TempoChanges)
						UpdateTempoMap = true;
					else if (IsNotesList(data.List))
						UpdateNotes = true;
				}
			}

			void Undo() override
			{
				for (const auto& list : NewData) {
					for (const auto& data : list)
						TryRemoveGenericStruct(*Course, data.List, data.Value);
				}
				for (const auto& data : ReplacedData)
					TryAddOrReplaceGenericStruct(*Course, data.List, data.Value);
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			void Redo() override
			{
				ReplacedData.clear();
				for (const auto& list : NewData) {
					for (const auto& data : list)
						TryAddOrFuncGenericStruct(*Course, data.List, data.Value, [&](auto& v, auto&& vNew) { ReplacedData.emplace_back(data.List, std::move(v)); v = vNew; }); // safe replace
				}
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { "Add Items" }; }

			ChartCourse* Course;
			BeatSortedList<GenericListStructWithType> NewData[EnumCount<GenericList>];
			std::vector<GenericListStructWithType> ReplacedData;
			b8 UpdateTempoMap, UpdateNotes;
		};

		struct RemoveMultipleGenericItems : Undo::Command
		{
			RemoveMultipleGenericItems(ChartCourse* course, std::vector<GenericListStructWithType> oldData) : Course(course), OldData(std::move(oldData)), UpdateTempoMap(false)
			{
				for (const auto& data : OldData)
				{
					if (data.List == GenericList::TempoChanges)
						UpdateTempoMap = true;
					else if (IsNotesList(data.List))
						UpdateNotes = true;
				}
			}

			void Undo() override
			{
				for (const auto& data : OldData)
					TryAddOrReplaceGenericStruct(*Course, data.List, data.Value);
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			void Redo() override
			{
				for (const auto& data : OldData)
					TryRemoveGenericStruct(*Course, data.List, data.Value);
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { "Remove Items" }; }

			ChartCourse* Course;
			std::vector<GenericListStructWithType> OldData;
			b8 UpdateTempoMap, UpdateNotes;
		};

		struct AddMultipleGenericItems_Paste : AddMultipleGenericItems
		{
			using AddMultipleGenericItems::AddMultipleGenericItems;
			Undo::CommandInfo GetInfo() const override { return { "Paste Items" }; }
		};

		struct RemoveMultipleGenericItems_Cut : RemoveMultipleGenericItems
		{
			using RemoveMultipleGenericItems::RemoveMultipleGenericItems;
			Undo::CommandInfo GetInfo() const override { return { "Cut Items" }; }
		};

		struct ChangeMultipleGenericProperties : Undo::Command
		{
			// TODO: Separate string vector for cstr ownership stuff
			struct Data
			{
				size_t Index;
				GenericList List;
				GenericMember Member;
				GenericMemberUnion NewValue, OldValue;

				Data() {

				}
				Data(const Data& other) {
					Index = other.Index;
					List = other.List;
					Member = other.Member;
					NewValue = other.NewValue;
					OldValue = other.OldValue;
				}
			};

			ChangeMultipleGenericProperties(ChartCourse* course, std::vector<Data> newData)
				: Course(course), NewData(std::move(newData)), UpdateTempoMap(false)
			{
				for (auto& data : NewData)
				{
					const b8 success = TryGet(*Course, data.List, data.Index, data.Member, data.OldValue);
					assert(success);
					if (data.List == GenericList::TempoChanges)
						UpdateTempoMap = true;
					else if (IsNotesList(data.List))
						UpdateNotes = true;
				}
			}

			void Undo() override
			{
				for (const auto& newData : NewData)
					TrySet(*Course, newData.List, newData.Index, newData.Member, newData.OldValue);
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			void Redo() override
			{
				for (const auto& newData : NewData)
					TrySet(*Course, newData.List, newData.Index, newData.Member, newData.NewValue);
				if (UpdateTempoMap)
					Course->TempoMap.RebuildAccelerationStructure();
				if (UpdateTempoMap || UpdateNotes)
					Course->RecalculateSENotes();
			}

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override
			{
				auto* other = static_cast<decltype(this)>(&commandToMerge);
				if (other->Course != Course)
					return Undo::MergeResult::Failed;

				if (other->NewData.size() != NewData.size())
					return Undo::MergeResult::Failed;

				for (size_t i = 0; i < NewData.size(); i++)
				{
					const auto& data = NewData[i];
					const auto& otherData = other->NewData[i];
					if (data.Index != otherData.Index || data.List != otherData.List || data.Member != otherData.Member)
						return Undo::MergeResult::Failed;
				}

				for (size_t i = 0; i < NewData.size(); i++)
					NewData[i].NewValue = other->NewData[i].NewValue;

				return Undo::MergeResult::ValueUpdated;
			}

			Undo::CommandInfo GetInfo() const override { return { "Change Properties" }; }

			ChartCourse* Course;
			std::vector<Data> NewData;
			b8 UpdateTempoMap, UpdateNotes;
		};

		struct ChangeMultipleGenericProperties_MoveItems : ChangeMultipleGenericProperties
		{
			using ChangeMultipleGenericProperties::ChangeMultipleGenericProperties;
			Undo::CommandInfo GetInfo() const override { return { "Move Items" }; }
		};

		struct ChangeMultipleGenericProperties_AdjustItemDurations : ChangeMultipleGenericProperties
		{
			using ChangeMultipleGenericProperties::ChangeMultipleGenericProperties;
			Undo::CommandInfo GetInfo() const override { return { "Adjust Item Durations" }; }
		};

		struct RemoveThenAddMultipleGenericItems : Undo::Command
		{
			RemoveThenAddMultipleGenericItems(ChartCourse* course, std::vector<GenericListStructWithType> removeData, std::vector<GenericListStructWithType> addData)
				: RemoveCommand(course, std::move(removeData)), AddCommand(course, std::move(addData))
			{
			}

			void Undo() override { AddCommand.Undo(); RemoveCommand.Undo(); }
			void Redo() override { RemoveCommand.Redo(); AddCommand.Redo(); }

			Undo::MergeResult TryMerge(Undo::Command& commandToMerge) override { return Undo::MergeResult::Failed; }
			Undo::CommandInfo GetInfo() const override { return { "Remove and Add Items" }; }

			RemoveMultipleGenericItems RemoveCommand;
			AddMultipleGenericItems AddCommand;
		};

		struct RemoveThenAddMultipleGenericItems_ExpandItems : RemoveThenAddMultipleGenericItems
		{
			using RemoveThenAddMultipleGenericItems::RemoveThenAddMultipleGenericItems;
			Undo::CommandInfo GetInfo() const override { return { "Expand Items" }; }
		};

		struct RemoveThenAddMultipleGenericItems_CompressItems : RemoveThenAddMultipleGenericItems
		{
			using RemoveThenAddMultipleGenericItems::RemoveThenAddMultipleGenericItems;
			Undo::CommandInfo GetInfo() const override { return { "Compress Items" }; }
		};

		struct RemoveThenAddMultipleGenericItems_ReverseItems : RemoveThenAddMultipleGenericItems
		{
			using RemoveThenAddMultipleGenericItems::RemoveThenAddMultipleGenericItems;
			Undo::CommandInfo GetInfo() const override { return { "Reverse Items" }; }
		};

		template <typename TCommand>
		struct ChangeRangeSelection : TCommand
		{
			template <typename... TArgs>
			ChangeRangeSelection(std::pair<Beat*, Beat*> selectedRange, std::pair<Beat, Beat> rangeDataNew, TArgs&&... args)
				: SelectedRange(selectedRange), RangeDataOld(*selectedRange.first, *selectedRange.second), RangeDataNew(rangeDataNew), TCommand(args...)
			{
			}

			void Undo() override { TCommand::Undo(); *SelectedRange.first = RangeDataOld.first; *SelectedRange.second = RangeDataOld.second; }
			void Redo() override { TCommand::Redo(); *SelectedRange.first = RangeDataNew.first; *SelectedRange.second = RangeDataNew.second; }

			std::pair<Beat*, Beat*> SelectedRange;
			std::pair<Beat, Beat> RangeDataOld, RangeDataNew;
		};

		struct RemoveThenAddMultipleGenericItems_ExpandRange : ChangeRangeSelection<RemoveThenAddMultipleGenericItems>
		{
			using ChangeRangeSelection::ChangeRangeSelection;
			Undo::CommandInfo GetInfo() const override { return { "Expand Range" }; }
		};

		struct RemoveThenAddMultipleGenericItems_CompressRange : ChangeRangeSelection<RemoveThenAddMultipleGenericItems>
		{
			using ChangeRangeSelection::ChangeRangeSelection;
			Undo::CommandInfo GetInfo() const override { return { "Compress Range" }; }
		};

		struct RemoveThenAddMultipleGenericItems_ReverseRange : ChangeRangeSelection<RemoveThenAddMultipleGenericItems>
		{
			using ChangeRangeSelection::ChangeRangeSelection;
			Undo::CommandInfo GetInfo() const override { return { "Reverse Range" }; }
		};
	}
}
