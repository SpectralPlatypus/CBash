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
#include "MOVTRecord.h"
#include <vector>

namespace Sk {


	MOVTRecord::MOVTRecord(unsigned char *_recData) :
        TES5Record(_recData)
    {
        //
    }

	MOVTRecord::MOVTRecord(MOVTRecord *srcRecord) :
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
        MNAM = srcRecord->MNAM;
		SPED = srcRecord->SPED;
		INAM = srcRecord->INAM;

        recData = srcRecord->recData;
        if (!srcRecord->IsChanged())
            return;

        return;
    }

	MOVTRecord::~MOVTRecord()
    {
        //
    }

    uint32_t MOVTRecord::GetType()
    {
        return REV32(MOVT);
    }

    char * MOVTRecord::GetStrType()
    {
        return "MOVT";
    }


    bool MOVTRecord::VisitFormIDs(FormIDOp &op)
    {
        if (!IsLoaded())
            return false;

        return op.Stop();
    }

    int32_t MOVTRecord::ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk)
    {
        uint32_t subType = 0;
        uint32_t subSize = 0;
		uint32_t nextSubType = 0;
        SKCondition *current_condition = NULL;

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

            case REV32(MNAM):
				MNAM.Read(buffer, subSize, CompressedOnDisk);
                break;

            case REV32(SPED):
				//read 40 bytes
				static_assert(sizeof(MOVTSPED) == 40, "Error in MOVTSPED");
				static_assert(sizeof(EMOVTSPED) == 44, "Error in EMOVTSPED");
				nextSubType = *(uint32_t *)(buffer+sizeof(MOVTSPED));
				if (nextSubType == REV32(INAM))
					SPED.Read(buffer, subSize);
				else
					ESPED.Read(buffer, subSize);
                break;

            case REV32(INAM):
				INAM.Read(buffer, subSize);
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

    int32_t MOVTRecord::Unload()
    {
        IsChanged(false);
        IsLoaded(false);


        EDID.Unload();
		MNAM.Unload();
		SPED.Unload();
		ESPED.Unload();
		INAM.Unload();

        return 1;
    }

    int32_t MOVTRecord::WriteRecord(FileWriter &writer)
    {
        WRITE(EDID);
        WRITE(MNAM);
		if (INAM.IsLoaded())
		{
			WRITE(SPED);
			WRITE(INAM);
		}
		else
			ESPED.Write(REV32(SPED), writer);

        return -1;
    }

    bool MOVTRecord::operator ==(const MOVTRecord &other) const
    {
        return (EDID.equalsi(other.EDID) &&
			MNAM.equals(other.MNAM) &&
			SPED == other.SPED &&
            ESPED == other.ESPED &&
			INAM == other.INAM
            );
    }

    bool MOVTRecord::operator !=(const MOVTRecord &other) const
    {
        return !(*this == other);
    }

    bool MOVTRecord::equals(Record *other)
    {
        return *this == *(MOVTRecord *)other;
    }

}