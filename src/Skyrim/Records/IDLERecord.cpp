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
#include "../../Common.h"
#include "IDLERecord.h"

namespace Sk
{
	bool IDLEDATA::operator ==(const IDLEDATA &other) const
	{
		return (min == other.min &&
			max == other.max &&
			flags == other.flags &&
			unk == other.unk &&
			replay == other.replay
			);
	}

	bool IDLEDATA::operator !=(const IDLEDATA &other) const
	{
		return !(*this == other);
	}

	bool IDLEANAM::operator ==(const IDLEANAM &other) const
	{
		return (parent == other.parent &&
			sibling == other.sibling);
	}

	bool IDLEANAM::operator !=(const IDLEANAM &other) const
	{
		return !(*this == other);
	}


	IDLERecord::IDLERecord(unsigned char *_recData):
    TES5Record(_recData)
    {
    //
    }

	IDLERecord::IDLERecord(IDLERecord *srcRecord):
    TES5Record()
    {
    if(srcRecord == NULL)
        return;

    flags = srcRecord->flags;
    formID = srcRecord->formID;
    flagsUnk = srcRecord->flagsUnk;
    formVersion = srcRecord->formVersion;
    versionControl2[0] = srcRecord->versionControl2[0];
    versionControl2[1] = srcRecord->versionControl2[1];

    recData = srcRecord->recData;
    if(!srcRecord->IsChanged())
        return;

    EDID = srcRecord->EDID;
	CTDA = srcRecord->CTDA;
	DNAM = srcRecord->DNAM;
	ENAM = srcRecord->ENAM;
	ANAM = srcRecord->ANAM;
	DATA = srcRecord->DATA;

    return;
    }

	IDLERecord::~IDLERecord()
    {
    //
    }
    
uint32_t IDLERecord::GetType()
    {
    return REV32(IDLE);
    }

char * IDLERecord::GetStrType()
    {
    return "IDLE";
    }

bool IDLERecord::VisitFormIDs(FormIDOp &op)
{
    if (!IsLoaded())
        return false;

    op.Accept(ANAM.value.parent);
	op.Accept(ANAM.value.sibling);

    return op.Stop();
}


int32_t IDLERecord::ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk)
    {
    uint32_t subType = 0;
    uint32_t subSize = 0;
	SKCondition *current_condition = NULL;
	bool isFirst = true;
    while(buffer < end_buffer){
        subType = *(uint32_t *)buffer;
        buffer += 4;
        switch(subType)
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
        switch(subType)
            {
            case REV32(EDID):
                EDID.Read(buffer, subSize, CompressedOnDisk);
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

				break;
			case REV32(DNAM):
				DNAM.Read(buffer, subSize, CompressedOnDisk);
				break;
			case REV32(ENAM):
				ENAM.Read(buffer, subSize, CompressedOnDisk);
				break;
			case REV32(ANAM):
				ANAM.Read(buffer, subSize);
				break;
			case REV32(DATA):
				DATA.Read(buffer, subSize);
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

int32_t IDLERecord::Unload()
    {
    IsChanged(false);
    IsLoaded(false);


    EDID.Unload();
	CTDA.Unload(); //Conditions
	DNAM.Unload(); //Path to hkx Behaviour.
	ENAM.Unload(); //Behaviour event.
	ANAM.Unload();
	DATA.Unload();

    
    return 1;
    }

int32_t IDLERecord::WriteRecord(FileWriter &writer)
    {
    WRITE(EDID);
    WRITE(CTDA);
    WRITE(DNAM);
	WRITE(ENAM);
	WRITE(ANAM);
    WRITE(DATA);
    return -1;
    }

bool IDLERecord::operator ==(const IDLERecord &other) const
    {
    return (EDID.equalsi(other.EDID) &&
		CTDA == CTDA &&
		DNAM.equals(other.DNAM) &&
		ENAM.equals(other.ENAM) &&
		ANAM == other.ANAM &&
		DATA == other.DATA
            );
    }

bool IDLERecord::operator !=(const IDLERecord &other) const
    {
    return !(*this == other);
    }

bool IDLERecord::equals(Record *other)
    {
    return *this == *(IDLERecord *)other;
    }
}