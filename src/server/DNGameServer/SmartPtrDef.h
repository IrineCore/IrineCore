#pragma once

#include "SmartPtr.h"

class CDnActor;
typedef CMultiSmartPtr< CDnActor > DnActorHandle;

class CDnWeapon;
typedef CMultiSmartPtr< CDnWeapon > DnWeaponHandle;

class CDnWorldProp;
typedef CMultiSmartPtr< CDnWorldProp > DnPropHandle;

class CDnMonsterActor;
typedef CMultiSmartPtr< CDnMonsterActor > DnMonsterActorHandle;

class CDnSkillBase;
typedef CMultiSmartPtr< CDnSkillBase > DnSkillHandle_;

class CDnSkill;
typedef CMultiSmartPtr< CDnSkill > DnSkillHandle;

class CDnItem;
typedef CMultiSmartPtr< CDnItem > DnItemHandle;

class CDnDropItem;
typedef CMultiSmartPtr< CDnDropItem > DnDropItemHandle;

class CDnParts;
typedef CMultiSmartPtr<CDnParts> DnPartsHandle;

class CDnBlow;
typedef CMultiSmartPtr<CDnBlow> DnBlowHandle;

class CDnGlyph;
typedef CMultiSmartPtr<CDnGlyph> DnGlyphHandle;

#if defined(PRE_ADD_TALISMAN_SYSTEM)
class CDnTalisman;
typedef CMultiSmartPtr<CDnTalisman> DnTalismanHandle;
#endif


class CDnVehicleActor;
typedef CSmartPtr<CDnVehicleActor> DnVehicleHandle;

class CDnSimpleParts;
typedef CMultiSmartPtr<CDnSimpleParts> DnSimplePartsHandle;