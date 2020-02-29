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

	struct SOUNSNDD {

		uint8_t		minimumAttenuationDistance; //		Multiplied by 5.
		uint8_t		maximumAttenuationDistance; //		Multiplied by 100.
		int8_t		frequencyAdjustmentPercentage; 
		byte		unused;
		uint32_t	flags; //	uint32	See below for values.
		int16_t 	staticAttenuationcdB;
		uint8_t		stopTime;
		uint8_t		startTime;
		int16_t		attenuationPoint1;	//	The first point on the attenuation curve.
		int16_t		attenuationPoint2;	//	The second point on the attenuation curve.
		int16_t		attenuationPoint3;	//	The third point on the attenuation curve.
		int16_t		attenuationPoint4;	//	The fourth point on the attenuation curve.
		int16_t		attenuationPoint5;	//	The fifth point on the attenuation curve.
		int16_t		reverbAttenuationControl;
		int32_t		priority;
		byte		unknown[8];

		SOUNSNDD()
		{
		}

		bool operator ==(const SOUNSNDD &other) const {
			return (
				minimumAttenuationDistance == other.minimumAttenuationDistance &&
				maximumAttenuationDistance == other.maximumAttenuationDistance &&
				frequencyAdjustmentPercentage == other.frequencyAdjustmentPercentage &&
				flags == other.flags &&
				staticAttenuationcdB == other.staticAttenuationcdB &&
				stopTime == other.stopTime &&
				startTime == other.startTime &&
				attenuationPoint1 == other.attenuationPoint1 &&
				attenuationPoint2 == other.attenuationPoint2 &&
				attenuationPoint3 == other.attenuationPoint3 &&
				attenuationPoint4 == other.attenuationPoint4 &&
				attenuationPoint5 == other.attenuationPoint5 &&
				reverbAttenuationControl == other.reverbAttenuationControl &&
				priority == other.priority
			);
		}

		bool operator !=(const SOUNSNDD &other) const
		{
			return !(*this == other);
		}
	};

	class SOUNRecord : public TES5Record {
	
    public:
        StringRecord EDID; //Editor ID
		ReqSubRecord<GENOBND> OBND; //Sets the bounds for the acoustic space.
		StringRecord FNAM; //Relative directory to a .wav
		OptSubRecord<SOUNSNDD> SNDD;
		ReqSubRecord<FORMID> SDSC; //12-byte struct

		
		SOUNRecord(unsigned char *_recData = NULL);
		SOUNRecord(SOUNRecord *srcRecord);
		~SOUNRecord();

		uint32_t  GetType();
		char *  GetStrType();

		bool   VisitFormIDs(FormIDOp &op);

		int32_t  ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk);
		int32_t  Unload();
		int32_t  WriteRecord(FileWriter &writer);
		char *GetEditorIDKey() { return EDID.value; }


        bool operator ==(const SOUNRecord &other) const;
        bool operator !=(const SOUNRecord &other) const;
        bool equals(Record *other);

	};


}