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

#include "../GenericChunks.h"

namespace Sk {

// LVLO subrecord for leveled lists.  Different than previous games
// in that 'level' is a uint16_t now instead of a int16_t
struct SKLVLO
    {
        uint16_t  level;
        uint8_t   unused1[2];
        FORMID  listId;
        int16_t  count;
        uint8_t   unused2[2];

        SKLVLO();
        ~SKLVLO();

        void Write(FileWriter &writer);

        bool operator ==(const SKLVLO &other) const;
        bool operator !=(const SKLVLO &other) const;
    };
SIZE_CHECK(SKLVLO, 12);

// LVLO/COED pair for LVLI records
struct SKLVLOCOED
    {
        ReqSubRecord<SKLVLO> LVLO;
        OptSubRecord<GENCOED> COED;

        bool IsGlobal() const;
        bool IsRank() const;

        void Write(FileWriter &writer);

        bool operator ==(const SKLVLOCOED &other) const;
        bool operator !=(const SKLVLOCOED &other) const;
    };

// CNTO/COED pair for NPC_ records
struct SKCNTOCOED
    {
        ReqSubRecord<GENCNTO> CNTO;
        OptSubRecord<GENCOED> COED;

        bool IsGlobal() const;
        bool IsRank() const;

        void Write(FileWriter &writer);

        bool operator ==(const SKCNTOCOED &other) const;
        bool operator !=(const SKCNTOCOED &other) const;
    };

struct MODEL
    {
        StringRecord MODL;  // Model Filename
        RawRecord MODT;     // Texture Files Hashes

        MODEL();
        ~MODEL();

        void Write(FileWriter &write);

        bool operator ==(const MODEL &other) const;
        bool operator !=(const MODEL &other) const;
    };

struct GENCNAM  // Color
    {
        uint8_t red, green, blue, unk1; // unk in xEdit, unused on UESPWiki

        GENCNAM();
        ~GENCNAM();

        bool operator == (const GENCNAM &other) const;
        bool operator != (const GENCNAM &other) const;
    };
SIZE_CHECK(GENCNAM, 4);

struct SKDESTSTAGE
{
    ReqSubRecord<DESTDSTD> DSTD;
    StringRecord DMDL;
    RawRecord DMDT;
    FNVAlternateTextures DMDS;

    void Write(FileWriter &writer);
    void VisitFormIDs(FormIDOp &op);

    bool operator == (const SKDESTSTAGE &other) const;
    bool operator != (const SKDESTSTAGE &other) const;
};
struct sortSKDESTStages
{
    bool operator()(const SKDESTSTAGE *lhs, const SKDESTSTAGE *rhs) const;
};

struct SKDESTRUCT //Destructable
{
    ReqSubRecord<GENDEST> DEST; //Destructable Header
    OrderedSparseArray<SKDESTSTAGE *, sortSKDESTStages> Stages; //Destructable Stages

    enum flagsFlags
    {
        fIsVATSTargetable = 0x01
    };

    void Write(FileWriter &writer);
    void VisitFormIDs(FormIDOp &op);

    bool operator == (const SKDESTRUCT &other) const;
    bool operator != (const SKDESTRUCT &other) const;
};

struct SKCTDA //Condition
{
    enum CastingSource : uint32_t {
        Left = 0,
        Right,
        Voice,
        Instant
    };

    enum function : uint16_t {
        fn_GetWantBlocking = 0,
        fn_GetDistance = 1,
        fn_GetLocked = 5,
        fn_GetPos = 6,
        fn_GetAngle = 8,
        fn_GetStartingPos = 10,
        fn_GetStartingAngle = 11,
        fn_GetSecondsPassed = 12,
        fn_GetActorValue = 14,
        fn_GetCurrentTime = 18,
        fn_GetScale = 24,
        fn_IsMoving = 25,
        fn_IsTurning = 26,
        fn_GetLineOfSight = 27,
        fn_GetInSameCell = 32,
        fn_GetDisabled = 35,
        fn_MenuMode = 36,
        fn_GetDisease = 39,
        fn_GetClothingValue = 41,
        fn_SameFaction = 42,
        fn_SameRace = 43,
        fn_SameSex = 44,
        fn_GetDetected = 45,
        fn_GetDead = 46,
        fn_GetItemCount = 47,
        fn_GetGold = 48,
        fn_GetSleeping = 49,
        fn_GetTalkedToPC = 50,
        fn_GetScriptVariable = 53,
        fn_GetQuestRunning = 56,
        fn_GetStage = 58,
        fn_GetStageDone = 59,
        fn_GetFactionRankDifference = 60,
        fn_GetAlarmed = 61,
        fn_IsRaining = 62,
        fn_GetAttacked = 63,
        fn_GetIsCreature = 64,
        fn_GetLockLevel = 65,
        fn_GetShouldAttack = 66,
        fn_GetInCell = 67,
        fn_GetIsClass = 68,
        fn_GetIsRace = 69,
        fn_GetIsSex = 70,
        fn_GetInFaction = 71,
        fn_GetIsID = 72,
        fn_GetFactionRank = 73,
        fn_GetGlobalValue = 74,
        fn_IsSnowing = 75,
        fn_GetRandomPercent = 77,
        fn_GetQuestVariable = 79,
        fn_GetLevel = 80,
        fn_IsRotating = 81,
        fn_GetDeadCount = 84,
        fn_GetIsAlerted = 91,
        fn_GetPlayerControlsDisabled = 98,
        fn_GetHeadingAngle = 99,
        fn_IsWeaponMagicOut = 101,
        fn_IsTorchOut = 102,
        fn_IsShieldOut = 103,
        fn_IsFacingUp = 106,
        fn_GetKnockedState = 107,
        fn_GetWeaponAnimType = 108,
        fn_IsWeaponSkillType = 109,
        fn_GetCurrentAIPackage = 110,
        fn_IsWaiting = 111,
        fn_IsIdlePlaying = 112,
        fn_IsIntimidatedbyPlayer = 116,
        fn_IsPlayerInRegion = 117,
        fn_GetActorAggroRadiusViolated = 118,
        fn_GetCrime = 122,
        fn_IsGreetingPlayer = 123,
        fn_IsGuard = 125,
        fn_HasBeenEaten = 127,
        fn_GetStaminaPercentage = 128,
        fn_GetPCIsClass = 129,
        fn_GetPCIsRace = 130,
        fn_GetPCIsSex = 131,
        fn_GetPCInFaction = 132,
        fn_SameFactionAsPC = 133,
        fn_SameRaceAsPC = 134,
        fn_SameSexAsPC = 135,
        fn_GetIsReference = 136,
        fn_IsTalking = 141,
        fn_GetWalkSpeed = 142,
        fn_GetCurrentAIProcedure = 143,
        fn_GetTrespassWarningLevel = 144,
        fn_IsTrespassing = 145,
        fn_IsInMyOwnedCell = 146,
        fn_GetWindSpeed = 147,
        fn_GetCurrentWeatherPercent = 148,
        fn_GetIsCurrentWeather = 149,
        fn_IsContinuingPackagePCNear = 150,
        fn_GetIsCrimeFaction = 152,
        fn_CanHaveFlames = 153,
        fn_HasFlames = 154,
        fn_GetOpenState = 157,
        fn_GetSitting = 159,
        fn_GetIsCurrentPackage = 161,
        fn_IsCurrentFurnitureRef = 162,
        fn_IsCurrentFurnitureObj = 163,
        fn_GetDayOfWeek = 170,
        fn_GetTalkedToPCParam = 172,
        fn_IsPCSleeping = 175,
        fn_IsPCAMurderer = 176,
        fn_HasSameEditorLocAsRef = 180,
        fn_HasSameEditorLocAsRefAlias = 181,
        fn_GetEquipped = 182,
        fn_IsSwimming = 185,
        fn_GetAmountSoldStolen = 190,
        fn_GetIgnoreCrime = 192,
        fn_GetPCExpelled = 193,
        fn_GetPCFactionMurder = 195,
        fn_GetPCEnemyofFaction = 197,
        fn_GetPCFactionAttack = 199,
        fn_GetDestroyed = 203,
        fn_HasMagicEffect = 214,
        fn_GetDefaultOpen = 215,
        fn_GetAnimAction = 219,
        fn_IsSpellTarget = 223,
        fn_GetVATSMode = 224,
        fn_GetPersuasionNumber = 225,
        fn_GetVampireFeed = 226,
        fn_GetCannibal = 227,
        fn_GetIsClassDefault = 228,
        fn_GetClassDefaultMatch = 229,
        fn_GetInCellParam = 230,
        fn_GetVatsTargetHeight = 235,
        fn_GetIsGhost = 237,
        fn_GetUnconscious = 242,
        fn_GetRestrained = 244,
        fn_GetIsUsedItem = 246,
        fn_GetIsUsedItemType = 247,
        fn_IsScenePlaying = 248,
        fn_IsInDialogueWithPlayer = 249,
        fn_GetLocationCleared = 250,
        fn_GetIsPlayableRace = 254,
        fn_GetOffersServicesNow = 255,
        fn_HasAssociationType = 258,
        fn_HasFamilyRelationship = 259,
        fn_HasParentRelationship = 261,
        fn_IsWarningAbout = 262,
        fn_IsWeaponOut = 263,
        fn_HasSpell = 264,
        fn_IsTimePassing = 265,
        fn_IsPleasant = 266,
        fn_IsCloudy = 267,
        fn_IsSmallBump = 274,
        fn_GetBaseActorValue = 277,
        fn_IsOwner = 278,
        fn_IsCellOwner = 280,
        fn_IsHorseStolen = 282,
        fn_IsLeftUp = 285,
        fn_IsSneaking = 286,
        fn_IsRunning = 287,
        fn_GetFriendHit = 288,
        fn_IsInCombat = 289,
        fn_IsInInterior = 300,
        fn_IsWaterObject = 304,
        fn_GetPlayerAction = 305,
        fn_IsActorUsingATorch = 306,
        fn_IsXBox = 309,
        fn_GetInWorldspace = 310,
        fn_GetPCMiscStat = 312,
        fn_GetPairedAnimation = 313,
        fn_IsActorAVictim = 314,
        fn_GetTotalPersuasionNumber = 315,
        fn_GetIdleDoneOnce = 318,
        fn_GetNoRumors = 320,
        fn_GetCombatState = 323,
        fn_GetWithinPackageLocation = 325,
        fn_IsRidingMount = 327,
        fn_IsFleeing = 329,
        fn_IsInDangerousWater = 332,
        fn_GetIgnoreFriendlyHits = 338,
        fn_IsPlayersLastRiddenMount = 339,
        fn_IsActor = 353,
        fn_IsEssential = 354,
        fn_IsPlayerMovingIntoNewSpace = 358,
        fn_GetInCurrentLoc = 359,
        fn_GetInCurrentLocAlias = 360,
        fn_GetTimeDead = 361,
        fn_HasLinkedRef = 362,
        fn_IsChild = 365,
        fn_GetStolenItemValueNoCrime = 366,
        fn_GetLastPlayerAction = 367,
        fn_IsPlayerActionActive = 368,
        fn_IsTalkingActivatorActor = 370,
        fn_IsInList = 372,
        fn_GetStolenItemValue = 373,
        fn_GetCrimeGoldViolent = 375,
        fn_GetCrimeGoldNonviolent = 376,
        fn_HasShout = 378,
        fn_GetHasNote = 381,
        fn_GetHitLocation = 390,
        fn_IsPC1stPerson = 391,
        fn_GetCauseofDeath = 396,
        fn_IsLimbGone = 397,
        fn_IsWeaponInList = 398,
        fn_IsBribedbyPlayer = 402,
        fn_GetRelationshipRank = 403,
        fn_GetVATSValue = 407,
        fn_IsKiller = 408,
        fn_IsKillerObject = 409,
        fn_GetFactionCombatReaction = 410,
        fn_Exists = 414,
        fn_GetGroupMemberCount = 415,
        fn_GetGroupTargetCount = 416,
        fn_GetIsVoiceType = 426,
        fn_GetPlantedExplosive = 427,
        fn_IsScenePackageRunning = 429,
        fn_GetHealthPercentage = 430,
        fn_GetIsObjectType = 432,
        fn_GetDialogueEmotion = 434,
        fn_GetDialogueEmotionValue = 435,
        fn_GetIsCreatureType = 437,
        fn_GetInCurrentLocFormList = 444,
        fn_GetInZone = 445,
        fn_GetVelocity = 446,
        fn_GetGraphVariableFloat = 447,
        fn_HasPerk = 448,
        fn_GetFactionRelation = 449,
        fn_IsLastIdlePlayed = 450,
        fn_GetPlayerTeammate = 453,
        fn_GetPlayerTeammateCount = 454,
        fn_GetActorCrimePlayerEnemy = 458,
        fn_GetCrimeGold = 459,
        fn_IsPlayerGrabbedRef = 463,
        fn_GetKeywordItemCount = 465,
        fn_GetDestructionStage = 470,
        fn_GetIsAlignment = 473,
        fn_IsProtected = 476,
        fn_GetThreatRatio = 477,
        fn_GetIsUsedItemEquipType = 479,
        fn_IsCarryable = 487,
        fn_GetConcussed = 488,
        fn_GetMapMarkerVisible = 491,
        fn_PlayerKnows = 493,
        fn_GetPermanentActorValue = 494,
        fn_GetKillingBlowLimb = 495,
        fn_CanPayCrimeGold = 497,
        fn_GetDaysInJail = 499,
        fn_EPAlchemyGetMakingPoison = 500,
        fn_EPAlchemyEffectHasKeyword = 501,
        fn_GetAllowWorldInteractions = 503,
        fn_GetLastHitCritical = 508,
        fn_IsCombatTarget = 513,
        fn_GetVATSRightAreaFree = 515,
        fn_GetVATSLeftAreaFree = 516,
        fn_GetVATSBackAreaFree = 517,
        fn_GetVATSFrontAreaFree = 518,
        fn_GetLockIsBroken = 519,
        fn_IsPS3 = 520,
        fn_IsWin32 = 521,
        fn_GetVATSRightTargetVisible = 522,
        fn_GetVATSLeftTargetVisible = 523,
        fn_GetVATSBackTargetVisible = 524,
        fn_GetVATSFrontTargetVisible = 525,
        fn_IsInCriticalStage = 528,
        fn_GetXPForNextLevel = 530,
        fn_GetInfamy = 533,
        fn_GetInfamyViolent = 534,
        fn_GetInfamyNonViolent = 535,
        fn_GetQuestCompleted = 543,
        fn_IsGoreDisabled = 547,
        fn_IsSceneActionComplete = 550,
        fn_GetSpellUsageNum = 552,
        fn_GetActorsInHigh = 554,
        fn_HasLoaded3D = 555,
        fn_HasKeyword = 560,
        fn_HasRefType = 561,
        fn_LocationHasKeyword = 562,
        fn_LocationHasRefType = 563,
        fn_GetIsEditorLocation = 565,
        fn_GetIsAliasRef = 566,
        fn_GetIsEditorLocAlias = 567,
        fn_IsSprinting = 568,
        fn_IsBlocking = 569,
        fn_HasEquippedSpell = 570,
        fn_GetCurrentCastingType = 571,
        fn_GetCurrentDeliveryType = 572,
        fn_GetAttackState = 574,
        fn_GetEventData = 576,
        fn_IsCloserToAThanB = 577,
        fn_GetEquippedShout = 579,
        fn_IsBleedingOut = 580,
        fn_GetRelativeAngle = 584,
        fn_GetMovementDirection = 589,
        fn_IsInScene = 590,
        fn_GetRefTypeDeadCount = 591,
        fn_GetRefTypeAliveCount = 592,
        fn_GetIsFlying = 594,
        fn_IsCurrentSpell = 595,
        fn_SpellHasKeyword = 596,
        fn_GetEquippedItemType = 597,
        fn_GetLocationAliasCleared = 598,
        fn_GetLocAliasRefTypeDeadCount = 600,
        fn_GetLocAliasRefTypeAliveCount = 601,
        fn_IsWardState = 602,
        fn_IsInSameCurrentLocAsRef = 603,
        fn_IsInSameCurrentLocAsRefAlias = 604,
        fn_LocAliasIsLocation = 605,
        fn_GetKeywordDataForLocation = 606,
        fn_GetKeywordDataForAlias = 608,
        fn_LocAliasHasKeyword = 610,
        fn_IsNullPackageData = 611,
        fn_GetNumericPackageData = 612,
        fn_IsFurnitureAnimType = 613,
        fn_IsFurnitureEntryType = 614,
        fn_GetHighestRelationshipRank = 615,
        fn_GetLowestRelationshipRank = 616,
        fn_HasAssociationTypeAny = 617,
        fn_HasFamilyRelationshipAny = 618,
        fn_GetPathingTargetOffset = 619,
        fn_GetPathingTargetAngleOffset = 620,
        fn_GetPathingTargetSpeed = 621,
        fn_GetPathingTargetSpeedAngle = 622,
        fn_GetMovementSpeed = 623,
        fn_GetInContainer = 624,
        fn_IsLocationLoaded = 625,
        fn_IsLocAliasLoaded = 626,
        fn_IsDualCasting = 627,
        fn_GetVMQuestVariable = 629,
        fn_GetVMScriptVariable = 630,
        fn_IsEnteringInteractionQuick = 631,
        fn_IsCasting = 632,
        fn_GetFlyingState = 633,
        fn_IsInFavorState = 635,
        fn_HasTwoHandedWeaponEquipped = 636,
        fn_IsExitingInstant = 637,
        fn_IsInFriendStateWithPlayer = 638,
        fn_GetWithinDistance = 639,
        fn_GetActorValuePercent = 640,
        fn_IsUnique = 641,
        fn_GetLastBumpDirection = 642,
        fn_IsInFurnitureState = 644,
        fn_GetIsInjured = 645,
        fn_GetIsCrashLandRequest = 646,
        fn_GetIsHastyLandRequest = 647,
        fn_IsLinkedTo = 650,
        fn_GetKeywordDataForCurrentLocation = 651,
        fn_GetInSharedCrimeFaction = 652,
        fn_GetBribeSuccess = 654,
        fn_GetIntimidateSuccess = 655,
        fn_GetArrestedState = 656,
        fn_GetArrestingActor = 657,
        fn_EPTemperingItemIsEnchanted = 659,
        fn_EPTemperingItemHasKeyword = 660,
        fn_GetReplacedItemType = 664,
        fn_IsAttacking = 672,
        fn_IsPowerAttacking = 673,
        fn_IsLastHostileActor = 674,
        fn_GetGraphVariableInt = 675,
        fn_GetCurrentShoutVariation = 676,
        fn_ShouldAttackKill = 678,
        fn_GetActivatorHeight = 680,
        fn_EPMagic_IsAdvanceSkill = 681,
        fn_WornHasKeyword = 682,
        fn_GetPathingCurrentSpeed = 683,
        fn_GetPathingCurrentSpeedAngle = 684,
        fn_EPModSkillUsage_AdvanceObjectHasKeyword = 691,
        fn_EPModSkillUsage_IsAdvanceAction = 692,
        fn_EPMagic_SpellHasKeyword = 693,
        fn_GetNoBleedoutRecovery = 694,
        fn_EPMagic_SpellHasSkill = 696,
        fn_IsAttackType = 697,
        fn_IsAllowedToFly = 698,
        fn_HasMagicEffectKeyword = 699,
        fn_IsCommandedActor = 700,
        fn_IsStaggered = 701,
        fn_IsRecoiling = 702,
        fn_IsExitingInteractionQuick = 703,
        fn_IsPathing = 704,
        fn_GetShouldHelp = 705,
        fn_HasBoundWeaponEquipped = 706,
        fn_GetCombatTargetHasKeyword = 707,
        fn_GetCombatGroupMemberCount = 709,
        fn_IsIgnoringCombat = 710,
        fn_GetLightLevel = 711,
        fn_SpellHasCastingPerk = 713,
        fn_IsBeingRidden = 714,
        fn_IsUndead = 715,
        fn_GetRealHoursPassed = 716,
        fn_IsUnlockedDoor = 718,
        fn_IsHostileToActor = 719,
        fn_GetTargetHeight = 720,
        fn_IsPoison = 721,
        fn_WornApparelHasKeywordCount = 722,
        fn_GetItemHealthPercent = 723,
        fn_EffectWasDualCast = 724,
        fn_GetKnockedStateEnum = 725,
        fn_DoesNotExist = 726,
        fn_IsOnFlyingMount = 730,
        fn_CanFlyHere = 731,
        fn_IsFlyingMountPatrolQueud = 732,
        fn_IsFlyingMountFastTravelling = 733,
        fn_IsOverEncumbered = 734,
        GetActorWarmth = 735,

        // Added by SKSE
        fn_GetSKSEVersion = 1024,
        fn_GetSKSEVersionMinor = 1025,
        fn_GetSKSEVersionBeta = 1026,
        fn_GetSKSERelease = 1027,
        fn_ClearInvalidRegistrations = 1028
    };


    uint8_t   operType, unused1[3]; //Type, Unused
    FORMID_OR_FLOAT32 compValue; //Float or Global (if fIsUseGlobal is true)
    function  ifunc; // function index
    uint8_t   unused2[2]; // padding
    FORMID_OR_UINT32 param1, param2;
    //Below are not always present on chunk...
    uint32_t  runOnType; //Run On
    FORMID_OR_UINT32 reference; //Reference (if runOn == 2) or Unused
    int32_t  param3;

    enum operTypeType
    {
        eEqual = 0<<5,
        eNotEqual = 1<<5,
        eGreater = 2<<5,
        eGreaterOrEqual = 3<<5,
        eLess = 4<<5,
        eLessOrEqual = 5<<5,
        eOperTypeMask = 0xE0 // First 3 bits
    };

    enum operTypeFlag
    {
        fIsNone = 0x00,
        fIsOr = 0x01,
        fIsUseAliases = 0x02,
        fIsUseGlobal = 0x04,
        fIsUsePackData = 0x08,
        fIsSwapSubject = 0x10,
        fOperFlagMask = 0x1F // Last 5 bits
    };

    enum operRunOnType
    {
        eSubject = 0,
        eTarget,
        eReference,
        eCombatTarget,
        eLinkedReference,
        eQuestAlias,
        ePackageData,
        eEventData
    };

    SKCTDA();
    ~SKCTDA();

    bool VisitFormIDs(FormIDOp &op);
    void Write(FileWriter &writer);

    bool operator ==(const SKCTDA &other) const;
    bool operator !=(const SKCTDA &other) const;

    bool IsEqual() const;
    void IsEqual(bool value);
    bool IsNotEqual() const;
    void IsNotEqual(bool value);
    bool IsGreater() const;
    void IsGreater(bool value);
    bool IsGreaterOrEqual() const;
    void IsGreaterOrEqual(bool value);
    bool IsLess() const;
    void IsLess(bool value);
    bool IsLessOrEqual() const;
    void IsLessOrEqual(bool value);
    bool IsType(uint8_t Type) const;
    void SetType(uint8_t Type);

    bool IsNone() const;
    void IsNone(bool value);
    bool IsOr() const;
    void IsOr(bool value);
    bool IsUseAliases() const;
    void IsUseAliases(bool value);
    bool IsUseGlobal() const;
    void IsUseGlobal(bool value);
    bool IsUsePackData() const;
    void IsUsePackData(bool value);
    bool IsSwapSubject() const;
    void IsSwapSubject(bool value);
    bool IsFlagMask(uint8_t Mask, bool Exact = false) const;
    void SetFlagMask(uint8_t Mask);

    bool IsResultOnSubject() const;
    void IsResultOnSubject(bool value);
    bool IsResultOnTarget() const;
    void IsResultOnTarget(bool value);
    bool IsResultOnReference() const;
    void IsResultOnReference(bool value);
    bool IsResultOnCombatTarget() const;
    void IsResultOnCombatTarget(bool value);
    bool IsResultOnLinkedReference() const;
    void IsResultOnLinkedReference(bool value);
    bool IsResultOnQuestAlias() const;
    void IsResultOnQuestAlias(bool value);
    bool IsResultOnPackageData() const;
    void IsResultOnPackageData(bool value);
    bool IsResultOnEventData() const;
    void IsResultOnEventData(bool value);
    bool IsResultOnType(uint32_t Type) const;
    void SetResultOnType(uint32_t Type);
};
SIZE_CHECK(SKCTDA, 32);

struct SKCondition
{
    ReqSubRecord<SKCTDA> CTDA; // Condition
    StringRecord CIS1; // Parameter 1
    StringRecord CIS2; // Parameter 2

    bool VisitFormIDs(FormIDOp &op);
    void Write(FileWriter &writer);

    bool operator ==(const SKCondition &other) const;
    bool operator !=(const SKCondition &other) const;
};
#if _WIN64
SIZE_CHECK(SKCondition, 64);//WTM:  Was 48.  I wish I could ask Aerisarn why this line was added.
#else
SIZE_CHECK(SKCondition, 48);
#endif


struct SKEFIT
{
    float magnitude;
    uint32_t  area, duration;

    SKEFIT();
    ~SKEFIT();

    bool operator ==(const SKEFIT &other) const;
    bool operator !=(const SKEFIT &other) const;
};
SIZE_CHECK(SKEFIT, 12);

struct SKEffect
{
    ReqSimpleSubRecord<FORMID> EFID; //Effect ID
    ReqSubRecord<SKEFIT> EFIT; //Effect Data
    OrderedSparseArray<SKCondition *> CTDA; //Conditions

    bool VisitFormIDs(FormIDOp &op);
    void Write(FileWriter &writer);

    bool operator ==(const SKEffect &other) const;
    bool operator !=(const SKEffect &other) const;
};

struct SKBODT
{
    uint32_t body_part;
    uint8_t flags;
    uint8_t junk_data[3];
    uint32_t skill;

    bool operator ==(const SKBODT& other) const;
    bool operator !=(const SKBODT& other) const;
};
SIZE_CHECK(SKBODT, 12);

struct SKBOD2
{
    enum BodyParts {
        bpHead = 0x00000001,  //'30 - Head',
        bpHair = 0x00000002,  //'31 - Hair',
        bpBody = 0x00000004,  //'32 - Body',
        bpHands = 0x00000008,  //'33 - Hands',
        bpForearms = 0x00000010,  //'34 - Forearms',
        bpAmulet = 0x00000020,  //'35 - Amulet',
        bpRing = 0x00000040,  //36 - Ring',
        bpFeet = 0x00000080,  //37 - Feet',
        bpCalves = 0x00000100,  //38 - Calves',
        bpShield = 0x00000200,  //39 - Shield',
        bpTail = 0x00000400,  //40 - Tail',
        bpLongHair = 0x00000800,  //41 - LongHair',
        bpCirclet = 0x00001000,  //42 - Circlet',
        bpEars = 0x00002000,  //43 - Ears',
        bpUnnamed44 = 0x00004000,  //44 - Unnamed',
        bpUnnamed45 = 0x00008000,  //45 - Unnamed',
        bpUnnamed46 = 0x00010000,  //46 - Unnamed',
        bpUnnamed47 = 0x00020000,  //47 - Unnamed',
        bpUnnamed48 = 0x00040000,  //48 - Unnamed',
        bpUnnamed49 = 0x00080000,  //49 - Unnamed',
        bpDecapitateHead = 0x00100000,  //50 - DecapitateHead',
        bpDecapitate = 0x00200000,  //51 - Decapitate',
        bpUnnamed52 = 0x00400000,  //52 - Unnamed',
        bpUnnamed53 = 0x00800000,  //53 - Unnamed',
        bpUnnamed54 = 0x01000000,  //54 - Unnamed',
        bpUnnamed55 = 0x02000000,  //55 - Unnamed',
        bpUnnamed56 = 0x04000000,  //56 - Unnamed',
        bpUnnamed57 = 0x08000000,  //57 - Unnamed',
        bpUnnamed58 = 0x10000000,  //58 - Unnamed',
        bpUnnamed59 = 0x20000000,  //59 - Unnamed',
        bpUnnamed60 = 0x40000000,  //60 - Unnamed',
        bpUnnamed61 = 0x80000000  //61 - FX01'
    };

    uint32_t body_part;
    uint32_t skill;

    bool operator ==(const SKBOD2& other) const;
    bool operator !=(const SKBOD2& other) const;
};
SIZE_CHECK(SKBOD2, 8);

struct SKRACEDATA
{

    enum Flags01 : uint32_t {
        Playable =                      0x00000001,
        FaceGenHead =                   0x00000002,
        Child =                         0x00000004,
        TiltFrontBack =                 0x00000008,
        TiltLeftRight =                 0x00000010,
        NoShadow =                      0x00000020,
        Swims =                         0x00000040,
        Flies =                         0x00000080,
        Walks =                         0x00000100,
        Immobile =                      0x00000200,
        NotPushable =                   0x00000400,
        NoCombatInWater =               0x00000800,
        NoRotatingToHeadTrack =         0x00001000,
        DontShowBloodSpray =            0x00002000,
        DontShowBloodDecal =            0x00004000,
        UsesHeadTrackAnims =            0x00008000,
        SpellsAlignWithMagicNode =      0x00010000,
        UseWorldRaycastsForFootIK =     0x00020000,
        AllowRagdollCollision =         0x00040000,
        RegenHPInCombat =               0x00080000,
        CantOpenDoors =                 0x00100000,
        AllowPCDialogue =               0x00200000,
        NoKnockdowns =                  0x00400000,
        AllowPickpocket =               0x00800000,
        AlwaysUseProxyController =      0x01000000,
        DontShowWeaponBlood =           0x02000000,
        OverlayHeadPartList =           0x04000000, // { >> > Only one can be active << < }
        OverrideHeadPartList =          0x08000000, // { >> > Only one can be active << < }
        CanPickupItems =                0x10000000,
        AllowMultipleMembraneShaders =  0x20000000,
        CanDualWield =                  0x40000000,
        AvoidsRoads =                   0x80000000
    };


    RACESKILL skills[7];
    uint8_t   unused1[2];
    float maleHeight, femaleHeight, maleWeight, femaleWeight;
    Flags01  flags;
    float startingHealth, startingMagicka, startingStamina; 
    float baseCarryWeight, baseMass; 
    float accelerationRate, decelerationRate;
    uint32_t size; //lookup: 0=Small, 1=Medium, 2=Large, 3=Extra Large
    uint32_t headBipedObject; //lookup: Biped Object Names
    uint32_t hairBipedObject; //lookup: Biped Object Names
    float injuredHealthPct; // (value ranging from 0 to 1)
    uint32_t shieldBipedObject; // (lookup: Biped Object Names)
    float healthRegen, magickaRegen, staminaRegen;
    float unarmedDamage, unarmedReach;
    uint32_t bodyBipedObject; // (lookup: Biped Object Names)
    float aimAngleTolerance;
    float flightRadius;
    float angularAccelerationRate;
    float angularTolerance;
    uint32_t flags2; // .Only identified value is listed
    //                //0x00000001 - Use Advanced Avoidance
    //                //0x00000002 - Non - Hostile
    //                //0x00000010 - Allow Mounted Combat
    float mountOffsetX, mountOffsetY, mountOffsetZ;
    float dismountOffsetX, dismountOffsetY, dismountOffsetZ;
    float mountCameraOffsetX, mountCameraOffsetY, mountCameraOffsetZ;

    bool operator ==(const SKRACEDATA& other) const;
    bool operator !=(const SKRACEDATA& other) const;
};


//char checker(int);
//char checkSizeOfInt[sizeof(SKRACEDATA)] = { checker(&checkSizeOfInt) };
SIZE_CHECK(SKRACEDATA, 164);

struct GENATTACK
{
	struct ATTACKDATA
	{
		
        float damageMult;
        float attackChance;
        FORMID attackSpell;
        uint32_t flags;
        /*
		0x01 - Ignore Weapon
		0x02 - Bash Attack
		0x04 - Power Attack
		0x08 - Left Attack
		0x10 - Rotating Attack
        */

        float attackAngle;
        float strikeAngle;
        float stagger;
        FORMID attackType;
        float knockdown;
        float recoveryTime;
        float fatigueMult;


		bool operator ==(const ATTACKDATA &other) const;
		bool operator !=(const ATTACKDATA &other) const;
		
	};
    ReqSubRecord<ATTACKDATA> ATKD; // attack data
	StringRecord ATKE; // attack event

	void Write(FileWriter &writer);

    bool operator ==(const GENATTACK &other) const;
    bool operator !=(const GENATTACK &other) const;
};

struct GENMODEL {

};

struct GENPRKR
{
	FORMID  perk;
	uint8_t   rank, unused1[3];

	GENPRKR();
	~GENPRKR();

	void Write(FileWriter &writer);

	bool operator ==(const GENPRKR &other) const;
	bool operator !=(const GENPRKR &other) const;
};


struct XWCU {
	float x, y, z, unk;

	bool operator ==(const XWCU &other) const {
		return (this->x == other.x &&
			this->y == other.y &&
			this->z == other.z);
	}
	bool operator !=(const XWCU &other) const {
		return !(*this == other);
	}
};

struct GenXLKR {
	FORMID kywd;
	FORMID formid;
	bool operator ==(const GenXLKR &other) const {
		return (this->kywd == other.kywd &&
			this->formid == other.formid);
	}
	bool operator !=(const GenXLKR &other) const {
		return !(*this == other);
	}
};

struct GenXLOC {
	uint32_t difficulty;
	FORMID key;
	uint32_t flags;
	uint8_t zeros[8];

	bool operator ==(const GenXLOC &other) const {
		return (this->difficulty == other.difficulty &&
			this->key == other.key &&
			this->flags == other.flags);
	}
	bool operator !=(const GenXLOC &other) const {
		return !(*this == other);
	}
};

/**
* When not lazy to wait for compilation - move to common
*/
struct COORDINATES {
	float x, y, z;
	bool operator ==(const COORDINATES &other) const {
		return (this->x == other.x &&
			this->y == other.y &&
			this->z == other.z
			);
	}
	bool operator !=(const COORDINATES &other) const {
		return !(*this == other);
	}
};

struct ContainerItems {
	FORMID itemID;
	uint32_t itemCount;

	bool operator ==(const ContainerItems  &other) const {

		return (
			this->itemID == other.itemID &&
			this->itemCount == other.itemCount);

	}

	bool operator !=(const ContainerItems &other) const {
		return !(*this == other);
	}
};


struct EffectItem {
	float magnitude;
	uint32_t areaOfEffect;
	uint32_t duration;

	bool operator == (const EffectItem &other) const {
		return (
			this->magnitude == other.magnitude &&
			this->areaOfEffect == other.areaOfEffect &&
			this->duration == other.duration
		);
	}
	bool operator != (const EffectItem &other) const {
		return !(*this == other);
	}
};

struct SpellEffect {

	ReqSimpleSubRecord<FORMID> EFID;
	ReqSubRecord<EffectItem> EFIT;
	UnorderedSparseArray<SKCTDA *> CTDA;

	bool operator == (const SpellEffect &other) const {
		return (
			this->EFID == other.EFID &&
			this->EFIT == other.EFIT &&
			this->CTDA == other.CTDA 
			);
	}
	bool operator != (const SpellEffect &other) {
		return !(*this == other);
	}

	void Write(FileWriter &writer)
	{
		WRITE(EFID);
		WRITE(EFIT);
		WRITE(CTDA);
	}
};

struct GenericModel {
    StringRecord MODL; //model filename
    RawRecord MODT;
    RawRecord MODS;

    bool operator ==(const GenericModel& other) const;
    bool operator !=(const GenericModel& other) const;
};

struct BodyPart {
    SubRecord<uint32_t> INDX; //Enum index
    OptSubRecord<GenericModel> model;

    bool operator ==(const BodyPart& other) const;
    bool operator !=(const BodyPart& other) const;
};

} // namespace Sk
