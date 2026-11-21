
#pragma once

///############################################################################################################################
// Patch Complete Define
//#########################################################################################################################################


//#########################################################################################################################################
// Patch Define
//#########################################################################################################################################

#define PRE_LEVELUPREWARD_DIRECT					// saset 2013-02-28 // #83918 [Main] Level-up reward improvement - put reward directly into cash inventory

#define PRE_ADD_ADDITIOANL_SKILL_ACTION             // Rotha 2013-02-19 // #79082 [Skill] Add management for chainable actions in PlayAniProcess 
#define PRE_ADD_SKILLSLOT_EXTEND					// verygoodd21,karl 2013-01-30 // #80813,#80811 Skill slot expansion
#define PRE_FIX_PARTSMONSTER_AI_TARGETTING          // Rotha 2013-02-28 // #77819 Change criteria for parts monster AI and target handling.
#define PRE_ADD_RETURN_VILLAGE_SHOWNAME				// elkain03, karl 2012-12-31 // #77189 Improve popup upon stage clear to show destination town where user returns
#define PRE_ADD_ITEM_GAINTABLE						// kalliste 2013-02-06 // #81396 [Client] Separate UI string for item acquisition source

#define PRE_ADD_DWC // anno414, devkang, stupidfox 2013-01-08 // #78018 DWC Colosseum tournament work
#define PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG // Post-Cap60 Steam NA v353 quest texture dialog Lua APIs & packets


#define PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME				// karl 2013-02-18 // #82707 [DN] Request to add weekly event mission end date
#define PRE_FIX_83091                               // Rotha 2013-02-20 // #83091 Remove stance change state effect when afflicted with abnormal status from state effect
#define PRE_FIX_FRIEND_SORT							// kalliste 2013-02-22 // #83578 Request to change friend list sort order to alphabetical
#define PRE_ADD_DECREASE_EFFECT						// kwcsc 2013-01-15 // #78795 Display "Effect Reduced" when effect is reduced due to monster resistance

#define PRE_ADD_CASHSHOP_RENEWAL					// bintitle  2013-02-06      // #79054 Cash Shop improvements.


//#if defined(PRE_ADD_CASHSHOP_RENEWAL) && defined(_WORK)
//#define PRE_ADD_CASHSHOPBANNER_RENEWAL // bintitle 2013-02-06 // #79339 Cash Shop improvement - town banner rolling feature.
//#endif // PRE_ADD_CASHSHOP_RENEWAL
#define PRE_FIX_81750                               // Rotha 2013-02-06 // #81750 [Skill] Error when skill is canceled by hit while maintaining stance-type skill.
#define PRE_ADD_START_POPUP_QUEUE					// kalliste 2013-02-07 // #81879 Request to change order of event popup and stamp popup
#define PRE_MOD_SKILLCOOLTIME						// kalliste 2013-01-24 // #81845 Change skill cooldown seconds color
#define PRE_ADD_GUILD_CONTRIBUTION					// elkain03, karl 2012-01-07 // #77634, #77632 Guild contribution system
#define PRE_ADD_ENABLE_EFFECT_SIGNAL				// elkain03 2012-07-18 // #62455 Party member particle effect ON / OFF feature
#define PRE_ADD_GUILDREWARDITEM						// dyss   2012-11-13  // #73578 Unlock guild level and add rewards
#define PRE_DRAGONBUFF								// dyss   2013-01-08  // #75876 Dragon Buff
#define PRE_ADD_MISSION_COUPON						// stupidfox 2012-10-29 // #68801 Grant discount coupon as mission reward

//#define PRE_ADD_EQUIPLOCK // kalliste, karl 2013-01-21 // #76027 Equipment lock system for enhanced equipment security

#define PRE_ADD_CHARACTERCHECKSUM					// devkang  2013-01-25  // #80315 Check for arbitrary modification of character info DB 
#define PRE_MOD_QUEST_COUNTING						// devkang  2013-01-23  // #80165 Fix quest counting to process only up to the specified target count	
#define PRE_MOD_PVP_ROUNDMODE_PENALTY				// devkang  2013-02-05  // #80756 Grant penalty based on win count of winning team in PVP round mode
#define PRE_ADD_DOORS_SELFKICK						// 2hogi 2013-02-07 // #78286 Task #76376: DN App 0.8 abnormal disconnect handling
#define PRE_ADD_STAGE_USECOUNT_ITEM					// dyss   2013-02-12  // #82099 Request handling usage count for Desert Dragon exclusive food (stage item usage count limit)


#define PRE_MOD_DUPLICATION_LOGIN_MES				// karl 2013-01-10 // #78491 Request to check message on same-account login
#define PRE_ADD_LEVELUP_CASHGIFT_NOTIFY				// karl 2012-11-22 // #72716 [Main][All Regions] Display system message when obtaining a box from level-up reward box (Box in Box)
#define PRE_ADD_SALE_COUPON_CATEGORY				// kwcsc 2013-01-22 // #80207 Open up to designated subcategory when clicking coupon item

#define PRE_ADD_SKILLCOOLTIME						// kalliste 2012-12-18 // #76255 Development request for displaying skill cooldown in seconds
#define PRE_ADD_MONTHLY_MISSION						// elkain03, karl 2012-12-11 // #76067, #76066 Monthly mission system
#define PRE_ADD_ITEM_ENCHANT_TOOLTIP				// hayannal2009 2012-12-13 // #76093 Add tooltip related to Item Protection Jelly.
#define PRE_ADD_LADDER_TAB_TOOLTIP                  // Rotha        2012-11-20      // #73279 Show entry requirements in tooltip on mouseover of Ladder tab

#define PRE_MOD_PARTY_CREATE_SORT					// kalliste 2013-01-02 // #77508 [Dragon Expedition] Dragon Expedition missing in party creation.
#define PRE_ADD_PVP_EXPUP_ITEM						// hayannal2009 2012-12-03 // #75243 Colosseum EXP boost item
#define PRE_ADD_DRAGON_FELLOWSHIP_GLYPH				// haling, verygoodd21 2012-12-03 // #74079 [Main] Add heraldry slot to equip unique heraldries rewarded from Dragon Expedition
#define PRE_ADD_CP_RANK								// haling, verygoodd21 2012-12-14 // #73387 [Main] CP 2nd improvement
#define PRE_ADD_DRAGON_FELLOWSHIP					// kwcsc, karl 2012-11-20 // #74091 [Dragon Expedition] Add Dragon Expedition map subtype (11) (#75660 server)
#define PRE_RECEIVEGIFTALL							// saset, anno414 2012-11-14 // #63242 [Main] Receive all from gift box
#define PRE_MOD_PVP_ROOM_CREATE						// anno414  2012-11-13  // #69125 [Main] Create Colosseum rooms centered on mode
#define PRE_FIX_76282								// devkang  2012-12-18  // #76282 Change handling when forfeiting and leaving mid-PVP
#define PRE_ADD_CHAR_STATUS_SECOND_RENEW            // Rotha        2012-12-06      // #60650 Status window improvement phase 2
#define PRE_ADD_STATUS_BUFF_INFORMATION             // Rotha        2012-05-30      // #57461 Request to display buff stats separately in status window
#define PRE_FIX_CHARSTATUS_REFRESH                  // Rotha        2012-06-13      // CharStatus structure improvement.
#define PRE_ADD_BASIC_STATE_LIMIT                   // Rotha        2012-08-22      // #66173 [Balance, Combat Balance] Min/max cap on stat buffs.
#define PRE_ADD_ELEMENT_DEFENSE_LIMIT               // Rotha        2012-09-21      // #67654 Issue where attacks of an element deal no damage when elemental defense reaches 100%
#define PRE_ADD_SLOW_DEBUFF_LIMIT					// kwcsc 2012-11-15 // #67997 Limit minimum factor for slow-related debuffs
#define PRE_ADD_ALTHEIA_NEWCAPTION_TRIGGER			// anno414  2012-10-30  // #71731 [Althea Continent Exploration] Add blind caption with NPC image
#define PRE_ADD_RENEW_RANDOM_MAP                    // Rotha        2012-11-26      // #74693 [Dragon Expedition] Convert random map link table setting to map ID
#define PRE_FIX_74404								// dyss   2012-11-27  // #74404 Change shop purchase-limited items to be handled per shop
#define PRE_ADD_STAMPSYSTEM							// devkang  2012-11-16  // #73767 Stamp system

#define PRE_ADD_PROJECTILE_SE_INFO                  // Rotha        2012-11-19      // #72402 Refactored to store state effect info at the time projectile is fired.
#define PRE_MOD_GLOBAL_SKILLICON					// verygoodd21 2012-12-17 // #76703 Change display criteria for "Learnable Icon"
#define PRE_ADD_AI_NOAGGRO_STAND					// kalliste 2012-12-13 // #76297 [Desert Dragon] Feature to prevent monsters from wandering when they have no aggro
#define PRE_ADD_MISSION_HELPER_AUTO_REGISTER_LEVEL	// hayannal2009 2012-11-22 // #74479 Mission helper auto-registration level limit.
#define PRE_ADD_JOBCHANGE_RESTRICT_JOB				// kalliste 2012-11-14 // #72833 [Dancer] Block switching to Dancer via job change ticket
#define PRE_ADD_ACTIVEMISSION						// bintitle. 2012.09.24  // #69363 Active Mission - Client, #69362 Server - stupidfox
#define PRE_MOD_ITEM_COMPOUND_DIALOG_RENEWAL		// elkain03 2012-10-11 // #69922 Crafting window UI renewal
#define PRE_MOD_ALWAYS_SHOW_DROP_ITEM				// elkain03 2012-11-21 // #74000 [QoL] Request to always show dropped item names
#define PRE_MOD_APPELLATIONBOOK_RENEWAL				// anno414, karl 2012-09-17  // #64745, #69504 [Main] Improve Collection Title system features and request additional collection titles
#define PRE_MOD_ALWAY_SEE_SELL_PRICE_TOOLTIP		// anno414  2012-11-08  // #73192 Develop feature to always show shop sell price in item tooltip
#define PRE_FIX_67546								// dyss   2012-09-07  // #67546 DB middleware thread distribution work

#define PRE_WORLDCOMBINE_PVP						// dyss   2012-09-18  // #55446 Cross-world integrated battlefield PVP

#define PRE_ADD_PART_SWAP_RESTRICT_TRIGGER			// anno414  2012-12-05  // #75545 [Trigger] Develop trigger to toggle allowing equipment swap On/Off

#define PRE_MOD_DARKLAIR_RECONNECT					// elkain03 2012-10-22 // #71970 Fix Dark Lair reconnected users not seeing top-right round and all rounds showing as Final Round
#define PRE_MOD_ITEM_COMPOUND_TOOLTIP				// elkain03 2012-11-31 // #75170 'Cube of Suffix Removal' tooltip incorrectly shows (Potential)
#define PRE_ADD_MONSTER_PARTS_UI_TRIGGER			// elkain03 2012-10-04 // #70240 Request trigger to display monster part HP on bottom screen gauge
#define PRE_DETAILCLEARRANK							// saset 2012-11-13 // #73745 Subdivide stage clear reward box ranks
#define PRE_ADD_FORCE_RIDE_ENABLE_TRIGGER			// verygoodd21 2012-08-17 // #66097 [Desert Dragon] Trigger - Create mountable/unmountable toggle trigger
#define PRE_MOD_LOCK_TARGET_LOOK					// verygoodd21 2012-08-14 // #65845 [Desert Dragon] Please add LockTargetLook feature.
#define PRE_ADD_CP_RENEWAL							// verygoodd21 2012-11-14 // #73287 CP renewal development
#define PRE_MOD_POTENTIAL_JEWEL_RENEWAL				// anno414, karl 2012-10-12  // #71351, #71757 Improve potential granting feature
#define PRE_ADD_PVP_COMBOEXERCISE					// devkang  2012-10-10  // #69059 Add Colosseum combo practice mode
#define PRE_FIX_73833                               // Rotha        2012-11-28      // #73833 [Desert Dragon] Boss monsters get pushed back by Academic Force Out
#define PRE_ADD_71342                               // Rotha        2012-10-22      // #71342 Colosseum - Display captain position in Protect the Captain mode_Client
#define PRE_ADD_PVP_SHOW_PARTY_GAUGE                // Rotha        2012-10-26      // #71579 Display ally HP in Colosseum team match_Client
#define PRE_ADD_STAGE_LIMIT_INTERFACE               // Rotha        2012-11-02      // #72747 Combat Balance_Attack/Damage/Healing max cap_UI
#define PRE_ADD_STAGE_DAMAGE_LIMIT                  // Rotha        2012-09-10      // #67194 [Combat Balance, Max Damage Cap] Detailed spec for max damage cap.
#define PRE_ADD_MONSTER_NORMAL_ACTION_PRIORITY      // Rotha        2012-08-29      // #67720 Set priority on A.I basic actions
#define PRE_FIX_INVINCIBLE_AT_BLOW_SEND_RESIST      // Rotha        2012-06-29      // #61324 (Additional SKILL_DEFENSE effect dev) Expand skill restriction effect scope and set Resist-displayable parameter.
#define PRE_ADD_TRIGGER_BY_HIT_CONDITION            // Rotha        2012-09-27      // #70206 [Desert Dragon] Feature to fire TriggerEvent when Hit signal successfully hits a specific target
#define PRE_ADD_MAX_DAMAGE_LIMIT_BLOW               // Rotha        2012-10-17      // #71659 Desert Dragon max damage limit state effect
#define PRE_FIX_67656                               // Rotha        2012-08-30      // #67656 Configure to apply damage to parts damage as well when dealing damage via state effect.
#define PRE_FIX_66687								// semozz 2012-08-20 // #66687 [Desert Dragon] Develop skill effect converting part HP to percentage
#define PRE_ADD_57090								// semozz 2012-04-30 // #57090 [Desert Dragon] Feature to prevent a specific target from being attacked
#define PRE_ADD_IGNORE_RESTORE_HP					// kwcsc 2012-10-17 // #71663 [Desert Dragon] Add heal immunity state effect
#define PRE_FIX_63315								// bintitle     2012-07.27      // #63315 [Desert Dragon] Issue where CanHit signal False state is ignored.
#define PRE_ADD_MARK_PROJECTILE						// kwcsc 2012-08-16 // #65889 Add Desert Dragon projectile landing indicator action and signal
#define PRE_ADD_DAMAGEDPARTS						// bintitle  2012-07-11  // #60073 [Desert Dragon] Implement additional feature for monster parts.
#define PRE_ADD_DESERTDRAGON						// robust, bintitle 2012-05-22 // #57365 [Desert Dragon] Add trigger to check current weather (ENV)
#define PRE_MOD_AIMULTITARGET						// robust 2012-02-15 // #51072 Fix AI multi-target handling (aggro->random, whether to handle summons) : Must go in with Desert Dragon patch!!
#define PRE_FIX_67719								// dyss   2012-08-27  // #67719 [Desert Dragon] Also apply to random target skills
#define PRE_FIX_68096								// dyss   2012-08-30  // #68096 [Desert Dragon] Use multiple globalcooltime simultaneously in AI
#define PRE_ADD_SERVER_WAREHOUSE					// haling,kalliste 2012-10-10 // #69258 [Main] Server Storage


#define PRE_ADD_PRIVATECHAT_CHANNEL					// dyss   2012-10-16  // ##70610 Private channel improvements

#define PRE_ADD_MAILBOX_OPEN						// semozz 2012-07-12 // #58230 [Main] Allow opening Mailbox UI anywhere in town
#define PRE_ADD_INTEGERATE_EVENTUI					// semozz 2012-07-12 // #58230 [Main] Allow opening Mailbox UI anywhere in town - integrate Event window
#define PRE_ADD_INTEGERATE_QUEST_REPUT				// semozz 2012-07-12 // #58230 [Main] Allow opening Mailbox UI anywhere in town - integrate Quest/Affinity window
#define PRE_FIX_SYNC_ENCHANT_SKILL					// kalliste 2012-11-05 // #72640 [Skill] Electrocution count icon for Inquisitor skill 'Mind Breaker EX' not shown in Colosseum
#define PRE_ADD_PVP_TOURNAMENT_WINNERXP				// kalliste 2012-10-17 // #71205 [UI] Total Pts and Colosseum EXP for top 3 not displayed on Tournament mode reward screen (scheduled for 11/22 KR)
#define PRE_ADD_COMPARETOOLTIP						// bintitle. 2012-09-18         // #69087 [Main] Fix issue where UI goes off-screen when comparing accessory ring item stats.

#define PRE_ADD_EXSKILL_TOOLTIP						// kwcsc 2012-10-25 // #72315 Change tooltip only for EX 2nd skills (hardcoded via SlotIndex since there is no way to distinguish)
#define PRE_ADD_REPUTATION_EXPOSURE					// anno414, devkang 2012-09-11 // #48214 [Main] Affinity Collection enhancement
#define PRE_ADD_PVP_HELP_MESSAGE					// anno414 2012-09-06 // #66948 [Main] Colosseum guide system (help and additional features)
#define PRE_MOD_OPTION_EFFECT_QUALITY				// anno414 2012-07-05 // #61805 Separate out effect adjustment option
#define PRE_MOD_PVP_LADDER_XP						// stupidfox 2012-10-11 // #69122 Grant Colosseum EXP during Colosseum Ladder matches
#define PRE_ADD_NEST_DIFFICULTY_TRANSFORM_FORCING	// elkain03 2012-09-27 // #69577 [Nest60Lv] Difficulty UI string conversion feature at top right
#define PRE_ADD_2vs2_LADDER                         // Rotha        2012-10-09      // #70691 [Client] Additional work for Colosseum Ladder 2:2 mode
#define PRE_FIX_PIERCE_WITH_HIT_AREA                // Rotha        2012-10-31      // #72551 Fixed pierce not applying when projectile hit is processed via hit area.
#define PRE_MOD_71820								// devkang  2012-11-06  // #71820 Dimension Key not granted to guild master when guild member reaches max level


#define PRE_ADD_SMARTMOVE_PACKAGEBOX				// kalliste 2012-10-18 // #71852 [Halloween] Request to support smart move to confirmation window when opening Halloween Special Pandora's Box
#define PRE_ADD_SHORTCUT_HELP_DIALOG				// anno414 2012-08-30 // #62824 In-game system UI help link system
#define PRE_MOD_SHOW_CHAR_INFO_IN_VILLAGE			// elkain03 2012-10-18 // #71777 Fix inability to inspect party member info via portrait in town
#define	PRE_ADD_MISSION_NEST_TAB					// elkain03 2012-09-04 // #67924 Add Nest tab in Mission UI 	
#define PRE_ADD_QUESTCOUNT_IN_STAGELIST				// elkain03 2012-09-20 // #68877 Add quest count next to stage name displayed on map 
#define PRE_ADD_LOWLEVEL_QUEST_HIDE					// elkain03 2012-09-18 // #68061 Show low-level quest marker when level gap exceeds threshold during NPC quest dialog 	
#define PRE_ADD_PREDICTIVE_TEXT						// elkain03 2012-07-04 // #61563 Target stage string autocomplete via key input in party creation window  



#define PRE_MOD_68531_NEW							// verygoodd21 2012-09-24 // #68531 Issue where skill level cannot be raised to MAX after equipping Technique/costume (added define per fix request)
#define PRE_FIX_PETALGIFTCARD						// saset 2012-10-02 // #70497 Modify source for Petal gift card
#define PRE_ADD_REMOTE_QUEST						// stupidfox 2012-08-23 // #65255 Remote quest system
#define PRE_ADD_JOINGUILD_SUPPORT					// 2hogi 2012-09-07 // #68260 Task #68259: Normal guild application system
#define PRE_ADD_BEGINNERGUILD						// 2hogi 2012-06-11 // #59431 Beginner Guild System_DB, SERVER
#define PRE_FIX_68828								// haling, kalliste 2012-09-18 // #68828 Print system message when picking up item from treasure box [Client]



#define PRE_ADD_68286								// semozz 2012-09-12 // #68286 [Main] Execute action when clicking mail/gift box icon
#define PRE_ADD_DARKLAIR_HEAL_REGULATION            // Rotha        2012-09-19      // #69663 Challenge-type Dark Lair adjustment / added heal adjustment.
#define PRE_FIX_DISSOLVE_SKILL_FAILED               // Rotha        2012-09-21      // #53146 Changed skill failure handling for dispel-type state effects like Relieve.
#define PRE_FIX_69709								// haling 2012-09-20 // #69709 Disassembly restriction feature at specific enhancement levels (related to #41409)
#define PRE_FIX_QUEST_ITEM_ONCOUNT					// haling 2012-09-19 // #69723 Fix looping by item count when checking quest items.
#define PRE_ADD_CHALLENGE_DARKLAIR					// verygoodd21 2012-08-16 // #64675 Challenge-type Dark Lair development
#define PRE_ADD_EQUIPED_ITEM_ENCHANT				// stupidfox 2012-08-20 // #66349 Enhance equipped gear 

#define PRE_ADD_GETITEM_WITH_RANDOMVALUE 			// elkain03, 2hogi 2012-08-03 // #64713 Implement acquiring random count of items from random box
#define PRE_FIX_BUFFITEM							// dyss   2012-08-21  // #63228 Improve elixirs - modify named items related to elixir development

#define PRE_ADD_CADGE_CASH							// kwcsc, saset 2012-06-11 // #57611, #57612 Add cash item request (begging) feature.

#define PRE_SPECIALBOX								// saset, Rotha 2012-08-29 // #63017, #63016 special storage
#define PRE_ADD_DIRECT_BUY_UPGRADEITEM				// anno414, saset  2012-08-14  // #65639 Remote purchase of enhancement materials via Trading House from enhancement window
#define PRE_ADD_CASH_COSTUME_AURA					// anno414 2012-07-26 // Create cash costume aura effect


#define PRE_ADD_64990								// semozz 2012-08-29 // #64990 [Skill] Distinguish PvE and PvP in AI scripts
#define PRE_FIX_68898								// semozz 2012-09-11 // #68898 [EX Skill] Inferno EX End action plays twice (add Input signal feature)
#define PRE_FIX_64312								// semozz 2012-07-31 // #64312 [EX Skill] Implement summon EX skill support
#define PRE_FIX_68645								// semozz 2012-09-10 // #68645 [EX Skill] Sacred Hammering EX bug.
#if !defined(PRE_MOD_68531_NEW)
#define PRE_MOD_68531								// verygoodd21 2012-09-05 // #68531 Issue where skill level cannot be raised to MAX after equipping Technique/costume
#endif	// #if !defined(PRE_MOD_68531_NEW)
#define PRE_ADD_SKILL_RESET_COMMENT					// verygoodd21 2012-07-31 // #63994 Notice for free low-level skill reset distribution - client
#define PRE_ADD_ONLY_SKILLBOOK						// verygoodd21 2012-08-27 // #67454 Develop system to learn skills only via skill books
#define PRE_ADD_PRESET_SKILLTREE					// verygoodd21 2012-08-25 // #66961 Provide skill window presets
#define PRE_ADD_PVP_RANKING							// stupidfox 2012-08-02 // #62216 Ranking system - PVP ranking
#define PRE_MOD_MISSION_HELPER						// stupidfox 2012-07-27 // #63227 Improve mission helper 
#define PRE_FIX_62281								// stupidfox 2012-07-12 // Party renewal. Update party invite failure conditions
#define PRE_ADD_QUEST_SORT_BY_JOURNAL_MAP           // Rotha        2012-06-28      // #60990 Configure quest journal to sort by the map it points to.
#define PRE_ADD_QUEST_BACK_DIALOG_BUTTON			// anno414 2012-07-09 // #62032 Add back button feature to NPC dialog screen
#define PRE_ADD_TOTAL_LEVEL_SKILL					// semozz 2012-08-01 // #64098 [Client] Unified level system work
#define PRE_PRIVATECHAT_CHANNEL						// dyss, kwcsc 2012-07-16  // #59031, #62366  Add private chat channel work
#define PRE_ADD_REPAIR_NPC                          // Rotha, haling 2012-07-16 // #62618, #62617 Place world zone repair NPC
#define PRE_ADD_NONE_EFFECT_APPELLATION_NOTIFY		// anno414 2012-08-17 // #66997 Add 'No Stats' text for cover title - client
#define PRE_FIX_PROTRAIT_FOR_GHOST                  // Rotha        2012-01-11      // #48625 Fixed portrait not updating when in ghost state.
#define PRE_ADD_PVP_TOURNAMENT						// haling,kalliste 2012-08-06 // #64890, #64891 Develop Colosseum Tournament mode



#define PRE_MOD_ITEMUPGRADE_COOLTIME				// kalliste 2012-07-13 // #62593 Reduce wait time during enhancement - client
#define PRE_ADD_ITEMCAT_TOOLTIP_INFO				// kalliste 2012-07-30 // #63358 [Main] Show enhancement/potential/suffix tooltips more clearly
#define PRE_ADD_INSTANT_CASH_BUY					// kwcsc, saset 2012-05-14 // #56200, #56201 Add cash item quick purchase. #63311 [Main] Improve discount coupon recognition, #64475 [Main] Allow using pet horn directly by clicking pet
#define PRE_ADD_PET_EXTEND_PERIOD					// kwcsc 2012-08-08 // #64485 Add pet horn usage feature
#define PRE_ADD_DIRECTNBUFF							//  dyss,Rotha 2012-08-06  // #64968 Returning player reward item buff work
#define PRE_ADD_SKILL_LEVELUP_RESERVATION           // haling,verygoodd21 2012-07-31 // #63234 Skill window improvement 1st - Apply process after skill setup
#define PRE_FIX_63975                               // Rotha        2012-07-30      // #63975 Remove billboard when PC cafe reward is claimed (minimap character)
#define PRE_ADD_VIEW_OPITION_PARTY_HEAL             // Rotha        2012-07-13      // #51827 Added option to display party members' heal info.



#define PRE_ADD_PARTSITEM_TOOLTIP_INFO				// kalliste 2012-07-30 // #63423 [Main] Add suffix/enhancement/potential info to equipment items
#define PRE_FIX_COLOCHAT_ICON						// kalliste 2012-07-31 // #55511 Chat tab iconization and whisper tab addition_UI / #63969 Issue where characters are appended to chat tab name when entering Colosseum game
#define PRE_ADD_63603								// semozz 2012-07-24 // #63603 [Main] Show per-item interaction info - client work
#define PRE_FIX_63375								// semozz 2012-07-20 // #63375 [Treasure Stage] HP/MP fails to restore to pre-adjusted values when returning to stage entrance after stage play
#define PRE_MOD_RC_MODE_OBSERVER_BE_NOT_OPEN_PETDLG // elkian03 2012-07-13 // #62470 Fix pet dialog forcibly opening when entering Colosseum as observer  
#define PRE_MOD_PARTY_TOGGLE_OPT					// elkain03 2012-07-12 // #62319 Fix "Etc" toggle button bug when creating a party 
#define PER_ADD_PROP_TRAP_DIFFICULT_SET				// anno414 2012-06-29 // #61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#define PRE_ADD_SUBTYPE_TREASURESTAGE				// verygoodd21 2012-07-13 // #62775 Request for map subtype
#if defined(PRE_ADD_SUBTYPE_TREASURESTAGE)
#define PRE_ADD_TSCLEARCOUNTEX						// saset 2012-07-24 // #63549 Develop Treasure Stage entry count expansion
#endif	// #if defined(PRE_ADD_SUBTYPE_TREASURESTAGE)
#define PRE_FIX_ESCAPE                              // Rotha        2012-02-27      // #52349 Escape feature modification request
#define PRE_FIX_ELEMENT_REGULATION_RENEW            // Rotha        2012-07-12      // #60463 [Skill, Element] Rework elemental attack/defense formula to fit existing structure.
#define PRE_SHARE_MAP_CLEARCOUNT					// stupidfox 2012-07-02 // #61150 Share nest map clear count
#define PRE_ADD_62072								// stupidfox 2012-07-10 // #62072 Add stage clear count info trigger API
#define PRE_ADD_REVENGE								// stupidfox 2012-07-17 // #62999 Colosseum Revenge 
#define PRE_ADD_PVPLEVEL_MISSION					// stupidfox 2012-08-01 // #64706 Add PVP rank achievement reward mission
#define PRE_ADD_EXPUP_ITEM							// haling 2012-06-20 // #60218 Develop consumable bonus EXP item [Server] (adds flat EXP amount)
#define PRE_ADD_RACING_MODE							// haling, verygoodd21 2012-05-08 // #55504 [Main] Create Colosseum Wonderful Racing mode
#define PRE_ADD_DUNGEONCLEARINFO					// dyss   2012-01-03  // #48575 Move from DungeonClearInfo user to game room
#define PRE_FIX_49129								// dyss   2012-01-03  // #49129 Extend api_trigger_GetPartyActor functionality
#define PRE_EXPANDGATE								// dyss   2012-06-18  // #60097 Request to increase setting gate count (10 -> 15)
#define PRE_NORMALSTAGE_REGULATION					// dyss   2012-06-18  // #60092 Request feature to apply equalization to normal stages
#define PRE_ADD_LIMITED_SHOP						// dyss   2012-07-17  // #61850 Develop limited-quantity item shop (daily, weekly reset)
#define PRE_WORLDCOMBINE_PARTY						// kimbap 2012-04-26 // #56555 Develop cross-world integrated party room
#define PRE_FIX_WORLDCOMBINEPARTY					// dyss   2012-09-12  // Pin cross-world party room to a single GameServer
#define PRE_FIX_63305								// semozz 2012-07-20 // #63305 HP/MP does not refill when moving to stage entrance after 'Reason to Grow Stronger' quest
#define PRE_FIX_40328                               // Rotha        2012-02-27      // #40328 Hide unnecessary NPC dialog options
#define PRE_ADD_CHAT_RENEWAL						// bintitle. 2012-04-12  // #55596 Iconify chat window tabs and add whisper tab.
#define PRE_ADD_COMBINEDSHOP_PERIOD					// stupidfox 2012-07-05 // #60272 Sell timed cash items in in-game shop, #63956 [Main] Timed in-game item sales

#define PRE_MOD_PVPRANK								// 2hogi 2012-07-23 // #62825 Colosseum rank migration and improvement : Server
#define PRE_MOD_PVPOBSERVER							// 2hogi 2012-07-17 // #63098 Expand and improve Colosseum spectator count_Server
#if defined(PRE_MOD_PVPOBSERVER)
#define PRE_ADD_IMPROVE_MASTER_OBSERVER             // Rotha        2012-07-23      // #63217 Colosseum broadcast (observer) mode_Client
#endif

#define PRE_ADD_NOTIFY_ITEM_COMPOUND                // Rotha        2012-07-26      // #63414 Suffix crafting notifier


#define PRE_FIX_61382								// semozz 2012-07-02 // #61382 [Kali] Update Puppet effect
#define PRE_ADD_58449                               // Rotha        2012-05-21      // #58449 Added NPC notification type for PC cafe event.
#define PRE_FIX_PASSIVE_BUFF                        // Rotha        2012-06-14      // #59927 Modified to check by buff type when it is a passive buff
#define PRE_ADD_DOORS_PROJECT						// haling 2012-03-23 // #54291 Save Doors character stats



#define PRE_ADD_EASYGAMECASH						// dyss   2012-05-25  // #56185 Make game easier with cash (develop box that opens all linked items on open)
#define PRE_ADD_COSRANDMIX							// kalliste 2011-12-09 // #47470 Request for domestic costume synthesis work
#define PRE_ADD_56253								// semozz 2012-04-19 // #56253 [Kali] Fix effect of duration type "Time Toggle"
#define PRE_ADD_ANIOBJ_SHADOWOFF					// kalliste 2012-04-12 // #55520 [Kali] Remove shadow of mesh rendered as ActionObject
#define PRE_FIX_55461								// semozz 2012-04-05 // #55461 [Kali] Fix issue where another action transitions on Jump Kick hit
#define PRE_FIX_55383								// semozz 2012-04-04 // #55383 [Typhoon Kim Nest] Aura skill target type (enemy) and StateEffect fail to apply properly.
#define PRE_FIX_55378								// semozz 2012-04-04 // #55378 [Typhoon Kim] Projectiles fail to spawn properly when monster uses Charger skill.
#define PRE_ADD_55295								// semozz 2012-04-03 // #55295 [Kali] Fix vector value of projectile Hit action
//#define PRE_ADD_50903 // semozz 2012-02-03 // #50903 [Typhoon Kim Nest] Added skill effect ID:236 "Damage Distribution"
#define PRE_ADD_50917								// semozz 2012-01-31 // #50917 [Typhoon Kim Nest] Additional skill effect dev ID:238 "Monster Weapon Swap"
#define PRE_ADD_50907								// semozz 2012-02-02 // #50907 [Typhoon Kim Nest] Additional skill effect dev ID:237 "Disarm"
#define PRE_ADD_50923								// semozz 2012-01-31 // #50923 [Professor K Nest] Additional skill effect development


#define PRE_ADD_BUFF_ADD_INFORMATION                // Rotha        2012-03-09      // #51835 Buff improvement / displaying tooltips on buffs.
#define PRE_ADD_NAMEDITEM_SYSTEM					// dyss   2012-02-13  // #51841 Named item work
#define PRE_ADD_GUILD_EASYSYSTEM					// dyss   2012-05-16  // #57338 Enhance guild convenience
#define PRE_FIX_59347								// semozz 2012-06-07 // #59347 [Kali] Fix "Puppet" skill effect issue
#define PRE_FIX_59238								// semozz 2012-06-04 // #59238 Soul Gate duplicate hit bug when Kali casts Summon Puppet
#define PRE_FIX_CLIENT_SKILL_MAX                    // Rotha        2012-06-20      // #60103 Skill MaxLevel behaves differently on client, so apply table MaxLevel value for display only.
#define PRE_MOD_SELL_SEALEDITEM						// 2hogi 2012-04-30 // #51583 [Main] Sell sealed items to shop without unsealing

#define PRE_MOD_60583								// stupidfox 2012-06-27 // #60583 Consume unsealed items first when purchasing goods with items
#define PRE_FIX_CLASS_EXP							// haling 2012-06-20 // #36358 Per-character differential bonus EXP setting system (changed to drop as raw.)






#define PRE_FIX_CHARRENAME_MSG						// kalliste 2012-06-07 // #59426 [Korea] UI script error when changing character name to 1 character
#define PRE_ADD_PARTYTARGETMAPTOGGLE				// kalliste 2012-06-07 // #58336 [Party Renewal] Need additional categories such as Etc besides Stage/Nest/All



#define PRE_FIX_59336								// semozz 2012-06-05 // #59336 Elemental Lord Freezing Spike damage reduction bug by hit count
#define PRE_FIX_ELEMENT_REGULATION                  // Rotha        2012-06-02      // #58966 [Adjustment] Apply elemental gear adjustment for Ladder, Guild War, Dark Lair. 

#define PRE_ADD_COLOSSEUM_BEGINNER					// jhk8211, 2hogi 2012-04-25 // #55974 Created Colosseum beginner channel.
#define PRE_FIX_58505								// semozz 2012-05-23 // #58505 [Skill, Suffix] Suffix skill rebalancing

#define PRE_ADD_EXCHANGE_POTENTIAL					// 2hogi 2012-02-22 // #52315 #52314 Potential transfer

//#define PRE_ADD_COMEBACK							// bintitle, 2hogi  2012-04-16     // Disabled: absent in Steam NA v353 client/PAK
#define PRE_ADD_PREFIXSKILL_PVP						// semozz 2012-01-11 // #45842 [System] Add suffix effect balancing in Ladder
#define PRE_ADD_REVEAL_REGULATION_VALUE             // Rotha        2012-05-14      // #22342 Configure client to display adjusted applied values
#define PRE_FIX_MODIFY_AVERAGE_DAMAGE_REGULATION    // Rotha        2011-12-14      // #47134 Configure to apply P.Atk / M.Atk adjustment using average value.


#define PRE_PARTY_DB								// kimbap 2012-01-09 // Move party to DB


#define PRE_ADD_PVP_DUAL_INFO                       // Rotha        2012-03-29      // #48293 Feature to display opponent name in PVP Ladder 1:1 or All-Kill match. 


#define PRE_ADD_SKILLBUF_RENEW						// jhk8211 2011-09-19 // #38650 Skill revamp. Separated calculation/application locations for buff/debuff and skill state effects.
#if defined (PRE_ADD_SKILLBUF_RENEW)
#define PRE_FIX_51491								// semozz 2012-04-18 // #51491 [Skill] Fix critical formula - change damage calculation on critical resist
#define PRE_FIX_53274								// semozz 2012-03-08 // #53274 [Skill] Add A/D mashing feature to Time Restriction and Time Interruption (added to FrameStopBlow)
#define PRE_ADD_BUFF_STATE_LIMIT                    // Rotha        2012-02-17      // #52322 Set State Limit increased by buffs for skill improvement
#define PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP			// semozz 2012-01-18 // #49584 [Skill Revamp] Skill level-up condition				
#define PRE_ADD_49166								// semozz 2012-01-09 // [Skill] Change Cleric block trigger condition

#define PRE_FIX_53266								// semozz 2012-03-08 // #53266 [Skill] Request to add burn damage handling
#define PRE_ADD_50903								// semozz 2012-02-03 // #50903 [Typhoon Kim Nest] Additional skill effect dev ID:236 "Damage Distribution"

#endif // PRE_ADD_SKILLBUF_RENEW



#define PRE_ADD_DONATION							// hgoori, bintitle 2011-12-27 // Donation

#define PRE_ADD_CASHREMOVE							// saset, bintitle 2012-01-26 // #50317 Cash inventory item delete feature.

#define PRE_ADD_JELLYCOUNT							// bintitle     2012-02-08  // #51457 Improve convenience of Protection Jelly confirmation during enhancement.
#define PRE_FIX_51048								// semozz 2012-03-07 // #51048 No server-to-client packet was sent when adding state effect for monster passive skills; send packet for client sync
#define PRE_ADD_PVP_VILLAGE_ACCESS                  // Rotha,haling 2012-01-26 // #50888 Improve Colosseum access from town
#define PRE_ADD_54166								// semozz 2012-03-28 // #54166 2nd Pandora's Box system revamp - add optional contents preview feature

//#define PRE_ADD_UISTRING_DIVIDE // jhk8211 2012-02-27 // #47557 UIString.xml separated into an item-only file.
#define PRE_ADD_AUTOUNPACK							// bintitle     2012-01-11      // #49509 Auto-open large quantities of treasure chests. 
#define PRE_MOD_NESTREBIRTH							// kalliste 2012-01-19 // #47733 Sea Dragon Nest resurrection tooltip error

#define PRE_ADD_48682								// bintitle     2012-01-05      // Tooltip improvement - systemize text for item usable regions.
#define PRE_ADD_QUICK_PVP							// haling 2011-11-01 // #44645 Request to implement [Duel Request] feature in town
#define PRE_ADD_SALE_COUPON							// haling 2011-12-19 // #48428 [Server] Develop new coupon system
#define PRE_MOD_INTEG_SYSTEM_STATE					// kalliste 2011-11-22 // #44645 Request to implement [Duel Request] feature in town
#define PRE_ADD_REBIRTH_EVENT						// haling 2011-11-28 // #45852 Event system that lifts resurrection limits for a specific Nest during a specified time
#define PRE_MOD_REPUTE_NOMALICE2					// kalliste 2012-02-15 // #51874 Fixed disfavor removal bug - disfavor remains in sort combo box


#define PRE_FIX_PARTYRESTORE_DGNCLEAR				// kalliste 2011-12-22 // #48496 Reward result window abnormal when joining party right before stage clear progress bar finishes
#define PRE_FIX_48865                               // Rotha        2012-01-02      // #48865 Added quest notification close feature.
#define PRE_ADD_MODIFY_PLAYER_CANNON                // Rotha        2011-12-09      // #47261 Additional cannon work.
#define PRE_FIX_REPUTE_PRESENTCOUNT					// kalliste 2012-01-04 // #48973 [Affinity Improvement] Error when gifting with blank count input, #48975 [Affinity Improvement] No affiliation message not shown when gifting to NPC without faction
#define PRE_REMOVE_NPCREPUTATION_MALICE				// kimbap 2011-12-13 // #47802 Affinity improvement - remove dislike
#define PRE_MOD_REPUTE_NOMALICE						// kalliste 2011-12-13 // #47802 Affinity improvement_Removed disfavor
#define PRE_MOD_CHATBG								// kalliste 2011-11-29 // #30688 Chat window improvement related to Helpful DN Guide

#define PRE_ADD_GUILD_RENEWSYSTEM					// robust 2011-12-12 // Guild revamp system


#define PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL			// jhk8211 2011-12-14 // #44260 Modified to test box hit signal not properly excluding mindistance with collision mesh.

#define PRE_ADD_CHAR_SATUS_RENEW                    // Rotha        2011-10-12      // #38752 Character status window improvement 


#define PRE_UNIONSHOP_RENEWAL						// kimbap 2011-10-21 // #43891 [Shop QoL] Change Union shop to revamped shop method
#define PRE_ADD_36870								// kimbap 2011-09-26 // #36870 Add Round Action log



#define PRE_ADD_VEHICLE_ACTION_STRING               // Rotha        2011-11-23      // #46519 Configure to apply varied rider actions depending on mount type.
#define PRE_ADD_AURA_FOR_MONSTER                    // Rotha        2011-08-31      // #40899 [G-Dragon] Work to allow [Aura]-type skills to apply to monsters
#define PRE_FIX_REFLECT_SE_LIMIT					// jhk8211 2011-10-24 // #44245 Modified Divine Punishment skill effect. Capped max damage reflection value.
#define PRE_ADD_LEGEND_ITEM_UI						// verygoodd21 2011-09-30 // #42663 Request to improve auction house and item UI needed for adding Legend item grade


#define PRE_ADD_LEVELUP_GUIDE						// kalliste 2011-07-22 // #30684 Related to Helpful DN Guide_Level-up and skill notifier


#define PRE_ADD_CASH_AMULET                         // Rotha        2011-08-16      // #13810 Cash : Necklace item
#define PRE_ADD_37745                               // Rotha        2011-08-29      // #37745 Added feature to hide weapon when out of combat
#define PRE_ADD_REMOVE_PREFIX						// semozz 2011-09-20 // #41778 Feature request for crafting suffix removal item [Cube of Removal]


#define PRE_FIX_MEMOPT_EXT							// kalliste 2011-08-26 // Memory optimization: table diet
#define PRE_MOD_OPTION_TEXTURE_QUALITY				// kwcsc 2011-08-16 // #39853 Client optimization (allow selecting texture quality in options)


#define PRE_ADD_39644                               // Rotha        2011-08-11      // #39644 Added invincible state effect signal parameter for summoned monsters.
#define PRE_ADD_PREFIX_SYSTE_RENEW					// semozz 2011-06-14 // Change prefix skill activation condition


#define PRE_ADD_AUTO_DICE                           // Rotha        2011-08-08      // #26939 Auto-set dice selection.

#define PRE_ADD_SKILL_ADDTIONAL_BUFF                // Rotha        2011-07-05      // #35691 Exception handling when a single skill applies to multiple targets for different purposes in specific situations 


#if !defined(PRE_ADD_2vs2_LADDER)
#define PRE_FIX_TEAM_LADDER_3vs3                    // Rotha        2011-05-25      // #34428 Exception handling when applying only 3:3 excluding 2:2 in team ladder system.
#endif


#define PRE_ADD_LOTUSGOLEM							// kalliste 2011-03-09 // #29998 Implemented Ancient Swamp Golem Parts

///////////////////////////////////////////////////////////////////////////////////
///Organize defines
///////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Farm related
//////////////////////////////////////////////////////////////////////////////////
#define PRE_ADD_FARM_DOWNSCALE						// kimbap 2011-06-24 // #36223 Farm scale-down work
#define PRE_ADD_VIP_FARM							// kimbap 2011-05-17 // #33887 Farm QoL improvements (VIP membership)

//////////////////////////////////////////////////////////////////////////////////
// Guild war related
//////////////////////////////////////////////////////////////////////////////////
#define PRE_ADD_GUILDWAR_ONSCHEDULE					// haling 2011-10-14 // Guild war schedule switch.. turns On/Off here. Fetches guild war schedule when On
#define PRE_REMOVE_GUILD_WAR_UI						// kwcsc Hide guild war related UI if this define is not set.

//////////////////////////////////////////////////////////////////////////////////
// Related to new character (Cap 60: 6 Classes)
//////////////////////////////////////////////////////////////////////////////////
#define PRE_ADD_ACADEMIC							// hayannal2009 2011-05-02 // 5th class. Academic.
#define PRE_MOD_SELECT_CHAR							// kwcsc, saset 2012-03-20 // #53376 Character select and creation screen revamp
#define PRE_ADD_KALI								// jhk8211 2012-01-31 // #50492 Added new character Kali.
#define PRE_MOD_PVP_DBSP							// robust 2012-03-08 // #52889 Request SP handling for new character creation
#define PRE_CHARLIST_SORTING						// kwcsc, saset 2012-09-18 // #62192, #68598 Improve character select screen sorting
#define PRE_ADD_CHARSELECT_PREVIEW_CASHITEM			// anno414 2012-08-28 // #67840 Add cash costume preview try-on feature on character select screen
#if defined(PRE_ADD_COMEBACK)
#define PRE_ADD_NEWCOMEBACK							// dyss, bintitle   2012-08-27  // #65499,#65498 Returning player system improvement work
#endif	// #if defined(PRE_ADD_COMEBACK)

/////////////////////////////////////////////////////////////////////////////////
// Unified service feature (Cap 60 Unified Release)
/////////////////////////////////////////////////////////////////////////////////
#define PRE_MOD_COSRANDMIX_NPC_CLOSE				// kalliste 2012-10-25 // #71923 [SNDA] Request to prevent crafting window from switching immediately upon synthesis completion
//#define PRE_ADD_STAGE_CLEAR_ADD_REWARD			// verygoodd21, karl 2012-10-08 // Disabled: absent in Steam NA v353 client/PAK
#define PRE_ADD_68838								// karl 2012-09-13 // #68838 Request to review new feature development for Treasure Stage entry method
//#define PRE_ADD_BESTFRIEND						// robust 2012-04-10 // Disabled: absent in Steam NA v353 client/PAK
//#define PRE_PVP_GAMBLEROOM						// dyss, kwcsc 2013-01-17 // Disabled: absent in Steam NA v353 client/PAK
#define PRE_ADD_MUTE_USERCHATTING					// karl 2012-11-02 // #72847 [JPN][New Feature] Develop GM command (chat ban) to prevent disruptive behavior
#define PRE_ALTEIAWORLD_EXPLORE
#define PRE_ADD_LEVELUP_REWARD_NOTIFIER
#define PRE_ADD_COSTUME_SKILL

/////////////////////////////////////////////////////////////////////////////////
// Extended Client & Server Parity Defines
/////////////////////////////////////////////////////////////////////////////////
#define PRE_ADD_FADE_TRIGGER
#define PRE_ADD_CRAZYDUC_UI
#define PRE_ADD_PVPRANK_INFORM
#define PRE_ADD_HPBELOWDOSKILLBLOW
#define PRE_ADD_CASHINVENTAB
//#define PRE_ADD_CASHSHOP_CREDIT					// Excluded: Nexon NA NX Credit/Prepaid dual-wallet billing; standard minicashshopdlg.ui is in PAK
#define PRE_ADD_WORLD_MSG_RED
#define PRE_ADD_STAGE_WEIGHT
#define PRE_ADD_CHAT_MISSION
#define PRE_ADD_PCBANG_RENTAL_ITEM
#define PRE_ADD_TALISMAN_SYSTEM
