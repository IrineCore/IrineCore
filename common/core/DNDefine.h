#pragma once

// 2013. 03 -----------------------------------------------
#define PRE_FIX_STUCK_LANDING_ACTION                // Rotha 2013-03-05 // #82453 Fix issue where actor stays in aerial action due to action interference when transitioning to LandingAction
#define PRE_FIX_NPCHIDE_ON_BREAKINTO				// kalliste 2013-03-05 // #82723 [Memoria] NPC does not exist after reconnecting from force-quit during NPC dialogue quest in Memoria dungeon while in party.

// 2013. 02 -----------------------------------------------
#define PRE_MOD_PVP_ROUNDMODE_PENALTY				// devkang  2013-02-05  // #80756 Grant penalty based on win count of winning team in PVP round mode
#define PRE_ADD_CHECK_MOVESPEED_HACK                // Rotha 2013-02-26 // #81387 [Live Ver.155] Speed hack issue via PAK file < Add movement speed hack detection routine >
#if defined(_FINAL)
#define PRE_FIX_CLIENT_FREEZING						// verygoodd21 2013-02-20 // #80025 Report on Dragon Nest Not Responding issue (testing with shortened hang crash timeout)
#endif

#define PRE_DELETE_DUNGEONCLEAR						// saset 2013-02-21 // Deleted for now since dungeon clear data is no longer used (can restore later when re-enabling Abyss check)
#define PRE_FIX_82147								// verygoodd21 2013-02-15 // #82147 [Skill] Paladin skill [Aerial Block] cooldown is abnormal
#define PRE_FIX_82502								// verygoodd21	2013-02-15		// #82502 Dungeon ranking exploit.
#define PRE_FIX_SQLCONNECTFAIL_LOG					// karl 2013-02-01 // Fix to log detailed error when SQL Server Connect fails
#define PRE_FIX_LOGINSERVER_CONNECT					// karl 2013-02-14 // Fix bug where client connects only to login server 1 when returning to character select screen
#define PRE_FIX_REMOVE_STATE_EFFECT_PACKET          // Rotha 2013-02-13 // Improve SC_CMDREMOVESTATEEFFECTFROMID structure
#define PRE_FIX_3DSOUND_DISTANCE					// kalliste 2013-02-12 // #82207 [Dragon Mount] Request to check sound when pressing space bar in place
#define PRE_FIX_SCROLL_PAGE_NUMBER					// kalliste 2013-02-12 // #82215 [Guild Level Unlock] Cannot advance to page 28
#define PRE_FIX_MASTERSYSTEM_CHECK_PUPILBLOCKDATE	// karl 2013-02-06 // #81718 Rejection message only shown to apprentice when applying to a user within 1 day of deleting a master
#define PRE_FIX_70011								// anno414  2013-02-06  // #70011 Fix bug where hit-stiff motion fails to occur when Ice Fraction triggers via Freeze
#define PRE_ADD_LISTBOX_ELLIPSIS					// kwcsc 2013-02-13 // #82241 Ellipsis truncation for EU trading house listbox
#define PRE_ADD_GM_APPELLATION						// kwcsc 2013-02-14 // #82151 GM title


// 2013. 01 -----------------------------------------------
#define PRE_FIX_61545								// kimbap, kalliste 2012-07-05 // #61545 [System] Entering 3 keywords in PT search box fails to find matching PT 
#define PRE_ADD_VEHICLE_SPECIAL_ACTION              // Rotha 2013-01-18 // #79817 [Dragon Mount] Request to develop feature controlling special mount actions in specific/designated maps.
//#define PRE_ADD_CHAGNESERVER_LOGGING // 2hogi 2012-01-29 // Added logs for issues during Game->Village and Village->Game transitions
#define PRE_FIX_CHANGESTAND_HACK                    // Rotha 2013-01-29 // #73212 Fix hack check using stance change state effect (cancel skill if used during stance change)
#if !defined(_FINAL_BUILD)
#define PRE_ADD_WORLD_EVENT							// haling 2012-10-22 // #71975 [DOLIS] Develop world event
#endif
#define PRE_FIX_DUNGEONENTER_CLOSE_BY_QUEST			// kalliste 2013-01-21 // #79949 In Garden of Time and Space (Lv60) 'False Stronghold', waiting on entry screen at day change leaves Ready state stuck and unable to move through portal
#define PRE_FIX_PVPRANK_NOSERVICE_JOB				// kalliste		2013-01-16		// #79237 Ranking UI. Need to remove unnecessary tabs
#define PRE_FIX_APPELATION_TOOLTIP					// kalliste 2013-01-15 // #79182 Opening help window while collection book is open shows collection book description on help mouseover
#define PRE_FIX_APPELLATION_BITSET_LENGTHCHECK		// karl 2013-01-07 // Fix to check buffer size when verifying acquired title (buffer overrun fix)
#define PRE_FIX_DEBUGSET_INIT						// stupidfox 2013-01-04 // Fix runtime crash when XML parsing fails in InitApp on Master/Login/Cash servers
#define PRE_ADD_REDUCE_TABLE_RESOURCE				// kwcsc 2012-12-20 // Reduce table resources
#define PRE_ADD_LOGSERVER_HEARTBEAT					// stupidfox 2012-11-26 // Add LiveCheck for log server

// 2012. 12 -----------------------------------------------
#define PRE_ADD_ACTION_DYNAMIC_OPTIMIZE				// hayannal2009 2012-12-31 // Added dynamic optimization feature for action signals.
#define PRE_ADD_CHAOSCUBE							// robust, bintitle 2012-03-20 // #53978 Mix Box system - Server / #53980 Client
#define PRE_ADD_CHOICECUBE							// bintitle.dyss 2012-05-31  // #59015 Choice Cube

#define PRE_ADD_77490								// haling 2012-12-26 // #77490 Fix items only appearing in a fixed order when using Double Gacha on world map
#define PRE_ADD_CHECK_PART_MONSTER					// anno414  2012-12-07  // #75630 Investigate issue where 2 or more monster portraits do not appear
#define PRE_FIX_DMIX_NOACTION						// kalliste 2012-12-18 // #76943 [Costume] Report that Whale costume motion is invisible after costume synthesis
#define PRE_MOD_SERVER_WAREHOUSE_NO_REMOTE			// kalliste 2012-12-17 // #76780 Remote Storage item_Changed to disallow using Account Storage
#define PRE_FIX_76603                               // Rotha 2012-12-17 // #76603 Set End handling for transform state effect
#define PRE_FIX_PLAYER_CROSSHAIR_NOBATTLE			// kalliste 2012-12-17 // #76666 [System] Cannot display 'Community' menu with other characters in Desert Dragon Oasis area
#define PRE_MOD_LIKEABILITY_QUEST_TEXT				// elkain03 2012-08-13 // #65623 Improve duplicate quest type display for affinity unlock quests 
#define PRE_FIX_65287								// semozz 2012-08-09 // #65287 [System] Damage boost of Epic skill enhancement crest 'Despair Needle' not applied accurately 
#define PRE_MOD_RELEASE_SOUND_SIGNAL				// hayannal2009 2012-12-14 // For sound signals that play once and end, no need to keep them in memory. Loading is fast anyway, so delete after playing.
#define PRE_MOD_PRELOAD_MONSTER						// hayannal2009 2012-12-14 // Change monster loading structure. Stop using preload and add structure to flush periodically.
#define PRE_MOD_75892								// 2hogi 2012-12-11 // #75892 Improve potential transfer feature
#define PRE_FIX_PET_UNEQUIP							// kalliste 2012-12-11 // #75550 Hairstyle does not restore after unequipping Manti Pet Santa Hat
#define PRE_FIX_75807								// haling 2012-12-10 // #75807 Reset previous winning guild farm if no winning guild, and grant rewards without next schedule
#define PRE_FIX_REPUTE_DISCOUNT						// kalliste 2012-12-10 // #75858 [VIP Membership] Golden Goose Guild 15% sale price increase not applied
#define PRE_ADD_58761								// dyss   2012-07-16  // #58761 System to check clear time per 8-man Nest gate
#define PRE_REMOVE_MINISITE							// kalliste 2012-12-07 // #75814 Remove DNTV icon from default screen
#define PRE_FIX_TEXTDLG_TIMER						// kalliste 2012-12-07 // #75793 Issue where window-type message box does not disappear
#define PRE_ADD_FRAMEDELAY_LOG						// 2hogi 2012-12-04 // #74675 Ver148 GameDelayed issue <1122> : add diagnostic logs
#define PRE_FIX_MAKECHARMITEM						// kalliste 2012-12-04 // #74682 [Live Ver.148] Treasure Package contents display issue
#define PRE_FIX_75305								// karl 2012-12-03 // #75305 [Superior Seal Stamp] Bug where Imperfect Seal Stamp is consumed when using Superior Seal Stamp
#define PRE_FIX_77172								// kwcsc 2012-12-28 // #77172, 77216 Keep flying to the end even if damaged mid-jump during Burn or Increase Soul state effect

// 2012. 11 -----------------------------------------------
#define PRE_FIX_SUMMON_ACTOR_HPBAR					// anno414  2012-11-28  // #73648 HP bar of "Summon Alfredo" disappears when event movie plays
#define PRE_MOD_INSPECT_DURABILITY					// hayannal2009 2012-11-28 // #74128 Remove durability display from Inspect.
#define PRE_ADD_HEAD_SCALE_POTION					// hayannal2009 2012-11-27 // #74414 Big-head potion
#define PRE_FIX_CASHSHOP_CHARGE_WNDPROC				// kalliste 2012-11-26 // #74747 [Actoz] Client crash when clicking charge button in Cash Shop.
#define PRE_MOD_SKILLRESETITEM						// kalliste 2012-10-24 // #72083 [Main] Actoz service price/policy/feature change_Make Oblivion Contract (skill reset) free
#if !defined(PRE_FIX_FALL_PROP_COLLISION)
#define PRE_MOD_VELOCITYACCEL_SIGNAL				// hayannal2009 2012-11-22 // Fix falling through collision when velocity accel signal arrives on top of collision.
#endif // #if !defined(PRE_FIX_FALL_PROP_COLLISION)
#define PRE_FIX_74387								// haling 2012-11021 // #74387 US (ResellItem) Changed to process after receiving DB response.
#define PRE_ADD_SHOWOPTION_PVPMARK					// hayannal2009 2012-11-21 // #74010 Add on/off option to show Colosseum rank mark.
#define PRE_MOD_CASHFAIRY							// hayannal2009 2012-11-20 // #73215 Change spirit to follow actively.
#define PRE_MOD_74203								// 2hogi 2012-11-19 // #74203 Hero Academy issue: add forbidden word check on guild creation
#define PRE_MOD_HEAD_EFFECT_HEIGHT					// hayannal2009 2012-11-19 // #74090 Adjust Head Effect height.
#define PRE_FIX_73930								// karl 2012-11-15 // #73930 [System] HP and MP adjustments are applied when entering 'Albtraum of Jealousy' after leaving 'Formid of Infinity' 
#define PRE_FIX_SHOWVER								// kalliste 2012-11-14 // #73803 [EU] Client dump occurs when using cheat /showver
#define PRE_DEL_ONNULL								// haling 2012-07-20 // Remove CashServer OnNull function..
#define PRE_FIX_LIVE_CONNECT						// haling 2012-09-10 // Support US server port Live check	

#define PRE_FIX_69469                               // Rotha        2012-10-30      // #69469 Issue where wrong action plays when using another skill after a skill with cancel action.
#define PRE_ADD_POSITION_SYNC_BY_SKILL_USAGE        // Rotha        2012-11-05      // #72992 Intermittent desync in Colosseum. (Added because position sync was not done when using skills.)
#define PRE_MOD_LOAD_DARKLAIR_MONSTER				// hayannal2009 2012-11-06 // Change Dark Lair monster loading structure. Stop using preload, and delete unnecessary Act per round.
#define PRE_MOD_LOAD_SOUND_SIGNAL					// hayannal2009 2012-11-06 // Remove sound signal preload. Change to load on demand.
#define PRE_FIX_73183								// haling 2012-11-06 // #73183 [System] One general inventory slot becomes unusable when 'Low-Grade Seal Stamp' drops from 'Colosseum Hero's Pouch'
#define PRE_FIX_63822								// 2hogi 2012-11-06 // #63822 [System] Non-reusable cash items can be used multiple times during lag 
#define PRE_FIX_MAIL_HEADICON						// kalliste 2012-11-02 // #72848 Guild name is pushed back when another user views a guild member using remote mailbox
#define PRE_FIX_73312								// stupidfox 2012-11-07 // #73312 Fix stage clear window showing 2+ times (Dark Lair only)
#define PRE_FIX_INITSTATEANDSYNC					// stupidfox 2012-11-15 // Calling InitStateAndSync via trigger API can conflict between GameRoom process and background loader
#define PRE_FIX_DELAY_MASTER_CHECKVILLAGECHANNEL	// stupidfox 2012-11-26 // Delay Master Server CheckVillageChannel() until after startup -> fixes a channel opening when all village channels have visibility=false at startup
#define PRE_FIX_WEAPON_DUPLICATE_RENDER				// kwcsc 2012-11-27 // #74530 Weapons overlap in creation screen after changing costume on character select screen

// 2012. 10 -----------------------------------------------
#define PRE_FIX_LADDERSHOP_SKILL					// kalliste 2012-10-31 // #72288 Cannot purchase item when holding only discounted points after learning union skill 'Ladder Shop Purchase Discount' 
#define PRE_FIX_72521								// stupidfox 2012-10-29 // #72521 Fix time-restricted gate ignoring other conditions and activating
#define PRE_FIX_70618								// verygoodd21 2012-10-23 // #70618 Issue where trigger blocking mana-using skills is forcibly turned off in the next stage
#define PRE_FIX_71954								// haling 2012-10-23 // #71954 Granted horse does not disappear when selecting stage move after Wonderful Racing ends
#define PRE_FIX_GUIDEDARROW_ON_PVPTOURNAMENT		// kalliste 2012-10-16 // #71207 [Skill] Bowmaster skill 'Guided Shot' targets characters in spectator seats in Tournament mode
#define PRE_ADD_DOORS								// 2hogi 2012-06-20 // #60319 [DN App Phase 1] Message - Server

#define PRE_FIX_RENDER_GATENAME_POS					// kalliste 2012-10-10 // #71081 Saint Haven South Gate portal 'Hermalte Port' text position misaligned
#define PRE_FIX_CASHSHOP_FIXEDPRICE					// kalliste 2012-10-10 // #71052 Radiant Star Fragment Ring price display error
#define PRE_FIX_PROP_SHOW_BY_TRIGGER				// kalliste 2012-10-08 // #70651 Bug where hide/show prop trigger does not apply to 'Chest' property props
#define PRE_ADD_COSRANDMIX_ACCESSORY				// kalliste 2012-09-26 // #68900 [Main] Accessory synthesizer development / 2012-10-02 China pre-patch. Needs to be applied to other regions later
#define PRE_ADD_COSRANDMIX_MIXERTYPE				// kalliste 2012-10-10 // #70513 [Main] Accessory synthesizer development
#define PRE_ADD_PVP_TOURNAMENT_ADDITIONAL			// kalliste 2012-10-02 // Additional work for PVP tournament.
#define PRE_FIX_QUEST_COUNT_WORLDZONE				// kalliste 2012-10-02 // #70155 [Quest] Sub quest 'Beyond the Nightmare' mismatched available quest count in world zone entry confirmation window
#define PRE_FIX_QUEST_GUILD_BOARD					// stupidfox 2012-10-10 // #71094 Repeatable quest abuse fix
#define PRE_ADD_TRANSFORM_POTION                    // Rotha,haling 2012-09-13 // #66053 Develop transform potion
#define PRE_FIX_71455_REPUTE_RENEW					// kwcsc 2012-10-16 // #71455 Add expiration handling for NPC affinity union membership
#define PRE_FIX_71333								// dyss   2012-10-23  // #71333 Professor K boss loop action related to anomaly 

// 2012. 09 -----------------------------------------------
#define PRE_FIX_REBIRTHCOIN_REMAIN					// kalliste 2012-09-25 // #70186 Issue where Dragon Nest remaining resurrection count appears stuck at previous Nest's value
#define PRE_FIX_RECURSIVE_DICE						// kalliste 2012-09-27 // #70119 [Live Ver.140] Issue related to infinite dice rolling
#define PRE_REMOVE_CASHSHOP_GIFT_CADGE_MSGMAX		// kalliste 2012-09-24 // #69952 [Request Gift] Gift/request message can be sent up to 50 chars instead of 20 chars
#define PRE_MOD_QUEST_NOTIFIER_FULLCHECK			// elkain03 2012-09-20 // #68785 Fix quest window checkbox disabled issue when quest notification list is full

#define PRE_FIX_69108								// stupidfox 2012-09-14 // #69108 Fix Dark Lair ranking not being saved
#define PRE_FIX_PVP_SPECTATOR_MOVE					// kalliste 2012-09-11 // Fixed PVP spectator movement issue.
#define PRE_FIX_GUILDCREATE_DISABLEBTN				// kalliste 2012-09-06 // #68528 When entering only 1 char for guild name, 'Create' button is disabled but creation is still possible
#define PRE_MOD_SET_ACHIEVED_MISSION_GAINFLAG		// stupidfox 2012-09-07 // Process Gain for items marked Achieve in DB but not yet Gained

// 2012. 08 -----------------------------------------------
#define PRE_FIX_66175								// semozz 2012-08-23 // #66175 Swap EX skill UsableChecker with base skill.
#define PRE_FIX_BOW_ACTION_MISMATCH					// kalliste 2012-08-14 // #66029 [Other] Abnormal weapon motion in Cash Shop preview when Archer equips Crossbow and 2012 Summer limited weapon costume
#define PRE_FIX_63356								// semozz 2012-08-13 // #63356 [Skill] Mercenary's 'Circle Bomber' fails to hit Fujinos's abdomen in Archbishop Nest

#define PRE_MOD_CHRAMBOX_ROTATE						// elkain03 2012-08-20 // #66350 Fix reversed chest image above character head when using treasure chest

#define PRE_FIX_MOMSTER_PROCESS_DELTA               // Rotha        2012-05-16      // #56383 Configure not to reduce monster frame rate related to issue.

#define PRE_FIX_CHANGEJOBITEM_CHECK_NEEDJOB			// kalliste 2012-08-13 // #65337 [UI] Kali name missing in job change ticket tooltip 
#define PRE_FIX_COSMIX_NORESPONSE_ON_ERROR			// kalliste 2012-08-12 // #66018 Cannot control game after using 'Mix Box' as Kali
#define PRE_FIX_65655								// haling 2012-08-10 // #65655 Allow leaving guild during guild war registration period (cannot delete character while in guild)
#define PRE_FIX_PACKAGE_RESULT_CREATEITEM			// kalliste 2012-08-09 // #65696 Sub-weapon type not displayed in confirmation window when opening level-up reward box

#define PRE_FIX_LOAD_GACHA_DATA						// kalliste 2012-08-01 // #64478 [TDN] Cannot obtain costume via Gachapon NPC.
#define PRE_ADD_FISHING_RESULT_SOUND				// anno414 2012-07-31 // #63752[Fishing] "Success" and "Fail" sound effects are missing when fishing (added success and fail sounds)
#define PRE_FIX_GAMESERVER_USE_GHOST_MODE           // Rotha        2012-07-25      // #61945 Sync setting between game server and client for issue < SwapSingleSkin Gameserver applied >
#define PRE_FIX_FARMERRORMSG_USA					// kalliste		2012-07-23		// #63422 Game crashes when planting two plants at the right most farming section of the map.

// 2012. 07 -----------------------------------------------
#define PRE_FIX_63219								// semozz 2012-07-25 // #63219 [Skill] Casting 'Divine Vengeance' while holding a movement key plays the animation without dealing damage
#define PRE_MOD_CASHSHOP_NOETERNITY_TOOLTIP			// kalliste 2012-07-19 // #63377 Marine Look set / Oblivion Contract tradability - In all countries, show non-tradable tooltip for timed items even if ItemReversion is ITEMREVERSION_TRADE
#define PRE_FIX_61438								// semozz 2012-07-18 // #61438 [Kali] Summon Puppet spawn position bug in 8-player nest
#define PRE_FIX_NESTREBIRTH							// kalliste 2012-07-18 // #62432 [RB Ver.128] Sea Dragon Nest Hardcore resurrection count display issue
#define PRE_FIX_62845								// anno414 2012-07-17 // #62845 Output error message when trigger is lost due to trigger Lua error
#define PRE_FIX_CREATE_ITEM_TOOLTIP_POS				// anno414 2012-07-13 // #62632 At certain resolutions in item crafting window, shift tooltip right when switching tooltip to PVP.
#define PRE_FIX_52267								// semozz 2012-07-11 // #52267 [Skill] In PVP, Alchemist's 'Injection' heal rate differs from tooltip
#define PRE_FIX_62309								// semozz 2012-07-11 // #62309 [Skill] Missing elemental indicator on damage from explosion-type skills in dungeons or nests
#define PRE_FIX_PARTYINDEX_DUMP						// kalliste 2012-07-10 // Fixed dump related to party index
#define PRE_FIX_62052								// semozz 2012-07-10 // #51052 [Skill] Ultimate skills can be used consecutively in a specific sequence
#define PRE_FIX_61821								// semozz 2012-07-05 // #61821 [UI] Cooldown icon disappears if passive skill levels up while on cooldown after activation 
#define PRE_FIX_SKILL_TOOLTIP_REFRESH				// anno414 2012-07-04 // #61556 Bug where tooltip content does not update immediately when increasing skill Level.
#define PRE_FIX_NONESTTOOLTIP_NODRAGONNEST			// kalliste 2012-07-03 // #61540 [UI] Missing description text for resurrection limit count in Typhoon Kim Nest
#define PRE_FIX_CHARDELWAITDLG_TIME_DUMP			// kalliste 2012-07-02 // Fixed dump related to using localtime function.

// 2012. 06 -----------------------------------------------
#define PRE_FIX_GUILDWARSKILLTREE_BUFFEROVERRUN		// kalliste 2012-06-28 // Fix related to string buffer overrun.
#define PRE_FIX_60525								// semozz 2012-06-28 // #60525 Apply suffix enhancement to sealed equipment items
#define PRE_FIX_PROPMON_PROJ						// kalliste 2012-06-15 // #59952 Client crash when hit by snake statue in Catacombs Hall dungeon
#define PRE_FIX_CANCELTRADE							// kalliste 2012-06-14 // #57369 Cash shop fails to open in specific sequence
#define PRE_FIX_59680								// semozz 2012-06-13 // #59680 [Kali] Heal skill bug when casting Summon Puppet
#define PRE_FIX_59308								// semozz 2012-06-14 // #59308 [Kali] Critical display bug on Summon Puppet
#define PRE_FIX_59939								// semozz 2012-06-15 // #59939 [Kali] Bug with curse effect

// 2012. 05 -----------------------------------------------
#define PRE_TESTLOG_58816							// kalliste 2012-05-30 // #58816 GameServer ROOM Crash issue <0526> (test log)
#define PRE_FIX_SOURCEITEM_TOOLTIP					// kalliste 2012-05-23 // #57755 [System] Client crashes if equipped Origin item effect duration reaches 0 while tooltip is displayed
#define PRE_FIX_57852								// semozz 2012-05-18 // #57852 [UI] Mail/gift mini-icon tooltip text not displayed in non-(Wide) resolutions

#define PRE_FIX_57706								// semozz 2012-05-14 // #57706 [Skill Revamp] Duration bug when overlapping Time-type skills
#define PRE_ADD_CUTSCENE_PADSKIP					// kalliste 2012-05-12 // #57758 [System] Cannot skip event movie with gamepad 'System Menu' key 
#if !defined (_FINAL_BUILD)
#define PRE_SOURCEITEM_TIMECHEAT					// kalliste 2012-05-11 // Cheat for checking Origin item tooltip duration
#endif
#define PRE_ADD_HITDIRECTION						// jhk8211 2012-05-10 // #56267 [Desert Dragon] Feature to check Hit based on facing direction

#define PRE_FIX_55618								// semozz 2012-05-08 // #55618 Alfredo abnormal movement issue (speed synchronization)

// 2012. 04 -----------------------------------------------
#define PRE_FIX_QUICKPVP_EXCLUSIVEREQ				// kalliste 2012-04-23 // #56593 [System] Crash when receiving duel request in specific sequence / #56606 [System] Can force-close opponent PC using 'Duel Request' system
#define PRE_FIX_GM_BREAKINTO_DGNCLEAR				// kalliste		2012-04-06		// #55464 Live Server Ver 59: GM Bug

// 2012. 03 -----------------------------------------------
#define PRE_MOD_KALI_WEAPON							// hayannal2009 2012-03-09 // Copy and show Kali main weapon mesh on left hand as well when equipped.


// 2012. 02 -----------------------------------------------
#define PRE_FIX_52329								// semozz 2012-02-24 // #52329 [Gigantes Hell] Funnel and missiles become invisible when hugging a wall while affected by funnel skill
#if !defined(_FINAL_BUILD)
#define PRE_ADD_EXPORT_DPS_INFORMATION              // Rotha        2012-02-08      // Skill damage meter for skill improvement
#endif
#define PRE_FIX_51999								// semozz 2012-02-21 // #51999 [System] Skill icon disappears in the skill window under a specific sequence.
#define PRE_FIX_TRY_REBIRTH_BEFORE_SYNC				// kalliste 2012-02-21 // #52168 Cannot resurrect after moving maps in stage in a specific sequence
#define PRE_FIX_51727								// semozz 2012-02-20 // #51727 [RB Ver.108] Tooltip display issue when learning a skill
#define PRE_FIX_51988								// semozz 2012-02-20 // #51988 Disable RemoveStateIndex feature
#define PRE_FIX_50470								// semozz 2012-02-17 // #50470 [Battle] In Green Dragon Nest, adding the 'Wisdom' prefix effect deals massive damage 
#define PRE_FIX_NOMOVE_BROWSER						// kalliste 2011-08-03 // #39446 Removed browser movement
#define PRE_FIX_50007								// semozz 2012-01-17 // #50007 Death animation does not play when dying while hit by a dance skill in Slave Master's Gate
#define PRE_FIX_50482								// semozz 2012-02-06 // #50482 Fix team inversion effect bug when FreezingPrision state effect overlaps

// 2012. 01 -----------------------------------------------
#define PRE_ADD_HIT_PROP_ADD_SE						// jhk8211 2012-01-02 // #49083 Allowed props to apply hit and state effects simultaneously like actors. Included early as it is a feature-related issue.
#define PRE_FIX_50460								// semozz 2012-01-31 // #50460 [System] In Dark Lair 'Pessadero of Hatred', clicking the stage icon on the expected reward screen resets 'Remaining Time'
#define PRE_FIX_49403								// bintitle  2012-01-19  // #49403 [System] In special order, cannot board airship even after spending money.
#define PRE_FIX_49208								// semozz 2012-01-18 // #49208 Elemental Lord invincible BUG (fix removal bug when already OnBegin state effect is duplicated and CanBegin fails)
#define PRE_FIX_ONCANCEL_USEITEM					// kalliste 2012-01-17 // #48765 [System] Using the newly added costume synthesis system in a specific sequence makes PC uncontrollable
#define PRE_ADD_49660								// jhk8211 2012-01-16 // #49660 Modified state effects reflected by Divine Punishment to reflect light attribute attack power.

#define PRE_FIX_48724								// semozz 2012-01-04 // #48724 Using "/escape" CMD_MSG_ESCAPE while stunned clears the stun state.

// 2011. 12 -----------------------------------------------
#define PRE_FIX_NEXTSKILLINFO						// semozz 2011-12-26 // Fix next-level info setup since skill level data may not be contiguous
#define PRE_FIX_48494								// semozz 2011-12-22 // #48494 Display aura type duration as infinite
#define PRE_ADD_GUILD_GESTURE						// semozz 2011-12-22 // Add guild gesture
//#define PRE_FIX_48551 // jhk8211 2011-12-20 // #48551 Make Ping-Pong Bomb exception handling slightly lighter.
#define PRE_FIX_HOLD_COSTUMEMIX_INVEN				// kalliste 2011-12-14 // #48009 Synthesis window closes automatically when trying to open design synthesis window
#define PRE_FIX_COMBOBOX_ELLIPSIS					// kalliste 2011-12-08 // #47480 In party purpose search field, truncated text is not shown when hovering over ellipsis
#define PRE_ADD_COSMIX_SORT							// kalliste 2011-12-02 // #46494 In costume synthesis window, 17th costume is displayed first 
#define PRE_FIX_45899								// semozz 2011-12-02 // #45899 [Skill] Activating a suffix skill at the moment of equipping a prefix accessory causes attacks not to hit enemies 
#define PRE_FIX_LOADINGSCREEN						// jhk8211 2011-12-02 // No issue number. Render map loading screen only once to avoid rendering during map loading.

// 2011. 11 -----------------------------------------------
//#define PRE_ADD_VSYNC_OFF // jhk8211 2011-11-28 // No issue number. Force vsync on first due to client dumps.
#define PRE_FIX_GAMEQUIT_TO_VILLAGE					// kalliste 2011-11-23 // #46534 Issue where quitting the game while in a party in world zone or dungeon returns to town
#define PRE_FIX_46730								// semozz 2011-11-28 // #46730 DropItem sliding issue - fix position correction with RandomSeed (add bFixPosition to DropItem signal)
#define PRE_FIX_BLOCK_CONDITION						// semozz 2011-11-24 // #25384 Fix block success check when HitSignal Damage ratio is 0 (blockable when opposing side / not blockable when opposing side)
#define PRE_FIX_46381								// semozz 2011-11-21 // #46381 Elemental Lord - shield remains active when out of MP
#define PRE_FIX_PARTY_STAGECLEAR_CHANGEMASTER		// kalliste 2011-11-10 // #44641 [System] Pressing 'F12 key' with specific operation sequence displays 'M:337' message (C/G)
#define PRE_ADD_ONESTOP_TRADECHECK					// kalliste 2011-11-10 // Added unified interface for tradable check
#define PRE_ADD_DMIX_DESIGNNAME						// kalliste 2012-02-01 // #50436 [Feb 22] Avatar design/grade synthesis item improvement proposal_revision proposal
#define PRE_FIX_HOMINGTARGET						// semozz 2011-11-09 // #44998 To display targeted objective due to Guided Shot target setup issue.
#define PRE_FIX_44884								// semozz 2011-11-08 // #44884 [System] In Chaos Field Baira, 'Milodo Garon - Orc' freezes and attacks stop hitting in a specific sequence
#define PRE_MOD_RESTRICT_IDENTITY_IP				// 2hogi 2011-11-07 // #44912 Limit client connections per IP to block bots

// 2011. 10 -----------------------------------------------
#define PRE_FIX_44486								// semozz 2011-10-31 // #44486 Reset cooldown when equipping/unequipping Technique item.
#define PRE_FIX_43986								// semozz 2011-10-26 // {Cash} Add charm item feature - roulette feature
#define PRE_FIX_PROP_RANDOM_VISIBLE					// jhk8211 2011-10-04 // #42473 Unable to reproduce, so added routine where each client queries server after SyncComplete. By default, create all and hide if not created on server side.

// 2011. 09 -----------------------------------------------
#if defined (_GAMESERVER)
#define PRE_ADD_THREADAFFINITY						// 2hogi 2011-09-27 // #42461 Supplement handling for Thread Affinity Setting
#endif		//#if defined (_GAMESERVER)

#define PRE_FIX_MEMOPT_SIGNALH						// kalliste 2011-09-23 // Memory optimization

// 2011. 08 -----------------------------------------------
#define PRE_FIX_CLIENT_MEMOPTIMIZE					// siva 2011-08-03 // Client memory optimization work
#define PRE_FIX_GAMESERVER_OPTIMIZE					// siva 2011-08-01 // Game server optimization.

// 2011. 07 -----------------------------------------------
#define PRE_FIX_GAMESERVER_PERFOMANCE				// jhk8211 2001-07-25 // GameServer performance improvement
#define PRE_MOD_ANIMATIONUI_PLAY_CONTROL			// kalliste 2011-07-15 // #37737 Added animation UI template properties (note: waiting to update until animations made with that UI tool are created)
#define PRE_ADD_RELOGIN_PROCESS						// kwcsc 2011-10-14 // #43496 Change to retry login on duplicate login failure
#define PRE_ADD_DONT_USE_FREETYPE					// jhk8211 2011-07-08 // #35682 Do not use freetype. Use Windows font output. Used for Thai. Size criteria differ from freetype.
#define PRE_ADD_HEARTBEAT_TO_SQL					// 2hogi 2011-07-01 // #36759 Login server Heartbeat SP between Membership and WorldDB

// 2011. 06 -----------------------------------------------
#define PRE_FIX_SKILLLIST							// jhk8211 2011-06-24 // Changed skill handle list management method due to occasional dumps on GameServer.

// 2011. 04 -----------------------------------------------
#define PRE_ADD_CHANGEJOB_CASHITEM					// jhk8211 2011-04-29 // Added job change cash item feature.
#define PRE_ADD_MONSTER_CATCH						// jhk8211 2011-04-14 // #28532 Implemented feature for monster grabbing player

#define PRE_FIX_SP_REVISION_CHANGE_SP				// jhk8211 2012-06-13 // #57860 Changed DB SP used in skill point correction routine.
#define PRE_ADD_SP_REVISION							// jhk8211 2011-04-04 // #29463 Added define because DB-side application of skill point check routine was delayed.

// 2011. 02 -----------------------------------------------
//#define PRE_ADD_LOOP_PROJECTILE // jhk8211 2011-02-15 // Account for server failing to keep up with verifying projectiles (Pinpoint Shot) fired in short-interval loop actions.

// 2011. 01 -----------------------------------------------
#if defined( _DBSERVER ) || defined (_LOGINSERVER) || defined(_CASHSERVER)
#define PRE_ADD_QUERYTIMELOG							// kimbap 2011-01-03 // Log DB query time 'Added because login server also needs logging - 2hogi'
#endif // #if defined( _DBSERVER ) || defined (_LOGINSERVER)
#define PRE_ADD_TRANSPARENCY_COSTUME				// hayannal2009 2011-01-19 // #27573 Add transparent costume.
//#define PRE_ADD_PETALTRADE						// verygoodd21 2011-01-06 // Disabled: Steam NA v353 client uses standard Market*.ui (no *_wPetal.ui)
#define PRE_FIX_34367								// haling 2012-01-11 // [Suggestion] Trading House - Petal voucher <-> Petal purchase setting (block registering Petal voucher for Petal)
#define PRE_MOD_ALLOW_INSPECT						// hayannal2009 2010-01-05 // #27007 Remove inspect block feature. Not completely removed from community options, just hidden in UI (so it can be restored later..)

// 2010. 12 -----------------------------------------------
#define PRE_FIX_26377								// Robust 2010-12-20 // #26377 [RB Ver.48] Issue where client lags for a long time before showing user guide popup page when talking to some NPCs

// 2010. 11 -----------------------------------------------

// One set like this~
#define PRE_ADD_SECONDARY_SKILL						// kimbap 2010-09-06 // Secondary skill system
#if defined( PRE_ADD_SECONDARY_SKILL )
#define PRE_ADD_COOKING_SYSTEM						// kwcsc 2010-10-30 // #17486 Add cooking system
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

// One mount set
// #define PRE_FIX_HIDE_VEHICLETAB // Rotha 2011-05-30 // Hide Mount and Pet inventory Status tab. 

//#define PRE_FIX_HITSTIFF // jhk8211 2010-12-09 // #25855 Prevent hitstiff during Frame Change/Frame Stop state effects. -> Removed per #31862 so designers can control it directly.
#define PRE_MOD_PROJECTILE_HACK						// siva 2010-11-24 // #24912 Fix projectile for auto-hack prevention.

#define PRE_ADD_LOGIN_USERCOUNT						// haling 2010-11-26 // #25021 LoginServer CCU (count only authenticated users in CCU.. kick users connected for 15+ minutes.)

#define PRE_ADD_NPC_REPUTATION_SYSTEM				// kimbap 2010-07-14 // NPC reputation system
#define PRE_MOD_WARRIOR_RIGHTHAND					// hayannal2009 2010-07-19 // Skip rendering right-hand glove mesh when Warrior equips gauntlet.
//#define PRE_ADD_VIP // kalliste 2010-06-08 // #15073 VIP Service
#define PRE_FIX_VIP_MAXPTS							// kalliste 2010-08-02 // #19094 Issue with points acquired by purchasing VIP items
#define PRE_ADD_VIP_CLEAR_BOX						// kalliste 2010-12-21 // #21982 [Monetization] Feature for VIP users to open 2 normal stage reward chests
//#define PRE_ADD_GACHA_JAPAN						// jhk8211 2010-04-21 // Disabled: absent in Steam NA v353 client/PAK
#define PRE_ADD_AIM_ASSIST_BUTTON					// hayannal2009 2010-03-30 // Aim assist button feature
//#define PRE_REMOVE_DOORS_UITEMP // kalliste 2012-09-24 // Temporarily remove app auth button to block user access in 27th build
//#define PRE_SWAP_QUICKSLOT // hayannal2009 2010-03-24 // Switch quickslot page with Ctrl key
//#define PRE_REMOVE_CASHSHOP // kalliste 2010-03-23 // #12308 Remove Cash Shop UI
//#define PRE_ADD_NO_HELP_DLG // kalliste 2011-06-07 // Disable Help dialog #35210 [FEEDBACK] Help Menu (J) should be disabled for CBT
// #define PRE_ADD_NO_REPUTATION_DLG // kalliste 2011-06-03 // Disable Friendship Album #34721 The friendship menu should be removed for CBT

//#if defined(_FINAL_BUILD)
#define PRE_MEMORY_SHUFFLE							// siva 2010-03-11 // Attached focusing on State values related to memory shuffle.
//#endif	// #if defined(_FINAL_BUILD)
//#define PRE_FIX_LOADING_TASK // mapping 2010-02-23 // Fixed loading task interfering with render frame
#define PRE_ADD_CYCLEREPORT							// 2hogi 2010-02-20 // Periodic state report from servicemanager to gsm

#if !defined( _FINAL_BUILD )
#define PRE_FILELOG									// kimbap 2010-02-19 // Leave server FileLog
#endif // #if defined( _FINAL_BUILD )
//#define PRE_ADD_CUTSCENE_ADD_SCALELOCK_INFO // jhk8211 2010-02-19 // Added Scale Lock feature to cutscene tool; handled in client cutscene task.
#define PRE_CRASH_CHECK_BACKUP						// mapping 2010-02-18 // For registering debug variables to catch crashes
#define	PRE_MONITOR_SUPER_NOTE						// kalliste 2010-02-15 // Money duplication check code
//#define PRE_PLAYER_CAMERA // realgaia 2010-02-02 // Player camera improvements

//#define PRE_TEST_PACKETMODULE // kimbap 2010-01-12 // Packet module test

//#define PRE_TRIGGER_LOG // kimbap 2009-12-24 // Added trigger logging -> logs once when game room is destroyed
//#define PRE_QUESTSCRIPT_LOG // kimbap 2009-12-16 // Quest script performance log
//#define PRE_TRIGGER_UNITTEST_LOG // kimbap 2009-12-15 // Trigger logging -> writes a file log whenever a trigger executes

#include "DNDefine_local.h"
#include "DNDefine_Work.h"
#include "DNDefine_Patch.h"

#define PRE_ADD_INVEN_EXTENSION						// hayannal2009 2012-01-06 // #49177 Prompt inventory expansion

//#########################################################################################################################################
// Define that must never be removed
//#########################################################################################################################################

#if defined(_CLIENT)
#define _PACKET_COMP

#endif	// _CLIENT

#if defined( _LOGINSERVER )

const int _SERVER_TYPEID = 1;

#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap, 2hogi 2012-02-13 // #no-issue Additional fix for dangling issue between send thread and worker thread

#define _SERVER
#define _USE_ACCEPTEX
#define _SERVERLOG

#elif defined(_DBSERVER)

#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap 2010-04-13 // #no-issue Fix SocketContext DanglingPtr
const int _SERVER_TYPEID = 2;

#define _SERVER
#define _SERVERLOG

#elif defined(_MASTERSERVER)

const int _SERVER_TYPEID = 3;

#define _SERVER
#define _SERVERLOG

#elif defined(_VILLAGESERVER)

#define _USE_SENDCONTEXTPOOL
#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap 2010-04-13 // #no-issue Fix SocketContext DanglingPtr

const int _SERVER_TYPEID = 4;

#define _SERVER
#define TIXML_USE_STL
#define _PACKET_COMP
#define _SERVERLOG

#elif defined(_GAMESERVER)
#define _USE_SENDCONTEXTPOOL
#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap 2010-04-13 // #no-issue Fix SocketContext DanglingPtr

const int _SERVER_TYPEID = 5;

#define _SERVER
#define TIXML_USE_STL
#define _PACKET_COMP
#define _SKIP_BLOCK
#define _SERVERLOG

#elif defined(_LOGSERVER)

#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap 2010-04-13 // #no-issue Fix SocketContext DanglingPtr

const int _SERVER_TYPEID = 6;

#define _SERVER

#elif defined(_CASHSERVER)

const int _SERVER_TYPEID = 7;

#define _SERVER

#elif defined(_SERVICEMANAGER)

const int _SERVER_TYPEID = 8;
#define PRE_FIX_SOCKETCONTEXT_DANGLINGPTR			// kimbap 2010-04-13 // #no-issue Fix SocketContext DanglingPtr, enabled by 2hogi

#define _SERVER

#else

const int _SERVER_TYPEID = 0;

#endif

//#define PRE_MOD_CREATE_CHAR // kwcsc 2010-10-18 // #18093 Separated only character creation part name change from the define below
//#define PRE_MOD_NAVIGATION_PATH // kwcsc 2010-08-31 // #19554 Auto-move feature in Map UI
//#define _TOOLCOMPILE

// Game server performance improvement define
#define PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER		// 2hogi 2011-12-21 // #48509 Remove MasterLock in game server
#if !defined (_SERVICEMANAGER) && !defined (_SERVICEMANAGER_EX)
#define PRE_DELETE_LOGLOCK							// kimbap 2011-12-21 // Remove Log Lock
#endif // #if !defined (_SERVICEMANAGER) && !defined (_SERVICEMANAGER_EX)
#define PRE_WORLDUSERSTATE_OPTIMIZE					// kimbap 2011-12-14 // WorldUserState game server optimization

#define PRE_MOD_MAGICALBREEZE_CHANGE_BLOW			// kwcsc 2012-09-05 // #67841 Remove physical->magical state effect conversion for Magical Breeze buffs
#define PRE_MOD_FROSTBITE_EXCEPTION					// kwcsc 2012-09-19 // #68598 Add exception handling to prevent frostbite while slowed after Icy Fraction activates normally