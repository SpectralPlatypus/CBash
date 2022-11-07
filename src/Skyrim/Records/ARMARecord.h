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
#include "../../Common.h"
#include "../../GenericRecord.h"
#include "../SkyrimChunks.h"

namespace Sk {

/*
+		EDID	editorId	zstring	Editor id
+	pre 1.6.91	BODT	
+	1.6.91	BOD2	Body Template	BOD2	8-byte struct Body Template. Appears to be mutually exclusive with BODT.
+		RNAM	
+		DNAM	unknown	struct	12 bytes
uint8 - Male Priority
uint8 - Female Priority
uint32 - Unknown
uint8 - Detection Sound Value
uint8 - Unknown
float - Weapon Adjust
+		MOD2	model	zstring	Models\ relative .nif
-		MO2T	model data	struct[]	(MODT) varying count of repeated 12 byte struct
*		MO2S	alternate texture sets	struct	uint32 number of alternate textures.
Followed by sets of:
uint32 - string name length
string - Alt texture name
formid - Alt texture formID (TXST)
uint32 - model index
-		MOD3	model	zstring	Models\ relative .nif
-		MO3T	model data	struct[]	(MODT) varying count of repeated 12 byte struct
*		MO3S	alternate texture sets	struct	Same as MO2S
-		MOD4	model	zstring	Models\ relative .nif
-		MO4T	model data	struct[]	(MODT) varying count of repeated 12 byte struct
*		MO4S	alternate texture sets	struct	Same as MO2S
-		MOD5	model	zstring	Models\ relative .nif
-		MO5T	model data	struct[]	(MODT) varying count of repeated 12 byte struct
*		MO5S	alternate texture sets	struct	Same as MO2S
-		NAM0	base male texture	formid	(TXST) formid - male
-		NAM1	base female texture	formid	(TXST) formid - female
-		NAM2	base male 1st texture	formid	(TXST) formid - male 1st person
-		NAM3	base female 1st texture	formid	(TXST) formid - female 1st person
*		MODL	included race	formid	(RACE) formid - repeated for each race this is applicable to.
-		SNDD	footstep sound	formid	(FSTS) formid - mostly creatures
-		ONAM	Art Object	formid	(ARTO) formid - mostly creatures

*/
// essentialy a number of world models for different weights (Enabled = 2 models _0.nif and _1.nif)
    struct ARMADNAM {
        uint8_t MalePriority;
        uint8_t FemalePriority;
        uint8_t WeightsliderMale; // essentialy a number of world models for different weights (Enabled = 2 models _0.nif and _1.nif)
        uint8_t WeightsliderFemale; // essentialy a number of world models for different weights (Enabled = 2 models _0.nif and _1.nif)
        uint8_t Unknown[2];
        int8_t DetectionSoundValue;
        uint8_t Unknown1;
        float WeaponAdjust;

        bool operator ==(const ARMADNAM& other) const;
        bool operator !=(const ARMADNAM& other) const;
    };

    SIZE_CHECK(ARMADNAM, 12);

class ARMARecord : public TES5Record // Action record
    {
    public:
        StringRecord EDID; // Editor ID
        ReqSubRecord<SKBOD2> BOD2; // Body Template	BODT	12-byte struct Body Template. Appears to be mutually exclusive with BOD2.
        ReqSubRecord<FORMID> RNAM; // "primary" race	formid	RACE note: there is no preference given for races listed but must have at least this 1.
        ReqSubRecord<ARMADNAM> DNAM;
        ReqSubRecord<GenericModel> MO2;
        ReqSubRecord<GenericModel> MO3;
        ReqSubRecord<GenericModel> MO4;
        ReqSubRecord<GenericModel> MO5;
        OptSubRecord<FORMID> NAM0;
        OptSubRecord<FORMID> NAM1;
        OptSubRecord<FORMID> NAM2;
        OptSubRecord<FORMID> NAM3;
        UnorderedSparseArray<FORMID> MODL;
        OptSubRecord<FORMID> SNDD;
        OptSubRecord<FORMID> ONAM;

        ARMARecord(unsigned char *_recData=NULL);
        ARMARecord(ARMARecord*srcRecord);
        ~ARMARecord();

        uint32_t GetType();
        char * GetStrType();

        int32_t ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk=false);
        int32_t Unload();
        int32_t WriteRecord(FileWriter &writer);
		char *GetEditorIDKey() { return EDID.value; }

        bool operator ==(const ARMARecord &other) const;
        bool operator !=(const ARMARecord &other) const;
        bool equals(Record *other);
    };

} // namespace Sk
