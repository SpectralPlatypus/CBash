/* ***** BEGIN LICENSE BLOCK *****
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License Version
* 1.1 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
* for the specific language governing rights and limitations under the
* License.
*
* The Original Code is CBash code.
*
* The Initial Developer of the Original Code is
* Waruddar.
* Portions created by the Initial Developer are Copyright (C) 2010
* the Initial Developer. All Rights Reserved.
*
* Contributor(s):
*
* Alternatively, the contents of this file may be used under the terms of
* either the GNU General Public License Version 2 or later (the "GPL"), or
* the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
* in which case the provisions of the GPL or the LGPL are applicable instead
* of those above. If you wish to allow use of your version of this file only
* under the terms of either the GPL or the LGPL, and not to allow others to
* use your version of this file under the terms of the MPL, indicate your
* decision by deleting the provisions above and replace them with the notice
* and other provisions required by the GPL or the LGPL. If you do not delete
* the provisions above, a recipient may use your version of this file under
* the terms of any one of the MPL, the GPL or the LGPL.
*
* ***** END LICENSE BLOCK ***** */
#pragma once
// BaseRecord.h
#include <fstream>//WTM:  Change:  Added
#include <vector>
#include "../Common.h"
#include "../Oblivion/Records/PACKRecord.h"
#include "../Oblivion/Records/DIALRecord.h"
#include "../Skyrim/Records/PACKRecord.h"
#include "../Skyrim/Records/DIALRecord.h"
#include "../Skyrim/Records/DLBRRecord.h"
#include "../Skyrim/SkyrimChunks.h"
#include "../Collection.h"
#include "md5.h"
#include "CELLToLCTNMap.h"

namespace Skyblivion {

	class SkyblivionScript {

		private:
			std::string scriptPrefix;
			std::string scriptName;
			std::string buildName;
			std::string rootScriptBuildDirectory;
			std::string cachedScriptName;
			std::string extends;//WTM:  Change:  Added

		public:

			SkyblivionScript(std::string buildName, 
			std::string scriptPrefix, 
			std::string scriptName, 
			std::string rootScriptBuildDirectory):
			scriptPrefix(scriptPrefix),
			scriptName(scriptName),
			buildName(buildName),
			rootScriptBuildDirectory(rootScriptBuildDirectory) {
				//..
			}
			
			std::string getBuildName() const { return this->buildName; }
			std::string getScriptName() {
				if(this->cachedScriptName.empty()) {
					this->cachedScriptName = getPrefixedName(this->scriptPrefix, this->scriptName);
				}
				return this->cachedScriptName;
			}

			static std::string getPrefixedName(std::string prefix, std::string scriptName)//WTM:  Change:  Added
			{
				std::string nonprefixedName = scriptName;
				std::transform(nonprefixedName.begin(), nonprefixedName.end(), nonprefixedName.begin(), ::tolower);
				std::string newScriptName = prefix + nonprefixedName;
				if (newScriptName.size() > 38) {
					MD5 md5obj;
					char* md5 = md5obj.digestString((char*)nonprefixedName.c_str());
					newScriptName = prefix + std::string(md5);
				}
				return newScriptName;
			}

			std::string getScriptPath() {
				return rootScriptBuildDirectory + "Transpiled/" + this->buildName + "/" + this->getScriptName() + ".psc";
			}

			std::string getBuiltScriptPath () {
				std::string scriptName = this->scriptName;
				std::transform(scriptName.begin(), scriptName.end(), scriptName.begin(), ::tolower);			
				return rootScriptBuildDirectory + "Artifacts/" + this->buildName + "/" + this->getScriptName() + ".pex";
			}

			std::string getExtends()//WTM:  Change:  Added
			{
				if (!extends.empty()) { return extends; }
				std::string path = getScriptPath();
				std::ifstream infile(path);
				std::string firstLine;
				std::getline(infile, firstLine);
				infile.close();
				const std::regex declarationExtendsRegex("ScriptName +[A-Za-z0-9_]+ +extends +([A-Za-z0-9_]+)( |\r|\n)", std::regex::icase);
				std::smatch results;
				std::regex_search(firstLine, results, declarationExtendsRegex);
				extends = results[1].str();
				return extends;
			}
	};

	class SkyblivionConverter {


	private:
		Sk::PACKRecord::PACKPTDA convertTargetType(Ob::PACKRecord &p);
		Sk::PACKRecord::PACKPLDT convertLocationType(Ob::PACKRecord &p);
		Sk::PACKRecord::PACKPDTO convertTopicType(Ob::PACKRecord& p);//WTM:  Added

		/**
		 * Converts a formid from Oblivion's collection to Skyblivion's ESM
		 * It assumes that Oblivion.esm is expanded into 00 Load Order,
		 * and Skyblivion.esm is expanded into 01 Load Order,
		 */
		FORMID convertFormid(FORMID sourceFormid) {

			//			if (sourceFormid == 0x07) {
			//				sourceFormid = 0x14; //Oblivion old player ref to new player ref
			//			}

						//greeting topic
			if (sourceFormid == 0xC8) {
				sourceFormid = 0x010011C8; //Mapping out of masterspace, above 0x120
			}
			//rumors topic
			if (sourceFormid == 0xD7) {
				sourceFormid = 0x010011D7; //Mapping out of masterspace, above 0x120
			}
			//Races
			else if (sourceFormid == 0x907) {
				sourceFormid = 0x13744; //Imperial
			}
			else if (sourceFormid == 0xD43) {
				sourceFormid = 0x13748; //Redguard
			}
			else if (sourceFormid == 0x191C0) {
				sourceFormid = 0x13747; //Orc
			}
			else if (sourceFormid == 0x191C1) {
				sourceFormid = 0x13742; //Dark Elf
			}
			else if (sourceFormid == 0x19204) {
				sourceFormid = 0x13743; //High Elf
			}
			else if (sourceFormid == 0x223C7) {
				sourceFormid = 0x13745; //Khajiit
			}
			else if (sourceFormid == 0x223C8) {
				sourceFormid = 0x13749; //Wood Elf
			}
			else if (sourceFormid == 0x224FC) {
				sourceFormid = 0x13741; //Breton
			}
			else if (sourceFormid == 0x224FD) {
				sourceFormid = 0x13746; //Nord
			}
			else if (sourceFormid == 0x23FE9) {
				sourceFormid = 0x13740; //Argonian
			}
			else if (sourceFormid == 0x38010) {
				sourceFormid = 0x131F0; //Dremora
			}
			else if (sourceFormid == 0x0B) { 
				sourceFormid = 0x3A070; //Skeleton key
			}
			else if (sourceFormid == 0x191) {
				sourceFormid = 0x0100081E; //Welkynd stone has different formid..
			}
			else if (sourceFormid == 0x192) {
				sourceFormid = 0x0002E500; //SoulGemBlack
			}
			else if (sourceFormid == 0x193) {
				sourceFormid = 0x63B27; //Azura's star
			}
			else if (sourceFormid == 0x1208E) {
				sourceFormid = 0x13747; //DarkSeducer not present in code, mapped to orc.. FIX IT as its not proper!
			}
			else if (sourceFormid == 0x1208F) {
				sourceFormid = 0x13747; //GoldenSaint not present in code, mapped to orc.. FIX IT as its not proper!
			}
			else if (sourceFormid == 0x00000011) {
				sourceFormid = 0x01000881; //TES4StolenGoods "Evidence" [CONT:01000881]
			}
			else if (sourceFormid == 0x00000015) {
				sourceFormid = 0x010008A2; //JailPants
			}
			else if (sourceFormid == 0x00000016) {
				sourceFormid = 0x010008A3; //JailShoes
			}
			else if (sourceFormid == 0x00000017) {
				sourceFormid = 0x010008A4; //JailShirt
			}
			else if (sourceFormid == 0x00000136) {//DefaultPlayerSpell "Heal Minor Wounds" [SPEL:00000136]
				sourceFormid = 0x01000819; //TES4Tamriel "Tamriel" [WRLD:01000819]
			}
			else if (sourceFormid == 0x0000003c) {
				sourceFormid = 0x01000819; //TES4Tamriel "Tamriel" [WRLD:01000819]
			}
			else if (
				sourceFormid != 0x00 &&//null
				sourceFormid != 0x07 &&//Player "Prisoner" [NPC_:00000007]
				sourceFormid != 0x0A &&//Lockpick "Lockpick" [MISC:0000000A]
				sourceFormid != 0x0F &&//Gold001 "Gold" [MISC:0000000F]
				sourceFormid != 0x13 &&//CreatureFaction "Creature Faction" [FACT:00000013]
				sourceFormid != 0x14 &&//PlayerRef [PLYR:00000014]
				sourceFormid != 0x37 &&//GameDay [GLOB:00000037]
				sourceFormid != 0x38 &&//GameHour [GLOB:00000038]
				sourceFormid != 0x39 &&//GameDaysPassed [GLOB:00000039]
				sourceFormid != 0x03C810 &&//DRSWoodSingleLight01Close [SNDR:0003C810] (used with DOOR)
				sourceFormid != 0x03C813//DRSWoodSingleLight01Open [SNDR:0003C813] (used with DOOR)
				) {
				sourceFormid = (sourceFormid & 0xFFFFFF) + 0x01000000;
			}

			std::map<std::string, FORMID>* edidMap = new std::map <std::string, FORMID>();
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4defaultplayerspell"), 0x12FCC));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4welkyndstone"), 0x0100081E));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4blacksoulgem"), 0x2E500));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4azurasstar"), 0x63B27));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gamedayspassed"), 0x39));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gamehour"), 0x38));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gameday"), 0x37));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gamemonth"), 0x36));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gameyear"), 0x35));

			//Hardcoded effects from Skyrim
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4effectseff"), 0x14A));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4effectchml"), 0x1EA6A)); //Chameleon is replaced with invisibility
			//edidMap->insert(std::pair<std::string, FORMID>(std::string("effectzwra"), 0x35)); //TODO - Replace with another summon? Probably needs manual script rewriting so that we dont need this stupid "effect"
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4effectrefa"), 0x2F3B9));
			edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4effectinvi"), 0x1EA6A));

			return sourceFormid;

		}

		const std::string rootBuildPath;
		std::map<std::string, uint32_t> *edidMap;
		std::map<std::string, std::map<uint32_t, uint32_t> *> *targetsMapping;
		std::map<FORMID, FORMID>* oldTopicFormIDToNewTopicFormID;//WTM:  Change:  Added
		Collection &oblivionCollection;
		Collection &skyrimCollection;
		std::vector<Record*, std::allocator<Record*>> scripts;
		std::map<Ob::INFORecord*, Sk::INFORecord*> infosToBind;
		std::map<Ob::QUSTRecord*, Sk::QUSTRecord*> qustsToBind;
		std::map<std::string, Script*> qfsToBind;
		std::vector<Sk::SOUNRecord*, std::allocator<Sk::SOUNRecord*>> addedSOUNRecords;//WTM:  Change:  Added

		Record *findRecordByFormidGeneric(FORMID_OR_UINT32 formid);
		Record *findRecordByFormid(FORMID_OR_UINT32 formid, uint32_t type);

	public:

		SkyblivionConverter(Collection &oblivionCollection, Collection &skyrimCollection, const std::string rootPath);

		const std::string ROOT_BUILD_PATH() const { return rootBuildPath; }
		std::string BUILD_TARGET_STANDALONE() { return "Standalone"; };
		std::string BUILD_TARGET_DIALOGUES() { return "TIF"; }
		std::string BUILD_TARGET_PACKAGES() { return"PF";  }
		std::string BUILD_TARGET_QUESTS() { return "QF"; }

		Sk::QUSTOBJECTIVE * convertObjective(Sk::QUSTRecord *skq, Ob::QUSTRecord::QUSTTarget *obTarget, uint32_t newIndex, std::map<FORMID, int>& formIdToAliasId);
		SkyblivionScript getSkyblivionScript(Ob::SCPTRecord* scpt);//WTM:  Change:  Added
		Script* createVirtualMachineScriptBySkyblivionScript(SkyblivionScript skScript);//WTM:  Change:  Added
		Script* createVirtualMachineScriptFor(Ob::SCPTRecord* scpt);
		void convertPACKFromOblivion(TES4File* oblivionMod, TES5File* skyrimMod);
		void convertPACKFromOblivion(Ob::PACKRecord& srcRecord, Sk::PACKRecord& dstRecord);
		std::vector<Sk::SKCondition*> convertCTDAFromOblivion(GENCTDA* srcRecord);
        std::vector <Sk::DIALRecord*> * convertDIALFromOblivion();
        std::vector <Sk::QUSTRecord*> * convertQUSTFromOblivion();
		int getReferenceFormID(Record* record, std::string realPropertyEdid, std::string hardcodedFilename);
		std::string getTIFScriptNameNotExt(Ob::INFORecord* info);
		void bindProperties(Script* script, SkyblivionScript &skScript);
		void bindScriptProperties(std::vector<Sk::DIALRecord*>* dialogueVector, std::vector<Sk::QUSTRecord *>* questVector);

		std::map<std::string, std::map<uint32_t, uint32_t>*>* getTargetsMappingTable();

		std::map<std::string, FORMID> *prepareSkyblivionEdidMap();

		void insertToEdidMap(std::string edid, FORMID formid);
		FORMID findRecordFormidByEDID(std::string edid);
		std::string getEDIDByFormID(FORMID formID);//WTM:  Change:  Added
		
		TES4File *getOblivionFile() { return (TES4File*)this->oblivionCollection.ModFileByName("Oblivion.esm"); }
		TES5File *getSkyrimFile() { return (TES5File*)this->skyrimCollection.ModFileByName("Skyrim.esm"); }
		TES5File *getSkyblivionFile() { return (TES5File*)this->skyrimCollection.ModFileByName("Skyblivion.esm"); }
		TES5File *getGeckFile() { return (TES5File*)this->skyrimCollection.ModFileByName("GECK.esp"); }

		const std::string getTargetsMappingPath() {
			return this->rootBuildPath + "TargetsMapping.txt";//WTM:  Change:  Added .txt
		}

		const std::map<std::string, uint32_t> &getEdidMap() const { return (*this->edidMap); }  
		const std::vector<Record*, std::allocator<Record*>> &getScripts() const { return this->scripts; }

		void addSOUNFromSNDR();//WTM:  Change:  Added
		void insertToOldTopicFormIDToNewTopicFormID(FORMID oldFormID, FORMID newFormID);//WTM:  Change:  Added
		CELLToLCTNMap cellToLCTNMap;//WTM:  Change:  Added
	};

}