#pragma once

const int STATE_BLOW_ARGUMENT_MAX_SIZE = 64;

#define SB_NONE					0x00000000
#define SB_ONBEGIN				0x00000001
#define SB_ONEND				0x00000002
#define SB_ONCHANGEDWEAPON		0x00000004
#define SB_ONDEFENSEATTACK		0x00000008
#define SB_ONUSEMP				0x00000010
#define SB_ONTARGETHIT			0x00000020
#define SB_ONCALCDAMAGE			0x00000040
#define SB_AFTERONDEFENSEATTACK	0x00000080
#define SB_ONBEGIN_SKILL		0x00000100

// Note: State effect index is widely depended upon from design specs to permanent items
// Core index of the skill system; modifying existing indices is prohibited.
// When adding a state effect, add its number here, and add it to CDnCreateBlow's IsBasicBlow and the creation switch statement for state effects with dedicated classes.
// When adding a negative state effect (debuff), also add it separately to DissolveDebufBlow so it can be dispelled.
struct STATE_BLOW
{
	enum emBLOW_INDEX
	{
		BLOW_NONE = 0,
		BLOW_001 = 1,	// 1) Flat physical attack power
		BLOW_002 = 2,	// 2) Physical attack power ratio
		BLOW_003 = 3,	// 3) Physical defense absolute
		BLOW_004 = 4,	// 4) Physical defense ratio
		BLOW_005 = 5,	// 5) Increase Strength
		BLOW_006 = 6,	// 6) Increase Agility
		BLOW_007 = 7,	// 7) Increase Intelligence
		BLOW_008 = 8,	// 8) Increase Vitality
		BLOW_009 = 9,	// 9) Increase Max HP
		BLOW_010 = 10,	// 10) Increase max MP
		BLOW_011 = 11,	// 11) Continuous flat HP increase
		BLOW_012 = 12,	// 12) Continuous ratio HP increase
		BLOW_013 = 13,	// 12) Continuous flat MP increase
		BLOW_014 = 14,	// 14) Continuous ratio MP increase
		BLOW_015 = 15,	// 15) Current flat HP increase
		BLOW_016 = 16,	// 16) Current ratio HP increase
		BLOW_017 = 17,	// 17) Current flat MP increase
		BLOW_018 = 18,	// 18) Current MP increase ratio
		BLOW_019 = 19,	// 19) Increase stiffness power
		BLOW_020 = 20,	// 20) Increase stiffness resistance
		BLOW_021 = 21,	// 21) Increase critical
		BLOW_022 = 22,	// 22) Increase critical resistance
		BLOW_023 = 23,	// 23) Increase stun chance
		BLOW_024 = 24,	// 24) Increase stun resistance
		BLOW_025 = 25,	// 25) Increase frame
		BLOW_026 = 26,	// 26) Increase down delay
		BLOW_028 = 28,	// 28) Magic attack absolute
		BLOW_029 = 29,	// 29) Magic attack ratio
		BLOW_030 = 30,	// 30) Block
		BLOW_031 = 31,	// 31) Parry
		BLOW_032 = 32,	// 32) Increase fire attack
		BLOW_033 = 33,	// 33) Increase ice attack
		BLOW_034 = 34,	// 34) Increase light attack
		BLOW_035 = 35,	// 35) Increase dark attack
		BLOW_036 = 36,	// 36) Increase fire resistance
		BLOW_037 = 37,	// 37) Increase ice resistance
		BLOW_038 = 38,	// 38) Increase light resistance
		BLOW_039 = 39,	// 39) Increase dark resistance
		BLOW_040 = 40,	// 40) Charge power
		BLOW_041 = 41,	// 41) Freeze
		BLOW_042 = 42,	// 42) Burn
		BLOW_043 = 43,	// 43) Shock
		BLOW_044 = 44,	// 44) Poison
		BLOW_045 = 45,	// 45) Sleep
		BLOW_048 = 48,	// 48) Skill point
		BLOW_049 = 49,	// 49) Increase EXP
		BLOW_050 = 50,	// 50) Increase damage.
		BLOW_051 = 51,	// 51) #53454 Puppet state effect: transfers damage to owner actor (puppet monster must have this state effect applied by default in table)
		BLOW_055 = 55,	// 55) Guardian Force (shield absorbing absolute amount of damage)
		BLOW_056 = 56,	// 56) Adds value to the probability in the parrying state effect.
		BLOW_057 = 57,	// 57) Resurrection
		BLOW_058 = 58,	// 58) Max HP ratio
		BLOW_059 = 59,	// 59) Max SP ratio
		BLOW_060 = 60,	// 60) Chain attack
		BLOW_061 = 61,	// 61) Increase Super Armor (absolute)
		BLOW_062 = 62,	// 62) Charger state effect
		BLOW_063 = 63,	// 63) Petrify
		BLOW_064 = 64,  // 64) Increase Super Armor (ratio)
		BLOW_065 = 65,	// 65) Replace weapon range.
		BLOW_066 = 66,	// 66) Aggro adjustment state effect
		BLOW_067 = 67,	// 67) Restrict active skill usage
		BLOW_068 = 68,	// 68) Payback Mana. Applies ratio to MP consumption with given probability
		BLOW_069 = 69,	// 69) Dispel debuff state effect
		BLOW_070 = 70,	// 70) Cannot move
		BLOW_071 = 71,	// 71) Cannot act
		BLOW_072 = 72,	// 72) Gather energy (used only in Cleric Impact Blow skill)
		BLOW_073 = 73,	// 73) Invisibility
		BLOW_074 = 74,	// 74) Max attack power
		BLOW_075 = 75,	// 75) Move speed change (absolute)
		BLOW_076 = 76,	// 76) Move speed change (ratio)
		BLOW_077 = 77,	// 77) Effect immunity (does not affect already applied state effects)
		BLOW_078 = 78,	// 78) State effect preventing use of MP-consuming skills
		BLOW_079 = 79,	// 79) Super Armor Break (differs slightly from 2nd specialization (162); sets Super Armor to 0 without changing critical calculation)
		BLOW_082 = 82, //  82) Increase Final Damage ratio.
		BLOW_083 = 83,	// 83) Dispel buff state effect.
		BLOW_084 = 84,	// 84) Force state transition
		BLOW_086 = 86,	// 86) Invert team
		BLOW_087 = 87,	// 87) Increase Strength ratio
		BLOW_088 = 88,	// 88) Increase Agility ratio
		BLOW_089 = 89,	// 89) Increase Intelligence ratio
		BLOW_090 = 90,	// 90) Increase Health ratio
		BLOW_091 = 91,	// 91) Attacker HP absolute
		BLOW_092 = 92,	// 92) Attacker HP ratio 
		BLOW_093 = 93,	// 93) Magic Defense absolute
		BLOW_094 = 94,	// 94) Magic defense ratio
		BLOW_096 = 96,	// 96) Cooldown ratio adjustment state effect.
		BLOW_099 = 99,	// 99) Invincibility state effect used only by code
		BLOW_100 = 100,	// 100) State effect that only attaches a graphic effect
		BLOW_101 = 101, // 101) Flat physical melee defense change
		BLOW_102 = 102, // 102) Ratio physical melee defense change
		BLOW_103 = 103, // 103) Flat physical ranged defense change
		BLOW_104 = 104, // 104) Ratio physical ranged defense change
		BLOW_105 = 105, // 105) Flat magic melee defense change
		BLOW_106 = 106, // 106) Ratio magic melee defense change
		BLOW_107 = 107, // 107) Flat magic ranged defense change
		BLOW_108 = 108, // 108) Ratio magic ranged defense change
		BLOW_109 = 109, // 109) Apply target's physical attack power damage
		BLOW_110 = 110,	// 110) Apply target's magic attack power damage
		BLOW_111 = 111, // 111) Reflect physical melee damage
		BLOW_112 = 112, // 112) Reflect physical ranged damage
		BLOW_113 = 113, // 113) Reflect magic melee damage
		BLOW_114 = 114, // 114) Reflect magic ranged damage
		BLOW_115 = 115,	// 115) Poison explosion
		BLOW_116 = 116, // 116) Burn explosion
		BLOW_117 = 117,	// 117) Freeze explosion
		BLOW_118 = 118,	// 118) Shock explosion
		BLOW_120 = 120,	// 120) Item-disable state effect used during GM intrusion
		BLOW_121 = 121,	// 121) Replaces default Stand action with the specified action name.
		BLOW_123 = 123,	// 123) Change flinch power ratio
		BLOW_124 = 124,	// 124) Change flinch resistance ratio
		BLOW_125 = 125,	// 125) Critical ratio change
		BLOW_126 = 126,	// 126) Change critical resistance ratio
		BLOW_127 = 127,	// 127) Stun ratio change
		BLOW_128 = 128,	// 128) Change stun resistance ratio
		BLOW_129 = 129, // 129) State effect that changes action name in CDnActor::SetActionQueue using action-replace skill processor.
		BLOW_130 = 130,	// 130) Attack-disable state effect. Currently only used when stage clear score is displayed.
		BLOW_131 = 131,	// 131) Weight change state effect.
		BLOW_132 = 132,	// 132) Taunt state effect.
		BLOW_133 = 133,	// 133) Forced stun state effect. Ignores standard stun formula on hit and calculates stun using the state effect's probability value.
		BLOW_134 = 134, // 134) Physical incoming damage ratio change
		BLOW_135 = 135,	// 135) Magic incoming damage ratio change
		BLOW_136 = 136,	// 136) Contagion state effect.
		BLOW_137 = 137,	// 137) Delays monster death when HP reaches 0 and triggers a specific skill
		BLOW_138 = 138,	// 138) Always evaluates CanHit as true inside CDnActor::IsHittable() regardless of CanHit status (however, invincible state effect #99 takes higher priority)
		BLOW_139 = 139,	// 139) MP cost ratio change state effect
		BLOW_140 = 140, // 140) Heal state effect. Restores target's HP by caster's <magic attack power> * <effect ratio>.
		BLOW_141 = 141, // 141) Heal ratio change state effect. When recovering HP via healing actions (potions, healing skills, etc.), increases heal amount by <effect ratio>.
		BLOW_142 = 142,	// 142) Super Armor attack power ratio change state effect. For <duration>, increases the affected target's attack power ratio against Super Armor 1 by <effect ratio>.
		BLOW_143 = 143,	// 143) Highlander state effect. While active, if the buffed player's HP drops to 0 or below, removes the "Highlander" buff, forces HP to 1, and prevents HP from dropping below 1 for <duration 2>.
		BLOW_144 = 144,	// 144) Frostbite state effect. Reduces "frame/move speed" by a fixed amount for the duration and also reduces "ice resistance".
		BLOW_146 = 146, // 146) Complete time-stop effect (sets frame rate to 0); takes highest priority among all frame-related effects (ignores other frame speed modifiers).
		BLOW_148 = 148, // 148) Reset aggro when this state effect is applied. Valid only if target is a monster actor. (#21673)
		BLOW_149 = 149, // 149) Ice Prison state effect (#22364)
		BLOW_150 = 150,	// 150) Grants immunity to all state effect indices except those listed in the semicolon-delimited parameter.
		BLOW_151 = 151,	// 151) Camera distortion effect
		BLOW_152 = 152,	// 152) Mouse sensitivity slowdown effect
		BLOW_153 = 153, // 153) Parrying state effect with cooldown #22735 
		BLOW_154 = 154, // 154) Invincible against specific skills (ignores Hit signals from skills with specified IDs)
		BLOW_155 = 155, // 155) Cure specific state effects
		BLOW_156 = 156, // 156) Unbreakable Super Armor (prefix-related extra state effect - defensive)
		BLOW_157 = 157, // 157) DOT effect (currently used for bleed; deals fixed damage at fixed intervals over a duration)
		BLOW_158 = 158, // 158) Apply damage one more time
		BLOW_159 = 159, // 159) Finish Attack (deals extra damage proportional to how far below the configured HP ratio the target is)
		BLOW_160 = 160, // 160) Append Hit (on successful attack, deals extra damage equal to caster's max physical damage)
		BLOW_161 = 161, // 161) Bone Crash (deals extra flat damage per hit for a set duration)
		BLOW_162 = 162, // 162) Breaking Point (sets Super Armor to 0 and makes the attack deal critical damage)
		BLOW_164 = 164, // 164) Judge's Power (increases flat magic attack power by @ per 1 STR)
		BLOW_165 = 165, // 165) (increases flat physical attack power by @ per 1 INT)
		BLOW_166 = 166, // 166) Judgment Hammer (increases damage based on target's status ailment)(target state effect ID, damage increase %)
		BLOW_167 = 167,	// 167) When summoning a monster, sets the level of all its owned skills to the level defined in this state effect.
		BLOW_168 = 168, // 168) Shock Transition (light-element attack on a shocked target triggers an extra AoE shock)
		BLOW_169 = 169, // 169) Critical Break (on critical hit, deals extra damage equal to % of the dealt damage)
		BLOW_170 = 170, // 170) Triggers the skill's assigned explosion damage after <set time> following a hit
		BLOW_171 = 171, // 171) Showtime (modifies cooldown of skills of target type: Active(0), Passive(1))
		BLOW_172 = 172, // 172) Explosion (attacking a target with <target status ailment> using <element> has <probability> to trigger <state effect>)
		BLOW_173 = 173, // 173) Ice Barrier (creates a shield equal to % of magic attack power; when it expires, freezes enemies within @m radius for @ sec)
		BLOW_174 = 174, // 174) Buff Steal (count;duration: removes up to count buffs, and applies state effects from skills not requiring a weapon to the caster for the configured duration)
		BLOW_175 = 175, // 175) Pushback (0: Knockback, 1: Push)
		BLOW_176 = 176, // 176) Used in Ghoul mode. Restricts skill usage to only the specified skillId.
		BLOW_177 = 177, // 177) Simple state blow used only in code to display additional damage
		BLOW_178 = 178, // 178) Holy Shield (triggers target state blow when hit)
		BLOW_179 = 179, // 179) Transitions to the action defined by the state blow on the first hit to a target after this state blow is applied. Hurricane Dance (automatically transitions to the next action when the attack in the skill's first action hits.)
		BLOW_180 = 180, // 180) Magical Breeze (changes the damage type of all attacks cast by the user to magic damage)
		BLOW_181 = 181, // 181) State blow used in Ghoul mode (Holy Water)
		BLOW_182 = 182, // 182) Prefix - state blow that changes attack element
		BLOW_183 = 183, // 183) State effect that prevents prefix state effects in Ghoul mode
		BLOW_184 = 184, // 184) State blow on the attacker. Multiplies the final damage by the given ratio for physical attacks (0.3 means +30%).
		BLOW_185 = 185, // 185) State blow on the attacker. Multiplies the final damage by the given ratio for magic attacks (0.3 means +30%).
	
		// #32220 Attack-power-modifying state effects applied to equipment-adjusted stats before calculating skill state effects used in cooking.
		BLOW_200 = 200, // 200) State blow modifying physical attack absolute value. Calculated before skill state blows.
		BLOW_201 = 201, // 201) State blow modifying physical attack ratio. Calculated before skill state blows.
		BLOW_202 = 202, // 202) State blow modifying magic attack absolute value. Calculated before skill state blows.
		BLOW_203 = 203, // 203) State blow modifying magic attack ratio. Calculated before skill state blows.
		//////////////////////////////////////////////////////////////////////////

		// New character skill state effect
		BLOW_204 = 204,	// 204) Push ??
		BLOW_205 = 205, // 205) Teleprt_Object( "ObjectID", moves to the actor position of the owned ObjectID.
		BLOW_206 = 206, // 206) ActionChangeRatio("prob;action", transitions to "action" with "prob" when hit
		BLOW_207 = 207, // 207) DamageTransition("ratio", while active, if an ally takes damage, self takes damage * ratio instead
		BLOW_208 = 208, // 208) Pingpong("bounceCount;range(cm);ratio", after first hit, attack bounces to enemies in range. If no target to bounce to, deals "remaining bounces * ratio * damage" extra damage
		BLOW_209 = 209, // 209) BasisAttackDmgUp("ratio", increases normal attack damage)
		//////////////////////////////////////////////////////////////////////////

		BLOW_210 = 210, // 210) #32729 Guild war skill: display opponent position on minimap in guild war
		BLOW_211 = 211, // 211) #33739 Archbishop Nest Hell state blow for removing buffs/debuffs
		BLOW_212 = 212, // 212) #34163 Apply state blow to enemies within a certain range for a certain interval (unused).
		BLOW_213 = 213, // 213) #34770 - Increases attack power by calculated attack power + (physical attack * ratio)
		BLOW_214 = 214, // 214) #34770 - Increases attack power by calculated attack power + (magic attack * ratio)
		BLOW_215 = 215, // 215) #32426 State blow integrated with OrderMySummonedMonster signal that commands my summoned monster to use a specific skill.

		BLOW_216 = 216, // 216) State blow for new character monster summon skill
		BLOW_217 = 217, // 217) Added "Ghoul Mode only" transform state blow
		BLOW_218 = 218, // 218) Improved Freezing Escape state blow (freezeProb;iceDurability;shake(0/1);hitAction(optional))
		BLOW_219 = 219, // 219) Random Holy Water effect used in Ghoul mode
		BLOW_220 = 220, // 220) Lotus Marsh - state blow used by monsters appearing at Meteor Crash Site
		BLOW_221 = 221, // 221) State blow that adds attack power matching physical/magic attack
		BLOW_222 = 222, // 222) Increases critical rate when attacking an enemy in Burn state
		BLOW_223 = 223, // 223) MP explosion feature
		BLOW_224 = 224, // 224) Disable specific action feature
		BLOW_225 = 225, // 223) [G-Dragon] Request to implement additional skill effect "Confusion" (input inversion effect)
		BLOW_226 = 226,	// 226) Chance to ignore specific skill, used by Dark Lair reward origin item. (Processed until end of skill at the determined probability upon hitting the first hit signal of that skill)
		BLOW_227 = 227, // 227) HP Drain. Recovers own HP by % of damage dealt to the target.
		BLOW_228 = 228, // 228) Ignore HitSignal: spawns an effect while nullifying HitSignals used by the target under this skill effect at a certain probability
		BLOW_229 = 229, // 229) Trigger specific skill effect at a certain probability (probabilistically applies specific effect every @ seconds during duration to the target with this skill effect)
		BLOW_230 = 230, // 230) Spectator state used in All-Kill mode 
		BLOW_231 = 231,	// 231) Continuous MP increase based on base MP
		BLOW_232 = 232, // 232) General transform state effect, used inside stages
		BLOW_233 = 233, // 233) #48831 PRE_ADD_KILL_BLOW - State effect that definitely kills the user.
		BLOW_234 = 234, // 234) #49070 [Skill Pet] Develop CRITICAL_AV skill effect (critical absolute value)
		BLOW_235 = 235,	// 235) #51048 State effect for freeze, frostbite, shock, and burn that applies only the effect like Nest bosses without action penalties.
		BLOW_236 = 236, // 236) #50903 Damage distribution state effect
		BLOW_237 = 237, // 237) #50907 Disarm state effect
		BLOW_238 = 238, // 238) #50917 Weapon swap state effect.
		BLOW_239 = 239, // 239) #50923 Invincibility feature, invincibility by Type
		BLOW_240 = 240, // 240) #50923 Ignore invincibility feature, ignore invincibility by Type
		BLOW_241 = 241, // 241) #51081 Flashbang
		BLOW_242 = 242, // 242) #52332 [Skill Revamp] Implement effect that reduces damage after a certain number of HITs
		BLOW_243 = 243, // 243) #52499 Kali new state effect Mana Shield
		BLOW_244 = 244, // 244) #52501 [Kali] Add "Curse" skill effect
		BLOW_245 = 245, // 245) #52509 [Kali] Add "Curse Break" skill effect
		BLOW_246 = 246, // 246) #52808 [Kali] Implement "Stigma" skill effect
		BLOW_247 = 247, // 247) #53454 [Kali] Implement "Puppet" skill effect
		BLOW_248 = 248, // 248) #52809 [Kali] Implement "Heal on Attack" skill effect
		BLOW_249 = 249, // 249) #53448 [Kali] Implement "Additional Action on Stack" skill effect
		BLOW_250 = 250, // 250) #52906 [Kali] Implement "Add Target Critical Chance" skill effect
		BLOW_251 = 251, // 251) #53492 [Skill Revamp] Final critical chance % increase effect
		BLOW_252 = 252, // 252) #53722 [Late Lotus Marsh] Develop skill effect that grants immunity to specific skills
		BLOW_253 = 253, // 253) #52905 [Kali] Implement "Stack Additional Effect" skill effect
		BLOW_254 = 254, // 254) #60235 [Desert Dragon] Display StateEffect on aggro target

		//Unified level skill function state effects..
		BLOW_255 = 255, // 255) //1. Increase bonus hunting EXP by n%
		BLOW_256 = 256, // 255)//2. Increase pet EXP by n%
		BLOW_257 = 257, // 255)//3. Increase character move speed in town by n% ----> Unused..
		BLOW_258 = 258, // 255)//4. Increase mount move speed by n% ----> Unused
		BLOW_259 = 259, // 255)//5. Changed to reduce durability loss on character death
		BLOW_260 = 260, // 255)//6. Increase favorability by 10% when gifting to NPC
		BLOW_261 = 261, // 255)//7. Gain n extra Colosseum medals
		BLOW_262 = 262, // 255)//8. Reduce farm cultivation time by n%
		BLOW_263 = 263, // 255)//9. 1% discount on Ladder Shop items
		BLOW_264 = 264, // 255)//10. Reduce auto-fishing time by n seconds
		BLOW_265 = 265, // 255)//11. Increase cooking proficiency by n%
		BLOW_266 = 266, // 255)//12. Increase gold acquisition by n%
		//////////////////////////////////////////////////////////////////////////

		BLOW_267 = 267, // 267) Unified level skill offense state effect (applies state effect 251 (+n% critical) for n sec to attacker on critical hit, n sec cooldown)
		BLOW_268 = 268, // 268) Unified level skill defense state effect (increases damage reduction by n% when HP is below n%)
		BLOW_269 = 269, // 269) Unified level skill - reduce ultimate skill cooldown
		BLOW_270 = 270, // 270) Returnee system - increase party member drop rate
		BLOW_271 = 271, // 271) State effect that increases only final value based on character base stats
		BLOW_272 = 272, // 272) Increases HP restored by items while this state effect is active.
		BLOW_273 = 273, // 273) Ignores Gravity signal during duration.
		BLOW_274 = 274, // 274) #71659 [Desert Dragon] Skill effect - develop max damage cap effect
		BLOW_275 = 275, // 275) #71663 Ignore HP recovery except specific methods while state effect is active
		BLOW_276 = 276, // 276) #75202 [Skill] Implement skill (buff) effect applied only when performing specific skills
		BLOW_277 = 277, // 277) #75694 [70LV] ObjectVisualize development request 
		BLOW_278 = 278, // 278) #77123 [Skill] Create party effect for Finish Attack
		BLOW_279 = 279, // 279) #77123 [Skill] Create party effect for Bone Crush
		BLOW_280 = 280, // 280) #79363 Create Dragon Buff effect

		// Assassin-related state effect
		BLOW_281 = 281, // 281) #84056 Trigger specified skill the moment HP drops below a certain value

		// Post-Cap 60 / Steam NA v353 combat state effects
		BLOW_282 = 282, // 282) Suffer Force Critical Damage (CDnSufferForceCirticalDamage)
		BLOW_283 = 283, // 283) Sub-Skill On Target Hit (CDnSubSkillOnTargetHitBlow)
		BLOW_284 = 284, // 284) Time Goes Growing (CDnTimeGoesGrowingBlow)
		BLOW_285 = 285, // 285) Do When Skill Begin (CDnDoWhenSkillBeginBlow)
		BLOW_286 = 286, // 286) Check Immune Be Damage (CDnCheckImmuneBeDamage)
		BLOW_287 = 287, // 287) Monster Order (CDnMonsterOrderBlow)
		BLOW_288 = 288, // 288) Add State On Target Hit (CDnAddStateOnTargetHitBlow)
		BLOW_289 = 289, // 289) Add State On Skill Begin (CDnAddStateOnSkillBeginBlow)
		BLOW_290 = 290, // 290) Add State On Summon Monster (CDnAddStateOnSummonMonsterBlow)
		BLOW_291 = 291, // 291) Decrease Cooltime (CDnDecreaseCooltimeBlow)
		BLOW_292 = 292, // 292) Add Weapon Based Damage (CDnAddWeaponBasedDamageBlow)
		BLOW_293 = 293, // 293) Add Damage By Target Distance (CDnAddDamageByTargetDistanceBlow)
		BLOW_294 = 294, // 294) Add Damage By Weight (CDnAddDamageByWeightBlow)
		BLOW_295 = 295, // 295) Add State On Break Super Armor (CDnAddStateOnBreakSuperArmorBlow)
		BLOW_296 = 296, // 296) Add State On Appointed Skill (CDnAddStateOnAppointedSkillBlow)
		BLOW_297 = 297, // 297) Add State On Appointed Skill Hit (CDnAddStateOnAppointedSkillHitBlow)
		BLOW_298 = 298, // 298) Invisible Effect / Skin Swap (CDnInvisibleEffectBlow)
		BLOW_299 = 299, // 299) Damage Transition To Summon Monster (CDnDamageTransitionToSummonMonsterBlow)
		BLOW_300 = 300, // 300) Add State On Block (CDnAddStateOnBlockBlow)
		BLOW_301 = 301, // 301) Gauge Shield (CDnGaugeShieldBlow)
		BLOW_303 = 303, // 303) Duplicate Explosion (CDnDuplicateExplosionBlow)
		BLOW_304 = 304, // 304) Increase Cooltime Charging (CDnIncreaseCooltimeChargingBlow)
		BLOW_305 = 305, // 305) Fear (CDnFearBlow)
		BLOW_306 = 306, // 306) All Element Defense (BasicBlow)
		BLOW_307 = 307, // 307) All Element Attack (BasicBlow)
		BLOW_309 = 309, // 309) Physical + Magical Attack Absolute (BasicBlow)
		BLOW_310 = 310, // 310) Physical + Magical Attack Ratio (BasicBlow)
		BLOW_313 = 313, // 313) Final Damage Absolute (BasicBlow)
		BLOW_314 = 314, // 314) Decrease Super Armor (CDnDecreaseSuperArmorBlow)

		BLOW_MAX,
	};

	enum emBLOW_STATE
	{
		STATE_NONE,
		STATE_BEGIN,
		STATE_DURATION,
		STATE_END,
	};

	emBLOW_INDEX emBlowIndex;
	emBLOW_STATE emBlowState;

	std::string szValue;	// [2010/12/27 semozz] Changed to string to remove string length limit
	
	float fDurationTime;
	float fDelayTime;

	STATE_BLOW()
		: emBlowIndex(BLOW_NONE)
		, emBlowState(STATE_NONE)
		, fDurationTime(0.0f)
		, fDelayTime(0.0f)
	{

		
	}
};
