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
#include "../../Common.h"
#include "ARMARecord.h"

namespace Sk {

ARMARecord::ARMARecord(unsigned char *_recData)
    : TES5Record(_recData)
    {
        //
    }

ARMARecord::ARMARecord(ARMARecord *srcRecord)
    : TES5Record((TES5Record *)srcRecord)
    {
        if (srcRecord == NULL || !srcRecord->IsChanged())
            return;

        EDID = srcRecord->EDID;
        BOD2 = srcRecord->BOD2;
        RNAM = srcRecord->RNAM;
        DNAM = srcRecord->DNAM;
        MO3 = srcRecord->MO3;
        MO4 = srcRecord->MO4;
        MO5 = srcRecord->MO5;
        NAM0 = srcRecord->NAM0;
        NAM1 = srcRecord->NAM1;
        NAM2 = srcRecord->NAM2;
        NAM3 = srcRecord->NAM3;
        MODL = srcRecord->MODL;
        SNDD = srcRecord->SNDD;
        ONAM = srcRecord->ONAM;
    }

ARMARecord::~ARMARecord()
    {
        //
    }

uint32_t ARMARecord::GetType()
    {
        return REV32(ARMA);
    }

char * ARMARecord::GetStrType()
    {
        return "ARMA";
    }

int32_t ARMARecord::ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk)
    {
        uint32_t subType = 0;
        uint32_t subSize = 0;
        while(buffer < end_buffer)
        {
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
                case REV32(BODT):
                case REV32(BOD2):
                    BOD2.Read(buffer, subSize);
                    break;
                case REV32(RNAM):
                    RNAM.Read(buffer, subSize);
                    break;
                case REV32(DNAM):
                    DNAM.Read(buffer, subSize);
                    break;
                case REV32(MOD2):
                    MO2.Load();
                    MO2->MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO2T):
                    MO2.Load();
                    MO2->MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO2S):
                    MO2.Load();
                    MO2->MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MOD3):
                    MO3.Load();
                    MO3->MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO3T):
                    MO3.Load();
                    MO3->MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO3S):
                    MO3.Load();
                    MO3->MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MOD4):
                    MO4.Load();
                    MO4->MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO4T):
                    MO4.Load();
                    MO4->MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO4S):
                    MO4.Load();
                    MO4->MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MOD5):
                    MO5.Load();
                    MO5->MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO5T):
                    MO5.Load();
                    MO5->MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(MO5S):
                    MO5.Load();
                    MO5->MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case REV32(NAM0):
                    NAM0.Read(buffer, subSize);
                    break;
                case REV32(NAM1):
                    NAM1.Read(buffer, subSize);
                    break;
                case REV32(NAM2):
                    NAM2.Read(buffer, subSize);
                    break;
                case REV32(NAM3):
                    NAM3.Read(buffer, subSize);
                    break;
                case REV32(MODL):
                    MODL.Read(buffer, subSize);
                    break;
                case REV32(SNDD):
                    SNDD.Read(buffer, subSize);
                    break;
                case REV32(ONAM):
                    ONAM.Read(buffer, subSize);
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

int32_t ARMARecord::Unload()
    {
        IsLoaded(false);
        IsChanged(false);
        EDID.Unload();
        BOD2.Unload();
        RNAM.Unload();
        DNAM.Unload(); 
        MO2.Unload();
        MO3.Unload();
        MO4.Unload();
        MO5.Unload();
        NAM0.Unload();
        NAM1.Unload();
        NAM2.Unload();
        NAM3.Unload();
        MODL.Unload();
        SNDD.Unload();
        ONAM.Unload();
        return 1;
    }

int32_t ARMARecord::WriteRecord(FileWriter &writer)
    {
        WRITE(EDID);
        WRITE(BOD2);
        WRITE(RNAM);
        WRITE(DNAM);
        if (MO2.IsLoaded())
        {
            MO2->MODL.Write(REV32(MOD2), writer);
            MO2->MODT.Write(REV32(MO2T), writer);
            MO2->MODS.Write(REV32(MO2S), writer);
        }
        if (MO3.IsLoaded())
        {     
            MO3->MODL.Write(REV32(MOD3), writer);
            MO3->MODT.Write(REV32(MO3T), writer);
            MO3->MODS.Write(REV32(MO3S), writer);
        }
        if (MO4.IsLoaded())
        {
            MO4->MODL.Write(REV32(MOD4), writer);
            MO4->MODT.Write(REV32(MO4T), writer);
            MO4->MODS.Write(REV32(MO4S), writer);
        }
        if (MO5.IsLoaded())
        {
            MO5->MODL.Write(REV32(MOD5), writer);
            MO5->MODT.Write(REV32(MO5T), writer);
            MO5->MODS.Write(REV32(MO5S), writer);
        }
        WRITE(NAM0);
        WRITE(NAM1);
        WRITE(NAM2);
        WRITE(NAM3);
        WRITE(MODL);
        WRITE(SNDD);
        WRITE(ONAM);
        return -1;
    }

bool ARMARecord::operator ==(const ARMARecord &other) const
    {
        return (EDID.equalsi(other.EDID) &&
                BOD2 == other.BOD2 &&
                RNAM == other.RNAM &&
                DNAM == other.DNAM &&
                MO2 == other.MO2 &&
                MO3 == other.MO3 &&
                MO4 == other.MO4 &&
                MO5 == other.MO5 &&
                NAM0 == other.NAM0 &&
                NAM1 == other.NAM1 &&
                NAM2 == other.NAM2 &&
                NAM3 == other.NAM3 &&
                MODL == other.MODL &&
                SNDD == other.SNDD &&
                ONAM == other.ONAM
        );
    }

bool ARMARecord::operator !=(const ARMARecord &other) const
    {
        return !(*this == other);
    }

bool ARMARecord::equals(Record *other)
    {
        try
        {
            return *this == *dynamic_cast<const ARMARecord *>(other);
        }
        catch(...)
        {
            return false;
        }
    }

bool ARMADNAM::operator == (const ARMADNAM& other) const
{
    return  MalePriority == other.MalePriority &&
        FemalePriority == other.FemalePriority &&
        WeightsliderMale == other.WeightsliderMale &&
        WeightsliderFemale == other.WeightsliderFemale &&
        Unknown[0] == other.Unknown[0] &&
        Unknown[1] == other.Unknown[1] &&
        DetectionSoundValue == other.DetectionSoundValue &&
        Unknown1 == other.Unknown1 &&
        WeaponAdjust == other.WeaponAdjust;
}

bool ARMADNAM::operator != (const ARMADNAM& other) const
{
    return !(*this == other);
}

} // namespace Sk
