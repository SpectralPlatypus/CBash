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
#include "../../GenericChunks.h"
#include "SNDRRecord.h"
#include <vector>

namespace Sk {


	SNDRRecord::SNDRRecord(unsigned char *_recData) :
        TES5Record(_recData)
    {
        //
    }

	SNDRRecord::SNDRRecord(SNDRRecord *srcRecord) :
        TES5Record()
    {
        if (srcRecord == NULL)
            return;

        flags = srcRecord->flags;
        formID = srcRecord->formID;
        flagsUnk = srcRecord->flagsUnk;
        formVersion = srcRecord->formVersion;
        versionControl2[0] = srcRecord->versionControl2[0];
        versionControl2[1] = srcRecord->versionControl2[1];

        EDID = srcRecord->EDID;
		CNAM = srcRecord->CNAM;
		GNAM = srcRecord->GNAM;
		SNAM = srcRecord->SNAM;
		ANAM = srcRecord->ANAM;
		ONAM = srcRecord->ONAM;
		FNAM = srcRecord->FNAM;	
		CTDA = srcRecord->CTDA;
		LNAM = srcRecord->LNAM;
		BNAM = srcRecord->BNAM;

        recData = srcRecord->recData;
        if (!srcRecord->IsChanged())
            return;

        return;
    }

	SNDRRecord::~SNDRRecord()
    {
        //
    }

    uint32_t SNDRRecord::GetType()
    {
        return REV32(SNDR);
    }

    char * SNDRRecord::GetStrType()
    {
        return "SNDR";
    }


    bool SNDRRecord::VisitFormIDs(FormIDOp &op)
    {
        if (!IsLoaded())
            return false;

		op.Accept(GNAM.value);
		if (SNAM.IsLoaded() && SNAM.value != NULL)
			op.Accept(*SNAM.value);
		op.Accept(ONAM.value);

        return op.Stop();
    }

    int32_t SNDRRecord::ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk)
    {
        uint32_t subType = 0;
        uint32_t subSize = 0;
		uint32_t nextSubType = 0;
        SKCondition *current_condition = NULL;
		StringRecord* this_ANAM = NULL;

        while (buffer < end_buffer){
            subType = *(uint32_t *)buffer;
            buffer += 4;
            switch (subType)
            {
            case REV32(XXXX):
                buffer += 2;
                subSize = *(uint32_t *)buffer;
                buffer += 4;
                subType = *(uint32_t *)buffer;
                buffer += 6;
                break;
            default:
                subSize = *(uint16_t *)buffer;
                buffer += 2;
                break;
            }
            switch (subType)
            {
            case REV32(EDID):
                EDID.Read(buffer, subSize, CompressedOnDisk);
                break;
			case REV32(CNAM):
				CNAM.Read(buffer, subSize);
				break;
			case REV32(GNAM):
				GNAM.Read(buffer, subSize);
				break;
			case REV32(SNAM):
				SNAM.Read(buffer, subSize);
				break;
			case REV32(FNAM):
				FNAM.Read(buffer, subSize, CompressedOnDisk);
				break;
			case REV32(ANAM):
				this_ANAM = new StringRecord(); 
				this_ANAM->Read(buffer, subSize, CompressedOnDisk);
				ANAM.push_back(*this_ANAM);
				delete this_ANAM;
				break;
			case REV32(ONAM):
				ONAM.Read(buffer, subSize);
				break;
			case REV32(CTDA):
				current_condition = new SKCondition();
				CTDA.value.push_back(current_condition);
				current_condition->CTDA.Read(buffer, subSize);
				break;
			case REV32(CIS1):
				if (current_condition == NULL) {
					printer("  INFO: %08X - Reading CIS1 without current_condition set\n", formID);
					CBASH_CHUNK_DEBUG
						printer("  Size = %i\n", subSize);
					printer("  CurPos = %08x\n\n", buffer - 6);
					buffer = end_buffer;
				}
				else {
					current_condition->CIS1.Read(buffer, subSize, CompressedOnDisk);
				}

				break;

			case REV32(CIS2):
				if (current_condition == NULL) {
					printer("  INFO: %08X - Reading CIS2 without current_condition set\n", formID);
					CBASH_CHUNK_DEBUG
						printer("  Size = %i\n", subSize);
					printer("  CurPos = %08x\n\n", buffer - 6);
					buffer = end_buffer;
				}
				else {
					current_condition->CIS2.Read(buffer, subSize, CompressedOnDisk);
				}
			case REV32(LNAM):
				LNAM.Read(buffer, subSize);
				break;
			case REV32(BNAM):
				BNAM.Read(buffer, subSize);
				break;

            default:
                CBASH_SUBTYPE_UNKNOWN
                CBASH_CHUNK_DEBUG
                buffer = end_buffer;
                break;
            }
        };
        return 0;
    }

    int32_t SNDRRecord::Unload()
    {
        IsChanged(false);
        IsLoaded(false);

		EDID.Unload();
		CNAM.Unload();
		GNAM.Unload();
		SNAM.Unload();
		FNAM.Unload();
		for (auto& anam : ANAM) {
			anam.Unload();
		}
		ONAM.Unload();
		CTDA.Unload();
		LNAM.Unload();
		BNAM.Unload();

        return 1;
    }

    int32_t SNDRRecord::WriteRecord(FileWriter &writer)
    {

        WRITE(EDID);
        WRITE(CNAM);
		WRITE(GNAM);
		WRITE(SNAM);
		for (auto& anam : ANAM) {
			anam.Write(REV32(ANAM), writer);
		}
		WRITE(ONAM);
		WRITE(FNAM);
		CTDA.Write(writer);
		WRITE(LNAM);
		WRITE(BNAM);
        return -1;
    }

    bool SNDRRecord::operator ==(const SNDRRecord &other) const
    {
        return (EDID.equalsi(other.EDID) &&
			CNAM == other.CNAM &&
			GNAM == other.GNAM &&
			SNAM == other.SNAM &&
			FNAM.equals(other.FNAM) &&
			std::equal(ANAM.begin(), ANAM.end(), other.ANAM.begin(), other.ANAM.end(), [](const StringRecord& a, const StringRecord& b) {return a.equals(b); }) &&
			ONAM == other.ONAM &&
			CTDA == other.CTDA &&
			LNAM == other.LNAM &&
			BNAM == other.BNAM
            );
    }

    bool SNDRRecord::operator !=(const SNDRRecord &other) const
    {
        return !(*this == other);
    }

    bool SNDRRecord::equals(Record *other)
    {
        return *this == *(SNDRRecord *)other;
    }

}