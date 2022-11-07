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
#include "RACERecord.h"

namespace Sk
{

    void RACERecord::Texture::Write(FileWriter& writer)
    {
        WRITE(TINI); //Index
        WRITE(TINT); //File Name
        WRITE(TINP); //Mask Type
        WRITE(TIND); //Preset Default
    }

    void RACERecord::Preset::Write(FileWriter& writer)
    {
        WRITE(TINC); //Color
        WRITE(TINV); //Default Value
        WRITE(TIRS); //index
    }

    void RACERecord::TintAssets::Write(FileWriter& writer)
    {
        TintLayer.Write(writer);
        Presets.Write(writer);
    }

    void RACERecord::HeadGenderData::Write(FileWriter& writer)
    {
        for (auto& part : headParts.value)
        {
            part.INDX.Write(REV32(INDX), writer);
            part.HEAD.Write(REV32(HEAD), writer);
        }
        for (int i = 0; i < 4; ++i)
        {
            morphs[i].MPAI.Write(REV32(MPAI), writer);
            morphs[i].MPAV.Write(REV32(MPAV), writer);
        }
        WRITE(RPRM);
        WRITE(AHCM);
        WRITE(FTSM);
        WRITE(DFTM);
        tints.Write(writer);
    }

RACERecord::RACERecord(unsigned char *_recData):
    TES5Record(_recData)
    {
    init();
    }

void RACERecord::init()
{
    DATA.value.maleHeight = 0.5f;
    DATA.value.maleWeight = 0.5f;
    DATA.value.flags = SKRACEDATA::Flags01::NoCombatInWater;
    
    MTNM.value = {
        std::array<char, 4>({ 'B','L','D','O' }),
        std::array<char, 4>({ 'R','U','N','1' }),
        std::array<char, 4>({ 'S','N','E','K' }),
        std::array<char, 4>({ 'S','W','I','M' }),
        std::array<char, 4>({ 'W','A','L','K' })
    };

    //VTCK is necessary
    
    PNAM.value = 5.0f;
    UNAM.value = 3.0f;

    //GNAM is necessary

    uint32_t newSize = 1;
    bodyData.value.maleData.resize(newSize);
    bodyData.value.femaleData.resize(newSize);
    for (int i = 0; i < 32; i++)
    {
        //At least one should be non null
        if (i == 2)
        {
            StringRecord dummy; dummy.Copy("BODY");
            NAME.push_back(dummy);
        }
        else {
            StringRecord dummy; dummy.value = new char(0);
            NAME.push_back(dummy);
        }
    }
    VNAM.value = 0xFFFFFFFF;
}

RACERecord::RACERecord(RACERecord *srcRecord):
    TES5Record()
    {
    if (srcRecord == NULL)
    {
        init();
        return;
    }

    flags = srcRecord->flags;
    formID = srcRecord->formID;
    flagsUnk = srcRecord->flagsUnk;

    recData = srcRecord->recData;
    if(!srcRecord->IsChanged())
        return;

    EDID = srcRecord->EDID;
    FULL = srcRecord->FULL;
    DESC = srcRecord->DESC;
    SPLO = srcRecord->SPLO;
    WNAM = srcRecord->WNAM;
    BOD2 = srcRecord->BOD2;
    KWDA = srcRecord->KWDA; // Keywords
    DATA = srcRecord->DATA; //Data
    //ReqSubRecord<GENNAM> MNAM; //Male Marker (Empty)
    MNAM_ANAM = srcRecord->MNAM_ANAM;
    MNAM_MODT = srcRecord->MNAM_MODT; //Texture Files Hashes
    //ReqSubRecord<GENNAM> FNAM; //Female Marker (Empty)
    FNAM_ANAM = srcRecord->FNAM_ANAM; //Female Skeletal Model
    FNAM_MODT = srcRecord->FNAM_MODT; //Texture Files Hashes
    //ReqSubRecord<ModelInfo> NAM2; //Movement Marker (Empty)
    MTNM = srcRecord->MTNM;
    VTCK = srcRecord->VTCK; //default voice types	formid[2]	VTYP male / female
    DNAM = srcRecord->DNAM; //Decapitate armor	formid[2]	ARMO male/female
    HCLF = srcRecord->HCLF; //default hair color	formid[2]	CLFM male/female
    TINL = srcRecord->TINL; //Tint Index Number	uint16	This is the total number of Tints available to this race
    PNAM = srcRecord->PNAM; //FaceGen	float	FaceGen - Main clamp
    UNAM = srcRecord->UNAM; //FaceGen	float	FaceGen - Face clamp
    ATKR = srcRecord->ATKR; // attack race, found in one record in Dawnguard
    attacks = srcRecord->attacks;
    //ReqSubRecord<ModelInfo> NAM1; //Movement Marker (Empty)
    bodyData = srcRecord->bodyData;
    HNAM = srcRecord->HNAM;
    ENAM = srcRecord->ENAM; //eyes
    GNAM = srcRecord->GNAM; // Body part data (default BPTD:0000001D)
    //ReqSubRecord<ModelInfo> NAM2; //Marker #2
    //ReqSubRecord<ModelInfo> NAM3; //Marker #3
    behaviors = srcRecord->behaviors;
    NAM4 = srcRecord->NAM4; //Material Type
    NAM5 = srcRecord->NAM5; //Impact Data Set
    NAM7 = srcRecord->NAM7; //Decapitation FX
    ONAM = srcRecord->ONAM; //Open Loot Sound
    LNAM = srcRecord->LNAM; //Close Loot Sound
    NAME = srcRecord->NAME; //Biped Names
    movementTypes = srcRecord->movementTypes;
    VNAM = srcRecord->VNAM; //equipment flags
    QNAM = srcRecord->QNAM; //Equip Slots
    UNES = srcRecord->UNES;                      //Unarmed Equip Slot
    PHTN = srcRecord->PHTN; //Phoneme target names
    PHWT = srcRecord->PHWT;
    WKMV = srcRecord->WKMV; //Base Movement Default - Walk
    RNMV = srcRecord->RNMV; //Base Movement Default - Run,
    SWMV = srcRecord->SWMV; //Base Movement Default - Swim
    FLMV = srcRecord->FLMV; //Base Movement Default - Fly
    SNMV = srcRecord->SNMV; //Base Movement Default - Sneak
    SPMV = srcRecord->SNMV; //Base Movement Default - Sprint
    headData = srcRecord->headData;
    NAM8 = srcRecord->NAM8; //Morph race
    RNAM = srcRecord->RNAM; //Armor race
    }

RACERecord::~RACERecord()
    {
    Unload();
    }

bool RACERecord::VisitFormIDs(FormIDOp &op)
    {
    if(!IsLoaded())
        return false;

    if (SPLO.IsLoaded())
    {
        for (uint32_t ListIndex = 0; ListIndex < SPLO.value.size(); ListIndex++)
            op.Accept(SPLO.value[ListIndex]);
    }
    if(VNAM.IsLoaded())
    {
        op.Accept(VNAM.value);
    }
    if (KWDA.IsLoaded())
    {
        for (uint32_t ListIndex = 0; ListIndex < KWDA.value.size(); ListIndex++)
            op.Accept(KWDA.value[ListIndex]);
    }
    op.Accept(VTCK.value[0]);
    op.Accept(VTCK.value[1]);
    if (DNAM.IsLoaded() && NULL != DNAM.value)
    {
        op.Accept(DNAM.value->at(0));
        op.Accept(DNAM.value->at(1));
    }
    if (HCLF.IsLoaded() && NULL != HCLF.value)
    {
        op.Accept(HCLF.value->at(0));
        op.Accept(HCLF.value->at(1));
    }
    if (ATKR.IsLoaded() && ATKR.value != nullptr)
    {
        for (uint32_t ListIndex = 0; ListIndex < ATKR.value->value.size(); ListIndex++)
        {
            op.Accept(ATKR.value->value[ListIndex]);
        }
    }
    if (attacks.IsLoaded())
    {
        for (uint32_t ListIndex = 0; ListIndex < attacks.value.value.size(); ListIndex++)
        {
            op.Accept(attacks.value.value[ListIndex].ATKD.value.attackSpell);
            op.Accept(attacks.value.value[ListIndex].ATKD.value.attackType);
        }
    }
    if (HNAM.IsLoaded() && HNAM.value != NULL)
    {
        for (uint32_t ListIndex = 0; ListIndex < HNAM.value->value.size(); ListIndex++)
            op.Accept(HNAM.value->value[ListIndex]);
    }
    if (ENAM.IsLoaded() && ENAM.value != NULL)
    {
        for (uint32_t ListIndex = 0; ListIndex < ENAM.value->value.size(); ListIndex++)
            op.Accept(ENAM.value->value[ListIndex]);
    }
    if (GNAM.IsLoaded())
    {
        op.Accept(GNAM.value);
    }
    if (NAM4.IsLoaded())
    {
        op.Accept(NAM4.value);
    }
    if (NAM5.IsLoaded())
    {
        op.Accept(NAM5.value);
    }
    if (NAM7.IsLoaded())
    {
        op.Accept(NAM7.value);
    }
    if (ONAM.IsLoaded())
    {
        op.Accept(ONAM.value);
    }
    if (LNAM.IsLoaded())
    {
        op.Accept(LNAM.value);
    }
    if (movementTypes.IsLoaded())
    {
        for (uint32_t ListIndex = 0; ListIndex < movementTypes.value.size(); ListIndex++)
        {
            if (movementTypes.value[ListIndex].MTYP.IsLoaded())
            {
                op.Accept(movementTypes.value[ListIndex].MTYP.value);
            }
        }
    }
    if (QNAM.IsLoaded())
    {
        for (uint32_t ListIndex = 0; ListIndex < QNAM.value.size(); ListIndex++)
            op.Accept(QNAM.value[ListIndex]);
    }
    if (UNES.IsLoaded())
    {
        op.Accept(UNES.value);
    }
    if (WKMV.IsLoaded())
    {
        op.Accept(WKMV.value);
    }
    if (RNMV.IsLoaded())
    {
        op.Accept(RNMV.value);
    }
    if (SWMV.IsLoaded())
    {
        op.Accept(SWMV.value);
    }
    if (FLMV.IsLoaded())
    {
        op.Accept(FLMV.value);
    }
    if (SNMV.IsLoaded())
    {
        op.Accept(SNMV.value);
    }
    if (SPMV.IsLoaded())
    {
        op.Accept(SPMV.value);
    }
    if (NAM8.IsLoaded())
    {
        op.Accept(NAM8.value);
    }
    if (RNAM.IsLoaded())
    {
        op.Accept(RNAM.value);
    }

    return op.Stop();
    }

uint32_t RACERecord::GetType()
    {
    return REV32(RACE);
    }

char * RACERecord::GetStrType()
    {
    return "RACE";
    }

int32_t RACERecord::ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk)
    {
    enum faceEnum
        {
        eHead = 0,
        eMaleEars,
        eFemaleEars,
        eMouth,
        eLowerTeeth,
        eUpperTeeth,
        eTongue,
        eLeftEye,
        eRightEye
        };

    enum bodyEnum
        {
        eUpperBody = 0,
        eLowerBody,
        eHand,
        eFoot,
        eTail
        };

    enum namEnum
        {
        fIsFace   = 0x10000000,
        fIsBody   = 0x20000000,
        fIsMale   = 0x40000000,
        fIsFemale = 0x80000000
        };

    uint32_t subType = 0;
    uint32_t subSize = 0;
    uint32_t part_id = 0;
    uint32_t temp_id = 0;

    enum parserState
    {
        pNone = 0,
        pMaleModelInfo = 1,
        pFemaleModelInfo = 2,
        pBodyPart = 3,
        pMaleBodyPartInfo = 4,
        pFemaleBodyPartInfo = 5,
        pBehaviorGraph = 6,
        pBehaviorGraphMaleInfo = 7,
        pBehaviorGraphFemaleInfo = 8,
        pMovementTypes = 9,
        pHeadInfoStart = 10,
        pHeadInfoMale = 11,
        pHeadInfoFemale = 12
    };

    int rState = pNone;
    GENATTACK tempAttack;
    BodyPart tempBodyPart;
    GenericModel tempModel;
    MOVTYPE tempMovtype;
    HeadGenderData tempHeadData;
    int mpaiIndex = 0;
    Texture tempTexture;
    Preset tempPreset;
    HeadPart tempHeadPart;
    StringRecord tempString;
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
            case REV32(FULL):
                FULL.Read(buffer, subSize, CompressedOnDisk);
                break;
            case REV32(DESC):
                DESC.Read(buffer, subSize, CompressedOnDisk);
                break;
            case REV32(SPCT):
                // Number of SPLO
                buffer += subSize;
                break;
            case REV32(SPLO):
                SPLO.Read(buffer, subSize);
                break;
            case REV32(WNAM):
                WNAM.Read(buffer, subSize);
                break;
            case REV32(BODT):
            case REV32(BOD2):
                BOD2.Read(buffer, subSize);
                break;
            case REV32(KSIZ):
                // Number of KWDAs
                buffer += subSize;
                break;
            case REV32(KWDA):
                KWDA.Read(buffer, subSize);
                break;
            case REV32(DATA):
                DATA.Read(buffer, subSize);
                break;
            case REV32(MNAM):
                switch (rState) {
                case pNone:
                    rState = pMaleModelInfo;
                    break;
                case pBodyPart:
                    rState = pMaleBodyPartInfo;
                    break;
                case pFemaleBodyPartInfo:
                    bodyData.value.femaleData.value.push_back(tempBodyPart);
                    rState = pMaleBodyPartInfo;
                    break;
                case pBehaviorGraph:
                    rState = pBehaviorGraphMaleInfo;
                    break;
                case pBehaviorGraphMaleInfo:
                case pBehaviorGraphFemaleInfo:
                    behaviors.maleGraph.value.push_back(tempModel);
                    rState = pBehaviorGraphMaleInfo;
                    break;
                case pHeadInfoStart:
                    rState = pHeadInfoMale;
                default:
                    break;
                }
                break;
            case REV32(FNAM):
                switch (rState) {
                case pMaleModelInfo:
                    rState = pFemaleModelInfo;
                    break;
                case pMaleBodyPartInfo:
                    bodyData.value.maleData.value.push_back(tempBodyPart);
                    rState = pFemaleBodyPartInfo;
                    break;
                case pBehaviorGraphMaleInfo:
                case pBehaviorGraphFemaleInfo:
                    behaviors.maleGraph.value.push_back(tempModel);
                    rState = pBehaviorGraphFemaleInfo;
                    break;
                case pHeadInfoStart:
                    rState = pHeadInfoFemale;
                    break;
                default:
                    break;
                }
                break;
            case REV32(ANAM): 
                switch (rState) {
                case pMaleModelInfo:
                    MNAM_ANAM.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pFemaleModelInfo:
                    FNAM_ANAM.Read(buffer, subSize, CompressedOnDisk);
                    break;
                default:
                    break;
                }
                break;
            case REV32(MODT):
                switch (rState) {
                case pMaleModelInfo:
                    MNAM_MODT.Read(buffer, subSize);
                    break;
                case pFemaleModelInfo:
                    FNAM_MODT.Read(buffer, subSize);
                    break;
                case pMaleBodyPartInfo:
                case pFemaleBodyPartInfo:
                    if (!tempBodyPart.model.IsLoaded())
                        tempBodyPart.model.Load();
                    tempBodyPart.model.value->MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pBehaviorGraphMaleInfo:
                case pBehaviorGraphFemaleInfo:
                    tempModel.MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pHeadInfoMale:
                case pHeadInfoFemale:
                    tempHeadData.model.MODT.Read(buffer, subSize, CompressedOnDisk);
                    break;
                default:
                    break;
                }
                break;
            case REV32(NAM2):
                switch (rState) {
                case pFemaleModelInfo:
                    rState = pNone;
                    break;
                case pMaleBodyPartInfo:
                    bodyData.value.maleData.value.push_back(tempBodyPart);
                    rState = pNone;
                    break;
                case pFemaleBodyPartInfo:
                    bodyData.value.femaleData.value.push_back(tempBodyPart);
                    rState = pNone;
                    break;
                default:
                    break;
                }
                break;
            case REV32(MTNM):
                switch (rState) {
                case pFemaleModelInfo:
                    rState = pNone;
                    break;
                default:
                    break;
                }
                MTNM.Read(buffer, subSize);
                break;
            case REV32(VTCK):
                VTCK.Read(buffer, subSize);
                break;
            case REV32(DNAM):
                DNAM.Read(buffer, subSize);
                break;
            case REV32(HCLF):
                HCLF.Read(buffer, subSize);
                break;
            case REV32(TINL):
                TINL.Read(buffer, subSize);
                break;
            case REV32(PNAM):
                PNAM.Read(buffer, subSize);
                break;
            case REV32(UNAM):
                UNAM.Read(buffer, subSize);
                break;
            case REV32(ATKR):
                ATKR.Read(buffer, subSize);
                break;
            case REV32(ATKD):
                tempAttack.ATKD.Unload();
                tempAttack.ATKD.Read(buffer, subSize);
                break;
            case REV32(ATKE):
                tempAttack.ATKE.value = NULL;
                tempAttack.ATKE.Read(buffer, subSize, CompressedOnDisk);
                attacks.value.value.push_back(tempAttack);
                break;
            case REV32(NAM1):
                switch (rState) {
                case pNone:
                    rState = pBodyPart;
                    break;
                default:
                    break;
                }
            case REV32(INDX):
                switch (rState) {
                case pMaleBodyPartInfo:
                case pFemaleBodyPartInfo:
                    tempHeadPart.INDX.Read(buffer, subSize);
                    break;
                case pHeadInfoMale:
                case pHeadInfoFemale:
                    tempHeadPart.INDX.Read(buffer, subSize);
                    break;
                default:
                    break;
                }
                break;
            case REV32(MODL):
                switch (rState) {
                case pMaleBodyPartInfo:
                case pFemaleBodyPartInfo:
                    tempBodyPart.model.Load();
                    tempBodyPart.model->MODL.value = NULL;
                    tempBodyPart.model->MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pBehaviorGraphMaleInfo:
                case pBehaviorGraphFemaleInfo:
                    tempModel.MODL.value = NULL;
                    tempModel.MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pHeadInfoMale:
                case pHeadInfoFemale:
                    tempBodyPart.model.Load();
                    tempHeadData.model.MODL.value = NULL;
                    tempHeadData.model.MODL.Read(buffer, subSize, CompressedOnDisk);
                    break;
                default:
                    break;
                }
                break;
            case REV32(MODS):
                switch (rState) {
                case pMaleBodyPartInfo:
                case pFemaleBodyPartInfo:
                    tempBodyPart.model.Load();
                    tempBodyPart.model->MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pBehaviorGraphMaleInfo:
                case pBehaviorGraphFemaleInfo:
                    tempModel.MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                case pHeadInfoMale:
                case pHeadInfoFemale:
                    tempHeadData.model.MODS.Read(buffer, subSize, CompressedOnDisk);
                    break;
                default:
                    break;
                }
                break;
            case REV32(HNAM):
                HNAM.Read(buffer, subSize);
                break;
            case REV32(ENAM):
                ENAM.Read(buffer, subSize);
                break;
            case REV32(GNAM):
                GNAM.Read(buffer, subSize);
                break;
            case REV32(NAM3):
                switch (rState) {
                case pFemaleModelInfo:
                    rState = pBehaviorGraph;
                    break;
                case pMaleBodyPartInfo:
                    bodyData.value.maleData.value.push_back(tempBodyPart);
                    rState = pBehaviorGraph;
                    break;
                case pFemaleBodyPartInfo:
                    bodyData.value.femaleData.value.push_back(tempBodyPart);
                    rState = pBehaviorGraph;
                    break;
                default:
                    break;
                }
                break;
            case REV32(NAM4):
            case REV32(NAM5):
            case REV32(NAM7):
            case REV32(ONAM):
            case REV32(LNAM):
            case REV32(NAME):
                switch (rState) {
                case pFemaleModelInfo:
                    rState = pBehaviorGraph;
                    break;
                case pMaleBodyPartInfo:
                    bodyData.value.maleData.value.push_back(tempBodyPart);
                    rState = pBehaviorGraph;
                    break;
                case pFemaleBodyPartInfo:
                    bodyData.value.femaleData.value.push_back(tempBodyPart);
                    rState = pBehaviorGraph;
                    break;
                case pBehaviorGraphMaleInfo:
                    behaviors.maleGraph.value.push_back(tempModel);
                    rState = pNone;
                    break;
                case pBehaviorGraphFemaleInfo:
                    behaviors.femaleGraph.value.push_back(tempModel);
                    rState = pNone;
                    break;
                default:
                    break;
                }
                switch (subType)
                {
                case REV32(NAM4):
                    NAM4.Read(buffer, subSize);
                    break;
                case REV32(NAM5):
                    NAM5.Read(buffer, subSize);
                    break;
                case REV32(NAM7):
                    NAM7.Read(buffer, subSize);
                    break;
                case REV32(ONAM):
                    ONAM.Read(buffer, subSize);
                    break;
                case REV32(LNAM):
                    LNAM.Read(buffer, subSize);
                    break;
                case REV32(NAME):
                    tempString.value = NULL;
                    tempString.Read(buffer, subSize, CompressedOnDisk);
                    NAME.push_back(tempString);
                    break;
                default:
                    break;
                }
                break;
            case REV32(MTYP):
                tempMovtype.MTYP.Read(buffer, subSize);
                break;
            case REV32(SPED):
                tempMovtype.SPED.Read(buffer, subSize);
                movementTypes.value.push_back(tempMovtype);
                break;
            case REV32(VNAM):
                VNAM.Read(buffer, subSize);
                break;
            case REV32(QNAM):
                QNAM.Read(buffer, subSize);
                break;
            case REV32(UNES):
                UNES.Read(buffer, subSize);
                break;
            case REV32(PHTN):
                tempString.value = NULL;
                tempString.Read(buffer, subSize, CompressedOnDisk);
                PHTN.push_back(tempString);
                break;
            case REV32(PHWT):
                PHWT.Read(buffer, subSize);
                break;
            case REV32(WKMV):
                WKMV.Read(buffer, subSize);
                break;
            case REV32(RNMV):
                RNMV.Read(buffer, subSize);
                break;
            case REV32(SWMV):
                SWMV.Read(buffer, subSize);
                break;
            case REV32(FLMV):
                FLMV.Read(buffer, subSize);
                break;
            case REV32(SNMV):
                SNMV.Read(buffer, subSize);
                break;
            case REV32(SPMV):
                SPMV.Read(buffer, subSize);
                break;
            case REV32(NAM0):
                switch (rState) {
                case pHeadInfoMale:
                    headData.maleHeadData.value.push_back(tempHeadData);
                    break;
                case pHeadInfoFemale:
                    headData.femaleHeadData.value.push_back(tempHeadData);
                    break;
                default:
                    break;
                }
                rState = pHeadInfoStart;
                break;
            case REV32(MPAI):
                tempHeadData.morphs[mpaiIndex].MPAI.Read(buffer, subSize, CompressedOnDisk);
                break;
            case REV32(MPAV):
                tempHeadData.morphs[mpaiIndex].MPAV.Read(buffer, subSize);
                mpaiIndex = (mpaiIndex + 1) % 4;
                break;
            case REV32(RPRM):
                tempHeadData.RPRM.Read(buffer, subSize);
                break;
            case REV32(AHCM):
                tempHeadData.AHCM.Read(buffer, subSize);
                break;
            case REV32(FTSM):
                tempHeadData.FTSM.Read(buffer, subSize);
                break;
            case REV32(DFTM):
                tempHeadData.DFTM.Read(buffer, subSize);
                break;
            case REV32(RPRF):
                tempHeadData.RPRF.Read(buffer, subSize);
                break;
            case REV32(AHCF):
                tempHeadData.AHCF.Read(buffer, subSize);
                break;
            case REV32(FTSF):
                tempHeadData.FTSF.Read(buffer, subSize);
                break;
            case REV32(DFTF):
                tempHeadData.DFTF.Read(buffer, subSize);
                break;
            case REV32(TINI):
                tempTexture.TINI.Read(buffer, subSize);
                break;
            case REV32(TINT):
                tempTexture.TINT.value = NULL;
                tempTexture.TINT.Read(buffer, subSize, CompressedOnDisk);
                break;
            case REV32(TINP):
                tempTexture.TINP.Read(buffer, subSize);
                break;
            case REV32(TIND):
                tempTexture.TIND.Read(buffer, subSize);
                tempHeadData.tints.TintLayer.value.push_back(tempTexture);
                break;
            case REV32(TINC):
                tempPreset.TINC.Read(buffer, subSize);
                break;
            case REV32(TINV):
                tempPreset.TINV.Read(buffer, subSize);
                break;
            case REV32(TIRS):
                tempPreset.TIRS.Read(buffer, subSize);
                tempHeadData.tints.Presets.value.push_back(tempPreset);
                break;
            case REV32(NAM8):
            case REV32(RNAM):
                switch (rState) {
                case pHeadInfoMale:
                    headData.maleHeadData.value.push_back(tempHeadData);
                    break;
                case pHeadInfoFemale:
                    headData.femaleHeadData.value.push_back(tempHeadData);
                    break;
                default:
                    break;
                }
                rState = pNone;
                switch (subType)
                {
                case REV32(NAM8):
                    NAM8.Read(buffer, subSize);
                    break;
                case REV32(RNAM):
                    RNAM.Read(buffer, subSize);
                    break;
                }
                break;
            case REV32(HEAD):
                tempHeadPart.HEAD.value = NULL;
                tempHeadPart.HEAD.Read(buffer, subSize, CompressedOnDisk);
                tempHeadData.headParts.value.push_back(tempHeadPart);
                break;
            default:
                //printer("FileName = %s\n", FileName);
                printer("  RACE: Unknown subType = %04X\n", subType);
                printer("  Size = %i\n", subSize);
                printer("  CurPos = %08x\n\n", buffer - 6);
                buffer = end_buffer;
                break;
            }
        };
        switch (rState) {
        case pHeadInfoMale:
            headData.maleHeadData.value.push_back(tempHeadData);
            break;
        case pHeadInfoFemale:
            headData.femaleHeadData.value.push_back(tempHeadData);
            break;
        default:
            break;
        }
    return 0;
    }

int32_t RACERecord::Unload()
    {
    IsChanged(false);
    IsLoaded(false);
    EDID.Unload(); //Editor ID
    FULL.Unload(); //Name
    DESC.Unload(); //Description
    SPLO.Unload(); // Actor effects
    WNAM.Unload(); // Skin, ARMO record
    BOD2.Unload(); // Body Template
    KWDA.Unload(); // Keywords
    DATA.Unload(); //Data
    //ReqSubRecord<GENNAM> MNAM; //Male Marker (Empty)
    MNAM_ANAM.Unload(); //Male Skeletal Model
    MNAM_MODT.Unload(); //Texture Files Hashes
    //ReqSubRecord<GENNAM> FNAM; //Female Marker (Empty)
    FNAM_ANAM.Unload(); //Female Skeletal Model
    FNAM_MODT.Unload(); //Texture Files Hashes
    //ReqSubRecord<ModelInfo> NAM2; //Movement Marker (Empty)
    MTNM.Unload();
    VTCK.Unload(); //default voice types	formid[2]	VTYP male / female
    DNAM.Unload(); //Decapitate armor	formid[2]	ARMO male/female
    HCLF.Unload(); //default hair color	formid[2]	CLFM male/female
    TINL.Unload(); //Tint Index Number	uint16	This is the total number of Tints available to this race
    PNAM.Unload(); //FaceGen	float	FaceGen - Main clamp
    UNAM.Unload(); //FaceGen	float	FaceGen - Face clamp
    ATKR.Unload(); // attack race, found in one record in Dawnguard
    attacks.Unload(); // attacks ATKD ATKE, parsed from behaviors
    //ReqSubRecord<ModelInfo> NAM1; //Movement Marker (Empty)
    bodyData.Unload();
    HNAM.Unload(); //hairs
    ENAM.Unload(); //eyes
    GNAM.Unload(); // Body part data (default BPTD:0000001D)
    //ReqSubRecord<ModelInfo> NAM2; //Marker #2
    //ReqSubRecord<ModelInfo> NAM3; //Marker #3
    behaviors.maleGraph.Unload();
    behaviors.femaleGraph.Unload();
    NAM4.Unload(); //Material Type
    NAM5.Unload(); //Impact Data Set
    NAM7.Unload(); //Decapitation FX
    ONAM.Unload(); //Open Loot Sound
    LNAM.Unload(); //Close Loot Sound
    for (auto& record : NAME)
        record.Unload();
    NAME.clear(); //Biped Names, always 32
    movementTypes.Unload();
    VNAM.Unload(); //equipment flags
    QNAM.Unload(); //Equip Slots
    UNES.Unload();                      //Unarmed Equip Slot
    for (auto& record : PHTN)
        record.Unload();
    PHTN.clear(); //Phoneme target names
    PHWT.Unload();
    WKMV.Unload(); //Base Movement Default - Walk
    RNMV.Unload(); //Base Movement Default - Run,
    SWMV.Unload(); //Base Movement Default - Swim
    FLMV.Unload(); //Base Movement Default - Fly
    SNMV.Unload(); //Base Movement Default - Sneak
    SPMV.Unload(); //Base Movement Default - Sprint
    headData.maleHeadData.Unload();
    headData.femaleHeadData.Unload();
    NAM8.Unload(); //Morph race
    RNAM.Unload(); //Armor race
    return 1;
    }

int32_t RACERecord::WriteRecord(FileWriter &writer)
    {
    WRITE(EDID); //Editor ID
    WRITE(FULL); //Name
    WRITE(DESC); //Description
    WRITE(SPLO); // Actor effects
    WRITE(WNAM); // Skin, ARMO record
    BOD2.Write(REV32(BOD2), writer);
    WRITE(KWDA); // Keywords
    WRITE(DATA); //Data
    WRITEEMPTY(MNAM); //Male Marker (Empty)
    MNAM_ANAM.Write(REV32(ANAM), writer);
    MNAM_MODT.Write(REV32(MODT), writer); //Texture Files Hashes
    WRITEEMPTY(FNAM); //Female Marker (Empty)
    FNAM_ANAM.Write(REV32(ANAM), writer);
    FNAM_MODT.Write(REV32(MODT), writer); //Texture Files Hashes
    //NAM2
    WRITE(MTNM);
    WRITE(VTCK); //default voice types	formid[2]	VTYP male / female
    WRITE(DNAM); //Decapitate armor	formid[2]	ARMO male/female
    WRITE(HCLF); //default hair color	formid[2]	CLFM male/female
    WRITE(TINL); //Tint Index Number	uint16	This is the total number of Tints available to this race
    WRITE(PNAM); //FaceGen	float	FaceGen - Main clamp
    WRITE(UNAM); //FaceGen	float	FaceGen - Face clamp
    WRITE(ATKR); // attack race, found in one record in Dawnguard
    for (const auto& value : attacks.value.value)
    {
        writer.record_write_subrecord(REV32(ATKD), &value.ATKD.value, value.ATKD.GetSize());
        writer.record_write_subrecord(REV32(ATKD), &value.ATKE.value, value.ATKE.GetSize());
    }
    WRITEEMPTY(NAM1); //Movement Marker (Empty)
    WRITEEMPTY(MNAM); //Male Marker (Empty)
    for (auto& bodyPart : bodyData.value.maleData.value)
    {
        writer.record_write_subrecord(REV32(INDX), &bodyPart.INDX, 4);
        if (bodyPart.model.IsLoaded())
        {
            bodyPart.model->MODL.Write(REV32(MODL), writer);
            bodyPart.model->MODS.Write(REV32(MODS), writer);
        }
    }
    WRITEEMPTY(FNAM); //Male Marker (Empty)
    for (auto& bodyPart : bodyData.value.femaleData.value)
    {
        writer.record_write_subrecord(REV32(INDX), &bodyPart.INDX, 4);
        if (bodyPart.model.IsLoaded())
        {
            bodyPart.model->MODL.Write(REV32(MODL), writer);
            bodyPart.model->MODS.Write(REV32(MODS), writer);
        }
    }
    WRITE(HNAM); //hairs
    WRITE(ENAM); //eyes
    WRITE(GNAM); // Body part data (default BPTD:0000001D)
    //ReqSubRecord<ModelInfo> NAM2; //Marker #2
    //ReqSubRecord<ModelInfo> NAM3; //Marker #3
    WRITEEMPTY(NAM3); //Behavior graph
    WRITEEMPTY(MNAM); //Male Marker (Empty)
    for (auto& graph : behaviors.maleGraph.value)
    {
        graph.MODL.Write(REV32(MODL), writer);
        graph.MODS.Write(REV32(MODS), writer);
    }
    WRITEEMPTY(FNAM); //Male Marker (Empty)
    for (auto& graph : behaviors.femaleGraph.value)
    {
        graph.MODL.Write(REV32(MODL), writer);
        graph.MODS.Write(REV32(MODS), writer);
    }
    WRITE(NAM4); //Material Type
    WRITE(NAM5); //Impact Data Set
    WRITE(NAM7); //Decapitation FX
    WRITE(ONAM); //Open Loot Sound
    WRITE(LNAM); //Close Loot Sound
    for (auto& record : NAME)
        record.Write(REV32(NAME), writer);
    movementTypes.Write(REV32(MTYP), writer);
    WRITE(VNAM); //equipment flags
    WRITE(QNAM); //Equip Slots
    WRITE(UNES);                      //Unarmed Equip Slot
    for (auto& record : PHTN)
        record.Write(REV32(PHTN), writer);
    WRITE(PHWT);
    WRITE(WKMV); //Base Movement Default - Walk
    WRITE(RNMV); //Base Movement Default - Run,
    WRITE(SWMV); //Base Movement Default - Swim
    WRITE(FLMV); //Base Movement Default - Fly
    WRITE(SNMV); //Base Movement Default - Sneak
    WRITE(SPMV); //Base Movement Default - Sprint
    if (headData.maleHeadData.IsLoaded() || headData.femaleHeadData.IsLoaded())
    {
        WRITEEMPTY(NAM0); //Head data
        if (headData.maleHeadData.IsLoaded())
        {
            WRITEEMPTY(MNAM); //Male Marker (Empty)
            headData.maleHeadData.Write(writer);
        }
        if (headData.femaleHeadData.IsLoaded())
        {
            WRITEEMPTY(FNAM); //Female Marker (Empty)
            headData.femaleHeadData.Write(writer);
        }
    }
    WRITE(NAM8); //Morph race
    WRITE(RNAM); //Armor race

    return -1;
    }

bool RACERecord::operator ==(const RACERecord &other) const
    {
    return
        EDID.equalsi(other.EDID) &&
        FULL.equalsi(other.FULL) &&
        DESC.equals(other.DESC) &&
        SPLO == other.SPLO &&
        WNAM == other.WNAM &&
        BOD2 == other.BOD2 &&
        KWDA == other.KWDA &&
        DATA == other.DATA &&
        //WRITEEMPTY(MNAM); //Male Marker (Empty)
        MNAM_ANAM.equalsi(other.MNAM_ANAM) &&
        //MNAM_MODT == other.MNAM_MODT &&
        //WRITEEMPTY(FNAM); //Female Marker (Empty)
        FNAM_ANAM.equalsi(other.FNAM_ANAM) &&
        //FNAM_MODT == other.FNAM_MODT &&
        //NAM2
        MTNM == other.MTNM &&
        VTCK == other.VTCK &&
        DNAM == other.DNAM &&
        HCLF == other.HCLF &&
        TINL == other.TINL &&
        PNAM == other.PNAM &&
        UNAM == other.UNAM &&
        ATKR == other.ATKR &&
        attacks == other.attacks &&
        //WRITEEMPTY(NAM1); //Movement Marker (Empty)
        //WRITEEMPTY(MNAM); //Male Marker (Empty)
        //bodyData == other.bodyData &&
        HNAM == other.HNAM &&
        ENAM == other.ENAM &&
        GNAM == other.GNAM &&
        //ReqSubRecord<ModelInfo> NAM2; //Marker #2
        //ReqSubRecord<ModelInfo> NAM3; //Marker #3
        //WRITEEMPTY(NAM3); //Behavior graph
        //WRITEEMPTY(MNAM); //Male Marker (Empty)
        //behaviors.maleGraph.value[0] == other.behaviors.maleGraph.value[0] &&
        //behaviors.femaleGraph.value[0] == other.behaviors.femaleGraph.value[0] &&
        NAM4 == other.NAM4 &&
        NAM5 == other.NAM5 &&
        NAM7 == other.NAM7 &&
        ONAM == other.ONAM &&
        LNAM == other.LNAM &&
        //NAME == other.NAME &&
        //movementTypes == other.movementTypes &&
        VNAM == other.VNAM &&
        QNAM == other.QNAM &&
        UNES == other.UNES &&
        //PHTN == other.PHTN &&
        //PHWT == other.PHWT &&
        WKMV == other.WKMV &&
        RNMV == other.RNMV &&
        SWMV == other.SWMV &&
        FLMV == other.FLMV &&
        SNMV == other.SNMV &&
        SPMV == other.SPMV &&
        //WRITEEMPTY(NAM0); //Head data
        //WRITEEMPTY(MNAM); //Male Marker (Empty)
        //headData.maleHeadData == other.headData.maleHeadData &&
        //WRITEEMPTY(FNAM); //Female Marker (Empty)
        //headData.femaleHeadData == other.headData.femaleHeadData &&
        NAM8 == other.NAM8 &&
        RNAM == other.RNAM;
    }

bool RACERecord::operator !=(const RACERecord &other) const
    {
    return !(*this == other);
    }

bool RACERecord::equals(Record *other)
    {
    return *this == *(RACERecord *)other;
    }
}