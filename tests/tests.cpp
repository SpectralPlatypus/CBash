//#include <gtest/gtest.h>

#include <Skyrim/TES5File.h>
#include <Collection.h>
#include <ModFile.h>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <experimental/filesystem>

//// Demonstrate some basic assertions.
//TEST(HelloTest, BasicAssertions) {
//	// Expect two strings not to be equal.
//	EXPECT_STRNE("hello", "world");
//	// Expect equality.
//	EXPECT_EQ(7 * 6, 42);
//}
//
//TEST(LoadTest, skyrim_esm) {
//	Collection skyrimCollection = Collection("D:\\SteamLibrary\\steamapps\\common\\Skyrim\\Data", 3);
//	ModFlags masterFlags = ModFlags(0xA);
//	ModFile* esm = skyrimCollection.AddMod("Skyrim.esm", masterFlags);
//
//	char* argvv[4];
//	argvv[0] = new char();
//	argvv[1] = new char();
//	argvv[2] = new char();
//	argvv[3] = new char();
//	logger.init(4, argvv);
//
//	skyrimCollection.Load();
//}

		/*      {0x00000001}'Hand To Hand Melee',
				{0x00000002}'One Hand Sword',
				{0x00000004}'One Hand Dagger',
				{0x00000008}'One Hand Axe',
				{0x00000010}'One Hand Mace',
				{0x00000020}'Two Hand Sword',
				{0x00000040}'Two Hand Axe/Mace',
				{0x00000080}'Bow',
				{0x00000100}'Staff',
				{0x00000200}'Spell',
				{0x00000400}'Shield',
				{0x00000800}'Torch',
				{0x00001000}'Crossbow'*/

std::string VNAMtoString(uint32_t value)
{
	std::string out;
	if (value & 0x00000001) out += "Hand To Hand Melee|";
	if (value & 0x00000002) out += "One Hand Sword|";
	if (value & 0x00000004) out += "One Hand Dagger|";
	if (value & 0x00000008) out += "One Hand Axe|";
	if (value & 0x00000010) out += "One Hand Mace|";
	if (value & 0x00000020) out += "Two Hand Sword|";
	if (value & 0x00000040) out += "Two Hand Axe|";
	if (value & 0x00000080) out += "Bow|";
	if (value & 0x00000100) out += "Staff|";
	if (value & 0x00000200) out += "Spell|";
	if (value & 0x00000400) out += "Shield|";
	if (value & 0x00000800) out += "Torch|";
	if (value & 0x00001000) out += "Crossbow|";
	return out;
}

size_t CountVNAM(uint32_t value)
{
	size_t out = 0;
	if (value & 0x00000001) out += 1;
	if (value & 0x00000002) out += 1;
	if (value & 0x00000004) out += 1;
	if (value & 0x00000008) out += 1;
	if (value & 0x00000010) out += 1;
	if (value & 0x00000020) out += 1;
	if (value & 0x00000040) out += 1;
	return out;
}

bool isTestSet(std::string in)
{
	if (in == "DefaultFemale" ||
		in == "RieklingProject" ||
		in == "DraugrProject" ||
		in == "DraugrSkeletonProject" ||
		in == "SphereCenturion" ||
		in == "SteamProject" ||
		in == "FalmerProject")
		return true;
	return false;
}

/*
wbWeaponAnimTypeEnum := wbEnum([
	{0} 'HandToHandMelee',
	{1} 'OneHandSword',
	{2} 'OneHandDagger',
	{3} 'OneHandAxe',
	{4} 'OneHandMace',
	{5} 'TwoHandSword',
	{6} 'TwoHandAxe',
	{7} 'Bow /crossbow',
	{8} 'Staff',
	{9} 'Magic'
	10 Shield
	11 Torch
	12 Player crossbow
  ]);

*/

int main(int argc, char* argv[])
{
	Collection skyrimCollection = Collection("D:\\SteamLibrary\\steamapps\\common\\Skyrim Special Edition\\Data", 3);
	ModFlags espFlags = ModFlags(0x1818);
	espFlags.IsNoLoad = false;
	espFlags.IsFullLoad = true;
	espFlags.IsMinLoad = false;
	espFlags.IsCreateNew = true;
	ModFile* esp = skyrimCollection.AddMod("RACETEST.esp", espFlags);

	char* argvv[4];
	argvv[0] = new char();
	argvv[1] = new char();
	argvv[2] = new char();
	argvv[3] = new char();
	logger.init(4, argvv);

	int result = skyrimCollection.Load();

	char* race_EDID = new char[10];
	memset(race_EDID, 0, 10);
	memcpy(race_EDID, "Race", 5);
	Sk::RACERecord* race_record = (Sk::RACERecord*)skyrimCollection.CreateRecord(esp, REV32(RACE), NULL, race_EDID, NULL, 0);
	race_record->EDID.value = race_EDID;

	char* voice_type_m_EDID = new char[10];
	memset(voice_type_m_EDID, 0, 10);
	memcpy(voice_type_m_EDID, "VoiceM", 6);
	Sk::VTYPRecord* voice_type_m_record = (Sk::VTYPRecord*)skyrimCollection.CreateRecord(esp, REV32(VTYP), NULL, voice_type_m_EDID, NULL, 0);
	voice_type_m_record->EDID.value = voice_type_m_EDID;
	race_record->VTCK.value[0] = voice_type_m_record->formID;

	char* voice_type_f_EDID = new char[10];
	memset(voice_type_f_EDID, 0, 10);
	memcpy(voice_type_f_EDID, "VoiceF", 6);
	Sk::VTYPRecord* voice_type_f_record = (Sk::VTYPRecord*)skyrimCollection.CreateRecord(esp, REV32(VTYP), NULL, voice_type_f_EDID, NULL, 0);
	voice_type_f_record->EDID.value = voice_type_f_EDID;
	race_record->VTCK.value[1] = voice_type_f_record->formID;

	char* body_parts_EDID = new char[10];
	memset(body_parts_EDID, 0, 10);
	memcpy(body_parts_EDID, "Parts", 6);
	Sk::BPTDRecord* body_parts_record = (Sk::BPTDRecord*)skyrimCollection.CreateRecord(esp, REV32(BPTD), NULL, body_parts_EDID, NULL, 0);
	body_parts_record->EDID.value = body_parts_EDID;
	race_record->GNAM.value = body_parts_record->formID;

	char* arma_EDID = new char[10];
	memset(arma_EDID, 0, 10);
	memcpy(arma_EDID, "ArmorA", 7);
	Sk::ARMARecord* arma_record = (Sk::ARMARecord*)skyrimCollection.CreateRecord(esp, REV32(ARMA), NULL, arma_EDID, NULL, 0);
	arma_record->EDID.value = arma_EDID;
	arma_record->RNAM.value = race_record->formID;
	arma_record->BOD2.value.body_part = Sk::SKBOD2::BodyParts::bpBody;

	char* armo_EDID = new char[10];
	memset(armo_EDID, 0, 10);
	memcpy(armo_EDID, "Armor", 6);
	Sk::ARMORecord* armo_record = (Sk::ARMORecord*)skyrimCollection.CreateRecord(esp, REV32(ARMO), NULL, armo_EDID, NULL, 0);
	armo_record->EDID.value = armo_EDID;
	armo_record->RNAM.value = race_record->formID;
	armo_record->BOD2.value.body_part = Sk::SKBOD2::BodyParts::bpBody;
	armo_record->MODL.value.push_back(arma_record->formID);

	ModSaveFlags skSaveFlags = ModSaveFlags(2);
	skSaveFlags.IsCleanMasters = true;
	std::string esp_name = "RACETEST.esp";
	skyrimCollection.SaveMod(esp, skSaveFlags, (char* const)esp_name.c_str());

	//Collection skyrimCollection = Collection("D:\\SteamLibrary\\steamapps\\common\\Skyrim\\Data", 3);
	//ModFlags masterFlags = ModFlags(0xA);
	//ModFile* esm = skyrimCollection.AddMod("Skyrim.esm", masterFlags);
	//esm = skyrimCollection.AddMod("Update.esm", masterFlags);
	//esm = skyrimCollection.AddMod("Dragonborn.esm", masterFlags);

	//char* argvv[4];
	//argvv[0] = new char();
	//argvv[1] = new char();
	//argvv[2] = new char();
	//argvv[3] = new char();
	//logger.init(4, argvv);

	//skyrimCollection.Load();

	//std::map<std::string, std::set<std::string>> eq_races;
	//std::map<std::string, std::string> test_set;
	//std::set<std::string> bow_set;
	//std::map<size_t, std::set<std::string>> bohhh_set;

	//std::set<std::string> both_hands;
	//std::set<std::string> multiple_sets;

	//for (auto idle_record_it = skyrimCollection.FormID_ModFile_Record.begin(); idle_record_it != skyrimCollection.FormID_ModFile_Record.end(); idle_record_it++)
	//{
	//	Record* record = idle_record_it->second;
	//	if (record->GetType() == REV32(RACE)) {
	//		Sk::RACERecord* race = dynamic_cast<Sk::RACERecord*>(record);
	//		auto unarmed_equip = race->UNES.value;
	//		auto eq_slot = skyrimCollection.FormID_ModFile_Record.find(unarmed_equip);
	//		if (eq_slot == skyrimCollection.FormID_ModFile_Record.end())
	//		{
	//			__debugbreak();
	//		}
	//		bool left = false;
	//		bool right = false;
	//		for (auto& slot_formid : race->QNAM.value)
	//		{
	//			auto slot_it = skyrimCollection.FormID_ModFile_Record.find(slot_formid);
	//			if (slot_it == skyrimCollection.FormID_ModFile_Record.end())
	//			{
	//				__debugbreak();
	//			}
	//			Record* record = slot_it->second;
	//			
	//			if (record->GetType() == REV32(EQUP))
	//			{
	//				Sk::EQUPRecord* equip = dynamic_cast<Sk::EQUPRecord*>(record);
	//				if (std::string(equip->EDID.value) == "LeftHand")
	//				{
	//					left = true;
	//				}
	//				else if (std::string(equip->EDID.value) == "RightHand")
	//				{
	//					right = true;
	//				}
	//			}
	//		}

	//		std::string project = race->behaviors.maleGraph.value[0].MODL.value;
	//		std::string project_name = std::experimental::filesystem::path(project).filename().replace_extension("").string();

	//		if (left && right)
	//		{
	//			both_hands.insert(project_name);
	//		}
	//		if (CountVNAM(race->VNAM.value) > 1)
	//		{
	//			multiple_sets.insert(project_name);
	//		}

	//		//
	//		//if (isTestSet(project_name))
	//		//	test_set[project_name] = VNAMtoString(race->VNAM.value);

	//		//bohhh_set[race->QNAM.value.size()].insert(project);

	//		//if ((race->VNAM.value & 0x00000080 == 0x00000080) || (race->VNAM.value & 0x00001000 == 0x00001000) &&
	//		//	(race->VNAM.value & 0x00000001 == 0x00000001))
	//		//	bow_set.insert(project_name);


	//		//eq_races[VNAMtoString(race->VNAM.value)].insert(project);
	//	}
	//}
}
