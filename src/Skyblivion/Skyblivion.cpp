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
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <iomanip>
#include <regex>

//#include <boost/regex.hpp>
//#include <boost/algorithm/string/join.hpp>
#include <fstream>
#include "../Common.h"
#include "Skyblivion.h"
#include "../Oblivion/Records/DIALRecord.h"
#include "../Oblivion/Records/PACKRecord.h"
#include "../Skyrim/Records/PACKRecord.h"
#include "../Skyrim/VMAD/Script.h"
#include "../Skyrim/VMAD/Property/PropertyInt.h"
#include "../Skyrim/VMAD/Property/PropertyFloat.h"
#include "../Skyrim/VMAD/Property/PropertyObject.h"
#include "../Skyrim/VMAD/Fragment/FragmentQUST.h"
#include "../Skyrim/VMAD/Fragment/FragmentINFO.h"
#include "CELLToLCTNMap.h"
#include "DIALToGLOBMap.h"

#define RUMORS_FORMID 0xD7
#define GOODBYE_FORMID 0xD4
#define STANDALONE_SCRIPT_PREFIX "TES4"
#define TES4_UNDERSCORE_SCRIPT_PREFIX "TES4_"//WTM:  Change:  Added

namespace Skyblivion {

	static inline std::string join(const std::vector<std::string> my_vector, const char* separator) {
		std::ostringstream result;
		if (my_vector.size()) {
			std::copy(my_vector.begin(), my_vector.end() - 1,
				std::ostream_iterator<std::string>(result, separator));
			result << my_vector.back();
		}

		return result.str();
	}

	SkyblivionConverter::SkyblivionConverter(Collection &oblivionCollection, Collection &skyrimCollection, const std::string rootPath):
	rootBuildPath(rootPath),
	oblivionCollection(oblivionCollection),
	skyrimCollection(skyrimCollection)
	{
		this->edidMap = this->prepareSkyblivionEdidMap();
		this->oldTopicFormIDToNewTopicFormID = new std::map <FORMID, FORMID>();
		this->targetsMapping = this->getTargetsMappingTable();
		this->getOblivionFile()->SCPT.pool.MakeRecordsVector(this->scripts);		
	}


	void SkyblivionConverter::insertToEdidMap(std::string edid, FORMID formid)
	{
		this->edidMap->insert(std::pair<std::string,FORMID>(edid,formid));
	}

	FORMID SkyblivionConverter::findRecordFormidByEDID(std::string edid) {
		if (this->edidMap->find(edid) != this->edidMap->end())
			return (*this->edidMap)[edid];

		return NULL;
	}

	std::string SkyblivionConverter::getEDIDByFormID(FORMID formID)//WTM:  Added
	{
		auto it = (*this->edidMap).begin();
		while (it != (*this->edidMap).end())
		{
			if (it->second == formID) { return it->first; }
			it++;
		}
		throw new std::runtime_error("No record with a form ID " + std::to_string(formID) + " could be found.");
	}

	void SkyblivionConverter::insertToOldTopicFormIDToNewTopicFormID(FORMID oldFormID, FORMID newFormID)//WTM:  Added
	{
		this->oldTopicFormIDToNewTopicFormID->insert(std::pair<FORMID, FORMID>(oldFormID, newFormID));
	}

	Record * SkyblivionConverter::findRecordByFormidGeneric(FORMID_OR_UINT32 formid)
	{
		uint32_t count = this->oblivionCollection.FormID_ModFile_Record.count(formid);

		//Not found
		if (count == 0)
		{
			return NULL;
		}

		//Conflicting
		if (count > 1)
		{
			throw new std::runtime_error("Formid conflict when resolving formid via findRecordByFormidGeneric.");
		}

		auto it = this->oblivionCollection.FormID_ModFile_Record.equal_range(formid);

		Record *rec = it.first->second;

		return rec;

	}

	Record *SkyblivionConverter::findRecordByFormid(FORMID_OR_UINT32 formid, uint32_t type)
	{
		
		Record *rec = findRecordByFormidGeneric(formid);

		if (rec == NULL)
		{
			return NULL;
		}

		//Found but not this type - not found
		if (rec->GetType() != type)
		{
			return NULL;
		}

		return rec;

	}

	std::map<std::string, std::map<uint32_t, uint32_t>*>* SkyblivionConverter::getTargetsMappingTable() {
		std::string mappingPath = this->getTargetsMappingPath();
		std::map<std::string, std::map<uint32_t, uint32_t>*>* result = new std::map<std::string, std::map<uint32_t, uint32_t>*>();

		std::map<uint32_t, uint32_t> *currentMap = NULL;

		std::ifstream myFile = std::ifstream(mappingPath);
		std::string line;

		while (std::getline(myFile, line)) {
			if (line.substr(0, 2) == "qf") { //A bit hacky :/
				currentMap = new std::map<uint32_t, uint32_t>();

				std::stringstream ss(line);
				std::string segment;
				std::vector<std::string> seglist;

				while (std::getline(ss, segment, '_'))
				{
					seglist.push_back(segment);
				}


				if (seglist.size() < 2) {
					continue;
				}

				result->insert(std::pair<std::string, std::map<uint32_t, uint32_t>*>(seglist[1], currentMap));
			}
			else {
				if (currentMap != NULL) { //Indicates a bug if null
					std::stringstream ss(line);
					std::string segment;
					std::vector<uint32_t> seglist;
					uint32_t originalObjectiveIndex;
					std::istringstream duplicatedObjectiveIndexes;
					//TODO - Refactor to consistent space , not space and tabs -_-
					bool first = true;
					bool isOk = false;
					while (std::getline(ss, segment, ' '))
					{
						if (first) {
							originalObjectiveIndex = std::stoi(segment);
							first = false;
						}
						else {
							duplicatedObjectiveIndexes = std::istringstream(segment);
							isOk = true;
						}
					}

					if (!isOk) {
						continue;
					}

					while (std::getline(duplicatedObjectiveIndexes, segment, '	')) {
						currentMap->insert(std::pair<uint32_t, uint32_t>(std::stoi(segment), originalObjectiveIndex)); //Reverse index the data, i.e. new objective index => index to use data from
					}

				}
			}
		}

		return result;

	}

	//WTM:  Change:  Added
	SkyblivionScript SkyblivionConverter::getSkyblivionScript(Ob::SCPTRecord* scpt)	{
		std::string edid = std::string(scpt->EDID.value);
		SkyblivionScript skScript = SkyblivionScript(this->BUILD_TARGET_STANDALONE(), STANDALONE_SCRIPT_PREFIX, edid, this->ROOT_BUILD_PATH());
		return skScript;
	}

	//WTM:  Change:  Added
	Script* SkyblivionConverter::createVirtualMachineScriptBySkyblivionScript(SkyblivionScript skScript) {
		std::string scriptName = skScript.getScriptName();
		Script* theScript = new Script();

		theScript->name.Copy(const_cast<char*>(scriptName.c_str()));

		try {
			bindProperties(theScript, skScript);
		}
		catch (std::exception ex) {
			log_warning << ex.what() << "\n";
		}

		return theScript;

	}

	//WTM:  Change:  Modified to call getSkyblivionScript and createVirtualMachineScriptBySkyblivionScript
    Script* SkyblivionConverter::createVirtualMachineScriptFor(Ob::SCPTRecord* scpt) {
		SkyblivionScript skScript = getSkyblivionScript(scpt);
		return createVirtualMachineScriptBySkyblivionScript(skScript);
    }

	std::map<std::string, FORMID> *SkyblivionConverter::prepareSkyblivionEdidMap() {
		std::map<std::string, FORMID> *edidMap = new std::map <std::string, FORMID>();
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4lockpick"), 0xA));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4daskeletonkey"), 0x3A070));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4repairhammer"), 0xC));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4gold001"), 0xF));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4stolengoods"), 0x01000881));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4jailpants"), 0x010008A2));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4jailshoes"), 0x010008A3));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4jailshirt"), 0x010008A4));
		edidMap->insert(std::pair<std::string, FORMID>(std::string("tes4tamriel"), 0x01000819));
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
		//edidMap->insert(std::pair<std::string, FORMID>(std::string("effectslnc"), 0x35)); //Illusion removed and so was silencing.

		TES5File *geckFile = this->getGeckFile();
		FormIDResolver collapser(geckFile->FormIDHandler.CollapseTable, geckFile->FormIDHandler.FileStart, geckFile->FormIDHandler.FileEnd);
    
		/**
		 * Edid map in SkyblivionConverter is collapsed automatically into Geck context
		 * As all references in fields ought to be collapsed.
		 */
		for (auto it = this->skyrimCollection.EDIDIndex.begin(); it != this->skyrimCollection.EDIDIndex.end(); ++it) {
			std::string edid = std::string(it->first);
			std::transform(edid.begin(), edid.end(), edid.begin(), ::tolower);
			FORMID theFormid = it->second->formID;
			collapser.Accept(theFormid);
			edidMap->insert(std::pair<std::string, FORMID>(edid, theFormid));
		}

		return edidMap;
	}


	bool evaluateGetStageCondition(Sk::SKCTDA* skCTDA, uint32_t stage)
	{
		bool internalTargetConditionResult;

		//uint32_t ctdaStage = (int)a;
		/**
		 * Cast it to real float, and then use a static cast to uint
		 * Doh, fucked up FORMID_OR_FLOAT32 ..
		 */
		uint32_t ctdaStage = (uint32_t)(reinterpret_cast<float&>(skCTDA->compValue));

		switch ((skCTDA->operType >> 5)) {

			case 0: {
				internalTargetConditionResult = (ctdaStage == stage);
				break;
			}

			case 1: {
				internalTargetConditionResult = (ctdaStage != stage);
				break;

			}

			case 2: {
				internalTargetConditionResult = (stage > ctdaStage);
				break;

			}

			case 3: {
				internalTargetConditionResult = (stage >= ctdaStage);
				break;

			}

			case 4: {
				internalTargetConditionResult = (stage < ctdaStage);
				break;

			}

			case 5: {
				internalTargetConditionResult = (stage <= ctdaStage);
				break;

			}

			default: {
				throw new std::runtime_error("Unknown target operator?");
			}

		}

		return internalTargetConditionResult;
	}

	/**
	 * Convert an Oblivion target to Skyrim objective
	 * Receives a target index ( called newIndex ), because while mapping targets to objectives is one thing, we need to duplicate some of the targets under new indexes
	 */
	Sk::QUSTOBJECTIVE * SkyblivionConverter::convertObjective(Sk::QUSTRecord *skq, Ob::QUSTRecord::QUSTTarget *obTarget, uint32_t newIndex, std::map<FORMID, int>& formIdToAliasId) {
		Sk::QUSTOBJECTIVE* objective = new Sk::QUSTOBJECTIVE();
		objective->QOBJ.value = newIndex;
		objective->FNAM.value = 0;
		std::string nnamValue = "Complete objective ";

		char* castedInteger = new char[10];
		sprintf(castedInteger, "%i", newIndex);
		nnamValue.append(castedInteger);
		delete castedInteger;

		char* datacopy = new char[nnamValue.size() + 1];
		strcpy(datacopy, nnamValue.c_str());
		objective->NNAM.value = datacopy;

		Sk::QUSTCONDITIONALTARGET *target = new Sk::QUSTCONDITIONALTARGET();
		target->QSTA.value.flags = obTarget->QSTA.value.flags;

		int aliasId;
		//Alias construction                
		if (formIdToAliasId.find(obTarget->QSTA.value.targetId) == formIdToAliasId.end()) {
			int ANAM = formIdToAliasId.size() + 1; // Next ANAM
			formIdToAliasId[obTarget->QSTA.value.targetId] = ANAM;
			Sk::QUSTALIAS *alias = new Sk::QUSTALIAS();
			alias->aliasID.value = ANAM;
			alias->isLocation = false;

			std::string alidValue = "Alias_";

			//WTM:  Change:  I'm trying to use editor ID instead of form ID.
			//It didn't work.  GECK.esp properties didn't show up properly.  In Creation Kit, when moused over, the tooltip said, "Status: <unknown>."
			//Creation Kit also showed a Flags value of "<Unknown: 0>."
			/*Record* record = findRecordByFormidGeneric(obTarget->QSTA.value.targetId);
			char* edid = record->GetEditorIDKey();
			if (edid != NULL)
			{
				alidValue.append(edid);
			}
			else
			{*/
				char* castedAlidValue = new char[9];
				sprintf(castedAlidValue, "%08x", obTarget->QSTA.value.targetId);
				alidValue.append(castedAlidValue);
				delete castedAlidValue;
			//}
			alidValue.append("_p"); //Todo - maybe we'll remove it from here and from ScriptsConverter too



			char* alidCopy = new char[alidValue.size() + 1];
			strcpy(alidCopy, alidValue.c_str());
			alias->ALID.value = alidCopy;

			Sk::ALFRAliasFillType* aliasType = new Sk::ALFRAliasFillType();
			aliasType->ALFR.value = convertFormid(obTarget->QSTA.value.targetId);
			alias->aliasType = aliasType;
			skq->questAliases.value.push_back(alias);
			aliasId = ANAM;
		}
		else {
			aliasId = (*formIdToAliasId.find(obTarget->QSTA.value.targetId)).second;
		}

		target->QSTA.value.targetAlias = aliasId;

		/**
		* Convert the target conditions
		* GetStage conditions will be moved to skyrimTargetConditions, otherwise they will be pushed directly to the converted Skyrim target
		* We also do a bit of validation, as we're not able to use this magic we do here to convert a GetStage that ORs with non GetStage conditions ( lets say, GetStage(70) OR InCell ...)
		*/
		for (uint8_t u = 0; u < obTarget->CTDA.value.size(); ++u) {

			try {
				std::vector<Sk::SKCondition*> skCTDAs = convertCTDAFromOblivion(obTarget->CTDA.value[u]);
				Sk::SKCondition* skCTDA = skCTDAs.back(); //GetStage should be the last one in case we pushed the condition..

				if (skCTDA != NULL) {
					if (skCTDA->CTDA.value.ifunc == 58 && skCTDA->CTDA.value.param1 == skq->formID) { //is GetStage, skip.							

																												   /**
																												   * If this condition is ORed and next condition is not a getstage aswell, we raise it as it means we cannot convert it properly :(
																												   */
						if (skCTDA->CTDA.value.IsOr()) {
							if ((size_t)((u + 1)) < obTarget->CTDA.value.size()) {
								if (obTarget->CTDA.value[u + 1]->ifunc != 58) {
									char buffix[16];
									sprintf(buffix, "%x", obTarget->CTDA.value[u + 1]->ifunc);
									log_warning << "GetStage ORed with non GetStage on quest " << std::string(skq->EDID.value) << " ifunc is 0x" << std::string(buffix) << std::endl;
								}
							}
						}

					}
					else {
						for (int ctdaIt = 0; ctdaIt < skCTDAs.size(); ++ctdaIt) {
							target->CTDA.value.push_back(skCTDAs[ctdaIt]);
						}
					}
				}


			}
			catch (std::exception) {
				continue;
			}

		}

		objective->TGTS.value.push_back(target);
		return objective;
	}

	std::vector <Sk::QUSTRecord*> * SkyblivionConverter::convertQUSTFromOblivion() {
		std::vector<Record*, std::allocator<Record*>> scripts;
		std::vector<Record*, std::allocator<Record*>> quests;
		TES4File* d = (TES4File *)this->oblivionCollection.ModFiles[0];
		d->SCPT.pool.MakeRecordsVector(scripts);
		d->QUST.pool.MakeRecordsVector(quests);

        TES5File* skyrimMod = (TES5File*)this->skyrimCollection.ModFiles[2];

        std::vector <Sk::QUSTRecord*> * questsVector = new std::vector<Sk::QUSTRecord *>;

		for(uint32_t questsIt = 0; questsIt < quests.size(); ++questsIt) {
			Ob::QUSTRecord *q = (Ob::QUSTRecord *)quests[questsIt];
            Sk::QUSTRecord* skq = new Sk::QUSTRecord();
            skq->formID = convertFormid(q->formID);
            skq->formVersion = 43; //Idk why but without this scripts do not show up
            std::string skqEDID = "TES4";


            skqEDID.append(q->EDID.value);
            char* cpy = new char[skqEDID.size() + 1];
            strcpy(cpy, skqEDID.c_str());
            skq->EDID.value = cpy;

			skq->VMAD.value = new VMADRecord();

			if (q->SCRI.IsLoaded())
				qustsToBind[q] = skq;


            Script* QFScript = new Script();
            skq->VMAD.value->scripts.push_back(QFScript);
            skq->FULL.value = q->FULL.value;

            if (q->ICON.value == "Quests\\SE07Decanter.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSEMisc.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE07.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE03.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE11.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE08.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE04.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE09.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE02.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE06.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE01.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE13.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE05.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE12.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quests\\iconSE10.dds") {
                skq->DNAM.value.questType = 2;
            }
            else if (q->ICON.value == "Quest\\icon_thieves_guild.dds") {
                skq->DNAM.value.questType = 3;
            }
            else if (q->ICON.value == "Quest\\icon_tiber_septim's_armor.dds") {
                skq->DNAM.value.questType = 8;
            }
            else if (q->ICON.value == "Quest\\icon_master_trainer.dds") {
                skq->DNAM.value.questType = 8;
            }
            else if (q->ICON.value == "Quest\\icon_settlements_quest_icon.dds") {
                skq->DNAM.value.questType = 8;
            }
            else if (q->ICON.value == "Quest\\icon_arena.dds") {
                skq->DNAM.value.questType = 8;
            }
            else if (q->ICON.value == "Quest\\icon_miscellaneous.dds") {
                skq->DNAM.value.questType = 8;
            }
            else if (q->ICON.value == "Quest\\icon_mages_guild.dds") {
                skq->DNAM.value.questType = 3;
            }
            else if (q->ICON.value == "Quest\\icon_dark_brotherhood.dds") {
                skq->DNAM.value.questType = 4;
            }
            else if (q->ICON.value == "Quest\\icon_fighters_guild.dds") {
                skq->DNAM.value.questType = 5;
            }
            else if (q->ICON.value == "Quest\\icon_daedra.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_oblivion_gate.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_miscarcand.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_bruma.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_mythic_dawn.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_imperial_city.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_weynon_priory.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_kvatch.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_amulet_of_king.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_paradise.dds") {
                skq->DNAM.value.questType = 7;
            }
            else if (q->ICON.value == "Quest\\icon_dragonfires.dds") {
                skq->DNAM.value.questType = 7;
            }

            skq->DNAM.value.encounterFlags = q->DATA.value.flags & (0x9);
            skq->DNAM.value.generalFlags = 0x01;
            skq->DNAM.value.priority = q->DATA.value.priority;


            for (uint8_t i = 0; i < q->CTDA.value.size(); ++i) {
                try {
					GENCTDA* ctda = q->CTDA.value[i];
					//Subject.GetIsPlayableRace(None) == 1
					bool isSubject_GetIsPlayableRace_Equals_1 = ctda->operType == 0/*Subject*/ && ctda->ifunc == 254/*GetIsPlayableRace*/ && ctda->param1 == 0/*None*/ && ctda->compValue == 0x3f800000/*1 as a float*/;
					if (!isSubject_GetIsPlayableRace_Equals_1)
					{
						std::vector<Sk::SKCondition*>  skCTDAs = convertCTDAFromOblivion(ctda);

						for (int ctdaIt = 0; ctdaIt < skCTDAs.size(); ++ctdaIt) {
							skq->QDCTDA.value.push_back(skCTDAs[ctdaIt]);
						}
					}
                }
                catch (std::exception) {
                    continue;
                }

            }

			/**
			 * QUST target index => Skyrim QUST objective
			 */
			std::map<FORMID, int> formIdToAliasId = std::map<FORMID, int>();

			/**
			 * Convert targets one by one
			 */
			for (uint8_t i = 0; i < q->Targets.value.size(); ++i) {

				struct Ob::QUSTRecord::QUSTTarget *obTarget = q->Targets.value[i];
				Sk::QUSTOBJECTIVE *objective = convertObjective(skq, obTarget, i, formIdToAliasId);
				skq->questObjectives.value.push_back(objective);
			}

			/**
			 * Duplicate the targets as needed
			 */
			std::string lowerName = std::string(q->EDID.value);
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

			std::map<uint32_t, uint32_t> *duplicationTargetMap = (*this->targetsMapping)[lowerName];

			if (duplicationTargetMap != NULL) {
				uint32_t duplicatedId = q->Targets.value.size(); //Next ID index is the actual size of targets array
				while (duplicationTargetMap->count(duplicatedId)) {
					uint32_t basedOnId = duplicationTargetMap->find(duplicatedId)->second;
					struct Ob::QUSTRecord::QUSTTarget *obTarget = q->Targets.value[basedOnId];
					Sk::QUSTOBJECTIVE *objective = convertObjective(skq, obTarget, duplicatedId, formIdToAliasId);
					skq->questObjectives.value.push_back(objective);
					++duplicatedId;
				}
			}

			FragmentQUST* qustFragment = new FragmentQUST();
			qustFragment->unk1 = 0;
			std::string QFScriptName = "QF_";
			QFScriptName.append(q->EDID.value); //Without TES4, perhaps we'll change it at some point.
			QFScriptName.append("_");
			char* castedFormid = new char[9];
			sprintf(castedFormid, "%08x", q->formID + 0x01000000);
			QFScriptName.append(castedFormid);
			QFScriptName = SkyblivionScript::getPrefixedName(TES4_UNDERSCORE_SCRIPT_PREFIX, QFScriptName);//WTM:  Change:  Added
			delete castedFormid;

			char *QFScriptNameChar = (char *)QFScriptName.c_str();

			QFScript->name.Copy(QFScriptNameChar);
			qustFragment->fileName.Copy(QFScriptNameChar);

			/**
			 * Convert the quest stages
			 */
            for (uint8_t i = 0; i < q->Stages.value.size(); ++i) {
                Sk::QUSTSTAGE* stage = new Sk::QUSTSTAGE();
				int16_t stageId = q->Stages.value[i]->INDX.value;
				stage->INDX.value.journalIndex = stageId;
                stage->INDX.value.flags = 0;
                stage->INDX.value.unknown = 0;

				/**
				 * Convert quest stage's entries
				 */
                for (uint8_t k = 0; k < q->Stages.value[i]->Entries.value.size(); ++k) {
                    Sk::QUSTLOGENTRY* entry = new Sk::QUSTLOGENTRY();
                    entry->QSDT = q->Stages.value[i]->Entries.value[k]->QSDT;
					uint32_t stageIndex = k;

                    if (q->Stages.value[i]->Entries.value[k]->CNAM.value != NULL) {
                        char* cnamValue = new char[strlen(q->Stages.value[i]->Entries.value[k]->CNAM.value) + 1];
                        strcpy(cnamValue, q->Stages.value[i]->Entries.value[k]->CNAM.value);
                        entry->CNAM.value = cnamValue;
                    }

					FragmentQUST::Fragment* fragment = new FragmentQUST::Fragment();


					char* castedInt = new char[10];
					sprintf(castedInt, "%i", stageId);
					std::string fragmentName = "Fragment_" + std::string(castedInt);
					delete castedInt;

					if (stageIndex != 0) {
						char* castedInt = new char[10];
						sprintf(castedInt, "%i", stageIndex);
						fragmentName = fragmentName + "_" + std::string(castedInt);
						delete castedInt;

					}

					char* cpyFragmentName = new char[fragmentName.size() + 1];
					strcpy(cpyFragmentName, fragmentName.c_str());
					fragment->fragmentName.Copy(const_cast<char *>(fragmentName.c_str()));
					fragment->unk1 = 0;
					fragment->unk2 = 1;
					fragment->stage = stageId;
					fragment->stageIndex = stageIndex;
					unsigned char* pointer = reinterpret_cast<unsigned char*>(QFScript->name.value);
					fragment->scriptName.Read(pointer, QFScript->name.GetSize(), false);
					qustFragment->fragments.push_back(fragment);


                    for (uint8_t u = 0; u < q->Stages.value[i]->Entries.value[k]->CTDA.value.size(); ++u) {
                        
                        try {
                            std::vector<Sk::SKCondition*> skCTDAs = convertCTDAFromOblivion(q->Stages.value[i]->Entries.value[k]->CTDA.value[u]);
							for (int ctdaIt = 0; ctdaIt < skCTDAs.size(); ++ctdaIt) {
								entry->CTDA.value.push_back(skCTDAs[ctdaIt]);
							}
                        }
                        catch (std::exception) {
                            continue;
                        }
                    }

                    stage->LOGS.value.push_back(entry);
                }

                skq->questStages.value.push_back(stage);

            }


            skq->ANAM.value = formIdToAliasId.size() + 1; //Calculate ANAM
			skq->VMAD->fragment = qustFragment;





			/**
			 * Bind the aliases
			 */
            for (uint8_t it = 0; it < skq->questAliases.value.size(); ++it) {
                PropertyObject* vmadProperty = new PropertyObject();
                unsigned char* pointer = reinterpret_cast<unsigned char*>(skq->questAliases.value[it]->ALID.value);
                vmadProperty->name.Read(pointer, skq->questAliases.value[it]->ALID.GetSize(), false);
                vmadProperty->type = 1;
                vmadProperty->aliasId = skq->questAliases.value[it]->aliasID.value;
                vmadProperty->formId = skq->formID;
				vmadProperty->status = 0;
                QFScript->properties.push_back(vmadProperty);
            }

			qfsToBind[QFScriptName] = QFScript;

            questsVector->push_back(skq);

        }

		return questsVector;

    }

	std::vector <Sk::DIALRecord*> * SkyblivionConverter::convertDIALFromOblivion() {

		struct dialogueToConvert {
			bool isMapped;
			Ob::DIALRecord *dial;
			std::map<FORMID, FORMID> *qstiToFormid;
		};

        std::vector<Record*, std::allocator<Record*>> dialogues;
        std::vector<Sk::DLBRRecord*> branchVector;
        std::vector<Sk::DIALRecord*> * dialogueVector = new std::vector<Sk::DIALRecord *>();

		TES4File* oblivionFile = this->getOblivionFile();
        ModFile* skyblivionFile = this->getSkyblivionFile();
		TES5File* geckFile = this->getGeckFile();
		oblivionFile->DIAL.dial_pool.MakeRecordsVector(dialogues);

		std::vector<Record*, std::allocator<Record*>> qustVector;
		std::map<FORMID, Ob::QUSTRecord*> quests;
		std::vector<Record*, std::allocator<Record*>> scripts;
		oblivionFile->SCPT.pool.MakeRecordsVector(scripts);
		oblivionFile->QUST.pool.MakeRecordsVector(qustVector);


		/**
		* Map to a map to have fast lookup
		*/
		for (uint32_t qustVectorIt = 0; qustVectorIt < qustVector.size(); ++qustVectorIt) {
			Ob::QUSTRecord *qust = (Ob::QUSTRecord*)qustVector[qustVectorIt];
			quests[qust->formID] = qust;
		}

        std::vector<FORMID> nonTopLevelDialogs = std::vector<FORMID>();
        int i = 0;
		//Because out of one Oblivion DIAL there might be N Skyrim DIALs, we cannot simply use DIAL formids. We need to map them
		//This map has all formids within the conversion process, however, if formid is not mapped ( i.e. there's one quest and we dont need to split the record ), then we'll save the same formid as original
		std::map<FORMID, dialogueToConvert *> formidMapping = std::map<FORMID, dialogueToConvert *>();
		DIALToGLOBMap dialToGLOBMap = DIALToGLOBMap();
		for (uint32_t dialoguesIt = 0; dialoguesIt < dialogues.size(); ++dialoguesIt) {
			Ob::DIALRecord *d = (Ob::DIALRecord*)dialogues[dialoguesIt];
		    try {

                if (d->EDID.value == NULL) {
                    //empty edid crashing conversion, skip
                    continue;
                }

				insertToEdidMap(d->EDID.value, d->formID);//WTM:  Change:  Some (possibly all) DIAL records don't get added, which causes problems with property association later on.

                if (d->DATA.value == 0x03) {
                    continue; //We do not convert persuasion tab which is non existing in skyrim
                }
                else if (d->DATA.value == 0x05 && strcmp(d->EDID.value, "Repair") != 0 && strcmp(d->EDID.value, "Training") != 0 && strcmp(d->EDID.value, "ServiceRefusal") != 0 && strcmp(d->EDID.value, "Recharge") != 0) {
                    continue; //From service tab,w e convert repair/training/recharge/ServiceRefusal only, barter is skipped
                }
                else if (strcmp(d->EDID.value, "InfoRefusal") == 0) { //Do not convert those records. @TODO - What to do with HELLOs ( on-approach-to-npc-texts ) since in Skyrim greetings are shared on approach and when actually talking??
                    continue;
                }
				else if (d->INFO.size() == 0) {
					//Skip dialogues without responses..
					//NOTE - Not exactly accurate. What we should be checking is whether - after splitting into N Skyrim dialogues, which ones might not have INFO references and discard there, instead of discarding here.
					//This will fix only the cases where we lack the INFOs altogether.
					continue;
				}

                FORMID firstQuestFormid = NULL;
				FORMID dialogueToConvert;
				struct dialogueToConvert *map = new struct dialogueToConvert();
				map->isMapped = false;
				map->dial = d;
				map->qstiToFormid = new std::map<FORMID, FORMID>();
                for (uint16_t i = 0; i < d->INFO.size(); i++) {
                    //We're searching thru INFO's to find the referenced dials.
                    Ob::INFORecord* info = (Ob::INFORecord*)d->INFO[i];

					for (uint16_t k = 0; k < info->TCLT.value.size(); k++) {
						if (info->TCLT.value[k] == RUMORS_FORMID) {
							//Do not treat linking to Rumours as actually qualifying for non top level branches.
							//Reason is that in Skyblivion, we drop this connection and actually force a goodbye.
							continue;
						}

						if (std::find(nonTopLevelDialogs.begin(), nonTopLevelDialogs.end(), info->TCLT.value[k]) == nonTopLevelDialogs.end()) {
							nonTopLevelDialogs.push_back(info->TCLT.value[k]);
						}                    
					}


					if (firstQuestFormid == NULL) {
						//First quest found - let's just put it there
						firstQuestFormid = info->QSTI.value;
					}
					else {
						//We found another one, so we will need to split
						if (info->QSTI.value != firstQuestFormid ) {
							//Assign a new formid
							if (!map->isMapped) {
								//Map the current questFormid aka the first quest
								dialogueToConvert = this->skyrimCollection.NextFreeExpandedFormID(skyblivionFile);
								map->qstiToFormid->insert(std::pair<FORMID, FORMID>(firstQuestFormid, dialogueToConvert));
								map->isMapped = true; //To avoid regeneration for 3 and more quests :)
							}

							//And the current one if its needed
							if (map->qstiToFormid->find(info->QSTI.value) == map->qstiToFormid->end()) {
								dialogueToConvert = this->skyrimCollection.NextFreeExpandedFormID(skyblivionFile);
								map->qstiToFormid->insert(std::pair<FORMID, FORMID>(info->QSTI.value, dialogueToConvert));
							}
							
						}
					}
                }

				formidMapping.insert(std::pair<FORMID,struct dialogueToConvert *>(d->formID, map));

            }
            catch (std::exception &e) {
				log_error << e.what() << "\n";
                continue;
            }

            ++i;
        }

		for (std::map<FORMID, struct dialogueToConvert *>::iterator it = formidMapping.begin(); it != formidMapping.end(); ++it) {

			struct dialogueToConvert *dialogueToConvertData = it->second;
			Ob::DIALRecord *d = dialogueToConvertData->dial;

            std::map<FORMID, Sk::DIALRecord*> formidToPointer = std::map<FORMID, Sk::DIALRecord*>();

			/**
			 * Used to track blocking branches, aka GREETINGs with 
			 */
			std::map<FORMID, Sk::DIALRecord*> blockingFormidToPointer = std::map<FORMID, Sk::DIALRecord*>();
			
			Sk::DIALRecord* currentDialForInfo;

			FORMID conditionGlobalVariableFormID = dialToGLOBMap.getGLOBFormID(d->formID);

			for(uint32_t infoIt = 0; infoIt < d->INFO.size(); ++infoIt) {

				Ob::INFORecord* info = (Ob::INFORecord*)d->INFO[infoIt];
				std::map<FORMID, Sk::DIALRecord*> *targetShardingCollection = &formidToPointer;
				bool isInfoBlocking = false;
				uint32_t presetInfoFlags = 0;
				Sk::INFORecord* newInfo = new Sk::INFORecord();

				/**
				 * GREETING dialogues with choices are transformed into blocking branches
				 */
				if (strcmp(d->EDID.value, "GREETING") == 0) {

					if (info->TCLT.value.size() > 0) {
						targetShardingCollection = &blockingFormidToPointer;
						isInfoBlocking = true;
						//WTM:  Change:  I commented the below line as requested by jadkai.
						//presetInfoFlags |= 0x2; //We will want a random on this, because HELO has native randomization of greetings, however, custom branches have to be randomized explicitly.
					}

					/**
					 * Push the condition that will make it in-dialogue-only
					 */
					Sk::SKCTDA*  inDialogueOnly = new Sk::SKCTDA();
					inDialogueOnly->operType = (0 << 5) + 0;
					inDialogueOnly->compValue = 0x3f800000; //FLOAT 1
					inDialogueOnly->ifunc = Sk::SKCTDA::fn_IsInDialogueWithPlayer; //IsInDialogueWithPlayer
					inDialogueOnly->param1 = 0;
					inDialogueOnly->param2 = 0;
					inDialogueOnly->runOnType = 0; //Subject
					inDialogueOnly->reference = 0;

					Sk::SKCondition *inDialogueOnlyAggr = new Sk::SKCondition();
					inDialogueOnlyAggr->CTDA.value = *inDialogueOnly;

					newInfo->CTDA.value.push_back(inDialogueOnlyAggr);
				}
				else if (strcmp(d->EDID.value, "HELLO") == 0) {

					/**
					 * Push the condition that will make it non-dialogue-only
					 */
					Sk::SKCTDA*  nonDialogueOnly = new Sk::SKCTDA();
					nonDialogueOnly->operType = (0 << 5) + 0;
					nonDialogueOnly->compValue = 0;
					nonDialogueOnly->ifunc = Sk::SKCTDA::fn_IsInDialogueWithPlayer; //IsnonDialogueWithPlayer
					nonDialogueOnly->param1 = 0;
					nonDialogueOnly->param2 = 0;
					nonDialogueOnly->runOnType = 0; //Subject
					nonDialogueOnly->reference = 0;
					Sk::SKCondition *nonDialogueOnlyAggr = new Sk::SKCondition();
					nonDialogueOnlyAggr->CTDA.value = *nonDialogueOnly;

					newInfo->CTDA.value.push_back(nonDialogueOnlyAggr);
					newInfo->CTDA.value.push_back(nonDialogueOnlyAggr);
				}

                if (targetShardingCollection->find(info->QSTI.value) == targetShardingCollection->end()) {	//there is no formid with that quest, so we create a new branch and topic for it.

					bool formidWasMapped = (dialogueToConvertData->isMapped);

                    FORMID dialFormid;

					if (isInfoBlocking) {
						/**
						 * If INFO is blocking, then we assume that we can generate a new FORMID. Those ( blocking ) branches should not be referenced in dialogues anyways, so probably all TCLT choices
						 * to "GREETING"s ( whatever that might be ), will probably end up to mapped non-blocking branches.
						 * It's complicated.
						 */
						dialFormid = this->skyrimCollection.NextFreeExpandedFormID(skyblivionFile);
					}
                    else if (formidWasMapped) {
						dialFormid = (*dialogueToConvertData->qstiToFormid)[info->QSTI.value];
                    }
                    else {
                        dialFormid = convertFormid(d->formID);
                    }

                    Sk::DIALRecord* newDialogue = new Sk::DIALRecord();
                    newDialogue->formID = dialFormid;

                    std::string DEDID = std::string();
                    DEDID = "TES4";
                    DEDID.append(d->EDID.value);

                    if (formidWasMapped) {
                        DEDID.append("_");
                        char buffer[16];
						sprintf(buffer, "%x", info->QSTI.value);
                        DEDID.append(buffer);
                    }

					if (isInfoBlocking) {
						DEDID.append("_Block");
					}

                    char *dstr = new char[DEDID.length() + 1];
                    strncpy(dstr, DEDID.c_str(), DEDID.length() + 1);

					Ob::QUSTRecord *questForDialogue = quests[info->QSTI.value];

					newDialogue->formVersion = 43;
                    newDialogue->EDID.value = dstr;
                    newDialogue->FULL = d->FULL;
					newDialogue->PNAM.value = questForDialogue->DATA.value.priority;
                    newDialogue->QNAM.value = convertFormid(info->QSTI.value);
					this->insertToOldTopicFormIDToNewTopicFormID(d->formID, dialFormid);

					bool needABranch = false;

                    //Conversations mapping to subtypes
                    if (strcmp(d->EDID.value, "GREETING") == 0) {

						/**
						 * In Oblivion, if a Greeting has links, that means that we're actually forcing topics from that greeting.
						 * To implement same behavior in Skyrim, we need to creating a blocking branch.
						 * GREETING will actually become a CUSTOM topic, and branch will be blocking.
						 */

						if (info->TCLT.value.size() > 0) {
							newDialogue->SNAM.value = REV32(CUST);
							newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom				
							needABranch = true;
						}
						else {
							newDialogue->SNAM.value = REV32(HELO);
							newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
						}

                    }
					else if (strcmp(d->EDID.value, "HELLO") == 0) {
						//Will be triggered only out of dialogues, as on-approach.
						newDialogue->SNAM.value = REV32(HELO);
						newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
					}
                    else if (strcmp(d->EDID.value, "GOODBYE") == 0) {
                        newDialogue->SNAM.value = REV32(GBYE);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "AcceptYield") == 0) {
                        newDialogue->SNAM.value = REV32(ACYI);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Assault") == 0) {
                        newDialogue->SNAM.value = REV32(ASSA);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "AssaultNoCrime") == 0) {
                        newDialogue->SNAM.value = REV32(ASNC);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Attack") == 0) {
                        newDialogue->SNAM.value = REV32(ATCK);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Flee") == 0) {
                        newDialogue->SNAM.value = REV32(FLEE);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Hit") == 0) {
                        newDialogue->SNAM.value = REV32(HIT_);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Murder") == 0) {
                        newDialogue->SNAM.value = REV32(MURD);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "MurderNoCrime") == 0) {
                        newDialogue->SNAM.value = REV32(MUNC);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Pickpocket") == 0) {
                        newDialogue->SNAM.value = REV32(PICC); //Might be PICT
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "PickpocketNoCrime") == 0) {
                        newDialogue->SNAM.value = REV32(PICN);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "PowerAttack") == 0) {
                        newDialogue->SNAM.value = REV32(POAT);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Steal") == 0) {
                        newDialogue->SNAM.value = REV32(STEA);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "StealNoCrime") == 0) {
                        newDialogue->SNAM.value = REV32(STFN);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Trespass") == 0) {
                        newDialogue->SNAM.value = REV32(TRES);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "TrespassNoCrime") == 0) {
                        newDialogue->SNAM.value = REV32(TRAN);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Yield") == 0) {
                        newDialogue->SNAM.value = REV32(YIEL);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }

                    //Persuasion tab omitted

                    //Detection tab
                    else if (strcmp(d->EDID.value, "Lost") == 0) {
                        newDialogue->SNAM.value = REV32(ALTN); //might be LostToNormal, but i think alert is better
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Noticed") == 0) {
                        newDialogue->SNAM.value = REV32(NOTA);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Seen") == 0) {
                        newDialogue->SNAM.value = REV32(ALTC);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Unseen") == 0) {
                        newDialogue->SNAM.value = REV32(COLO);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }

                    //Service tab
                    else if (strcmp(d->EDID.value, "Repair") == 0) {
                        newDialogue->SNAM.value = REV32(REPA);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Training") == 0) {
                        newDialogue->SNAM.value = REV32(TRAI); 
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "ServiceRefusal") == 0) {
                        newDialogue->SNAM.value = REV32(SERU); 
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Recharge") == 0) {
                        newDialogue->SNAM.value = REV32(RECH); 
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }

                    //Misc tab
                    else if (strcmp(d->EDID.value, "Corpse") == 0) {
                        newDialogue->SNAM.value = REV32(NOTI);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "Idle") == 0) {
                        newDialogue->SNAM.value = REV32(IDLE);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "ObserveCombat") == 0) {
                        newDialogue->SNAM.value = REV32(OBCO);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else if (strcmp(d->EDID.value, "TimeToGo") == 0) {
                        newDialogue->SNAM.value = REV32(TITG);
                        newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                    }
                    else {

						needABranch = true;
						
                        if (d->QSTI.value.size() > 1) {
                            newDialogue->SNAM.value = REV32(RUMO);
                            newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 2, 0); //Subtype ID Rumors
                        }
                        else {
                            newDialogue->SNAM.value = REV32(CUST);
                            newDialogue->DATA.value = Sk::DIALPACKDATA(false, 0, 0, 0); //Subtype ID Custom
                        }
                    }

					if (needABranch) {
						Sk::DLBRRecord* newBranch = new Sk::DLBRRecord();
						std::string EDID = std::string();
						EDID = "TES4";
						EDID.append(d->EDID.value);


						if (formidWasMapped) {
							EDID.append("_");

							char buffer[16];
							sprintf(buffer, "%x", info->QSTI.value); 
							EDID.append(buffer);
							//delete buffer;
						}

						if (isInfoBlocking) {
							EDID.append("_Block");
						}

						EDID.append("_Branch");

						char *cstr = new char[EDID.length() + 1];
						strncpy(cstr, EDID.c_str(), EDID.length() + 1);
						newBranch->EDID.value = cstr;
						newBranch->formID = this->skyrimCollection.NextFreeExpandedFormID(skyblivionFile);
						newBranch->formVersion = 43;
						newBranch->SNAM.value = dialFormid;
						newBranch->TNAM.value = 0;

						int branchType;

						if (isInfoBlocking) {
							//Blocking branch
							branchType = 2;
						} else {


							/**
							* Conversations should have normal branches.
							* They might be converted to SCENes later, or simply used in scripts..
							* One exception is Rumors
							*/
							if (d->DATA.value == 0x1 && strcmp(d->EDID.value, "INFOGENERAL") != 0) {
								branchType = 0;
							}
							else {
								if (std::find(nonTopLevelDialogs.begin(), nonTopLevelDialogs.end(), d->formID) != nonTopLevelDialogs.end()) {
									//It's not a top level, we set it to normal
									branchType = 0;
								}
								else {
									branchType = 1; //Top level.
								}
							}
						}

						newBranch->DNAM.value = branchType;
						newBranch->QNAM.value = convertFormid(info->QSTI.value);
						newDialogue->BNAM.value = newBranch->formID; //PROBABLY WONT WORK.

						branchVector.push_back(newBranch);
					}

                    newDialogue->TIFC.value = 0;

                    currentDialForInfo = newDialogue;
					targetShardingCollection->insert(std::pair<FORMID, Sk::DIALRecord*>(info->QSTI.value, currentDialForInfo));
                    
                    dialogueVector->push_back(newDialogue);

                }
                else {
                    currentDialForInfo = (*targetShardingCollection)[info->QSTI.value];
                }

                currentDialForInfo->TIFC.value += 1;
                newInfo->formID = convertFormid(info->formID);
				newInfo->formVersion = 43;
                newInfo->CNAM.value = 0;
				newInfo->ENAM.Load();
				newInfo->ENAM.value->flags = presetInfoFlags;

				/**
				 * Convert bit flags
				 */
				//Goodbye
				if (info->DATA.value.flags & 1) {
					newInfo->ENAM.value->flags |= 1;
				}

				//Random
				if (info->DATA.value.flags & 2) {
					newInfo->ENAM.value->flags |= 2;
				}
				//Say Once
				if (info->DATA.value.flags & 4) {
					newInfo->ENAM.value->flags |= 4;
				}
				//Random End
				if (info->DATA.value.flags & 0x20) {
					newInfo->ENAM.value->flags |= 0x10;
				}


				/**
				 * Somehow, in Oblivion Rumors have choice responses like it would be a further dialogue, but its not used. We're replicating that behavior with that if.
				 */
				if (strcmp(d->EDID.value, "INFOGENERAL") != 0) {
					for (uint32_t tcltIt = 0; tcltIt < info->TCLT.value.size(); ++tcltIt) {

						FORMID tcltLink = info->TCLT.value[tcltIt];
						if (tcltLink == GOODBYE_FORMID) {
							//Force a goodbye in case it was linking to Rumor and drop
							newInfo->ENAM.value->flags |= 1;
						}
						else {


							if (formidMapping.find(tcltLink) != formidMapping.end()) {
								struct dialogueToConvert *mapped = formidMapping[tcltLink];
								FORMID newTcltValue = NULL;

								if (mapped->isMapped) {
									if (mapped->qstiToFormid->find(info->QSTI.value) != mapped->qstiToFormid->end()) {
										newTcltValue = (*mapped->qstiToFormid)[info->QSTI.value];
									}
								}
								else {
									newTcltValue = convertFormid(tcltLink);
								}

								if (newTcltValue != NULL) {
									newInfo->TCLT.value.push_back(newTcltValue);
								}
							}
							else {
								log_warning << "Cannot map the TCLT..\n";
							}
						}
					}
				}

				for (uint32_t infoResponseIt = 0; infoResponseIt < info->Responses.value.size(); ++infoResponseIt) {
					Ob::INFORecord::INFOResponse* r = info->Responses.value[infoResponseIt];
					Sk::INFORecordResponse* responselink = new Sk::INFORecordResponse();

                    Sk::PACKTRDT trdtPack = Sk::PACKTRDT();
                    trdtPack.emotionType = r->TRDT.value.emotionType;
                    trdtPack.emotionValue = r->TRDT.value.emotionValue;
                    trdtPack.responseNumber = r->TRDT.value.responseNum;

                    responselink->NAM1 = r->NAM1;
                    responselink->NAM2 = r->NAM2;
                    responselink->NAM3.value = "";
                    responselink->TRDT.value = trdtPack;
                    newInfo->responses.push_back(responselink);

                }

				if (currentDialForInfo->SNAM.value == REV32(HIT_)) {//WTM:  I tried commenting this as oracus0's requested, but crashes remained.
					Sk::SKCondition *condition = new Sk::SKCondition();//Subject.GetIsID(PlayerNPC) == 0
					condition->CTDA->ifunc = Sk::SKCTDA::fn_GetIsID;//GetIsID
					condition->CTDA->operType = 0;//Equals
					condition->CTDA->compValue = 0;
					condition->CTDA->param1 = 0x7;//Player NPC
					condition->CTDA->param2 = 0;
					condition->CTDA->runOnType = 0;//Subject

					newInfo->CTDA.value.push_back(
						condition
						);
				}

                for (uint8_t i = 0; i < info->CTDA.value.size(); ++i) {

                    try {
                        std::vector<Sk::SKCondition*> skCTDAs = convertCTDAFromOblivion(info->CTDA.value[i]);
						for (int ctdaIt = 0; ctdaIt < skCTDAs.size(); ++ctdaIt) {
							Sk::SKCondition* condition = skCTDAs[ctdaIt];
							Sk::SKCTDA ctda = condition->CTDA.value;
							//Target.GetIsID(Player) == 1
							bool isTarget_GetIsID_PlayerNPC_Equals_1 = ctda.runOnType == 1/*Target*/ && ctda.ifunc == 72/*GetIsID*/ && ctda.param1 == 7/*Player NPC*/ && ctda.compValue == 0x3f800000/*1 as a float*/;
							if (!isTarget_GetIsID_PlayerNPC_Equals_1)
							{//At jadkai's request, don't include Target.GetIsID(Player) == 1.
								newInfo->CTDA.value.push_back(condition);
							}
						}
                    }
                    catch (std::exception) {
                        continue;
                    }
                }

				for (uint32_t ctdaIt = 0; ctdaIt < newInfo->CTDA.value.size(); ++ctdaIt) {
					Sk::SKCondition* condition = newInfo->CTDA.value[ctdaIt];
					Sk::SKCTDA ctda = condition->CTDA.value;
					if (ctda.ifunc == 72/*GetIsID*/ && ctda.IsEqual() && ctda.compValue > 0 || !ctda.IsEqual() && ctda.compValue == 0)
					{
						/*
						newInfo->ANAM.value =//Speaker
							//WTM:  Change:
							//formID 0102b47d causes CK to crash upon saving,
							//so I worked around it as Monocleus suggested by
							//not using the value from Oblivion but instead using Todd's Tester Man.
							newInfo->formID == 0x0102b47d ? 0x0100089A :
							ctda->CTDA.value.param1;
						*///WTM:  Change:  jadkai requested Speaker never be set.
					}
				}

				if (conditionGlobalVariableFormID != NULL)
				{
					Sk::SKCTDA* ctda = new Sk::SKCTDA();
					ctda->ifunc = Sk::SKCTDA::fn_GetGlobalValue;//GetGlobalValue
					ctda->param1 = conditionGlobalVariableFormID;
					ctda->operType = (0/*Equals*/ << 5) + 0 /*no other flags*/;
					ctda->compValue = 0x3f800000;//1 as a float
					Sk::SKCondition* condition = new Sk::SKCondition();
					condition->CTDA.value = *ctda;
					newInfo->CTDA.value.insert(newInfo->CTDA.value.begin(), condition);
				}

                //Creating Papyrus fragments
				bool addInfoToBind = false;
				if (info->SCTX.IsLoaded())
				{
					addInfoToBind = true;
				}
				else//WTM:  Change:  Added:
				{
					std::string tifScriptName = getTIFScriptNameNotExt(info);
					SkyblivionScript skScript = SkyblivionScript(this->BUILD_TARGET_DIALOGUES(), "", tifScriptName, this->ROOT_BUILD_PATH());
					std::string tifScriptPath = skScript.getScriptPath();
					std::ifstream tifFile(tifScriptPath.c_str());
					bool tifExists = tifFile.good();
					tifFile.close();
					if (tifExists)
					{
						addInfoToBind = true;
					}
				}
				if (addInfoToBind)
				{
					infosToBind[info] = newInfo;
				}

                currentDialForInfo->INFO.push_back(newInfo);



            }


        }

        for (uint32_t i = 0; i < branchVector.size(); i++) {
            geckFile->DLBR.pool.construct(branchVector.at(i), NULL, false);
        }

		return dialogueVector;

    }

	void SkyblivionConverter::addSOUNFromSNDR()
	{
		TES5File* skyblivionFile = this->getSkyblivionFile();
		ModFile* skyblivionFileMod = skyblivionFile;
		TES5File* geckFile = this->getGeckFile();
		std::vector<Record*, std::allocator<Record*>> skyrimSNDRRecords;
		skyblivionFile->SNDR.pool.MakeRecordsVector(skyrimSNDRRecords);
		std::vector<Record*, std::allocator<Record*>> skyrimSOUNRecords;
		skyblivionFile->SOUN.pool.MakeRecordsVector(skyrimSOUNRecords);
		std::vector<Record*, std::allocator<Record*>> modSOUNRecords;
		geckFile->SOUN.pool.MakeRecordsVector(modSOUNRecords);
		Sk::SNDRRecord* foundSNDR = NULL;
		for (uint32_t it = 0; it < skyrimSNDRRecords.size(); ++it) {
			Sk::SNDRRecord* sndr = (Sk::SNDRRecord*)(skyrimSNDRRecords)[it];
			bool foundSOUN = false;
			for (uint32_t it = 0; it < skyrimSOUNRecords.size(); ++it) {
				Sk::SOUNRecord* soun = (Sk::SOUNRecord*)(skyrimSOUNRecords)[it];
				if (soun->SDSC.value == sndr->formID)
				{
					foundSOUN = soun;
					break;
				}
			}
			if (!foundSOUN)
			{
				std::string sndrEDID = sndr->EDID.value;
				std::string newEDID = "TES4SOUN";
				std::string unprefixedSNDREDID = sndrEDID;
				if (unprefixedSNDREDID.find("TES4") == 0)
				{
					unprefixedSNDREDID = unprefixedSNDREDID.substr(4);
				}
				newEDID.append(unprefixedSNDREDID);
				char* newEDIDCharString = new char[newEDID.length() + 1];
				strncpy(newEDIDCharString, newEDID.c_str(), newEDID.length() + 1);
				Sk::SOUNRecord* soun = new Sk::SOUNRecord();
				soun->formID = this->skyrimCollection.NextFreeExpandedFormID(skyblivionFileMod);
				soun->formVersion = 43;
				soun->EDID.value = newEDIDCharString;
				soun->SDSC.value = sndr->formID;
				modSOUNRecords.push_back(soun);
				addedSOUNRecords.push_back(soun);
			}
		}
		for (uint32_t i = 0; i < modSOUNRecords.size(); i++) {
			geckFile->SOUN.pool.construct(modSOUNRecords.at(i), NULL, false);
		}
	}

	template< typename T >
	std::string int_to_hex(T i)//WTM:  Change:  Added
	{
		std::stringstream stream;
		stream << "0x"
			<< std::setfill('0') << std::setw(sizeof(T) * 2)
			<< std::hex << i;
		return stream.str();
	}

	void logMultipleFormIDsFound(int searchByFormID, std::string realPropertyEdid, std::vector<int> foundFormIDsAdjusted, int formIDByEDID, int returnFormIDAdjusted, std::string scriptPath)//WTM:  Change:  Added
	{
		std::vector<std::string> foundFormIDsAdjustedStrings = std::vector<std::string>();
		for (int i = 0; i < foundFormIDsAdjusted.size(); i++) { foundFormIDsAdjustedStrings.push_back(int_to_hex(foundFormIDsAdjusted[i])); }
		log_debug << "Multiple form IDs were found for " << int_to_hex(searchByFormID) << " (" << realPropertyEdid << "):  The NAME search found " << join(foundFormIDsAdjustedStrings, ", ") << ".  The EDID map search found " << int_to_hex(formIDByEDID) << ".  Returning " << int_to_hex(returnFormIDAdjusted) << ".  File:  " << scriptPath << "\n";
	}

	int SkyblivionConverter::getReferenceFormID(Record* record, std::string realPropertyEdid, std::string scriptPath) {//WTM:  Change:  Added
		//WTM:  Note:  I think this should take the form ID and look up the related record by NAME value.
		TES4File* oblivionFile = this->getOblivionFile();
		std::vector<Record*, std::allocator<Record*>> achrRecords;
		oblivionFile->CELL.achr_pool.MakeRecordsVector(achrRecords);
		std::vector<Record*, std::allocator<Record*>> refrRecords;
		oblivionFile->CELL.refr_pool.MakeRecordsVector(refrRecords);
		std::vector<Record*, std::allocator<Record*>> acreRecords;
		oblivionFile->CELL.acre_pool.MakeRecordsVector(acreRecords);

		//WTM:  Note:  find new form ID by searching the NAME values of other records
		std::vector<int> foundFormIDs = std::vector<int>();
		for (int i = 0; i < achrRecords.size(); i++)
		{
			Ob::ACHRRecord* achrRecord = (Ob::ACHRRecord*)achrRecords[i];
			if (achrRecord->NAME.value == record->formID)
			{
				foundFormIDs.push_back(achrRecord->formID);
			}
		}
		for (int i = 0; i < refrRecords.size(); i++)
		{
			Ob::REFRRecord* refrRecord = (Ob::REFRRecord*)refrRecords[i];
			if (refrRecord->Data->NAME.value == record->formID)
			{
				foundFormIDs.push_back(refrRecord->formID);
			}
		}
		for (int i = 0; i < acreRecords.size(); i++)
		{
			Ob::ACRERecord* acreRecord = (Ob::ACRERecord*)acreRecords[i];
			if (acreRecord->NAME.value == record->formID)
			{
				foundFormIDs.push_back(acreRecord->formID);
			}
		}

		//WTM:  Note:  This was the original algorithm:
		//Naive implementation - just look for XRef EDID.
		int formIDByEDID = 0;
		std::string refName = realPropertyEdid + "ref";
		if (this->edidMap->find(refName) != this->edidMap->end()) {
			log_info << "Found ref, binding instead\n";
			formIDByEDID = (*this->edidMap)[refName];
		}
		if (!foundFormIDs.empty())
		{
			std::vector<int> foundFormIDsAdjusted = std::vector<int>();
			for (int i = 0; i < foundFormIDs.size(); i++) { foundFormIDsAdjusted.push_back(convertFormid(foundFormIDs[i])); }
			if (foundFormIDsAdjusted.size() == 1)
			{
				if (formIDByEDID == 0 || formIDByEDID == foundFormIDsAdjusted[0]) { return foundFormIDsAdjusted[0]; }
				if (formIDByEDID != foundFormIDsAdjusted[0])
				{
					logMultipleFormIDsFound(record->formID, realPropertyEdid, foundFormIDsAdjusted, formIDByEDID, formIDByEDID, scriptPath);
					//WTM:  Note:  In this case, I find that formIDByEDID works better.  foundFormIDsAdjusted[0] contains something in a test cell.
					return formIDByEDID;
				}
			}
			else
			{
				int returnFormIDAdjusted = 0;
				if (formIDByEDID != 0 && std::find(foundFormIDsAdjusted.begin(), foundFormIDsAdjusted.end(), formIDByEDID) != foundFormIDsAdjusted.end())
				{
					returnFormIDAdjusted = formIDByEDID;//WTM:  Note:  Return formIDByEDID since it was found in foundFormIDsAdjusted.
				}
				else
				{
					returnFormIDAdjusted = foundFormIDsAdjusted.back();//WTM:  Note:  Return last of foundFormIDsAdjusted.  This will likely cause problems.
				}
				logMultipleFormIDsFound(record->formID, realPropertyEdid, foundFormIDsAdjusted, formIDByEDID, returnFormIDAdjusted, scriptPath);
				return returnFormIDAdjusted;
			}
		}
		else if (formIDByEDID != 0)
		{
			return formIDByEDID;
		}
		return 0;
	}

    void SkyblivionConverter::bindProperties(Script* script, SkyblivionScript &skScript) {
		std::string hardcodedFilename = skScript.getScriptPath();

        std::FILE* scriptHandle = std::fopen(hardcodedFilename.c_str(), "r");
        if (!scriptHandle) {
			std::string exceptionValS = "";
			if (skScript.getBuildName() != "TIF")
			{//TIF files are often empty in the source data, and thus no PSC file gets generated.
				//Sometimes, these are from scripts like GenericLoreScript [SCPT:00025811].
				exceptionValS.append("Non-TIF script file not found:  ");
			}
			char exceptionVal[0x180];
			sprintf(exceptionVal, "bindProperties - Cannot find script %s, errno %d.", hardcodedFilename.c_str(), errno);
			exceptionValS.append(exceptionVal);
            throw std::runtime_error(exceptionValS.c_str());
        }


        fseek(scriptHandle, 0, SEEK_END);
        size_t size = ftell(scriptHandle);
        char* scriptData = new char[size];
        rewind(scriptHandle);
        fread(scriptData, sizeof(char), size, scriptHandle);
        fclose(scriptHandle);
        std::string fullScript(scriptData);
        delete[] scriptData;

		TES5File* skyblivionFile = this->getSkyblivionFile();
		
		std::regex propRegex("(.*?) Property (.*?) Auto( Conditional)?(;TES4FormID:([0-9]+);)?(;TES5FormID:([0-9]+);)?");

        std::sregex_iterator properties(fullScript.begin(), fullScript.end(), propRegex/*, std::regex::match_not_dot_newline*/); //should be default
        std::sregex_iterator end;

		std::string colPrefix = "col_";

        for (; properties != end; ++properties) {
            std::string propertyType = (*properties)[1];

			if (propertyType == "ReferenceAlias") {
				continue; //Do nod bind aliases, they are bound by the game
			}

            std::string propertyName = (*properties)[2];

			std::string tes4FormIDString = (*properties)[5];
			int tes4FormID = 0;
			if (tes4FormIDString != "")
			{
				tes4FormID = std::stoi(tes4FormIDString);
			}
			std::string tes5FormIDString = (*properties)[7];
			int tes5FormID = 0;
			if (tes5FormIDString != "")
			{
				tes5FormID = std::stoi(tes5FormIDString);
			}

			std::string realPropertyName = propertyName;
			/**
			 * Strip the _p suffix
			 */
			//WTM:  Change:  The last two characters should not be removed if they aren't there (like for PlayerRef).
			if (realPropertyName.substr(realPropertyName.size() - 2) == "_p")
			{
				realPropertyName = realPropertyName.substr(0, propertyName.size() - 2);
			}

			/**
			 * Check for the collision prefix, if it's here, remove it and also remove the script ID from the suffix
			 */
			if (realPropertyName.substr(0, 4) == colPrefix) {
	
				//cut the col_prefix
				realPropertyName = realPropertyName.substr(4, realPropertyName.size() - 4);
				//cut the _xxxx suffix
				realPropertyName = realPropertyName.substr(0, realPropertyName.size() - 5);

			}

            if (propertyType == "Int") {
                PropertyInt* property = new PropertyInt();
				int32_t val;
				/**
				 * Different default value for messagebox results..
				 */
				if (propertyName == "TES4_MESSAGEBOX_RESULT_p") {
					val = -1;
				}
				else {
					val = 0;
				}

                unsigned char *cstr = new unsigned char[propertyName.size() + 1];                
                strcpy((char*)cstr, propertyName.c_str());
                property->name.Read(cstr, (uint32_t)propertyName.size(), false);
                property->value = val;
                property->status = 1;
                script->properties.push_back(property);
            }
            else if (propertyType == "Float") {
                PropertyFloat* property = new PropertyFloat();
                unsigned char *cstr = new unsigned char[propertyName.size() + 1];
                strcpy((char*)cstr, propertyName.c_str());
                property->name.Read(cstr, (uint32_t)propertyName.size(), false);
                property->value = 0;
                property->status = 1;
                script->properties.push_back(property);
            }
			else if (propertyType == "Sound") {
				/**
				 * Due to how the sounds are generated, we need to target the new sound ( SOUN ) objects instead of SNDRs
				 */
				//WTM:  Note:
				/*
				As far as I can tell, TES4 SOUN records were converted to Skyblivion.esm's SDSC (Sound Descriptor) records.  The SDSC has the SOUN record's form ID (+ 0x01000000).
				Additionally, Skyblivion.esm's SOUN (Sound Marker) records were generated which reference the SDSC.  But some TES4 SOUN records, like TES4 SOUN 0017C9D9/Skyblivion's SNDR 0117C9D9 (TES4ICArenaMonsters.AMBArenaCrowdCheerWin) have no Skyblivion SOUN.
				The local variable tes4FormID should not be used since it points to the SDSC.  edidMap does not contain SOUN records, so this procedure should always fail.
				*/
				/*
				std::string realPropertyEdid = "TES4SOUN" + realPropertyName;
				std::transform(realPropertyEdid.begin(), realPropertyEdid.end(), realPropertyEdid.begin(), ::tolower);

				if (this->edidMap->find(realPropertyEdid) != this->edidMap->end()) {
					PropertyObject* property = new PropertyObject();
					unsigned char *cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = (*this->edidMap)[realPropertyEdid];
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
				}
				*/
				//WTM:  Change:
				std::vector<Record*, std::allocator<Record*>> skyblivionSNDRRecords;
				skyblivionFile->SNDR.pool.MakeRecordsVector(skyblivionSNDRRecords);
				std::vector<Record*, std::allocator<Record*>> skyblivionSOUNRecords;
				skyblivionFile->SOUN.pool.MakeRecordsVector(skyblivionSOUNRecords);
				int tes5SNDRFormID = tes4FormID != 0 ? convertFormid(tes4FormID) : 0;
				Sk::SNDRRecord* foundSNDR = NULL;
				for (uint32_t it = 0; it < skyblivionSNDRRecords.size(); ++it) {
					Sk::SNDRRecord* sndr = (Sk::SNDRRecord*)(skyblivionSNDRRecords)[it];
					if (sndr->formID == tes5SNDRFormID)
					{
						foundSNDR = sndr;
						break;
					}
				}
				if (foundSNDR != NULL)
				{
					Sk::SOUNRecord* foundSOUN = NULL;
					for (uint32_t it = 0; it < skyblivionSOUNRecords.size(); ++it) {
						Sk::SOUNRecord* soun = (Sk::SOUNRecord*)(skyblivionSOUNRecords)[it];
						if (soun->SDSC.value == tes5SNDRFormID)
						{
							foundSOUN = soun;
							break;
						}
					}
					for (uint32_t it = 0; it < addedSOUNRecords.size(); ++it) {
						Sk::SOUNRecord* soun = addedSOUNRecords[it];
						if (soun->SDSC.value == tes5SNDRFormID)
						{
							foundSOUN = soun;
							break;
						}
					}
					if (foundSOUN == NULL)
					{
						throw new std::runtime_error("SOUN was not found.");
					}
					PropertyObject* property = new PropertyObject();
					unsigned char* cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = foundSOUN->formID;
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
				}
			}
			else if (propertyType == "Race") {//WTM:  Added:  Use Skyrim's Races
				std::string realPropertyEdid = realPropertyName + "Race";
				std::transform(realPropertyEdid.begin(), realPropertyEdid.end(), realPropertyEdid.begin(), ::tolower);

				if (this->edidMap->find(realPropertyEdid) != this->edidMap->end()) {
					PropertyObject* property = new PropertyObject();
					unsigned char* cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = (*this->edidMap)[realPropertyEdid];
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
				}
			}
            else {

				std::string realPropertyEdid = realPropertyName;
				if (realPropertyEdid != "PlayerRef")//WTM:  Change:  Despite this change, PlayerRef is not found in edidMap.  It must be set up manually.  This will be performed elsewhere.
				{
					std::transform(realPropertyEdid.begin(), realPropertyEdid.end(), realPropertyEdid.begin(), ::tolower);
					if (realPropertyEdid.find("tes4") != 0)
					{
						realPropertyEdid = "tes4" + realPropertyEdid;
					}
				}
				if (tes4FormID != 0 || tes5FormID != 0 || this->edidMap->find(realPropertyEdid) != this->edidMap->end()) {

					uint32_t boundFormid;
					if (tes4FormID != 0 || tes5FormID != 0)
					{
						if (propertyType == "Topic")
						{
							boundFormid = (*this->oldTopicFormIDToNewTopicFormID)[tes4FormID];
						}
						else if (tes4FormID != 0)
						{
							boundFormid = convertFormid(tes4FormID);
						}
						else if (tes5FormID != 0)
						{
							boundFormid = tes5FormID;
						}
						else
						{
							throw new std::runtime_error("No form ID found.");
						}
					}
					else
					{
						//EDID was found, now we need to check whether it doesn't lead to a base type
						//Somehow, Oblivion was able to bind scripts by base forms sometimes and ( automagically! ) it would find the ( probably only ) reference in the game and use it.

						boundFormid = (*this->edidMap)[realPropertyEdid];

						Record* record = findRecordByFormidGeneric(boundFormid - 0x01000000); //Hacky way to get back oblivion formid. TODO - Perhaps search in skyrim collection.

						if (record != NULL) { //Otherwise should be an error methinks?
							if (record->GetType() != REV32(REFR) && record->GetType() != REV32(ACHR) && record->GetType() != REV32(ACRE) && record->GetType() != REV32(QUST)) //If we try to bind a base form ( form which is not running in runtime )..
							{
								if (propertyType == "ObjectReference" || propertyType == "Actor" || propertyType.substr(0, 4) == "TES4") {
									log_warning << "Found connection to base form for a custom script or object reference / actor, name: " <<
										std::string(record->GetEditorIDKey()) << " property type requested: " << propertyType << "\n";
									int newFormID = getReferenceFormID(record, realPropertyEdid, hardcodedFilename);//WTM:  Change
									if (newFormID != 0)
									{
										boundFormid = newFormID;
									}
									else
									{
										int typeInt = record->GetType();
										if (!(
											typeInt == REV32(ARMO) ||
											typeInt == REV32(APPA) ||
											typeInt == REV32(BOOK) ||
											typeInt == REV32(CLOT) ||
											typeInt == REV32(INGR) ||
											typeInt == REV32(KEYM) ||
											typeInt == REV32(MISC) ||
											typeInt == REV32(SLGM) ||
											typeInt == REV32(WEAP)
											))//WTM:  Change:  Added:  I don't think a warning should be necessary for these.  I could be wrong.
										{
											log_error << "No suitable ref found for " << std::string(record->GetEditorIDKey()) <<
												", this will probably prevent script working in game\n";
										}
									}
								}
							}
						}
					}

					//std::cout << "Found EDID " << realPropertyEdid << std::endl;
					PropertyObject* property = new PropertyObject();
					unsigned char* cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = boundFormid;
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
					log_debug << "Property " << propertyName << " associated on " + skScript.getScriptName() << std::endl;
				}
				else if (realPropertyName == "tTimer") { //TES4TimerHelper
                    PropertyObject* property = new PropertyObject();
                    unsigned char *cstr = new unsigned char[propertyName.size() + 1];
                    strcpy((char*)cstr, propertyName.c_str());
                    property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = 0x010CBF21;
                    property->aliasId = -1;
                    property->status = 1;
                    script->properties.push_back(property);
				}
				else if (realPropertyName == "tContainer") { //TES4Container
					PropertyObject* property = new PropertyObject();
					unsigned char* cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = 0x010CBF22;
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
				}
				else if (realPropertyName == "PlayerRef") {//WTM:  Change:  Added
					PropertyObject* property = new PropertyObject();
					unsigned char* cstr = new unsigned char[propertyName.size() + 1];
					strcpy((char*)cstr, propertyName.c_str());
					property->name.Read(cstr, (uint32_t)propertyName.size(), false);
					property->formId = 0x00000014;
					property->aliasId = -1;
					property->status = 1;
					script->properties.push_back(property);
				}
				else {
					if (realPropertyEdid.find("tes4scene_") == 0)
						log_warning << "Scenes are not imported yet - " << realPropertyEdid << "\n";
					else if (realPropertyEdid.find("tes4messagebox") == 0)
						log_warning << "Messageboxes are not imported yet - " << realPropertyEdid << "\n";
					else if (realPropertyEdid.find("tes4attr") == 0)
						log_warning << "Cannot find TES4Attr " << realPropertyEdid << " in " << hardcodedFilename << "\n";
					else if (hardcodedFilename.find("/TES4se") != std::string::npos)
						log_warning << "Cannot find SE entry " << realPropertyEdid << " in " << hardcodedFilename << "\n";
					else
						log_warning << "Cannot find entry " << realPropertyEdid << " in " << hardcodedFilename << "\n";
				}

            }



        }

    }


	void SkyblivionConverter::convertPACKFromOblivion(TES4File* oblivionMod, TES5File* skyrimMod)
	{
		std::vector<Record*, std::allocator<Record*>> packages;
		oblivionMod->PACK.pool.MakeRecordsVector(packages);

		int count = packages.size();

		int i = 0;
		for (uint32_t it = 0; it < count; ++it) {
			Ob::PACKRecord* p = (Ob::PACKRecord*)packages[it];

			try {
				Sk::PACKRecord* skPack = new Sk::PACKRecord();
				convertPACKFromOblivion(*p, *skPack);
				skyrimMod->PACK.pool.construct(skPack, NULL, false);
			}
			catch (std::exception& e) {
				log_warning << e.what() << "\n";
				continue;
			}

			++i;
		}
	}

    void SkyblivionConverter::convertPACKFromOblivion(Ob::PACKRecord& srcRecord, Sk::PACKRecord& dstRecord) {

		if (srcRecord.EDID.IsLoaded()) {
			std::string tempEDID = srcRecord.EDID.value;
			std::string newEdid = srcRecord.EDID.value;
			newEdid = "TES4" + newEdid;
			dstRecord.EDID.value = new char[newEdid.length() + 1];
			std::strcpy(dstRecord.EDID.value, newEdid.c_str());
		}
        dstRecord.formID = convertFormid(srcRecord.formID);
        dstRecord.formVersion = 0x28;

        for (uint8_t i = 0; i < srcRecord.CTDA.value.size(); ++i) {
            std::vector<Sk::SKCondition*> skCTDAs = convertCTDAFromOblivion(srcRecord.CTDA.value[i]);
			for (int ctdaIt = 0; ctdaIt < skCTDAs.size(); ++ctdaIt) {
				dstRecord.CTDA.value.push_back(skCTDAs[ctdaIt]);
			}

        }

        dstRecord.PKCU = ReqSubRecord<Sk::PACKRecord::PACKPKCU>();
        dstRecord.PKDT = ReqSubRecord<Sk::PACKRecord::PACKPKDT>();
        dstRecord.PSDT = ReqSubRecord<Sk::PACKRecord::PACKPSDT>();
        dstRecord.PSDT.value.date = srcRecord.PSDT.value.date;
        dstRecord.PSDT.value.day = srcRecord.PSDT.value.day;
        dstRecord.PSDT.value.month = srcRecord.PSDT.value.month;
        dstRecord.PSDT.value.hour = srcRecord.PSDT.value.time;
        dstRecord.PSDT.value.minute = 0; //not supported in oblivion
        dstRecord.PSDT.value.duration = srcRecord.PSDT.value.duration * 60;
        //	srcRecord.PSDT.value.

        if (srcRecord.IsMustComplete()) {
            dstRecord.PKDT->flags |= Sk::PACKRecord::flagsFlags::fMustComplete;
        }

        if (srcRecord.IsOncePerDay()) {
            dstRecord.PKDT->flags |= Sk::PACKRecord::flagsFlags::fOncePerDay;
        }

        if (srcRecord.IsAlwaysSneak()) {
            dstRecord.PKDT->flags |= Sk::PACKRecord::flagsFlags::fAlwaysSneak;
        }

        if (srcRecord.IsAllowSwimming()) {
            dstRecord.PKDT->flags |= Sk::PACKRecord::flagsFlags::fAllowSwimming;
        }


        if (srcRecord.IsAIAccompany()) {
            //Accompany conversion.
            dstRecord.PKCU->packageTemplate = 0x00019B2C; //Follow package
            dstRecord.PKCU->dataInputCount = 6;
            dstRecord.PKCU->versionCounter = 4;
            dstRecord.versionControl2[0] = 0x06;

            float minRadius = 128.00;
            float maxRadius = 256;

            try {
                Sk::PACKRecord::PACKPTDA ptda = convertTargetType(srcRecord);
                dstRecord.addTargetTemplateSetting("SingleRef", ptda, 0);
            }
            catch (std::exception) {
                char reference[150];
                sprintf(reference, "Package %s ( Accompany ) : Cannot convert target type.", srcRecord.EDID.value);
                throw std::runtime_error(reference); //Cannot convert this package type.
            }

            dstRecord.addFloatTemplateSetting(minRadius, 1);
            dstRecord.addFloatTemplateSetting(maxRadius, 2);
            dstRecord.addBoolTemplateSetting(true, 4);
            dstRecord.addBoolTemplateSetting(false, 6);
            dstRecord.addBoolTemplateSetting(false, 8);
        }
        else if (srcRecord.IsAIAmbush()) {
			//WTM:  Change:  I'm going to use ForceGreet if the target is a ACHR or NPC_.
			Sk::PACKRecord::PACKPLDT triggerLocationPLDT = convertLocationType(srcRecord);
			Sk::PACKRecord::PACKPTDA pdta = convertTargetType(srcRecord);
			bool forceGreet = false;
			if (pdta.targetType == 0 || pdta.targetType == 1)
			{
				if (pdta.targetId == 0x14/*PlayerRef*/)
				{
					forceGreet = true;
				}
				else
				{
					Record* targetRecord = findRecordByFormidGeneric(srcRecord.PTDT.value->targetId);
					if (typeid(*targetRecord) == typeid(Ob::ACHRRecord) || typeid(*targetRecord) == typeid(Ob::NPC_Record))
					{
						forceGreet = true;
					}
				}
				if (forceGreet)
				{
					dstRecord.PKCU->packageTemplate = 0x0003C1C4;//ForceGreet
					//WTM:  Change:  I've tried to add very limited PDTO support to CBash so I could convert Ambush to ForceGreet these more accurately.
					dstRecord.PKCU->dataInputCount = 20;
					dstRecord.PKCU->versionCounter = 2;
					dstRecord.formVersion = 43;
					dstRecord.addTopicTemplateSetting(convertTopicType(srcRecord), 7);//Topic
					//A default ForceGreet package has trigger location radius of 500 and a wait location radius of 150.
					//The below shrinks the wait location radius so the ambush package can be most accurately reproduced for ForceGreet.
					Sk::PACKRecord::PACKPLDT waitLocationPLDT = convertLocationType(srcRecord);
					waitLocationPLDT.locRadius *= (150 / (float)500);
					dstRecord.addLocationTemplateSetting(waitLocationPLDT, 8);//NPC Wait Location (NPC hangs out here)
					dstRecord.addLocationTemplateSetting(triggerLocationPLDT, 62);//Trigger Location (Player here causes forcegreet)
					Sk::PACKRecord::PACKPLDT radiusPLDT = Sk::PACKRecord::PACKPLDT();
					radiusPLDT.locType = 0;//Near Reference
					radiusPLDT.locId = 0x14;//PlayerRef
					radiusPLDT.locRadius = 300;
					dstRecord.addLocationTemplateSetting(radiusPLDT, 75);//Forcegreet Distance (Don't change ref, just radius)
					dstRecord.addBoolTemplateSetting(false, 79);//Player must be detected?
					dstRecord.addBoolTemplateSetting(true, 65);//Greet Using Preferred Path?
					Sk::PACKRecord::PACKPTDA ptda = Sk::PACKRecord::PACKPTDA();
					ptda.targetType = 0;//Specific Reference
					ptda.targetId = 0x14;//PlayerRef
					ptda.targetCount = 0;
					dstRecord.addTargetTemplateSetting("SingleRef", ptda, 17);//Player
					Sk::PACKRecord::PACKPLDT forceGreetPLDT = Sk::PACKRecord::PACKPLDT();
					forceGreetPLDT.locType = 0;//Near Reference
					forceGreetPLDT.locId = 0x14;//PlayerRef
					forceGreetPLDT.locRadius = 5000;
					dstRecord.addLocationTemplateSetting(forceGreetPLDT, 22);//ForceGreetLoc
					dstRecord.addBoolTemplateSetting(false, 26);//AlwaysFalse
					dstRecord.addBoolTemplateSetting(false, 81);//Forcegreet if player on horseback?
					dstRecord.addBoolTemplateSetting(false, 40);//Sandbox While Waiting?
					dstRecord.addBoolTemplateSetting(false, 29);//AllowEating
					dstRecord.addBoolTemplateSetting(false, 30);//AllowSleeping
					dstRecord.addBoolTemplateSetting(false, 31);//AllowConversation
					dstRecord.addBoolTemplateSetting(false, 32);//AllowIdleMarkers
					dstRecord.addBoolTemplateSetting(false, 33);//AllowSitting
					dstRecord.addBoolTemplateSetting(false, 34);//AllowWandering
					dstRecord.addBoolTemplateSetting(false, 36);//WanderPreferredPath
					dstRecord.addFloatTemplateSetting(50, 67);//Energy
					dstRecord.addBoolTemplateSetting(false, 69);//Allow Special Furniture?
				}
			}
			if (!forceGreet)
			{
				dstRecord.PKCU->packageTemplate = 0x010CBE96;
				dstRecord.PKCU->dataInputCount = 5;
				dstRecord.PKCU->versionCounter = 2;
				dstRecord.formVersion = 43;
				dstRecord.addLocationTemplateSetting(triggerLocationPLDT, 0);
				dstRecord.addTargetTemplateSetting("TargetSelector", pdta, 3);
				dstRecord.addObjectListTemplateSetting(0, 5);
				dstRecord.addBoolTemplateSetting(false, 6);
				dstRecord.addIntTemplateSetting(1, 7);
			}
        }
        else if (srcRecord.IsAICastMagic()) {

            dstRecord.PKCU->packageTemplate = 0x000504F5; //UseMagic package
            dstRecord.versionControl2[0] = 0x04;
            dstRecord.PKCU->dataInputCount = 11;
            dstRecord.PKCU->versionCounter = 1;
            Sk::PACKRecord::PACKPLDT location = convertLocationType(srcRecord);
            dstRecord.addLocationTemplateSetting(location, 2);

            Sk::PACKRecord::PACKPTDA spell = Sk::PACKRecord::PACKPTDA();
            spell.targetType = 2; //object type is..
            spell.targetId = 0x15; // .. any spell
            spell.targetCount = 1;
            dstRecord.addTargetTemplateSetting("TargetSelector", spell, 3);
            dstRecord.addTargetTemplateSetting("SingleRef", convertTargetType(srcRecord), 4);
            dstRecord.addBoolTemplateSetting(false, 5);
            dstRecord.addFloatTemplateSetting(2, 6);
            dstRecord.addFloatTemplateSetting(3, 7);
            dstRecord.addFloatTemplateSetting(1, 8);
            dstRecord.addFloatTemplateSetting(3, 9);
            dstRecord.addIntTemplateSetting(1, 10);
            dstRecord.addIntTemplateSetting(1, 11);
            dstRecord.addBoolTemplateSetting(false, 12);
        }
        else if (srcRecord.IsAIEat()) {
            dstRecord.PKCU->packageTemplate = 0x00019714; //Eat package
            dstRecord.PKCU->dataInputCount = 21;
            dstRecord.PKCU->versionCounter = 8;
            dstRecord.versionControl2[0] = 0x0F;
            Sk::PACKRecord::PACKPLDT location = convertLocationType(srcRecord);
            dstRecord.addLocationTemplateSetting(location, 0);//Eat Location
            dstRecord.addBoolTemplateSetting(true, 21);//Unlock On Arrival?

            try{

                Sk::PACKRecord::PACKPTDA food;

				if (!srcRecord.PTDT.IsLoaded()) {
                    //Standard food targetSelector
                    food = Sk::PACKRecord::PACKPTDA();
                    food.targetType = 2;
                    food.targetId = 15;//Object Type on this page: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/PACK (Only 11 is considered known.) 
                    food.targetCount = 1;//https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/PACK:  "Count (0 in all Skyrim records)"
                }
                else {
                    food = convertTargetType(srcRecord);
                }
                dstRecord.addTargetTemplateSetting("TargetSelector", food, 1);//Food Criteria

                dstRecord.addObjectListTemplateSetting(0, 4);//Found Food
				Sk::PACKRecord::PACKPTDA chairsTarget = Sk::PACKRecord::PACKPTDA();
				chairsTarget.targetCount = 1;
				chairsTarget.targetType = 2;
				chairsTarget.targetId = 0x1b;
                dstRecord.addTargetTemplateSetting("TargetSelector", chairsTarget, 5);//Chair Target
                dstRecord.addObjectListTemplateSetting(0, 6);//Found Chair
                dstRecord.addIntTemplateSetting(1, 10);//NumFoodItems
                dstRecord.addBoolTemplateSetting(true, 12);//AllowAlreadyHeld
                dstRecord.addBoolTemplateSetting(false, 16);//RideHorseIfPossible
                dstRecord.addBoolTemplateSetting(false, 14);//False
                dstRecord.addBoolTemplateSetting(false, 23);//CreateFakeFood
                dstRecord.addBoolTemplateSetting(true, 25);//AllowEating
                dstRecord.addBoolTemplateSetting(false, 26);//AllowSleeping
                dstRecord.addBoolTemplateSetting(true, 27);//AllowConversation
                dstRecord.addBoolTemplateSetting(true, 28);//AllowIdleMarkers
                dstRecord.addBoolTemplateSetting(true, 33);//AllowSpecialFurniture
                dstRecord.addBoolTemplateSetting(true, 29);//AllowSitting
                dstRecord.addBoolTemplateSetting(true, 30);//AllowWandering
                dstRecord.addFloatTemplateSetting(300, 35);//MinWanderDistance
                dstRecord.addBoolTemplateSetting(false, 8);//Wander Preferred Path Only?
                dstRecord.addFloatTemplateSetting(0, 32);//Energy

                if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
                    // Handle Un/Locking
                    FORMID formid = findRecordFormidByEDID("TES4EatLockTemplate");

                    if (formid != NULL) {
                        dstRecord.PKCU->packageTemplate = formid;
                        dstRecord.PKCU->dataInputCount += 7;
                        dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 36); //Lock at Start?
                        dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 37); //Lock at Location?
                        dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 38); //Lock at End?
                        dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 39); //Unlock at Start?
                        dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 40); //Unlock at Location?
                        dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 41); //Unlock at End?
                        Sk::PACKRecord::PACKPLDT pldt;
                        pldt.locRadius = 1000;
                        pldt.locType = 12;
                        dstRecord.addLocationTemplateSetting(pldt, 42); //Near self
                    }
                }
            }
            catch (std::exception) {
                char reference[150];
                sprintf(reference, "Package %s ( Eat ) : Cannot convert target type.", srcRecord.EDID.value);
                throw std::runtime_error(reference); //Cannot convert this package type.
            }

        }
        else if (srcRecord.IsAIEscort()) {
            try {
                dstRecord.PKCU->packageTemplate = 0x00023B73; //Escort package
                dstRecord.PKCU->dataInputCount = 9;
                dstRecord.PKCU->versionCounter = 8;
                dstRecord.versionControl2[0] = 0x06;
                dstRecord.addTargetTemplateSetting("SingleRef", convertTargetType(srcRecord), 0xB);
				dstRecord.addIntTemplateSetting(1, 2);
				dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 3);
                dstRecord.addFloatTemplateSetting(512, 4);
                dstRecord.addFloatTemplateSetting(256, 5);
                dstRecord.addFloatTemplateSetting(512, 6);
                dstRecord.addBoolTemplateSetting(false, 0xD);
                dstRecord.addBoolTemplateSetting(false, 0xF);
                dstRecord.addFloatTemplateSetting(500, 0x11);
            }
            catch (std::exception) {
                char reference[150];
                sprintf(reference, "Package %s ( Escort ) : Cannot convert target type.", srcRecord.EDID.value);
                throw std::runtime_error(reference); //Cannot convert this package type.
            }
        }
        else if (srcRecord.IsAIFind()) {
            //Dependency on new .esp plugin.

            dstRecord.PKCU->packageTemplate = 0x010CBE95; //Find
            dstRecord.PKCU->dataInputCount = 7;
            dstRecord.PKCU->versionCounter = 2;
            dstRecord.formVersion = 43;
            dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);
            dstRecord.addTargetTemplateSetting("TargetSelector", convertTargetType(srcRecord), 1);
            dstRecord.addObjectListTemplateSetting(0, 3);
            dstRecord.addBoolTemplateSetting(true, 4);
            dstRecord.addIntTemplateSetting(1, 5);
            dstRecord.addFloatTemplateSetting(3, 6);
            dstRecord.addBoolTemplateSetting(false, 8);

            if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
                // Handle Un/Locking
                FORMID formid = findRecordFormidByEDID("TES4FindLockPackageTemplate");

                if (formid != NULL) {
                    dstRecord.PKCU->packageTemplate = formid;
                    dstRecord.PKCU->dataInputCount += 7;
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 0x9); //Lock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 0xa); //Lock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 0xb); //Lock at End?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 0xc); //Unlock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 0xd); //Unlock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 0xe); //Unlock at End?
                    Sk::PACKRecord::PACKPLDT pldt;
                    pldt.locRadius = 1000;
                    pldt.locType = 12;
                    dstRecord.addLocationTemplateSetting(pldt, 0xf); //Near self
                }
            }
            //throw std::exception("Not implemented");
        }
        else if (srcRecord.IsAIFleeNotCombat()) {
            /**
            dstRecord.PKCU->packageTemplate = 0x000197F1; //FleeFrom package
            try {

            Sk::PACKRecord::PACKPLDT location = convertLocationType(srcRecord);

            dstRecord.addTargetTemplateSetting("TargetSelector", fleeFromTarget, 0x7);
            dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 6);
            dstRecord.addFloatTemplateSetting(512, 3);
            dstRecord.addFloatTemplateSetting(256, 4);
            dstRecord.addFloatTemplateSetting(512, 5);
            dstRecord.addBoolTemplateSetting(false, 6);
            dstRecord.addBoolTemplateSetting(false, 0xD);
            dstRecord.addFloatTemplateSetting(500, 0x11);
            }
            catch (int e) {
            printer("Package %s ( Flee ) : Cannot convert target type.", srcRecord.EDID.value);
            }
            */
            //Not implemented, only a few and hard to implement.
            throw std::runtime_error("Flee not Combat Not implemented");
        }
		else if (srcRecord.IsAIFollow()) {
			//Follow conversion.
			bool hasTargetLocation = srcRecord.PLDT.IsLoaded();
			if (hasTargetLocation)
			{
				dstRecord.PKCU->packageTemplate = 0x00025E6D; //FollowTo

				dstRecord.PKCU->dataInputCount = 5;
				dstRecord.PKCU->versionCounter = 9;
				dstRecord.versionControl2[0] = 0x06;
				float minRadius = 128.00;
				float maxRadius = 256;

				try {
					Sk::PACKRecord::PACKPTDA ptda = convertTargetType(srcRecord);
					dstRecord.addTargetTemplateSetting("SingleRef", ptda, 2);
				}
				catch (std::exception) {
					char reference[150];
					sprintf(reference, "Package %s ( Accompany ) : Cannot convert target type.", srcRecord.EDID.value);
					throw std::runtime_error(reference); //Cannot convert this package type.
				}

				dstRecord.addFloatTemplateSetting(minRadius, 4);
				dstRecord.addFloatTemplateSetting(maxRadius, 5);
				try {
					dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 8);
				}
				catch (std::exception) {
					char reference[150];
					sprintf(reference, "Package %s ( FollowTo ) : Cannot convert target type.", srcRecord.EDID.value);
					throw std::runtime_error(reference); //Cannot convert this package type.
				}
			
				dstRecord.addBoolTemplateSetting(true, 6);
			}
			else {
				dstRecord.PKCU->packageTemplate = 0x00019B2C; //Follow package
				dstRecord.PKCU->dataInputCount = 6;
				dstRecord.PKCU->versionCounter = 4;
				dstRecord.versionControl2[0] = 0x06;
				float minRadius = 128.00;
				float maxRadius = 256;

				try {
					Sk::PACKRecord::PACKPTDA ptda = convertTargetType(srcRecord);
					dstRecord.addTargetTemplateSetting("SingleRef", ptda, 0);
				}
				catch (std::exception) {
					char reference[150];
					sprintf(reference, "Package %s ( Accompany ) : Cannot convert target type.", srcRecord.EDID.value);
					throw std::runtime_error(reference); //Cannot convert this package type.
				}

				dstRecord.addFloatTemplateSetting(minRadius, 1);
				dstRecord.addFloatTemplateSetting(maxRadius, 2);
				dstRecord.addBoolTemplateSetting(false, 4);
				dstRecord.addBoolTemplateSetting(false, 6);
				dstRecord.addBoolTemplateSetting(false, 8);
			}
		}
        else if (srcRecord.IsAISleep()) {
            dstRecord.PKCU->packageTemplate = 0x00019717; //Sleep package
            dstRecord.PKCU->dataInputCount = 17;
            dstRecord.PKCU->versionCounter = 6;
            dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);
            dstRecord.versionControl2[0] = 0x0E;

            Sk::PACKRecord::PACKPTDA bedsTarget = Sk::PACKRecord::PACKPTDA();
            bedsTarget.targetCount = 1;
            bedsTarget.targetType = 2;
            bedsTarget.targetId = 0x1a;
            dstRecord.addTargetTemplateSetting("TargetSelector", bedsTarget, 1);

            dstRecord.addBoolTemplateSetting(false, 0xF); //TODO - check flag OnPackageStart/OnPackageEnd lock doors.
            dstRecord.addBoolTemplateSetting(false, 0xD); //TODO - check flag OnPackageStart/OnPackageEnd lock doors.

            dstRecord.addBoolTemplateSetting(false, 0xB);
            dstRecord.addObjectListTemplateSetting(0, 0x2);
            dstRecord.addBoolTemplateSetting(false, 0x8);

            dstRecord.addBoolTemplateSetting(false, 0x11);
            dstRecord.addBoolTemplateSetting(true, 0x12);

            dstRecord.addBoolTemplateSetting(true, 0x13);
            dstRecord.addBoolTemplateSetting(true, 0x14);
            dstRecord.addBoolTemplateSetting(true, 0x15);
            dstRecord.addBoolTemplateSetting(true, 0x19);
            dstRecord.addBoolTemplateSetting(true, 0x16);

            dstRecord.addFloatTemplateSetting(300, 0x1A);
            dstRecord.addBoolTemplateSetting(false, 0x6);
            dstRecord.addFloatTemplateSetting(50, 0x18);

            if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
                // Handle Un/Locking
                FORMID formid = findRecordFormidByEDID("TES4SleepLockTemplate");

                if (formid != NULL) {
                    dstRecord.PKCU->packageTemplate = formid;
                    dstRecord.PKCU->dataInputCount += 7;
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 0x1b); //Lock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 0x1c); //Lock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 0x1d); //Lock at End?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 0x1e); //Unlock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 0x1f); //Unlock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 0x20); //Unlock at End?
                    Sk::PACKRecord::PACKPLDT pldt;
                    pldt.locRadius = 1000;
                    pldt.locType = 12;
                    dstRecord.addLocationTemplateSetting(pldt, 0x21); //Near self
                }
            }
        }
        else if (srcRecord.IsAITravel()) {

			//WTM:  Added:  Change:  I'm trying to change these Travel packages to SitTarget packages if the target is a specific chair.
			bool isSpecificChair = false, isSpecificBed = false;
			if (srcRecord.PLDT->locId != 0)
			{
				Record* locationRecord = findRecordByFormidGeneric(srcRecord.PLDT->locId);
				if (locationRecord != NULL)
				{
					if (typeid(*locationRecord) == typeid(Ob::REFRRecord))
					{
						Ob::REFRRecord* locationREFRRecord = (Ob::REFRRecord*)locationRecord;
						Record* locationREFRName = findRecordByFormidGeneric(locationREFRRecord->Data.value->NAME.value);
						if (typeid(*locationREFRName) == typeid(Ob::FURNRecord))
						{
							Ob::FURNRecord* locationFURNRecord = (Ob::FURNRecord*)locationREFRName;
							std::string furnEDID = locationFURNRecord->EDID.value;
							if (furnEDID.find("Chair") != std::string::npos || furnEDID.find("Pew") != std::string::npos || furnEDID.find("Bench") != std::string::npos || furnEDID.find("Throne") != std::string::npos || furnEDID.find("Stool") != std::string::npos)
							{
								isSpecificChair = true;
							}
							else if (furnEDID.find("Bed"))
							{
								isSpecificBed = true;//These might be only bedroll targets.
							}
						}
					}
					else if (typeid(*locationRecord) == typeid(Ob::ACHRRecord))
					{
						Ob::ACHRRecord* locationACHRRecord = (Ob::ACHRRecord*)locationRecord;
						Record* locationACHRName = findRecordByFormidGeneric(locationACHRRecord->NAME.value);
						if (typeid(*locationACHRName) != typeid(Ob::NPC_Record))
						{
							locationACHRName = locationACHRName;
						}
					}
					else if (typeid(*locationRecord) == typeid(Ob::ACRERecord))
					{
						Ob::ACRERecord* locationACRERecord = (Ob::ACRERecord*)locationRecord;
						Record* locationACREName = findRecordByFormidGeneric(locationACRERecord->NAME.value);
						if (typeid(*locationACREName) != typeid(Ob::CREARecord))
						{
							locationACREName = locationACREName;
						}
					}
					else if (typeid(*locationRecord) == typeid(Ob::FURNRecord))
					{
						Ob::FURNRecord* locationFURNRecord = (Ob::FURNRecord*)locationRecord;
						std::string furnEDID = locationFURNRecord->EDID.value;
						if (furnEDID.find("Chair") != std::string::npos || furnEDID.find("Pew") != std::string::npos || furnEDID.find("Bench") != std::string::npos || furnEDID.find("Throne") != std::string::npos || furnEDID.find("Stool") != std::string::npos)
						{
							isSpecificChair = true;
						}
					}
					else if (typeid(*locationRecord) != typeid(Ob::CELLRecord))
					{
						locationRecord = locationRecord;
					}
				}
			}
			if (isSpecificChair)
			{
				dstRecord.PKCU->packageTemplate = 0x000A9277;//SitTarget
				dstRecord.PKCU->dataInputCount = 3;
				dstRecord.PKCU->versionCounter = 3;
				dstRecord.versionControl2[0] = 0x0A;
				Sk::PACKRecord::PACKPTDA ptda = convertTargetType(srcRecord);
				dstRecord.addTargetTemplateSetting("SingleRef", ptda, 16);//target
				dstRecord.addIntTemplateSetting(0, 3);//Wait Time
				dstRecord.addBoolTemplateSetting(false, 4);//Stop Movement Flag
			}
			else if (isSpecificBed)
			{
				dstRecord.PKCU->packageTemplate = 0x00019717;//Sleep
				dstRecord.PKCU->dataInputCount = 16;
				dstRecord.PKCU->versionCounter = 3;
				dstRecord.versionControl2[0] = 0x0A;
				dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);//Sleep Location
				dstRecord.addTargetTemplateSetting("TargetSelector", convertTargetType(srcRecord), 0);//Search Criteria:
				dstRecord.addBoolTemplateSetting(false, 15);//Lock Doors?
				dstRecord.addBoolTemplateSetting(false, 13);//Warn Before Locking?
				dstRecord.addBoolTemplateSetting(false, 11);//RideHorseIfPossible
				dstRecord.addObjectListTemplateSetting(0, 2);//Found Bed
				dstRecord.addBoolTemplateSetting(false, 8);//False
				dstRecord.addBoolTemplateSetting(false, 17);//AllowEating
				dstRecord.addBoolTemplateSetting(true, 18);//AllowSleeping
				dstRecord.addBoolTemplateSetting(true, 19);//AllowConversation
				dstRecord.addBoolTemplateSetting(true, 20);//AllowIdleMarkers
				dstRecord.addBoolTemplateSetting(true, 21);//AllowSitting
				dstRecord.addBoolTemplateSetting(true, 25);//AllowSpecialFurniture
				dstRecord.addBoolTemplateSetting(true, 22);//AllowWandering
				dstRecord.addFloatTemplateSetting(300, 26);//MinWanderDistance
				dstRecord.addBoolTemplateSetting(false, 6);//Wander Preferred Path Only?
				dstRecord.addFloatTemplateSetting(50, 24);//Energy
			}
			else
			{
				dstRecord.PKCU->packageTemplate = 0x00016FAA; //Travel package
				dstRecord.PKCU->dataInputCount = 3;
				dstRecord.PKCU->versionCounter = 3;
				dstRecord.versionControl2[0] = 0x0A;
				dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);//Place to Travel
				dstRecord.addBoolTemplateSetting(false, 2);//Ride Horse if possible?
				dstRecord.addBoolTemplateSetting(false, 4);//Prefer Preferred Path?

				if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
					// Handle Un/Locking
					FORMID formid = findRecordFormidByEDID("TES4TravelLockTemplate");

					if (formid != NULL) {
						dstRecord.PKCU->packageTemplate = formid;
						dstRecord.PKCU->dataInputCount += 7;
						dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 0x5); //Lock at Start?
						dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 0x6); //Lock at Location?
						dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 0x7); //Lock at End?
						dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 0x8); //Unlock at Start?
						dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 0x9); //Unlock at Location?
						dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 0xa); //Unlock at End?
						Sk::PACKRecord::PACKPLDT pldt;
						pldt.locRadius = 1000;
						pldt.locType = 12;
						dstRecord.addLocationTemplateSetting(pldt, 0xb); //Near self
					}
				}
			}
        }
        else if (srcRecord.IsAIUseItemAt()) {
            //Needs to be converted 2 idles i think
            dstRecord.PKCU->packageTemplate = 0x010CBE97; //UseItemAt
            dstRecord.PKCU->dataInputCount = 4;
            dstRecord.PKCU->versionCounter = 0;
            dstRecord.formVersion = 43;
            dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);
            dstRecord.addTargetTemplateSetting("SingleRef", convertTargetType(srcRecord), 1);
            dstRecord.addIntTemplateSetting(1, 2);
            dstRecord.addBoolTemplateSetting(false, 4);

            if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
                // Handle Un/Locking
                FORMID formid = findRecordFormidByEDID("TES4UseItemAtLockPackageTemplate");

                if (formid != NULL) {
                    dstRecord.PKCU->packageTemplate = formid;
                    dstRecord.PKCU->dataInputCount += 7;
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 0x5); //Lock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 0x6); //Lock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 0x7); //Lock at End?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 0x8); //Unlock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 0x9); //Unlock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 0xa); //Unlock at End?
                    Sk::PACKRecord::PACKPLDT pldt;
                    pldt.locRadius = 1000;
                    pldt.locType = 12;
                    dstRecord.addLocationTemplateSetting(pldt, 0xb); //Near self
                }
            }
            //throw std::exception("Use item at Not implemented");
			//WTM:  Change
			//An early attempt at converting to ActivateWhenFound (or some such name)
			/*
			dstRecord.PKCU->packageTemplate = find form ID
			dstRecord.PKCU->dataInputCount = 11;
			dstRecord.PKCU->versionCounter = 0;
			dstRecord.formVersion = 43;
			dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);
			dstRecord.addTargetTemplateSetting("Target Criteria", convertTargetType(srcRecord), 1);
			dstRecord.addIntTemplateSetting(1, 2);
			dstRecord.addBoolTemplateSetting(false, 4);
			dstRecord.addBoolTemplateSetting(false, 9);*/
        }
        else if (srcRecord.IsAIWander()) {
            dstRecord.PKCU->packageTemplate = 0x0001C254; //Sandbox package.
            dstRecord.PKCU->dataInputCount = 12;
            dstRecord.PKCU->versionCounter = 10;
            dstRecord.versionControl2[0] = 0x0F;
            dstRecord.addLocationTemplateSetting(convertLocationType(srcRecord), 0);
            dstRecord.addBoolTemplateSetting(false, 0xE);
            dstRecord.addBoolTemplateSetting(false, 1);
            dstRecord.addBoolTemplateSetting(false, 3);
            dstRecord.addBoolTemplateSetting(false, 4);
            dstRecord.addBoolTemplateSetting(true, 5);
            dstRecord.addBoolTemplateSetting(true, 6);
            dstRecord.addBoolTemplateSetting(true, 0x1f);
            dstRecord.addBoolTemplateSetting(true, 7);
            dstRecord.addBoolTemplateSetting(false, 0x19);
            dstRecord.addBoolTemplateSetting(false, 0x1b);
            dstRecord.addFloatTemplateSetting(50, 0x1d);

            if ((srcRecord.PKDT.value.flags & 0x1F8) != 0) {
                // Handle Un/Locking
                FORMID formid = findRecordFormidByEDID("TES4SandboxLockTemplate");

                if (formid != NULL) {
                    dstRecord.PKCU->packageTemplate = formid;
                    dstRecord.PKCU->dataInputCount += 7;
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtStart(), 0x20); //Lock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtLocation(), 0x21); //Lock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsLockAtEnd(), 0x22); //Lock at End?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtStart(), 0x23); //Unlock at Start?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtLocation(), 0x24); //Unlock at Location?
                    dstRecord.addBoolTemplateSetting(srcRecord.IsUnlockAtEnd(), 0x25); //Unlock at End?
                    Sk::PACKRecord::PACKPLDT pldt;
                    pldt.locRadius = 1000;
                    pldt.locType = 12;
                    dstRecord.addLocationTemplateSetting(pldt, 0x26); //Near self
                }
            }

        }

    }

    Sk::PACKRecord::PACKPTDA SkyblivionConverter::convertTargetType(Ob::PACKRecord& p) {
        /*
        OBLIVION:
        SEObelisk13PriestFakeActivate: 0x0 -> Object Type NONE
        SECaldanaFightAfterSE05Fri20x2: 0x18 -> Object Type WEAPONS MEELE
        SEUshnarFightAfterSE05Mon20x2: 0xd -> Weapons any
        SEHalionReadsInLoft: 0x4 -> Books
        SECaldanaMonriusBugPeople: 0xf -> NPCs ( at least one value )
        XPXidittec1AlchemyPCKG: 0x13 -> Alchemy
        SEBreakneckPractice: 0x1b -> Spells RANGE TARGET
        SEBruscusDannusFindArmor: 0x3 -> Armor
        SEBruscusDannusDumpIngredients: 0x8 -> Indegrients
        SEBruscusDannusFindClothing: 0x5 -> Clothing
        MilcharSubBossCast02: 0x1d -> Range self
        SE08c2PriestSit: 0xc -> Furniture
        SE32AlthelShootFanatic2: 0x19 -> Weapons RANGED
        EdlaDarkHeartCollectArrows: 0xe -> AMMO
        AnvilThurindilFightHaulsRopesFaster1x3: 0x17 -> Weapons NONE
        CheydinhalMGTrayvondCastAlteration: 0x1e -> Spells School ALTERATION
        CheydinhalMGDeetsanMakePotions12x8: 0x2 -> Apparatus
        BravilMGDelphineJendCastDestruction: 0x20 -> Spells School DESTRUCTION
        BravilMGItaRienusCastMysticism: 0x22 -> Spells School Mysticism
        MageCast1: 0x1a -> Spells Any
        GasparPracticeIllusionSpellsAfternoon: 0x21 -> Spells School Illusion
        TempleMothPriestGather18x10: 0xb -> FLORA
        TempleMothPriestGather4x10: 0x14 -> FOOD
        TempleMothPriestStorage10x2: 0x6 -> Containers
        Dark08NevilleGearPackage: 0x15 -> Combat Wearable
        MQ06AcolyteTalkative: 0x23 -> Spells School RESTORATION
        CitySwimmerForage10x10: 0x1c -> Spells RANGE TOUCH
        BravilSundayHunt: 0x26 INVALID ( xD )

        Conversion map:
        0x00 -> 0x00
        0xd -> 0xb
        0x04 -> 0x03
        0x14 -> 0x19
        0x13 -> 0x0E
        0x1b -> 0x16
        0x3 -> 0x02
        0x8 -> 0x06
        0x5 -> 0x11 ( clothing, not really sure if conversion to ALL:WEARABLE is ok )
        0x1d -> 0x18
        0xc -> 0x1b ( to chairs, again, not sure if this is ok, it seems like furniture hits were aimed at sittable things in oblivion )
        0x19 -> 0x14
        0xe -> 0xc
        0x17 -> 0x12
        0x1e -> impossible ( cannot conditionalize by school )
        0x2 -> impossible ( no apparatus in skyrim )\
        0x20 -> impossible ( no apparatus in skyrim )\
        0x22 -> impossible ( no apparatus in skyrim )\
        0x1a -> 0x15
        0x21 -> impossible
        0xb -> 0x09
        0x14 -> 0xf
        0x06 -> 0x04
        0x15 -> 0x10
        0x23 -> impossible
        0x1c -> 0x17
        */

        Sk::PACKRecord::PACKPTDA ptda = Sk::PACKRecord::PACKPTDA();

		if (p.PTDT.IsLoaded())
		{
			ptda.targetCount = p.PTDT->targetCount;
			ptda.targetType = p.PTDT->targetType;

			if (ptda.targetType == 2) {
				//Convert object type.
				switch (p.PTDT->targetId) {
				case 0x00: { ptda.targetId = 0x00; break; }
				case 0xd: { ptda.targetId = 0xb; break; }
				case 0x04: { ptda.targetId = 0x03; break; }
				case 0x13: { ptda.targetId = 0x0E; break; }
				case 0x1b: { ptda.targetId = 0x16; break; }
				case 0x3: { ptda.targetId = 0x02; break; }
				case 0x8: { ptda.targetId = 0x06; break; }
				case 0x5: { ptda.targetId = 0x11; break; }
				case 0x1d: { ptda.targetId = 0x18; break; }
				case 0xc: { ptda.targetId = 0x1b; break; }
				case 0x19: { ptda.targetId = 0x14; break; }
				case 0xe: { ptda.targetId = 0xc; break; }
				case 0x17: { ptda.targetId = 0x12; break; }
				case 0x1e: {

					ptda.targetType = 3;
					ptda.targetId = 0x001076F1; //MagicSchoolAlteration KYWD
					break;

				}
				case 0x2: {
					ptda.targetType = 3;
					ptda.targetId = 0x010CBE98; //TES4ApparatusKeyword FORMID from Skyblivion
					break;
				}
				case 0x20: {

					ptda.targetType = 3;
					ptda.targetId = 0x001076F0; //MagicSchoolDestruction KYWD
					break;

				}
				case 0x22: {

					ptda.targetType = 3;
					ptda.targetId = 0x001076F1; //MagicSchoolAlteration ( no mysticism in skyrim .. ) KYWD
					break;

				}
				case 0x1a: { ptda.targetId = 0x15; break; }
				case 0x21: {

					ptda.targetType = 3;
					ptda.targetId = 0x001076F2; //MagicSchoolIllusion KYWD
					break;

				}
				case 0xb: { ptda.targetId = 0x09; break; }
				case 0x14: { ptda.targetId = 0xf; break; }
				case 0x06: { ptda.targetId = 0x04; break; }
				case 0x15: { ptda.targetId = 0x10; break; }
				case 0x23: {

					ptda.targetType = 3;
					ptda.targetId = 0x001076F3; //MagicSchoolRestoration KYWD
					break;

				}
				case 0x1c: { ptda.targetId = 0x17; break; }
				case 0xf/*NPCs*/: { ptda.targetId = 0x19/*Actors: Any*/; break; }
				}

			}
			else if (ptda.targetType == 0 || ptda.targetType == 1 || ptda.targetType == 3) {
				ptda.targetId = convertFormid(p.PTDT->targetId);
			}
			else {
				ptda.targetId = p.PTDT->targetId;
			}
		}
		else
		{
			//WTM:  Added:  Change:  Instead of throwing an exception, I'm trying something new.
			//throw std::runtime_error("Trying to convert a target type from an oblivion AIPack that does not have a target type!");
			if (!p.PLDT.IsLoaded())
			{
				throw std::runtime_error("Trying to convert a target type from an oblivion AIPack that does not have a target type!");
			}
			ptda.targetCount = 1;
			ptda.targetType = 0;
			ptda.targetId = convertFormid(p.PLDT.value->locId);
		}
        return ptda;
    }

    Sk::PACKRecord::PACKPLDT SkyblivionConverter::convertLocationType(Ob::PACKRecord &p) {

        if (!p.PLDT.IsLoaded()) {
            long defaultRadius = 500;
            //if not loaded, then near self.
            Sk::PACKRecord::PACKPLDT pack = Sk::PACKRecord::PACKPLDT();
            pack.locType = 12;//Near Self
            pack.locId = 0;
            pack.locRadius = defaultRadius;
            return pack;
        }

        Sk::PACKRecord::PACKPLDT pack = Sk::PACKRecord::PACKPLDT();
        if (p.PLDT->locType == 2) {
            pack.locType = 12;
        }
        else if (p.PLDT->locType == 4 || p.PLDT->locType == 5) { //object id object type is hard to convert - so we omit.
            throw std::runtime_error("Object ID/Object Type is hard to convert as a location in skyrim - we omit it.");
        }
        else {
            pack.locType = p.PLDT->locType;
        }

        if (pack.locType == 0 || pack.locType == 1 || pack.locType == 2 || pack.locType == 6 || pack.locType == 9) {
            pack.locId = convertFormid(p.PLDT->locId & 0xFFFFFF);
        }
        else {
            pack.locId = p.PLDT->locId;
        }
        pack.locRadius = p.PLDT->locRadius;


        return pack;

    }

	Sk::PACKRecord::PACKPDTO SkyblivionConverter::convertTopicType(Ob::PACKRecord& p) {//WTM:  Added

		Sk::PACKRecord::PACKPDTO pdto = Sk::PACKRecord::PACKPDTO();
		return pdto;

	}


    std::vector<Sk::SKCondition*> SkyblivionConverter::convertCTDAFromOblivion(GENCTDA* oCTDA) {

		std::vector<Sk::SKCondition*> result = std::vector<Sk::SKCondition*>();
        //Convert the operator.
        unsigned char oOperType = oCTDA->operType >> 4;
        unsigned char oOperFlags = oCTDA->operType & 0x0F;

        unsigned char sOperType = oOperType / 2;
        unsigned char sOperFlags = (oOperFlags & ~0x02); //Remove run on target oblivion field.;

		//If we want to set CIS1/CIS2 for a given condition..
		char *dstCIS1 = NULL;
		char *dstCIS2 = NULL;
		

		FORMID_OR_UINT32 parameterOne = oCTDA->param1;
		FORMID_OR_UINT32 parameterTwo = oCTDA->param2;

		Function_Arguments_Iterator curCTDAFunction = Function_Arguments.find(oCTDA->ifunc);
		if (curCTDAFunction != Function_Arguments.end())
		{
			const FunctionArguments &CTDAFunction = curCTDAFunction->second;
			if (CTDAFunction.first == eFORMID) {
				parameterOne = convertFormid(parameterOne);
			}
			if (CTDAFunction.second == eFORMID) {
				parameterTwo = convertFormid(parameterTwo);
			}
		}

		FORMID_OR_FLOAT32 comparisionValue;

		if ((sOperFlags & 0x04) == 0x04) {
			comparisionValue = convertFormid(oCTDA->compValue);
		}
		else {
			comparisionValue = oCTDA->compValue;
		}

		uint32_t runOn;
		if ((oOperFlags & 0x02) == 0x02) { //Has run on target flag?
			runOn = 0x01;
		}
		else {
			runOn = 0x00;
		}

		//Convert the indice.
		int skyrimIndice;

        switch (oCTDA->ifunc) {

        case 127: { skyrimIndice = 497; break; }
        case 81: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 274: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 264: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 116: { skyrimIndice = 459; parameterOne = 0x010CBF23; break; } //Force the TES4CyrodiilCrimeFaction check
        case 180: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 76: { skyrimIndice = 14; parameterOne = 0x44; break; } //GetDisposition => GetActorValue(Variable01)
        case 215: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 128: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 305: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 224: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 197: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 201: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 249: { skyrimIndice = 74; parameterOne = 0x010223C5; break; } //TES4Fame  
		case 251: { skyrimIndice = 74; parameterOne = 0x011955E4; break; } //TES4Infamy 
		case 365/*GetPlayerInSEWorld*/: { skyrimIndice = 359/*GetInCurrentLoc*/; parameterOne = 0x01193002/*TES4SEWorldLocation*/; break; }
		case 362: { skyrimIndice = 339; break;  }
        case 259: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 258: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 40: { skyrimIndice = 71; parameterOne = 0x01033F53; break; } //force tes4vampire faction check //{ skyrimIndice = 226; break;  }
        case 109: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 227: {  skyrimIndice = 226; break; } //Inaccurate?
        case 313: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 171: { 
				skyrimIndice = 499;
				float constOne = 1.0f;
				float floatComparisionValue = reinterpret_cast<float&>(comparisionValue);
				if (floatComparisionValue == constOne) {
					sOperType = 2;
				} else {
					sOperType = 0;
				}
				runOn = 0x1;
				comparisionValue = 0;
				break;  
		} //IsPlayerInJail == 0 -> GetDaysInJail == 0,  IsPlayerInJail == 1 -> GetDaysInJail > 0
		case 58: {
			/*
			 * In TES Games, Stage state is represented by a hashmap <int, bool>, for example:
			 * 0 => false
			 * 10 => false
			 * 20 => true
			 * 30 => true
			 * In oblivion, when doing a GetStage comparision, code would look for hashmap entries matching the predicate and check if any of them are true
			 * In skyrim however, the GetStage result is evaluated permanently, by scanning all hashmap results and taking the MAX key that had true value
			 * This can screw up two scenarios:
			 * 1) GetStage < N, in Oblivion it would not fire initially, because on a non-started quests, all states = false,
			 * however in Skyrim, it would return MAX(keys) = 0, and since 0 < N is true for any N, it would fire in all circumstances.
			 * 2) GetStage == 0, same drill as above
			 */
			
			skyrimIndice = 58;

			/**
			 * GetStage(Q) < N, convert to (IsQuestRunning(Q) == 1 && GetStage(Q) < N)
			 */
			/*
			if (oOperType == 8 || oOperType == 10 || (oOperType == 0 && comparisionValue == 0)) {
				Sk::SKCTDA *isRunning = new Sk::SKCTDA();
				isRunning->operType = 0;
				isRunning->compValue = 0x3f800000; //1.0f
				isRunning->ifunc = 56;
				isRunning->param1 = parameterOne;
				isRunning->param2 = 0;
				isRunning->runOnType = 0;
				isRunning->reference = 0;
				result.push_back(isRunning);
			}
			**/
			
			break;
		}		
		case 329: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 101: { skyrimIndice = 263; break; }
        case 104: { throw std::runtime_error("Not implemented in Skyrim"); }
        case 323: { throw std::runtime_error("Not implemented in Skyrim"); }
		case 125: { skyrimIndice = 68; parameterOne = 0x01034E6E; break; } //IsGuard -> GetIsClass(TES4Guard)

		case 53: {

			/**
			* Convert to GetVMScriptVariable
			*/

			if (oCTDA->param1 == NULL)
			{
				//Damaged oblivion record.. just skip the ctda
				return result;
			}


			Record *rec = findRecordByFormid(oCTDA->param1, REV32(ACHR));
			FORMID scriptId = 0;
			if (rec != NULL) {
				Ob::ACHRRecord *achr = (Ob::ACHRRecord *)rec;
				Record *crec = findRecordByFormid(achr->NAME.value, REV32(NPC_));

				if (crec == NULL)
				{
					throw new std::runtime_error("Cannot find base NPC_ for GetScriptVariable conversion.");
				}

				Ob::NPC_Record *npc = (Ob::NPC_Record*)crec;
				scriptId = npc->SCRI.value;
			}
			else {
				rec = findRecordByFormid(oCTDA->param1, REV32(ACRE));
				if (rec != NULL) {
					Ob::ACRERecord *acre = (Ob::ACRERecord *)rec;
					Record *crec = findRecordByFormid(acre->NAME.value, REV32(CREA));

					if (crec == NULL)
					{
						throw new std::runtime_error("Cannot find base CREA for GetScriptVariable conversion.");
					}

					Ob::CREARecord *crea = (Ob::CREARecord*)crec;
					scriptId = crea->SCRI.value;
				}
				else {
					rec = findRecordByFormid(oCTDA->param1, REV32(REFR));

					if (rec != NULL) {
						Ob::REFRRecord *refr = (Ob::REFRRecord *)rec;
						Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(ACTI));
						if (crec != NULL) {
							Ob::ACTIRecord* ACTIrec = (Ob::ACTIRecord*)crec;
							scriptId = ACTIrec->SCRI.value;
						}
						else {
							Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(ALCH));
							if (crec != NULL) {
								Ob::ALCHRecord* ALCHrec = (Ob::ALCHRecord*)crec;
								scriptId = ALCHrec->SCRI.value;
							}
							else {
								Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(APPA));
								if (crec != NULL) {
									Ob::APPARecord* APPArec = (Ob::APPARecord*)crec;
									scriptId = APPArec->SCRI.value;
								}
								else {
									Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(ARMO));
									if (crec != NULL) {
										Ob::ARMORecord* ARMOrec = (Ob::ARMORecord*)crec;
										scriptId = ARMOrec->SCRI.value;
									}
									else {
										Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(BOOK));
										if (crec != NULL) {
											Ob::BOOKRecord* BOOKrec = (Ob::BOOKRecord*)crec;
											scriptId = BOOKrec->SCRI.value;
										}
										else {
											Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(CLOT));
											if (crec != NULL) {
												Ob::CLOTRecord* CLOTrec = (Ob::CLOTRecord*)crec;
												scriptId = CLOTrec->SCRI.value;
											}
											else {
												Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(CONT));
												if (crec != NULL) {
													Ob::CONTRecord* CONTrec = (Ob::CONTRecord*)crec;
													scriptId = CONTrec->SCRI.value;
												}
												else {
													Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(DOOR));
													if (crec != NULL) {
														Ob::DOORRecord* DOORrec = (Ob::DOORRecord*)crec;
														scriptId = DOORrec->SCRI.value;
													}
													else {
														Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(FLOR));
														if (crec != NULL) {
															Ob::FLORRecord* FLORrec = (Ob::FLORRecord*)crec;
															scriptId = FLORrec->SCRI.value;
														}
														else {
															Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(FURN));
															if (crec != NULL) {
																Ob::FURNRecord* FURNrec = (Ob::FURNRecord*)crec;
																scriptId = FURNrec->SCRI.value;
															}
															else {
																Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(INGR));
																if (crec != NULL) {
																	Ob::INGRRecord* INGRrec = (Ob::INGRRecord*)crec;
																	scriptId = INGRrec->SCRI.value;
																}
																else {
																	Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(KEYM));
																	if (crec != NULL) {
																		Ob::KEYMRecord* KEYMrec = (Ob::KEYMRecord*)crec;
																		scriptId = KEYMrec->SCRI.value;
																	}
																	else {
																		Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(LIGH));
																		if (crec != NULL) {
																			Ob::LIGHRecord* LIGHrec = (Ob::LIGHRecord*)crec;
																			scriptId = LIGHrec->SCRI.value;
																		}
																		else {
																			Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(LVLC));
																			if (crec != NULL) {
																				Ob::LVLCRecord* LVLCrec = (Ob::LVLCRecord*)crec;
																				scriptId = LVLCrec->SCRI.value;
																			}
																			else {
																				Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(MISC));
																				if (crec != NULL) {
																					Ob::MISCRecord* MISCrec = (Ob::MISCRecord*)crec;
																					scriptId = MISCrec->SCRI.value;
																				}
																				else {
																					Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(QUST));
																					if (crec != NULL) {
																						Ob::QUSTRecord* QUSTrec = (Ob::QUSTRecord*)crec;
																						scriptId = QUSTrec->SCRI.value;
																					}
																					else {
																						Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(SGST));
																						if (crec != NULL) {
																							Ob::SGSTRecord* SGSTrec = (Ob::SGSTRecord*)crec;
																							scriptId = SGSTrec->SCRI.value;
																						}
																						else {
																							Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(SLGM));
																							if (crec != NULL) {
																								Ob::SLGMRecord* SLGMrec = (Ob::SLGMRecord*)crec;
																								scriptId = SLGMrec->SCRI.value;
																							}
																							else {
																								Record* crec = findRecordByFormid(refr->Data.value->NAME.value, REV32(WEAP));
																								if (crec != NULL) {
																									Ob::WEAPRecord* WEAPrec = (Ob::WEAPRecord*)crec;
																									scriptId = WEAPrec->SCRI.value;
																								}
																								else {
																									throw new std::runtime_error("Cannot find refr base obj for GetScriptVariable conversion.");
																								}
																							}
																						}
																					}
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
					else {
						throw new std::runtime_error("Cannot find refr/achr/acre for GetScriptVariable conversion.");

					}

				}
			}

			Record *srec = findRecordByFormid(scriptId, REV32(SCPT));

			//Actually this happens in vanilla oblivion too..
			if (srec == NULL) {
				char buffer[50];
				sprintf(buffer, "GetVMScriptVariable %04x No script found\n", oCTDA->param1);
				log_warning << buffer;
				return result;
			}

			Ob::SCPTRecord* script = (Ob::SCPTRecord*)srec;

			struct GENVARS *targetVariable = *std::find_if(script->VARS.value.begin(), script->VARS.value.end(), [=](const GENVARS* record) { return record->SLSD.value.index == oCTDA->param2;  });

			if (targetVariable == NULL) {
				throw new std::runtime_error("Cannot find script for GetQuestVariable conversion.");
			}

			std::string papyrusVariableName = "::" + std::string(targetVariable->SCVR.value) + "_p_var";

			dstCIS2 = new char[papyrusVariableName.size() + 1];
			memcpy(dstCIS2, papyrusVariableName.c_str(), papyrusVariableName.size() + 1);

			skyrimIndice = 630;

			break;

		}

		case 79: {
			/**
			 * Convert to GetVMQuestVariable
			 */
			Record *rec = findRecordByFormid(oCTDA->param1, REV32(QUST));

			if (rec == NULL) {
				throw new std::runtime_error("Cannot find quest for GetQuestVariable conversion.");
			}

			Ob::QUSTRecord* p = (Ob::QUSTRecord*)rec;

			Record *srec = findRecordByFormid(p->SCRI.value, REV32(SCPT));

			if (srec == NULL) {
				throw new std::runtime_error("Cannot find script for GetQuestVariable conversion.");
			}

			Ob::SCPTRecord* script = (Ob::SCPTRecord*)srec;

			struct GENVARS *targetVariable = *std::find_if(script->VARS.value.begin(), script->VARS.value.end(), [=](const GENVARS* record) { return record->SLSD.value.index == oCTDA->param2;  });

			if (targetVariable == NULL) {
				throw new std::runtime_error("Cannot find script for GetQuestVariable conversion.");
			}

			std::string papyrusVariableName = "::" + std::string(targetVariable->SCVR.value) + "_p_var";

			dstCIS2 = new char[papyrusVariableName.size() + 1];
			memcpy(dstCIS2, papyrusVariableName.c_str(), papyrusVariableName.size() + 1);

			skyrimIndice = 629;

			break;

		}


		case 14:
		case 277: {
			std::vector<FORMID_OR_UINT32> indexMap(72, 0xA5);

			indexMap[8] = 0x18; //Health
			indexMap[9] = 0x19; //Magicka
			indexMap[10] = 0x20; //Fatigue -> Stamina
			indexMap[11] = 0x1F; //Encumbrance -> InventoryWeight
			indexMap[12] = 0xA; //Armorer -> Smithing
			indexMap[14] = 0x6; //Blade -> OneHanded
			indexMap[15] = 0x9; //Block
			indexMap[16] = 0x6; //Blunt -> OneHanded
			indexMap[18] = 0xB; //HeavyArmor
			indexMap[19] = 0x10; //Alchemy
			indexMap[20] = 0x12; //Alteration
			indexMap[21] = 0x13; //Conjuration
			indexMap[22] = 0x14; //Destruction
			indexMap[23] = 0x15; //Illusion
			indexMap[24] = 0x15; //Mysticism -> Illusion
			indexMap[25] = 0x16; //Restoration
			indexMap[26] = 0xF; //Acrobatics -> Sneak
			indexMap[27] = 0xC; //LightArmor
			indexMap[28] = 0x8; //Marksman
			indexMap[29] = 0x11; //Mercantile -> Speechcraft
			indexMap[30] = 0xE; //Security -> Lockpicking
			indexMap[31] = 0xF; //Sneak
			indexMap[32] = 0x11; //Speechcraft
			indexMap[35] = 0x2; //Energy
			indexMap[45] = 0x54; //Blindness
			indexMap[47] = 0x36; //Invisibility
			indexMap[48] = 0x35; //Paralysis
			indexMap[52] = 0x53; //SpellAbsorbChance -> AbsorbChance
			indexMap[55] = 0x39; //WaterBreathing
			indexMap[56] = 0x3A; //WaterWalking
			indexMap[61] = 0x29; //ResistFire -> FireResist
			indexMap[62] = 0x2B; //ResistFrost -> FrostResist
			indexMap[63] = 0x2D; //ResistDisease -> DiseaseResist
			indexMap[64] = 0x2C; //ResistMagic -> MagicResist
			indexMap[67] = 0x28; //ResistPoison -> PoisonResist
			indexMap[68] = 0x2A; //ResistShock -> ElectricResist

			/**
			 * Change GetActorValue(Fame/Infamy) to checking global value. By the way, this behavior is even not recommended by CS.elderscrolls manual yet they were doing it.. <facepalm> 
			 */
			if (parameterOne == 0x26) {
				skyrimIndice = 74;
				parameterOne = 0x010223C5;
			}
			else if (parameterOne == 0x27) {
				skyrimIndice = 74;
				parameterOne = 0x011955E4;
			}
			else {
				skyrimIndice = oCTDA->ifunc;
				if (indexMap[parameterOne] == 0xA5) {
					throw std::runtime_error("Actor value is not implemented yet");
				}
				parameterOne = indexMap[parameterOne];
			}

			break;
		}

		case 52480:
		case 29281:
		case 51232:
		case 53168:
		case 13568:
		case 12615: {
			skyrimIndice = oCTDA->ifunc;
			break;
		}
		case 67: {//GetInCell
			skyrimIndice = 359;//GetInCurrentLoc
			FORMID tes4CELLFormID = oCTDA->param1;
			FORMID tes5CELLFormID = convertFormid(tes4CELLFormID);
			std::pair<int, std::string> lctn = cellToLCTNMap.getLCTN(tes5CELLFormID);
			int lctnFormID = lctn.first;
			std::string lctnEditorID = lctn.second;
			parameterOne = lctnFormID;
			if (tes4CELLFormID != NULL)//In at least one circumstance, oCTDA->param1 = 0, which will ultimately result in parameterOne being 0.
			{
				Ob::CELLRecord* cell = (Ob::CELLRecord*)findRecordByFormid(tes4CELLFormID, REV32(CELL));
				std::string cellEditorID = cell->EDID.value;
				std::string likelyLocationEditorID = "TES4" + cellEditorID + "Location";
				std::string likelyLocationEditorIDLower = likelyLocationEditorID;
				std::string lctnEditorIDLower = lctnEditorID;
				std::transform(likelyLocationEditorIDLower.begin(), likelyLocationEditorIDLower.end(), likelyLocationEditorIDLower.begin(), tolower);
				std::transform(lctnEditorIDLower.begin(), lctnEditorIDLower.end(), lctnEditorIDLower.begin(), tolower);
				if (likelyLocationEditorIDLower != lctnEditorIDLower)
				{
					log_warning << "CELL " << cellEditorID << " mapped to LCTN " << lctnEditorID << "\r\n";
				}
			}
			break;
		}

        default: { skyrimIndice = oCTDA->ifunc; }
        }

		Sk::SKCTDA* dstRecord = new Sk::SKCTDA();
		dstRecord->operType = (sOperType << 5) + sOperFlags;
		dstRecord->compValue = comparisionValue;
        dstRecord->ifunc = (Sk::SKCTDA::function)skyrimIndice;
		dstRecord->param1 = parameterOne;
		dstRecord->param2 = parameterTwo;
		dstRecord->runOnType = runOn;
        dstRecord->reference = 0;

		Sk::SKCondition * dstRecordAggr = new Sk::SKCondition();
		dstRecordAggr->CTDA.value = *dstRecord;

		if (dstCIS1 != NULL) {
			dstRecordAggr->CIS1.Copy(dstCIS1);
		}

		if (dstCIS2 != NULL) {
			dstRecordAggr->CIS2.Copy(dstCIS2);
		}

		result.push_back(dstRecordAggr);
		return result;


    }

	std::string SkyblivionConverter::getTIFScriptNameNotExt(Ob::INFORecord* info)
	{
		std::string scriptName = "TIF_";
		if (info->EDID.IsLoaded()) {
			scriptName.append(info->EDID.value);
		}
		scriptName.append("_");
		char* castedFormid = new char[9];
		sprintf(castedFormid, "%08x", convertFormid(info->formID));
		scriptName.append(castedFormid);
		scriptName = SkyblivionScript::getPrefixedName(TES4_UNDERSCORE_SCRIPT_PREFIX, scriptName);
		delete castedFormid;
		return scriptName;
	}

	void SkyblivionConverter::bindScriptProperties(std::vector<Sk::DIALRecord*>* dialogueVector, std::vector<Sk::QUSTRecord *>* questVector) {
		TES5File* skyrimMod = (TES5File*)this->skyrimCollection.ModFiles[2];

		for (auto it = infosToBind.begin(); it != infosToBind.end(); ++it) {
			Ob::INFORecord* info = it->first;
			Sk::INFORecord* newInfo = it->second;

			Script* TIFScript = new Script();

			std::string TIFScriptName = getTIFScriptNameNotExt(info);

			char* cpyTwo = new char[TIFScriptName.size() + 1];
			strcpy(cpyTwo, TIFScriptName.c_str());

			TIFScript->name.Copy(cpyTwo);

			FragmentINFO* infoFragment = new FragmentINFO();
			infoFragment->unk1 = 0;
			unsigned char* tifpointer = reinterpret_cast<unsigned char*>(TIFScript->name.value);
			infoFragment->fileName.Read(tifpointer, TIFScript->name.GetSize(), false);

			infoFragment->flags = 0x01; //Has onEnd

			std::string fragmentFragmentName = "Fragment_0";

			GenFragment* infoFragmentFragment = new GenFragment();
			infoFragmentFragment->fragmentName.Copy(const_cast<char *>(fragmentFragmentName.c_str()));


			unsigned char* tifsubpointer = reinterpret_cast<unsigned char*>(TIFScript->name.value);
			infoFragmentFragment->scriptName.Read(tifsubpointer, TIFScript->name.GetSize(), false);
			infoFragmentFragment->unk1 = 1;

			infoFragment->fragments.push_back(infoFragmentFragment);



			try {
				SkyblivionScript skScript = SkyblivionScript(this->BUILD_TARGET_DIALOGUES(), "", TIFScriptName, this->ROOT_BUILD_PATH());
				bindProperties(TIFScript, skScript);
				newInfo->VMAD.value = new VMADRecord();
				newInfo->VMAD.value->scripts.push_back(TIFScript);
				newInfo->VMAD.value->fragment = infoFragment;
			}
			catch (std::exception except) {
				log_warning << except.what() << "\n";
			}
			/* TODO - Check if needed, probably not
			std::string outputPath = scriptOutputPath + "/" + std::string(TIFScript->name.value) + ".psc";
			FILE* handle = fopen(outputPath.c_str(), "w+");
			std::string TIFFragmentScript = std::string(info->SCTX.value);
			const char* outputData = TIFFragmentScript.c_str();
			fwrite(outputData, 1, TIFFragmentScript.size(), handle);
			fclose(handle);
			*/
		}

		for (uint32_t i = 0; i < dialogueVector->size(); i++) {
			skyrimMod->DIAL.dial_pool.construct(dialogueVector->at(i), NULL, false);
		}

		std::vector<Record*, std::allocator<Record*>> scripts;
		TES4File* d = (TES4File *)this->oblivionCollection.ModFiles[0];
		d->SCPT.pool.MakeRecordsVector(scripts);

		for (auto it = qustsToBind.begin(); it != qustsToBind.end(); ++it) {
			Ob::QUSTRecord* qust = it->first;
			Sk::QUSTRecord* newQust = it->second;
			
			//WTM:  Change:  Added
			/*Ob::SCPTRecord* script = reinterpret_cast<Ob::SCPTRecord*>(*std::find_if(scripts.begin(), scripts.end(), [=](const Record* record) { return record->formID == qust->SCRI.value;  }));
			try {
				Script* convertedScript = createVirtualMachineScriptFor(script);
				newQust->VMAD.value->scripts.push_back(convertedScript);
			}
			catch (std::exception &ex)
			{
				log_warning << "Cannot bind script to QUST: " << std::string(ex.what()) << std::endl;
				//Silent catch - just do not do anything.
			}*/
			for (auto scriptIt = scripts.begin(); scriptIt != scripts.end(); ++scriptIt)
			{
				if ((*scriptIt)->formID == qust->SCRI.value)
				{
					Ob::SCPTRecord* script = (Ob::SCPTRecord*)*scriptIt;
					try {
						Script* convertedScript = createVirtualMachineScriptFor(script);
						newQust->VMAD.value->scripts.push_back(convertedScript);
					}
					catch (std::exception & ex)
					{
						log_warning << "Cannot bind script to QUST: " << std::string(ex.what()) << std::endl;
						//Silent catch - just do not do anything.
					}
				}
			}
		}

		for (auto it = qfsToBind.begin(); it != qfsToBind.end(); ++it) {
			std::string qfName = it->first;
			Script* qfScript = it->second;

			try {
				SkyblivionScript skScript = SkyblivionScript(this->BUILD_TARGET_QUESTS(), "", qfName, this->ROOT_BUILD_PATH());
				bindProperties(qfScript, skScript);
			}
			catch (std::exception ex) {
				log_warning << ex.what() << "\n";
			}
		}

		for (uint32_t i = 0; i < questVector->size(); i++) {
			skyrimMod->QUST.pool.construct(questVector->at(i), NULL, false);
		}

		// clear maps to prevent re-binding
		infosToBind.clear();
		qustsToBind.clear();
		qfsToBind.clear();
	}
}
/*
Manual Changes for TES4Charactergen:
Renote's stage 18 package (TES4CGRenoteOpenSecretDoor) "Once per Day" should be unchecked.

Manual Changes for TES4Charactergen for Others:
The AI for Glenroy, Renote and Baurus is set to Unaggressive.  This needs to be changed to Aggressive.
TES4BladesCG faction has TES4CGCreatureFaction, TES4MythicDawnCG and TES4MythicDawnCGAssassin Factions as Allies.  That should be changed at least to enemies.

Notes to Make TES4Charactergen Complete:
One of the following scripts is apparently supposed to move the quest to stage 27, but they don't:
	TES4_tif__010178c8.psc
	TES4_tif__01025df0.psc
	TES4_tif__0105144a.psc
The assassin behind the trap door is 0501e703.  Use console prid 0501e703 to disable and enable him if needed.
*/