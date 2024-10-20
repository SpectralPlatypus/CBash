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
*  Jacob "Lojack" Lojewski
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
#include "../../GenericRecord.h"
#include "../SkyrimChunks.h"
#include "../SkyrimCommon.h"

namespace Sk {

	struct ARMODATA {
		uint32_t value;
		float weight;

		bool operator ==(const ARMODATA  &other) const {

			return (this->value == other.value &&
				this->weight == other.weight);

		}

		bool operator !=(const ARMODATA &other) const {
			return !(*this == other);
		}
	};


	class ARMORecord : public TES5Record // Acoustic Space Record
	{
	public:
		StringRecord EDID; // Editor ID
		VMADRecord VMAD; // VM data
		ReqSubRecord<GENOBND> OBND; // Object Bounds
		LStringRecord FULL; // ambient
		OptSimpleSubRecord<FORMID> EITM; // Enchantment
		OptSimpleSubRecord<uint16_t, 0> EAMT; // Enchantment Amount
		StringRecord MOD2;
		RawRecord MO2T;
		RawRecord MO2S;
		StringRecord ICON;
		StringRecord MICO;
		StringRecord MOD4;
		RawRecord MO4T;
		RawRecord MO4S;
		StringRecord ICO2;
		StringRecord MIC2;
		RawRecord BODT;
		ReqSubRecord<SKBOD2> BOD2;
		OptSubRecord<GENDESTRUCT> DEST; // Destruction data
		ReqSimpleSubRecord<FORMID> YNAM; // Sound - pick up
		ReqSimpleSubRecord<FORMID> ZNAM; // Sound - drop
		OptSimpleSubRecord<FORMID> ETYP; // Enchantment
		OptSimpleSubRecord<FORMID> BIDS; // Enchantment
		OptSimpleSubRecord<FORMID> BAMT; // Enchantment
		ReqSimpleSubRecord<FORMID> RNAM; // Enchantment
		OptCounted<OrderedPackedArray<FORMID>, uint32_t, REV32(KSIZ)> KWDA;
		LStringRecord DESC; // Description
		UnorderedSparseArray<FORMID> MODL;
		OptSubRecord<ARMODATA> DATA;
		ReqSimpleSubRecord<uint32_t> DNAM; // Armor rating		
		OptSimpleSubRecord<FORMID> TNAM; // Template
										 //ONAM;

		ARMORecord(unsigned char *_recData = NULL);
		ARMORecord(ARMORecord *srcRecord);
		~ARMORecord();

		bool VisitFormIDs(FormIDOp &op);

		uint32_t GetType();
		char * GetStrType();

		int32_t ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk = false);
		int32_t Unload();
		int32_t WriteRecord(FileWriter &writer);
		char *GetEditorIDKey() { return EDID.value; }

		bool operator ==(const ARMORecord &other) const;
		bool operator !=(const ARMORecord &other) const;
		bool equals(Record *other);
	};
}