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
#include "../../Common.h"
#include "../../GenericRecord.h"
#include "../SkyrimChunks.h"
#include <array>

namespace Sk
{

class RACERecord : public TES5Record //Race
    {
    private:

        struct BODYDATA {
            //ReqSubRecord<GENNAM> MNAM; //Male Marker (Empty)
            OrderedSparseArray<BodyPart>  maleData;
            //ReqSubRecord<GENNAM> FNAM; //Female Marker (Empty)
            OrderedSparseArray<BodyPart>  femaleData;
        };

        struct BEHAVIORGRAPH {
            //ReqSubRecord<GENNAM> MNAM; //Male Marker (Empty)
            OrderedSparseArray<GenericModel> maleGraph;
            //ReqSubRecord<GENNAM> FNAM; //Female Marker (Empty)
            OrderedSparseArray<GenericModel> femaleGraph;
        };

        struct SpeedOverride {
            float leftWalk;
            float leftRun;
            float rightWalk;
            float rightRun;
            float forwardWalk;
            float forwardRun;
            float backWalk;
            float backRun;
            float rotateWalk;
            float rotateRun;
            float unknown;
        };

        struct MOVTYPE {
            ReqSimpleSubRecord<FORMID>          MTYP;
            ReqSubRecord<SpeedOverride>   SPED;
        };

        struct PhonemeTargetWeight
        {
            float Aah_LipBigAah;
            float BigAah_LipDST;
            float BMP_LipEee;
            float ChJsh_LipFV;
            float DST_LipK;
            float Eee_LipL;
            float Eh_LipR;
            float FV_LipTh;
            float I;
            float K;
            float N;
            float Oh;
            float OohQ;
            float R;
            float TH;
            float W;
        };

        struct FaceFXPhonemes 
        {
            PhonemeTargetWeight IY;
            PhonemeTargetWeight IH;
            PhonemeTargetWeight EH;
            PhonemeTargetWeight EY;
            PhonemeTargetWeight AE;
            PhonemeTargetWeight AA;
            PhonemeTargetWeight AW;
            PhonemeTargetWeight AY;
            PhonemeTargetWeight AH;
            PhonemeTargetWeight AO;
            PhonemeTargetWeight OY;
            PhonemeTargetWeight OW;
            PhonemeTargetWeight UH;
            PhonemeTargetWeight UW;
            PhonemeTargetWeight ER;
            PhonemeTargetWeight AX;
            PhonemeTargetWeight S;
            PhonemeTargetWeight SH;
            PhonemeTargetWeight Z;
            PhonemeTargetWeight ZH;
            PhonemeTargetWeight F;
            PhonemeTargetWeight TH;
            PhonemeTargetWeight V;
            PhonemeTargetWeight DH;
            PhonemeTargetWeight M;
            PhonemeTargetWeight N;
            PhonemeTargetWeight NG;
            PhonemeTargetWeight L;
            PhonemeTargetWeight R;
            PhonemeTargetWeight W;
            PhonemeTargetWeight Y;
            PhonemeTargetWeight HH;
            PhonemeTargetWeight B;
            PhonemeTargetWeight D;
            PhonemeTargetWeight JH;
            PhonemeTargetWeight G;
            PhonemeTargetWeight P;
            PhonemeTargetWeight T;
            PhonemeTargetWeight K;
            PhonemeTargetWeight CH;
            PhonemeTargetWeight SIL;
            PhonemeTargetWeight SHOTSIL;
            PhonemeTargetWeight FLAP;
        };

        struct Morphs {
            RawRecord MPAI;
            ReqSubRecord<std::array<uint32_t,8>>  MPAV;
        };

        struct Texture {
            ReqSimpleSubRecord<uint16_t> TINI; //Index
            StringRecord TINT; //File Name
            ReqSimpleSubRecord<uint16_t> TINP; //Mask Type
            ReqSimpleSubRecord<FORMID> TIND; //Preset Default
        };

        struct Preset {
            ReqSimpleSubRecord<FORMID> TINC; //Color
            ReqSimpleFloatSubRecord<> TINV; //Default Value
            ReqSimpleSubRecord<uint16_t> TIRS; //index
        };

        struct TintAssets {
            UnorderedSparseArray<Texture> TintLayer;
            UnorderedSparseArray<Preset> Presets;
        };

        struct HeadPart {
            ReqSimpleSubRecord<uint32_t> INDX;
            StringRecord                 HEAD;
        };

        struct HeadGenderDataMale
        {
            OrderedSparseArray<HeadPart>        headParts;
            Morphs                              morphs[4];
            OrderedSparseArray<FORMID>          RPRM;      //Race Presets Male
            OrderedSparseArray<FORMID>          AHCM;      //Available Hair Colors Male
            OrderedSparseArray<FORMID>          FTSM;      //Face Details Texture Set List Male
            OptSubRecord<FORMID>                DFTM;      //Default Face Texture Male
            TintAssets                          tints;
            GenericModel                        model;
        };

        struct HeadGenderData
        {
            OrderedSparseArray<HeadPart>        headParts;
            Morphs                              morphs[4];
            OrderedSparseArray<FORMID>          RPRM;      //Race Presets Male
            OrderedSparseArray<FORMID>          AHCM;      //Available Hair Colors Male
            OrderedSparseArray<FORMID>          FTSM;      //Face Details Texture Set List Male
            OptSimpleSubRecord<FORMID>          DFTM;      //Default Face Texture Male

            OrderedSparseArray<FORMID>          RPRF;      //Race Presets Male
            OrderedSparseArray<FORMID>          AHCF;      //Available Hair Colors Male
            OrderedSparseArray<FORMID>          FTSF;      //Face Details Texture Set List Male
            OptSimpleSubRecord<FORMID>          DFTF;      //Default Face Texture Male

            TintAssets                          tints;
            GenericModel                        model;
        };

        struct HeadData 
        {
            //ReqSubRecord<GENNAM> NAM0; //Head Marker (Empty)
            //ReqSubRecord<GENNAM> MNAM; //Head Marker (Empty)
            OrderedSparseArray<HeadGenderData> maleHeadData;
            //ReqSubRecord<GENNAM> FNAM; //Head Marker (Empty)
            OrderedSparseArray<HeadGenderData> femaleHeadData;
        };

    public:
        StringRecord EDID; //Editor ID
        StringRecord FULL; //Name
        StringRecord DESC; //Description
        OptCounted<OrderedPackedArray<FORMID>, uint32_t, REV32(SPCT)> SPLO; // Actor effects
        OptSimpleSubRecord<FORMID> WNAM; // Skin, ARMO record
        ReqSubRecord<SKBODT> BODT; // Body Template
        OptCounted<OrderedPackedArray<FORMID>, uint32_t, REV32(KSIZ)> KWDA; // Keywords
        ReqSubRecord<SKRACEDATA> DATA; //Data
        //ReqSubRecord<GENNAM> MNAM; //Male Marker (Empty)
        StringRecord MNAM_ANAM; //Male Skeletal Model
        ReqSubRecord<RawRecord> MNAM_MODT; //Texture Files Hashes
        //ReqSubRecord<GENNAM> FNAM; //Female Marker (Empty)
        StringRecord FNAM_ANAM; //Female Skeletal Model
        OptSubRecord<RawRecord> FNAM_MODT; //Texture Files Hashes
        //ReqSubRecord<ModelInfo> NAM2; //Movement Marker (Empty)
        OrderedSparseArray<std::array<char,4>> MTNM;
        ReqSubRecord<std::array<FORMID,2>> VTCK; //default voice types	formid[2]	VTYP male / female
        OptSubRecord<std::array<FORMID,2>> DNAM; //Decapitate armor	formid[2]	ARMO male/female
        OptSubRecord<std::array<FORMID,2>> HCLF; //default hair color	formid[2]	CLFM male/female
        OptSimpleSubRecord<uint16_t>      TINL; //Tint Index Number	uint16	This is the total number of Tints available to this race
        ReqSimpleFloatSubRecord<flt_0>     PNAM; //FaceGen	float	FaceGen - Main clamp
        ReqSimpleFloatSubRecord<flt_0>     UNAM; //FaceGen	float	FaceGen - Face clamp
        OptSubRecord<UnorderedSparseArray<FORMID>>       ATKR; // attack race, found in one record in Dawnguard
        ReqSubRecord<UnorderedSparseArray<GENATTACK>>    attacks; // attacks ATKD ATKE, parsed from behaviors
        //ReqSubRecord<ModelInfo> NAM1; //Movement Marker (Empty)
        ReqSubRecord<BODYDATA>             bodyData;
        OptSubRecord<UnorderedSparseArray<FORMID>> HNAM; //hairs
        OptSubRecord<UnorderedSparseArray<FORMID>> ENAM; //eyes
        ReqSubRecord<FORMID> GNAM; // Body part data (default BPTD:0000001D)
        //ReqSubRecord<ModelInfo> NAM2; //Marker #2
        //ReqSubRecord<ModelInfo> NAM3; //Marker #3
        BEHAVIORGRAPH behaviors;
        OptSimpleSubRecord<FORMID> NAM4; //Material Type
        OptSimpleSubRecord<FORMID> NAM5; //Impact Data Set
        OptSimpleSubRecord<FORMID> NAM7; //Decapitation FX
        OptSimpleSubRecord<FORMID> ONAM; //Open Loot Sound
        OptSimpleSubRecord<FORMID> LNAM; //Close Loot Sound
        std::vector<StringRecord> NAME; //Biped Names, always 32
        UnorderedSparseArray<MOVTYPE> movementTypes;
        ReqSubRecord<uint32_t> VNAM; //equipment flags
        /*      {0x00000001}'Hand To Hand Melee',
                {0x00000002}'One Hand Sword',
                {0x00000004}'One Hand Dagger',
                {0x00000008}'One Hand Axe',
                {0x00000010}'One Hand Mace',
                {0x00000020}'Two Hand Sword',
                {0x00000040}'Two Hand Axe',
                {0x00000080}'Bow',
                {0x00000100}'Staff',
                {0x00000200}'Spell',
                {0x00000400}'Shield',
                {0x00000800}'Torch',
                {0x00001000}'Crossbow'
        */
        OrderedSparseArray<FORMID> QNAM; //Equip Slots
        OptSimpleSubRecord<FORMID> UNES;                      //Unarmed Equip Slot
        std::vector<StringRecord> PHTN; //Phoneme target names
        OptSubRecord<FaceFXPhonemes> PHWT;
        OptSimpleSubRecord<FORMID> WKMV; //Base Movement Default - Walk
        OptSimpleSubRecord<FORMID> RNMV; //Base Movement Default - Run,
        OptSimpleSubRecord<FORMID> SWMV; //Base Movement Default - Swim
        OptSimpleSubRecord<FORMID> FLMV; //Base Movement Default - Fly
        OptSimpleSubRecord<FORMID> SNMV; //Base Movement Default - Sneak
        OptSimpleSubRecord<FORMID> SPMV; //Base Movement Default - Sprint
        HeadData             headData;
        OptSimpleSubRecord<FORMID> NAM8; //Morph race
        OptSimpleSubRecord<FORMID> RNAM; //Armor race


        RACERecord(unsigned char *_recData=NULL);
        RACERecord(RACERecord *srcRecord);
        ~RACERecord();

        bool   VisitFormIDs(FormIDOp &op);

        uint32_t GetType();
        char * GetStrType();

        int32_t ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk=false);
        int32_t Unload();
        int32_t WriteRecord(FileWriter &writer);

        bool operator ==(const RACERecord &other) const;
        bool operator !=(const RACERecord &other) const;
        bool equals(Record *other);
    };
}