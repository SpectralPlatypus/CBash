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

	struct MOVTSPED {

		float leftWalk;
		float LeftRun;
		float rightWalk;
		float rightRun;
		float forwardWalk;
		float forwardRun;
		float backWalk;
		float backRun;
		float rotateInPlaceWalk;
		float rotateInPlaceRun;

		MOVTSPED() :
			leftWalk(0.0),
			LeftRun(0.0),
			rightWalk(0.0),
			rightRun(0.0),
			forwardWalk(0.0),
			forwardRun(0.0),
			backWalk(0.0),
			backRun(0.0),
			rotateInPlaceWalk(0.0),
			rotateInPlaceRun(0.0)
		{
		}

		bool operator ==(const MOVTSPED &other) const {
			return (
				leftWalk == other.leftWalk &&
				LeftRun == other.LeftRun &&
				rightWalk == other.rightWalk &&
				rightRun == other.rightRun &&
				forwardWalk == other.forwardWalk &&
				forwardRun == other.forwardRun &&
				backWalk == other.backWalk &&
				backRun == other.backRun &&
				rotateInPlaceWalk == other.rotateInPlaceWalk &&
				rotateInPlaceRun == other.rotateInPlaceRun
			);
		}

		bool operator !=(const MOVTSPED &other) const
		{
			return !(*this == other);
		}
	};

	struct EMOVTSPED {
		float leftWalk;
		float LeftRun;
		float rightWalk;
		float rightRun;
		float forwardWalk;
		float forwardRun;
		float backWalk;
		float backRun;
		float rotateInPlaceWalk;
		float rotateInPlaceRun;
		float rotateWhileMovingRun;

		EMOVTSPED() :
			leftWalk(0.0),
			LeftRun(0.0),
			rightWalk(0.0),
			rightRun(0.0),
			forwardWalk(0.0),
			forwardRun(0.0),
			backWalk(0.0),
			backRun(0.0),
			rotateInPlaceWalk(0.0),
			rotateInPlaceRun(0.0),
			rotateWhileMovingRun(0.0)
		{
		}

		bool operator ==(const EMOVTSPED &other) const {
			return (
				leftWalk == other.leftWalk &&
				LeftRun == other.LeftRun &&
				rightWalk == other.rightWalk &&
				rightRun == other.rightRun &&
				forwardWalk == other.forwardWalk &&
				forwardRun == other.forwardRun &&
				backWalk == other.backWalk &&
				backRun == other.backRun &&
				rotateInPlaceWalk == other.rotateInPlaceWalk &&
				rotateInPlaceRun == other.rotateInPlaceRun &&
				rotateWhileMovingRun == other.rotateWhileMovingRun
				);
		}

		bool operator !=(const EMOVTSPED &other) const
		{
			return !(*this == other);
		}
	};

	struct MOVTINAM {

		float directionalScale = 57.296;
		float movementSpeedScale = 57.296;
		float rotationSpeedScale = 57.296;

		MOVTINAM() {}

		bool operator ==(const MOVTINAM &other) const {
			return (
				this->directionalScale == other.directionalScale &&
				this->movementSpeedScale == other.movementSpeedScale &&
				this->rotationSpeedScale == other.rotationSpeedScale
				);
		}

		bool operator !=(const MOVTINAM &other) const
		{
			return !(*this == other);
		}

		~MOVTINAM() {}

	};

	class MOVTRecord : public TES5Record {
	
    public:
        StringRecord EDID; //Editor ID
		StringRecord MNAM; //Behavior Movement state variable name (without iState prefix)
		ReqSubRecord<EMOVTSPED> ESPED;
		OptSubRecord<MOVTINAM> INAM; //12-byte struct

		
		MOVTRecord(unsigned char *_recData = NULL);
		MOVTRecord(MOVTRecord *srcRecord);
		~MOVTRecord();

		uint32_t  GetType();
		char *  GetStrType();

		bool   VisitFormIDs(FormIDOp &op);

		int32_t  ParseRecord(unsigned char *buffer, unsigned char *end_buffer, bool CompressedOnDisk);
		int32_t  Unload();
		int32_t  WriteRecord(FileWriter &writer);
		char *GetEditorIDKey() { return EDID.value; }


        bool operator ==(const MOVTRecord &other) const;
        bool operator !=(const MOVTRecord &other) const;
        bool equals(Record *other);

	};


}