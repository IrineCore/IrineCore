USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 32
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103150 = ???? WorldID? DB? ?? World? ???? ????.
103185 = ?? ??? ??? ?????.
history :
24 = WorldID? ???? ???? ????? ?? at 2010-10-21
27 = (50729) ??? ?? ???? ?? ?? by ??? at 2012-01-30
29 = ??? ?? ??? ??ID ????? ?? by ??? at 2012-09-06 
30 ~ 31 = ??? ??? ???? ?? ?? ????? ???? ?? ?? by ??? at 2013-05-08
32 = ??? ??? ??? ?? ?? ???? ???? ?? ?? by ??? at 2013-08-21
**/

CREATE PROCEDURE [dbo].[P_AddCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @intAccountID int --// ??? ?? ID
	, @nvcAccountName nvarchar(50) --// ??? ?? ??
	, @inyAccountLevelCode tinyint --// 0=?? ??, 10=??(??), 20=????(??), 30=???(??), 99=QA, 100=???
	, @intWorldID int --// ?? ID
	, @nvcCharacterName nvarchar(30) --// ??? ??
	, @inyCharacterClassCode tinyint --// 1=WARRIER, 2=ARCHER, 3=SOCERESS, 4=CLERIC
	, @inyCharacterIndex tinyint --// ??? ?? ???
	, @intDefaultBody int --// ?? Body
	, @intDefaultLeg int --// ?? Leg
	, @intDefaultHand int --// ?? Hand
	, @intDefaultFoot int --// ?? Foot
	, @intHairColor int --// ?? ??
	, @intEyeColor int --// ? ??
	, @intSkinColor int --// ?? ??
	, @intLastMapID int --// ?? ? ID
	, @intPositionX int --// X ??
	, @intPositionY int --// Y ??
	, @intPositionZ int --// Z ??
	, @fltRotate float --// ????
	, @inyRebirthCoin tinyint --// ?? ?? ??
	, @inyPCRoomRebirthCoin tinyint --// PC? ?? ??
	, @bitBeginnerGuildFlag bit = NULL --// ????? ????
	, @intBeginnerGuildMaxMemberCount int  = NULL--// ????? ?? ?? ??
	, @nvcBeginnerGuildTitle nvarchar(45) = NULL --// ????? ????
	, @nvcBeginnerGuildMemo nvarchar(200) = NULL --// ????? ???		
	, @intJoinedGuildID int = NULL OUTPUT --// ??? ??? ??ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dtmNow datetime = GETDATE()
	, @inyCharacterLevel tinyint
	, @inyJobCode tinyint
	, @nvcGuildName nvarchar(50)
	, @nvcStmt nvarchar(4000)
	, @bitMobileAuthenticationFlag bit = CAST (0 AS bit);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF NOT EXISTS (SELECT * FROM dbo.WorldCoverage WITH (NOLOCK) WHERE WorldID = @intWorldID)
	BEGIN
		SET @intReturnValue = 103150;
		GOTO ErrorHandler;
	END
	
	SELECT TOP 1 @bitMobileAuthenticationFlag = ISNULL(MobileAuthenticationFlag, CAST(0 AS bit))
	FROM dbo.Characters WITH (NOLOCK, INDEX(IX_NNI_Characters_AccountID), FORCESEEK)
	WHERE AccountID = @intAccountID
	ORDER BY MobileAuthenticationFlag DESC;

	BEGIN TRANSACTION;

	/**_# [Characters] ???? INSERT???.*/
	INSERT dbo.Characters (CharacterID, AccountID, AccountName, AccountLevelCode, CharacterName, CharacterClassCode, CharacterIndex, VillageFirstVisitFlag, DefaultBody, DefaultLeg, DefaultHand, DefaultFoot, DeleteFlag, SkillResetFlag, WorldID, MobileAuthenticationFlag)
	VALUES (@inbCharacterID, @intAccountID, @nvcAccountName, @inyAccountLevelCode, @nvcCharacterName, @inyCharacterClassCode, @inyCharacterIndex, 0, @intDefaultBody, @intDefaultLeg, @intDefaultHand, @intDefaultFoot, 0, 0, @intWorldID, @bitMobileAuthenticationFlag);

	/**_# [CharacterStatus] ???? INSERT???.*/
	INSERT dbo.CharacterStatus (CharacterID, CharacterLevel, CharacterExp, MissionScore, JobCode, HairColor, EyeColor, SkinColor, LastMapID, LastVillageMapID, LastVillageGateNumber, PositionX, PositionY, PositionZ, Rotate, Coin, WarehouseCoin, LastRebirthCoinDate, RebirthCoin, PCRoomRebirthCoin, SkillPoint, LastFatigueDate, Fatigue, PCRoomFatigue, WeeklyFatigue, CheckSumBin, LastDarkLairVillageMapID, ViewCashEquip1Flag, ViewCashEquip2Flag, ViewCashEquipmentBitmap)
	VALUES (@inbCharacterID, 1, 0, 0, @inyCharacterClassCode, @intHairColor, @intEyeColor, @intSkinColor, @intLastMapID, 0, 0, @intPositionX, @intPositionY, @intPositionZ, @fltRotate, 0, 0, CAST(DATEADD(HOUR, 4, DATEADD(DAY, DATEDIFF(DAY, 0, @dtmNow) + CASE WHEN DATEPART(HOUR, @dtmNow) BETWEEN 0 AND 3 THEN 0 ELSE 1 END, 0)) AS smalldatetime), @inyRebirthCoin, @inyPCRoomRebirthCoin, 0, '1900-01-01', 0, 0, 0, 0, 0, 0, 0, 65535);

	/**_# [JobChangeLogs] ???? INSERT???.*/
	INSERT dbo.JobChangeLogs (CharacterID, JobCode, LogDate)
	VALUES (@inbCharacterID, @inyCharacterClassCode, @dtmNow);	

	COMMIT TRANSACTION;	

	/**_# ??? ??? ??? ?? ???? ???.*/	
	IF (@bitBeginnerGuildFlag IS NOT NULL AND @bitBeginnerGuildFlag = 1)
	BEGIN
		EXEC P_AddBeginnerGuildMember 
				@inbCharacterID = @inbCharacterID
			  , @inyCharacterLevel = 1
			  , @intGuildMaxMemberCount = @intBeginnerGuildMaxMemberCount
			  , @nvcGuildTitle = @nvcBeginnerGuildTitle
			  , @nvcGuildMemo = @nvcBeginnerGuildMemo
			  , @intJoinedGuildID = @intJoinedGuildID OUTPUT;
	END

	/**_# CharacterStarterItem? ?? ?? ?????.*/
	INSERT dbo.MaterializedItems (ItemSerial, ItemID, ItemRemainCount, PayMethodCode, SenderCharacterID, ItemMaterializeCode, ItemMaterializeFKey, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, ItemMaterializeDate, ItemLifespan, EternityFlag, ItemExpireDate, OwnerCharacterID, OwnershipStartDate, ExpireCompleteFlag, TradeItemFlag, AdditiveItemID, ItemLocationCode, ItemLocationIndex, ItemPotentialMoveCount, ItemCount, ItemPrice, DragonJewelType)
	SELECT ABS(CAST(CAST(NEWID() AS binary(8)) AS bigint)), ItemID, ItemCount, 5, NULL, 10, NULL, ItemDurability, RandomSeed, CoolTime, ItemLevel, ItemPotential, SoulBoundFlag, SealCount, ItemOption, @dtmNow, ItemLifespan, EternityFlag, CASE EternityFlag WHEN 0 THEN CASE WHEN ItemLifespan > 0 THEN DATEADD(MINUTE, ItemLifespan, @dtmNow) ELSE DATEADD(YEAR, 1, @dtmNow) END ELSE DATEADD(YEAR, 1, @dtmNow) END, @inbCharacterID, @dtmNow, 0, NULL, NULL, ItemLocationCode, ItemLocationIndex, 0, ItemCount, 0, NULL
	FROM dbo.CharacterStarterItems WITH (NOLOCK)
	WHERE (ProvideStartDate IS NULL AND ProvideEndDate IS NULL) 
		OR (ProvideStartDate <= @dtmNow AND ProvideEndDate >= @dtmNow)
		OR (ProvideStartDate IS NULL AND ProvideEndDate >= @dtmNow)
		OR (ProvideEndDate IS NULL AND ProvideStartDate <= @dtmNow);	

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2601 AND ERROR_MESSAGE() LIKE N'%IX_UNF_Characters_CharacterName%'
		SET @intReturnValue = 103185;

	GOTO ErrorHandler;
END CATCH;

RETURN 0;
ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
