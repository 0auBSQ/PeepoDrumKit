#pragma once
#include "core_types.h"
#include "imgui/imgui_include.h"

#define PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST \
X("Game Preview",						u8"�Q�[���v���r���[") \
X("Chart Timeline",						u8"�^�C�����C��") \
X("Chart Timeline - Debug",				u8"�^�C�����C�� - �f�o�b�O") \
X("Chart Properties",					u8"���ʃv���p�e�B") \
X("Chart Tempo",						u8"���ʃ��Y��") \
X("Chart Lyrics",						u8"���ʉ̎�") \
X("Tempo Calculator",					u8"�e���|�v�Z�@") \
X("Undo History",						u8"�q�X�g���[") \
X("Chart Inspector",					u8"�C���X�y�N�^�[") \
X("Settings",							u8"���ݒ�") \
X("Usage Guide",						u8"�g�����ɂ���") \
X("TJA Export Debug View",				u8"TJA�G�N�X�|�[�g�e�X�g") \
X("TJA Import Test",					u8"TJA�C���|�[�g�e�X�g") \
X("Audio Test",							u8"�I�[�f�B�I�e�X�g") \
X("File",								u8"�t�@�C��") \
X("Edit",								u8"�ҏW") \
X("Selection",							u8"�I��") \
X("Transform",							u8"�ϊ�") \
X("Window",								u8"�E�B���h�E") \
X("Language",							u8"����") \
X("English",							u8"�p��") \
X("Japanese",							u8"���{��") \
X("%s (%s)",							u8"%s�i%s�j") \
X("Help",								u8"�w���v") \
X("Copy",								u8"�R�s�[") \
X("Delete",								u8"�폜") \
X("Save",								u8"�ۑ�") \
X("Undo",								u8"���ɖ߂�") \
X("Redo",								u8"��蒼��") \
X("Cut",								u8"�؂���") \
X("Paste",								u8"�\��t��") \
X("Open Recent",						u8"�ŋߎg�p�������ڂ��J��") \
X("Exit",								u8"�I��") \
X("New Chart",							u8"�V�K�t�@�C��") \
X("Open...",							u8"�t�@�C�����J��...") \
X("Clear Items",						u8"�ŋߎg�������̂��N���A") \
X("Open Chart Directory...",			u8"�G�N�X�v���[���[�ŕ\������...") \
X("Save As...",							u8"���O��t���ĕۑ�...") \
X("Refine Selection",					u8"�I�����i�荞��") \
X("Select All",							u8"���ׂđI��") \
X("Clear Selection",					u8"�I��������") \
X("Invert Selection",					u8"�I���𔽓]") \
X("Start Range Selection",				u8"�͈͑I�����J�n") \
X("End Range Selection",				u8"�͈͑I�����I��") \
X("From Range Selection",				u8"�͈͑I���ȓ����ڂ�I��") \
X("Shift selection Left",				u8"�I�������Ɉړ�") \
X("Shift selection Right",				u8"�I�����E�Ɉړ�") \
X("Select Item Pattern xo",				u8"�p�^�[���I�� xo") \
X("Select Item Pattern xoo",			u8"�p�^�[���I�� xoo") \
X("Select Item Pattern xooo",			u8"�p�^�[���I�� xooo") \
X("Select Item Pattern xxoo",			u8"�p�^�[���I�� xxoo") \
X("Add New Pattern...",					u8"�V�K�p�^�[����ǉ�...") \
X("Select Custom Pattern",				u8"�J�X�^���p�^�[��") \
X("Delete?",							u8"�폜�H") \
X("Flip Note Types",					u8"�����̎�ނ�ω�") \
X("Toggle Note Sizes",					u8"�����̑召��ω�") \
X("Expand Items",						u8"�I���������ڂ��L��") \
X("Compress Items",						u8"�I���������ڂ��k��") \
X("2:1 (8th to 4th)",					u8"2:1 (8�� �� 4��)") \
X("3:2 (12th to 8th)",					u8"3:2 (12�� �� 8��)") \
X("4:3 (16th to 12th)",					u8"4:3 (16�� �� 12��)") \
X("1:2 (4th to 8th)",					u8"1:2 (4�� �� 8��)") \
X("2:3 (8th to 12th)",					u8"2:3 (8�� �� 12��)") \
X("3:4 (12th to 16th)",					u8"3:4 (12�� �� 16��)") \
X("Toggle VSync",						u8"��������") \
X("Toggle Fullscreen",					u8"�S��ʕ\��") \
X("Window Size",						u8"�E�B���h�E�T�C�Y") \
X("Resize to",							u8"���T�C�Y") \
X("Current Size",						u8"���݂̃T�C�Y") \
X("DPI Scale",							u8"�𑜓x�X�P�[��") \
X("Zoom In",							u8"�g��") \
X("Zoom Out",							u8"�k��") \
X("Reset Zoom",							u8"�Y�[�������Z�b�g") \
X(" Reset ",							u8" ���Z�b�g ") \
X("Current Scale",						u8"���݂̃X�P�[��") \
X("Test Menu",							u8"�e�X�g") \
X("Show Audio Test",					u8"�I�[�f�B�I�e�X�g�\��") \
X("Show TJA Import Test",				u8"TJA�C���|�[�g�e�X�g�\��") \
X("Show TJA Export View",				u8"TJA�G�N�X�|�[�g�e�X�g�\��") \
X("Show ImGui Demo",					u8"ImGui Demo�\��") \
X("Show ImGui Style Editor",			u8"ImGui Style Editor�\��") \
X("Reset Style Colors",					u8"Style Colors�����Z�b�g") \
X("Copyright (c) 2022",					u8"Copyright (c) 2022") \
X("Build Time:",						u8"�r���h�̎���:") \
X("Build Date:",						u8"�r���h�̓��t:") \
X("Build Configuration:",				u8"�r���h�̍\��:") \
X("Debug",								u8"�f�o�b�O") \
X("Release",							u8"�����[�X") \
X("Courses",							u8"�R�[�X") \
X("Add New",							u8"�ǉ�") \
X("Edit...",							u8"�ҏW...") \
X("Open Audio Device",					u8"�I�[�f�B�I�f�o�C�X���J��") \
X("Close Audio Device",					u8"�I�[�f�B�I�f�o�C�X�����") \
X("Average: ",							u8"���ϒl: ") \
X("Min: ",								u8"�ŏ��l: ") \
X("Max: ",								u8"�ő�l: ") \
X("Use %s",								u8"%s���g�p") \
X("Peepo Drum Kit - Unsaved Changes",	u8"Peepo Drum Kit - �ۑ�����Ă��Ȃ��ύX") \
X("Save changes to the current file?",	u8"���̃t�@�C���̕ύX���e��ۑ����܂����H") \
X("Save Changes",						u8"�ۑ�") \
X("Discard Changes",					u8"�ۑ����Ȃ�") \
X("Cancel",								u8"�L�����Z��") \
X("Tempo",								u8"�e���|") \
X("Time Signature",						u8"���q�L��") \
X("Notes",								u8"����") \
X("Notes (Expert)",						u8"���� (���l)") \
X("Notes (Master)",						u8"���� (�B�l)") \
X("Scroll Speed",						u8"�X�N���[���X�s�[�h") \
X("Scroll Speed Tempo",					u8"�X�N���[���X�s�[�h�̃e���|") \
X("Bar Line Visibility",				u8"���ߐ��̕\��") \
X("Go-Go Time",							u8"�S�[�S�[�^�C��") \
X("Lyrics",								u8"�̎�") \
X("Sync",								u8"�^�C�~���O") \
X("Chart Duration",						u8"���ʂ̏I������") \
X("Song Demo Start",					u8"�����̃v���r���[����") \
X("Song Offset",						u8"�����̍Đ��J�n����") \
X("Selection to Scroll Changes",		u8"�I���������ڂ��X�N���[���X�s�[�h�ɕω�") \
X("Set Cursor",							u8"�J�[�\���Őݒ�") \
X("Add",								u8"�ǉ�") \
X("Remove",								u8"�폜") \
X("Clear",								u8"�N���A") \
X("Set from Range Selection",			u8"�͈͑I���Œǉ�") \
X("Chart",								u8"�n�안��") \
X("Chart Title",						u8"�Ȃ̃^�C�g��") \
X("Chart Subtitle",						u8"�Ȃ̃T�u�^�C�g��") \
X("Chart Creator",						u8"���ʂ̍�Җ�") \
X("Song File Name",						u8"�����̃t�@�C����") \
X("Song Volume",						u8"�����̉���") \
X("Sound Effect Volume",				u8"���F�̉���") \
X("Selected Course",					u8"���݂̃R�[�X") \
X("Difficulty Type",					u8"��Փx") \
X("Difficulty Level",					u8"��Փx�̃��x��") \
X("Selected Items",						u8"�I����������") \
X("( Nothing Selected )",				u8"( �I���Ȃ� )") \
X("Selected ",							u8"�I������") \
X("Items",								u8"����") \
X("Tempos",								u8"�e���|") \
X("Time Signatures",					u8"���q�L��") \
X("Scroll Speeds",						u8"�X�N���[���X�s�[�h") \
X("Bar Lines",							u8"���ߐ��̕\��") \
X("Go-Go Ranges",						u8"�S�[�S�[�^�C��") \
X("Bar Line Visible",					u8"���ߐ��̕\��") \
X("Visible",							u8"�\��") \
X("Hidden",								u8"��\��") \
X("Balloon Pop Count",					u8"���D�̘A�Ő�") \
X("Interpolate: Scroll Speed",			u8"���: �X�N���[���X�s�[�h") \
X("Interpolate: Scroll Speed Tempo",	u8"���: �X�N���[���X�s�[�h�̃e���|") \
X("Time Offset",						u8"���Ԃ̃I�t�Z�b�g") \
X("Note Type",							u8"�����̎��") \
X("Note Type Size",						u8"�����̑召") \
X("Don",								u8"�h��") \
X("DON",								u8"�h�� (��)") \
X("Ka",									u8"�J�b") \
X("KA",									u8"�J�b (��)") \
X("Drumroll",							u8"�A��") \
X("DRUMROLL",							u8"�A�� (��)") \
X("Balloon",							u8"���D") \
X("BALLOON",							u8"���D (��)") \
X("Small",								u8"��") \
X("Big",								u8"��") \
X("Easy",								u8"���񂽂�") \
X("Normal",								u8"�ӂ�") \
X("Hard",								u8"�ނ�������") \
X("Oni",								u8"����") \
X("Oni-Ura",							u8"���� (��)") \
X("Single",								u8"��l") \
X("Double",								u8"��l") \
X("Description",						u8"�L�q") \
X("Time",								u8"����") \
X("Initial State",						u8"�������") \
X("Lyrics Overview",					u8"�̎��̊T�v") \
X("Edit Line",							u8"�ҏW") \
X("(No Lyrics)",						u8"(�̎��Ȃ�)") \
X("Reset",								u8"���Z�b�g") \
X("Tap",								u8"�@����") \
X(" First Beat ",						u8"�ꔏ") \
X("Nearest Whole",						u8"���ρi�������j") \
X("Nearest",							u8"����") \
X("Min and Max",						u8"�ŏ��ƍő�") \
X("Timing Taps",						u8"�@������") \
X("First Beat",							u8"�ŏ��̔�") \
X("%d Taps",							u8"%d ��") \
X("",									u8"") \

#define UI_Str(in) i18n::HashToString(i18n::CompileTimeValidate<i18n::Hash(in)>(), SelectedGuiLanguage)
#define UI_StrRuntime(in) i18n::HashToString(i18n::Hash(in), SelectedGuiLanguage)
#define UI_WindowName(in) i18n::ToStableName(in, i18n::CompileTimeValidate<i18n::Hash(in)>(), SelectedGuiLanguage).Data

namespace PeepoDrumKit
{
	enum class GuiLanguage : u8 { EN, JA, Count };
	inline GuiLanguage SelectedGuiLanguage = GuiLanguage::EN;
}

namespace PeepoDrumKit::i18n
{
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
#define X(en, ja) Hash(en),
			PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST
#undef X
	};

	constexpr b8 IsValidHash(u32 inHash) { for (u32 it : AllValidHashes) { if (it == inHash) return true; } return false; }

	template <u32 InHash>
	constexpr u32 CompileTimeValidate() { static_assert(IsValidHash(InHash), "Unknown string"); return InHash; }

	constexpr cstr HashToString(u32 inHash, GuiLanguage outLanguage)
	{
		switch (inHash)
		{
#define X(en, ja) case ForceConsteval<Hash(en)>: return (outLanguage == GuiLanguage::EN) ? en : ja;
			PEEPODRUMKIT_UI_STRINGS_X_MACRO_LIST
#undef X
		}

#if PEEPO_DEBUG
		assert(!"Missing string entry"); return nullptr;
#endif
		return "(undefined)";
	}

	struct StableNameBuffer { char Data[128]; };
	inline StableNameBuffer ToStableName(cstr inString, u32 inHash, GuiLanguage outLanguage)
	{
		cstr translatedString = HashToString(inHash, outLanguage);
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
