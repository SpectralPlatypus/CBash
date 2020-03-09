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
#include "../../Common.h"
#include "../../GenericRecord.h"
#include "../SkyrimChunks.h"
#include "../VMAD/VMAD.h"
#include <vector>

namespace Sk {

	struct SNDRLNAM {

		uint16_t flags;
		uint8_t unused;
		uint8_t rumbleSend;

		SNDRLNAM()
		{
		}

		bool operator ==(const SNDRLNAM &other) const {
			return (
				flags == other.flags &&
				unused == other.unused &&
				rumbleSend == other.rumbleSend &&
				flags == other.flags
			);
		}

		bool operator !=(const SNDRLNAM &other) const
		{
			return !(*this == other);
		}
	};

	struct SNDRBNAM {

		int8_t percentageFrequencyShift;
		uint8_t percentageFrequencyVariance;
		uint8_t priority = 128;
		uint8_t dBVariance;
		uint16_t staticAttenuation; // (dB)Stored value is 100 times actual value

		SNDRBNAM()
		{
		}

		bool operator ==(const SNDRBNAM &other) const {
			return (
				percentageFrequencyShift == other.percentageFrequencyShift &&
				percentageFrequencyVariance == other.percentageFrequencyVariance &&
				priority == other.priority &&
				dBVariance == other.dBVariance &&
				staticAttenuation == other.staticAttenuation
				);
		}

		bool operator !=(const SNDRBNAM &other) const
		{
			return !(*this == other);
		}
	};

	class SNDRRecord : public TES5Record {
	
    public:
        StringRecord EDID; //Editor ID
		ReqSubRecord<uint32_t> CNAM; //hkCRC32 hash of the class name (i.e. "BGSStandardSoundDef").
		ReqSubRecord<FORMID> GNAM; //Sound Category (SNCT) formid
		OptSubRecord<FORMID> SNAM; //reference to another (SNDR) formid
		std::vector<StringRecord> ANAM; //path to.wav
		ReqSubRecord<FORMID> ONAM; //Output Model (SOPM) formid
		StringRecord FNAM; //tesvedit	
		UnorderedSparseArray<SKCondition*> CTDA; //Conditions
		OptSubRecord<SNDRLNAM> LNAM; //struct
		ReqSubRecord<SNDRBNAM> BNAM; //struct
		
		SNDRRecord(unsigned char *_recData = NULL);
		SNDRRecord(SNDRRecord *srcRecord);
		~SNDRRecord();

		uint32_t  GetType();
		char *  GetStrType();

		bool   VisitFormIDs(FormIDOp &op);

		int32_t  ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk);
		int32_t  Unload();
		int32_t  WriteRecord(FileWriter &writer);
		char *GetEditorIDKey() { return EDID.value; }


        bool operator ==(const SNDRRecord &other) const;
        bool operator !=(const SNDRRecord &other) const;
        bool equals(Record *other);

	};


}