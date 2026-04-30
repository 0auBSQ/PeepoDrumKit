#pragma once
#include "core_string.h"
#include "core_types.h"
#include "imgui/imgui_include.h"

#include <shared_mutex>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "chart_editor_settings.h"

/* should keep unchanged for compatibility */
#define PEEPODRUMKIT_UI_STRINGS_STABLE_X_MACRO_LIST_EN	\
X("TAB_TIMELINE_DEBUG",			"Chart Timeline - Debug") \
X("TAB_TIMELINE",				"Chart Timeline") \
X("TAB_EVENTS",					"Chart Tempo") \
X("TAB_LYRICS",					"Chart Lyrics") \
X("TAB_TEMPO_CALCULATOR",		"Tempo Calculator") \
X("TAB_TJA_EXPORT_DEBUG_VIEW",	"TJA Export Debug View") \
X("TAB_SETTINGS",				"Settings") \
X("TAB_USAGE_GUIDE",			"Usage Guide") \
X("TAB_UPDATE_NOTES",			"Update Notes") \
X("TAB_GAME_PREVIEW",			"Game Preview") \
X("TAB_AUDIO_TEST",				"Audio Test") \
X("TAB_TJA_IMPORT_TEST",		"TJA Import Test") \
X("TAB_UNDO_HISTORY",			"Undo History") \
X("TAB_CHART_PROPERTIES",		"Chart Properties") \
X("TAB_INSPECTOR",				"Chart Inspector") \
/* empty last line */

#define PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST_EN	\
/* tab names */ \
X("TAB_GAME_PREVIEW",								"Game Preview") \
X("TAB_TIMELINE",									"Chart Timeline") \
X("TAB_TIMELINE_DEBUG",								"Chart Timeline - Debug") \
X("TAB_CHART_PROPERTIES",							"Chart Properties") \
X("TAB_EVENTS",										"Chart Events") \
X("TAB_LYRICS",										"Chart Lyrics") \
X("TAB_TEMPO_CALCULATOR",							"Tempo Calculator") \
X("TAB_UNDO_HISTORY",								"Undo History") \
X("TAB_INSPECTOR",									"Chart Inspector") \
X("TAB_SETTINGS",									"Settings") \
X("TAB_USAGE_GUIDE",								"Usage Guide") \
X("TAB_UPDATE_NOTES",								"Update Notes") \
X("TAB_CHART_STATS",								"Chart Stats") \
X("TAB_TJA_EXPORT_DEBUG_VIEW",						"TJA Export Debug View") \
X("TAB_TJA_IMPORT_TEST",							"TJA Import Test") \
X("TAB_AUDIO_TEST",									"Audio Test") \
/* menu names */ \
X("MENU_FILE",										"File") \
X("MENU_EDIT",										"Edit") \
X("MENU_SELECTION",									"Selection") \
X("MENU_TRANSFORM",									"Transform") \
X("MENU_WINDOW",									"Window") \
X("MENU_LANGUAGE",									"Language") \
/* help menu */ \
X("MENU_HELP",										"Help") \
X("ACT_EDIT_COPY",									"Copy") \
X("ACT_EDIT_DELETE",								"Delete") \
X("ACT_EDIT_SAVE",									"Save") \
X("ACT_EDIT_UNDO",									"Undo") \
X("ACT_EDIT_REDO",									"Redo") \
X("ACT_EDIT_CUT",									"Cut") \
X("ACT_EDIT_PASTE",									"Paste") \
/* file menu */ \
X("ACT_FILE_OPEN_RECENT",							"Open Recent") \
X("ACT_FILE_EXIT",									"Exit") \
X("ACT_FILE_NEW_CHART",								"New Chart") \
X("ACT_FILE_OPEN",									"Open...") \
X("ACT_FILE_CLEAR_ITEMS",							"Clear Items") \
X("ACT_FILE_OPEN_CHART_DIRECTORY",					"Open Chart Directory...") \
X("ACT_FILE_SAVE_AS",								"Save As...") \
/* selection menu */ \
X("ACT_SELECTION_REFINE",							"Refine Selection") \
X("ACT_SELECTION_SELECT_ALL",						"Select All") \
X("ACT_SELECTION_SELECT_TO_CHART_END",				"Select to End of Chart") \
X("ACT_SELECTION_CLEAR",							"Clear Selection") \
X("ACT_SELECTION_INVERT",							"Invert Selection") \
X("ACT_SELECTION_START_RANGE",						"Start Range Selection") \
X("ACT_SELECTION_END_RANGE",						"End Range Selection") \
X("ACT_SELECTION_FROM_RANGE",						"From Range Selection") \
X("ACT_SELECTION_SHIFT_LEFT",						"Shift selection Left") \
X("ACT_SELECTION_SHIFT_RIGHT",						"Shift selection Right") \
X("ACT_SELECTION_ITEM_PATTERN_XO",					"Select Item Pattern xo") \
X("ACT_SELECTION_ITEM_PATTERN_XOO",					"Select Item Pattern xoo") \
X("ACT_SELECTION_ITEM_PATTERN_XOOO",				"Select Item Pattern xooo") \
X("ACT_SELECTION_ITEM_PATTERN_XXOO",				"Select Item Pattern xxoo") \
X("ACT_SELECTION_ADD_NEW_PATTERN",					"Add New Pattern...") \
X("ACT_SELECTION_CUSTOM_PATTERN",					"Select Custom Pattern") \
X("PROMPT_SELECTION_CUSTOM_PATTERN_DELETE",			"Delete?") \
/* transform menu */ \
X("ACT_TRANSFORM_FLIP_NOTE_TYPES",					"Flip Note Types") \
X("ACT_TRANSFORM_TOGGLE_NOTE_SIZES",				"Toggle Note Sizes") \
X("ACT_TRANSFORM_SCALE_ITEMS",						"Scale Items") \
X("ACT_TRANSFORM_SCALE_RANGE",						"Scale Range") \
X("ACT_TRANSFORM_SCALE_BY_TEMPO",					"Scale by Tempo") \
X("ACT_TRANSFORM_SCALE_KEEP_TIME_POSITION",			"Keep Time Position") \
X("ACT_TRANSFORM_SCALE_KEEP_TIME_SIGNATURE",		"Keep Time Signature") \
X("ACT_TRANSFORM_SCALE_KEEP_ITEM_DURATION",			"Keep Item Duration") \
X("ACT_TRANSFORM_RATIO_2_1",						"2:1 (8th to 4th)") \
X("ACT_TRANSFORM_RATIO_3_2",						"3:2 (12th to 8th)") \
X("ACT_TRANSFORM_RATIO_4_3",						"4:3 (16th to 12th)") \
X("ACT_TRANSFORM_RATIO_1_2",						"1:2 (4th to 8th)") \
X("ACT_TRANSFORM_RATIO_2_3",						"2:3 (8th to 12th)") \
X("ACT_TRANSFORM_RATIO_3_4",						"3:4 (12th to 16th)") \
X("ACT_TRANSFORM_RATIO_0_1",						"0:1 (remove time)") \
X("ACT_TRANSFORM_RATIO_N1_1_TIME",					"-1:1 (reverse time)") \
X("ACT_TRANSFORM_RATIO_N1_1_SCROLL",				"-1:1 (reverse scroll)") \
X("ACT_TRANSFORM_ADD_NEW_RATIO",					"Add New Ratio...") \
X("ACT_TRANSFORM_CUSTOM_RATIO",						"Select Custom Ratio") \
X("INFO_TRANSFORM_CUSTOM_RATIO_DELETE",				"Input 0:0 to delete") \
/* window menu */ \
X("ACT_WINDOW_TOGGLE_VSYNC",						"Toggle VSync") \
X("ACT_WINDOW_TOGGLE_FULLSCREEN",					"Toggle Fullscreen") \
X("ACT_WINDOW_SIZE",								"Window Size") \
X("ACT_WINDOW_RESIZE_TO",							"Resize to") \
X("INFO_WINDOW_CURRENT_SIZE",						"Current Size") \
X("ACT_WINDOW_DPI_SCALE",							"DPI Scale") \
X("ACT_WINDOW_DPI_SCALE_ZOOM_IN",					"Zoom In") \
X("ACT_WINDOW_DPI_SCALE_ZOOM_OUT",					"Zoom Out") \
X("ACT_WINDOW_DPI_SCALE_RESET_ZOOM",				"Reset Zoom") \
X("ACT_ZOOM_POPUP_RESET_ZOOM",						" Reset ") \
X("INFO_WINDOW_DPI_SCALE_CURRENT",					"Current Scale") \
/* test menu */ \
X("MENU_TEST",										"Test Menu") \
X("ACT_TEST_SHOW_AUDIO_TEST",						"Show Audio Test") \
X("ACT_TEST_SHOW_TJA_IMPORT_TEST",					"Show TJA Import Test") \
X("ACT_TEST_SHOW_TJA_EXPORT_VIEW",					"Show TJA Export View") \
X("ACT_TEST_SHOW_IMGUI_DEMO",						"Show ImGui Demo") \
X("ACT_TEST_SHOW_IMGUI_STYLE_EDITOR",				"Show ImGui Style Editor") \
X("ACT_TEST_RESET_STYLE_COLORS",					"Reset Style Colors") \
/* help menu */ \
X("INFO_HELP_COPYRIGHT_YEAR",						"Copyright (c) 2022") \
X("INFO_HELP_BUILD_TIME",							"Build Time:") \
X("INFO_HELP_BUILD_DATE",							"Build Date:") \
X("INFO_HELP_BUILD_CONFIGURATION",					"Build Configuration:") \
X("INFO_HELP_CURRENT_VERSION",						"Current Version:") \
X("BUILD_CONFIGURATION_DEBUG",						"Debug") \
X("BUILD_CONFIGURATION_RELEASE",					"Release") \
X("BUILD_CONFIGURATION_UNKNOWN",					"Unknown") \
/* difficulty menu */ \
X("MENU_COURSES",									"Courses") \
X("ACT_COURSES_ADD_NEW",							"Add New") \
X("ACT_COURSES_COMPARE_GROUP",						"Compare") \
X("ACT_COURSES_COMPARE_ALL",						"Compare All") \
X("ACT_COURSES_COMPARE_NONE",						"Clear Comparison") \
X("ACT_COURSES_COMPARE_ACROSS_DIFFICULTIES",		"Across Difficulties") \
X("ACT_COURSES_COMPARE_ACROSS_PLAYERCOUNTS",		"Across Player Counts") \
X("ACT_COURSES_COMPARE_ACROSS_PLAYERSIDES",			"Across Player Sides") \
X("ACT_COURSES_COMPARE_ACROSS_BRANCHES",			"Across Branches") \
X("ACT_COURSES_COMPARE_MODE",						"Comparison Mode") \
X("ACT_COURSES_COMPARE_THIS",						"Compare This") \
X("ACT_COURSES_EDIT",								"Edit...") \
/* audio menu */ \
X("ACT_AUDIO_OPEN_DEVICE",							"Open Audio Device") \
X("ACT_AUDIO_CLOSE_DEVICE",							"Close Audio Device") \
/* latency menu */ \
X("INFO_LATENCY_AVERAGE",							"Average: ") \
X("INFO_LATENCY_MIN",								"Min: ") \
X("INFO_LATENCY_MAX",								"Max: ") \
/* audio menu (contd.) */ \
X("ACT_AUDIO_USE_FMT_%s_DEVICE",					"Use %s") \
/* unsaved message box */ \
X("INFO_MSGBOX_UNSAVED",							"Peepo Drum Kit - Unsaved Changes") \
X("PROMPT_MSGBOX_UNSAVED_SAVE_CHANGES",				"Save changes to the current file?") \
X("ACT_MSGBOX_UNSAVED_SAVE_CHANGES",				"Save Changes") \
X("ACT_MSGBOX_UNSAVED_DISCARD_CHANGES",				"Discard Changes") \
X("ACT_MSGBOX_CANCEL",								"Cancel") \
/* chart events tab / timeline tab */ \
X("DETAILS_CHART_EVENT_EVENTS",						"Events") \
X("EVENT_TEMPO",									"Tempo") \
X("EVENT_TIME_SIGNATURE",							"Time Signature") \
X("EVENT_NOTES",									"Notes") \
X("EVENT_NOTES_EXPERT",								"Notes (Expert)") \
X("EVENT_NOTES_MASTER",								"Notes (Master)") \
X("EVENT_SCROLL_SPEED",								"Scroll Speed") \
X("EVENT_PROP_VERTICAL_SCROLL_SPEED",				"Vertical Scroll Speed") \
X("EVENT_PROP_SCROLL_SPEED_TEMPO",					"Scroll Speed Tempo") \
X("EVENT_BAR_LINE_VISIBILITY",						"Bar Line Visibility") \
X("EVENT_GO_GO_TIME",								"Go-Go Time") \
X("EVENT_LYRICS",									"Lyrics") \
X("EVENT_SCROLL_TYPE",								"Scroll Type") \
X("EVENT_JPOS_SCROLL",								"JPOS Scroll") \
X("EVENT_BRANCH_SECTIONS",							"Branch Sections") \
X("EVENT_SECTION_MARKERS",							"Section Markers") \
X("EVENT_PROP_BRANCH_CONDITION",					"Branch Condition") \
X("EVENT_PROP_BRANCH_REQ_EXPERT",					"Expert Requirement") \
X("EVENT_PROP_BRANCH_REQ_MASTER",					"Master Requirement") \
X("EVENT_PROP_JPOS_SCROLL_MOVE",					"JPOS Scroll Move") \
X("EVENT_PROP_VERTICAL_JPOS_SCROLL_MOVE",			"Vertical JPOS Scroll Move") \
X("EVENT_PROP_JPOS_SCROLL_DURATION",				"JPOS Scroll Duration") \
X("DETAILS_EVENTS_SYNC",							"Sync") \
X("SYNC_CHART_DURATION",							"Chart Duration") \
X("SYNC_SONG_DEMO_START",							"Song Demo Start") \
X("SYNC_SONG_OFFSET",								"Song Offset") \
X("ACT_EVENT_INSERT_AT_SELECTED_ITEMS",				"Insert at Selected Items") \
X("ACT_SYNC_SET_CURSOR",							"Set Cursor") \
X("ACT_EVENT_ADD",									"Add") \
X("ACT_EVENT_REMOVE",								"Remove") \
X("ACT_EVENT_CLEAR",								"Clear") \
X("ACT_EVENT_SET_FROM_RANGE_SELECTION",				"Set from Range Selection") \
/* chart properties tab */ \
X("DETAILS_CHART_PROPERTIES_CHART",					"Chart") \
X("CHART_PROP_TITLE",								"Chart Title") \
X("DETAILS_CHART_PROP_TITLE_LOCALIZED",				"Chart Title Localized") \
X("CHART_PROP_SUBTITLE",							"Chart Subtitle") \
X("DETAILS_CHART_PROP_SUBTITLE_LOCALIZED",			"Chart Subtitle Localized") \
X("ACT_ADD_NEW_LOCALE",								"Add New Locale:") \
X("CHART_PROP_CREATOR",								"Chart Creator") \
X("CHART_PROP_SONG_FILE_NAME",						"Song File Name") \
X("CHART_PROP_JACKET_FILE_NAME",					"Jacket File Name") \
X("CHART_PROP_SONG_VOLUME",							"Song Volume") \
X("CHART_PROP_SOUND_EFFECT_VOLUME",					"Sound Effect Volume") \
X("DETAILS_CHART_PROP_OTHER_METADATA",				"Other Chart Metadata") \
X("DETAILS_CHART_PROPERTIES_COURSE",				"Selected Course") \
X("COURSE_PROP_DIFFICULTY_TYPE",					"Difficulty Type") \
X("COURSE_PROP_DIFFICULTY_LEVEL",					"Difficulty Level") \
X("COURSE_PROP_DIFFICULTY_LEVEL_DECIMAL",			"Difficulty Level Decimal") \
X("COURSE_PROP_PLAYER_SIDE_COUNT",					"Player Side/Count") \
X("COURSE_PROP_TOWER_LIFE",							"Lives") \
X("COURSE_PROP_TOWER_SIDE",							"Side") \
X("COURSE_PROP_CREATOR",							"Course Creator") \
X("DETAILS_COURSE_PROP_OTHER_METADATA",				"Other Course Metadata") \
X("ACT_ADD_NEW_METADATA",							"Add New Metadata:") \
/* inspector tab / timeline tab (contd.) */ \
X("DETAILS_INSPECTOR_SELECTED_ITEMS",				"Selected Items") \
X("INFO_INSPECTOR_NOTHING_SELECTED",				"( Nothing Selected )") \
X("INFO_INSPECTOR_SELECTED_ITEM",					"Selected ") \
X("SELECTED_EVENTS_ITEMS",							"Items") \
X("SELECTED_EVENTS_TEMPOS",							"Tempos") \
X("SELECTED_EVENTS_TIME_SIGNATURES",				"Time Signatures") \
X("SELECTED_EVENTS_SCROLL_SPEEDS",					"Scroll Speeds") \
X("SELECTED_EVENTS_BAR_LINE_VISIBILITIES",			"Bar Lines") \
X("SELECTED_EVENTS_GO_GO_RANGES",					"Go-Go Ranges") \
X("EVENT_PROP_BAR_LINE_VISIBLE",					"Bar Line Visible") \
X("BAR_LINE_VISIBILITY_VISIBLE",					"Visible") \
X("BAR_LINE_VISIBILITY_HIDDEN",						"Hidden") \
X("SELECTED_EVENTS_SCROLL_TYPES",					"Scroll Types") \
X("SELECTED_EVENTS_JPOS_SCROLLS",					"JPOS Scrolls") \
X("SCROLL_TYPE_NMSCROLL",							"NMSCROLL") \
X("SCROLL_TYPE_HBSCROLL",							"HBSCROLL") \
X("SCROLL_TYPE_BMSCROLL",							"BMSCROLL") \
X("EVENT_PROP_BALLOON_POP_COUNT",					"Balloon Pop Count") \
X("EVENT_PROP_INTERPOLATE_SCROLL_SPEED",			"Interpolate: Scroll Speed") \
X("EVENT_PROP_INTERPOLATE_SCROLL_SPEED_TEMPO",		"Interpolate: Scroll Speed Tempo") \
X("EVENT_PROP_INTERPOLATE_VERTICAL_SCROLL_SPEED",	"Interpolate: Vertical Scroll Speed") \
X("EVENT_PROP_TIME_OFFSET",							"Time Offset") \
X("EVENT_PROP_NOTE_TYPE",							"Note Type") \
X("EVENT_PROP_NOTE_TYPE_SIZE",						"Note Type Size") \
X("NOTE_TYPE_DON",									"Don") \
X("NOTE_TYPE_DON_BIG",								"DON") \
X("NOTE_TYPE_KA",									"Ka") \
X("NOTE_TYPE_KA_BIG",								"KA") \
X("NOTE_TYPE_DRUMROLL",								"Drumroll") \
X("NOTE_TYPE_DRUMROLL_BIG",							"DRUMROLL") \
X("NOTE_TYPE_BALLOON",								"Balloon") \
X("NOTE_TYPE_BALLOON_EX",							"BALLOON") \
X("NOTE_TYPE_DON_HAND",								"DON (Hand)") \
X("NOTE_TYPE_KA_HAND",								"KA (Hand)") \
X("NOTE_TYPE_SIZE_SMALL",							"Small") \
X("NOTE_TYPE_SIZE_BIG",								"Big") \
X("NOTE_TYPE_SIZE_HAND",							"Hand") \
/* difficulty menu (contd) */ \
X("DIFFICULTY_TYPE_EASY",							"Easy") \
X("DIFFICULTY_TYPE_NORMAL",							"Normal") \
X("DIFFICULTY_TYPE_HARD",							"Hard") \
X("DIFFICULTY_TYPE_ONI",							"Extreme") \
X("DIFFICULTY_TYPE_ONI_URA",						"Extra") \
X("DIFFICULTY_TYPE_TOWER",							"Tower") \
X("DIFFICULTY_TYPE_DAN",							"Dan") \
X("PLAYER_SIDE_STYLE_SINGLE",						"Single") \
X("PLAYER_SIDE_STYLE_DOUBLE",						"Double") \
X("PLAYER_SIDE_STYLE_FMT_%d_STYLE",					"%d-players") \
X("PLAYER_SIDE_PLAYER_FMT_%d_PLAYER",				"P%d") \
X("TOWER_SIDE_NORMAL",								"Sweet (Normal)") \
X("TOWER_SIDE_EX",									"Spicy (Ex)") \
X("TOWER_SIDE_BOTH",								"Unspecified (Both)") \
/* undo history tab */ \
X("UNDO_HISTORY_DESCRIPTION",						"Description") \
X("UNDO_HISTORY_TIME",								"Time") \
X("UNDO_HISTORY_INITIAL_STATE",						"Initial State") \
/* lyrics tab */ \
X("DETAILS_LYRICS_OVERVIEW",						"Lyrics Overview") \
X("DETAILS_LYRICS_EDIT_LINE",						"Edit Line") \
X("INFO_LYRICS_NO_LYRICS",							"(No Lyrics)") \
/* tempo calculator tab */ \
X("ACT_TEMPO_CALCULATOR_RESET",						"Reset") \
X("ACT_TEMPO_CALCULATOR_TAP",						"Tap") \
X("INFO_TEMPO_CALCULATOR_TAP_FIRST_BEAT",			" First Beat ") \
X("LABEL_TEMPO_CALCULATOR_NEAREST_WHOLE",			"Nearest Whole") \
X("LABEL_TEMPO_CALCULATOR_NEAREST",					"Nearest") \
X("LABEL_TEMPO_CALCULATOR_MIN_AND_MAX",				"Min and Max") \
X("LABEL_TEMPO_CALCULATOR_TAPS",					"Timing Taps") \
X("INFO_TEMPO_CALCULATOR_TAPS_FIRST_BEAT",			"First Beat") \
X("INFO_TEMPO_CALCULATOR_TAPS_FMT_%d_TAPS",			"%d Taps") \
/* unused (?) */ \
X("STR_EMPTY",										"") \
/* inspector tab / timeline tab (contd.) */ \
X("NOTE_TYPE_KADON",								"KADON") \
X("NOTE_TYPE_BOMB",									"Bomb") \
X("NOTE_TYPE_ADLIB",								"Adlib") \
X("NOTE_TYPE_FUSEROLL",								"Fuseroll") \
/* language tab (contd) */ \
X("ACT_LANGUAGE_LOAD_FULL_CJKV_GLYPHS",				"Load Full CJKV Glyphs (slow)") \
/* empty last line */


#define UI_Str(in) i18n::HashToString(i18n::CompileTimeValidate<i18n::Hash(in)>())
#define UI_StrRuntime(in) i18n::HashToString(i18n::Hash(in))
#define UI_WindowName(in) i18n::ToStableName(in, i18n::CompileTimeValidate<i18n::Hash(in)>()).Data

namespace PeepoDrumKit
{
	inline std::string SelectedGuiLanguage = std::string("en");
	inline std::string SelectedGuiLanguageTJA = ASCII::IETFLangTagToTJALangTag(SelectedGuiLanguage);
}

namespace PeepoDrumKit::i18n
{
	struct LocaleEntry {
		std::string id;
		std::string name;
	};
	extern std::vector<LocaleEntry> LocaleEntries;

	void InitBuiltinLocale();
	void RefreshLocales();
	void ReloadLocaleFile(cstr languageId = "en");
	void ExportBuiltinLocaleFiles();
	cstr HashToString(u32 inHash);

	constexpr ImU32 Crc32LookupTable[256] =
	{
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
		0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
		0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
		0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
		0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
		0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
		0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
		0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
		0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
		0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
		0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
		0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
		0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
		0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
		0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
		0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
	};

	constexpr u32 Crc32(const char* data, size_t dataSize, u32 seed)
	{
		seed = ~seed; u32 crc = seed;
		while (dataSize-- != 0)
			crc = (crc >> 8) ^ Crc32LookupTable[(crc & 0xFF) ^ static_cast<u8>(*data++)];
		return ~crc;
	}

	constexpr u32 Hash(std::string_view data, u32 seed = 0xDEADBEEF) { return Crc32(data.data(), data.size(), seed); }

	constexpr u32 AllValidHashes[] =
	{
#define X(key, en) Hash(key),
			PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST_EN
#undef X
#define X(key, en) Hash(key),
			PEEPODRUMKIT_UI_STRINGS_STABLE_X_MACRO_LIST_EN
#undef X
	};

	constexpr b8 IsValidHash(u32 inHash) { for (u32 it : AllValidHashes) { if (it == inHash) return true; } return false; }

	template <u32 InHash>
	constexpr u32 CompileTimeValidate() { static_assert(IsValidHash(InHash), "Unknown string"); return InHash; }

	cstr HashToString(u32 inHash);

	constexpr cstr HashToStableString(u32 inHash)
	{
		switch (inHash) {
#define X(key, en) case Hash(key): return en;
			PEEPODRUMKIT_UI_STRINGS_STABLE_X_MACRO_LIST_EN
#undef X
			default:
				return nullptr;
		}
	}

	struct StableNameBuffer { char Data[128]; };
	inline StableNameBuffer ToStableName(cstr inString, u32 inHash)
	{
		cstr translatedString = HashToString(inHash);
		cstr stableString = HashToStableString(inHash);
		if (stableString != nullptr)
			inString = stableString;
		StableNameBuffer buffer;

		char* out = &buffer.Data[0];
		*out = '\0';
		while (*translatedString != '\0')
			*out++ = *translatedString++;
		*out++ = '#';
		*out++ = '#';
		*out++ = '#';
		while (*inString != '\0')
			*out++ = *inString++;
		*out = '\0';

		// strcpy_s(out.Data, translatedString);
		// strcat_s(out.Data, "###");
		// strcat_s(out.Data, inString);

		return buffer;
	}
}
