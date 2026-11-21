USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModCharacterStatus]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 18
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-12
description : ??? Status? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
12 = ??? ?? LastQuestCheckDate? ??? ? ??? ?? at 2010-06-28
13 = LastQuestCheckDate? ?? ???? UPDATE??? ?? at 2010-06-29
14 = ?? MapID? ?? ? MapID? ???? MapStayLog? ???? ??? ?? at 2010-08-24
15 = MapStayLog ??? ?? ? ?? ??? ?? ??? ??? ???? ?? ?? ????? ?? at 2010-08-24
16 = ??? ?? ?? at 2011-03-21
17 = ??? ??? ?? ?? at 2011-03-29
18 = input param. @inyCurrentSkillCode ?? at 2011-04-06
**/

CREATE PROCEDURE [dbo].[P_ModCharacterStatus]
	  @inbCharacterID bigint --// ??? ID
	, @intModFieldBitmap int --// ??? ??? bitmap
	, @intHairColor int --// ?? ?? (0)
	, @intEyeColor int --// ? ?? (1)
	, @intSkinColor int --// ?? ?? (2)
	, @intLastMapID int --// ?? ? ID (3)
	, @intLastVillageMapID int --// ?? ??? ? ID (4)
	, @intLastDarkLairVillageMapID int --// ?? ???? ??? ? ID (5)
	, @inyLastVillageGateNumber tinyint --// ?? ??? ??? ?? (6)
	, @intPositionX int --// X ?? (7)
	, @intPositionY int --// Y ?? (8)
	, @intPositionZ int --// Z ?? (9)
	, @fltRotate float --// ???? (10)
	, @bitViewCashEquip1Flag bit --// ?? ??1? View ?? ?? ?? (11)
	, @bitViewCashEquip2Flag bit --// ?? ??2? View ?? ?? ?? (12)
	, @bitLastLogoutDate bit = NULL --// ??? ?? ???? ?? ?? ?? (13)
	, @bitLastLoginDate bit = NULL --// ??? ?? ??? ?? ?? ?? (14)
	, @sdtLastQuestCheckDate smalldatetime = NULL --// ??? ?? ??? ?? ?? ?? (15)
	, @inyCurrentSkillCode tinyint = 1 --// 1=1? ??, 2=2? ?? (16)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	, @dt2Now datetime2(0) = GETDATE()
	, @inyJobCode tinyint
	, @inyCharacterLevel tinyint
	, @dt2LastMapEntranceDateBefore datetime2(0)
	, @dt2LastMapEntranceDateAfter datetime2(0)
	, @intLastMapIDBefore int
	, @dt2LastLoginDate datetime2(0);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [CharacterStatus] ???? UPDATE???.*/
	UPDATE dbo.CharacterStatus
	SET @dt2LastMapEntranceDateBefore = LastMapEntranceDate
		, @inyJobCode = JobCode
		, @inyCharacterLevel = CharacterLevel
		, @intLastMapIDBefore = LastMapID
		, HairColor = CASE @intModFieldBitmap & POWER(2, 0) WHEN 0 THEN HairColor ELSE @intHairColor END
		, EyeColor = CASE @intModFieldBitmap & POWER(2, 1) WHEN 0 THEN EyeColor ELSE @intEyeColor END
		, SkinColor = CASE @intModFieldBitmap & POWER(2, 2) WHEN 0 THEN SkinColor ELSE @intSkinColor END
		, LastMapID = CASE @intModFieldBitmap & POWER(2, 3) WHEN 0 THEN LastMapID ELSE @intLastMapID END
		, LastVillageMapID = CASE @intModFieldBitmap & POWER(2, 4) WHEN 0 THEN LastVillageMapID ELSE @intLastVillageMapID END
		, LastDarkLairVillageMapID = CASE @intModFieldBitmap & POWER(2, 5) WHEN 0 THEN LastDarkLairVillageMapID ELSE @intLastDarkLairVillageMapID END
		, LastVillageGateNumber = CASE @intModFieldBitmap & POWER(2, 6) WHEN 0 THEN LastVillageGateNumber ELSE @inyLastVillageGateNumber END
		, PositionX = CASE @intModFieldBitmap & POWER(2, 7) WHEN 0 THEN PositionX ELSE @intPositionX END
		, PositionY = CASE @intModFieldBitmap & POWER(2, 8) WHEN 0 THEN PositionY ELSE @intPositionY END
		, PositionZ = CASE @intModFieldBitmap & POWER(2, 9) WHEN 0 THEN PositionZ ELSE @intPositionZ END
		, Rotate = CASE @intModFieldBitmap & POWER(2, 10) WHEN 0 THEN Rotate ELSE @fltRotate END
		, ViewCashEquip1Flag = CASE @intModFieldBitmap & POWER(2, 11) WHEN 0 THEN ViewCashEquip1Flag ELSE @bitViewCashEquip1Flag END
		, ViewCashEquip2Flag = CASE @intModFieldBitmap & POWER(2, 12) WHEN 0 THEN ViewCashEquip2Flag ELSE @bitViewCashEquip2Flag END
		, LastLogoutDate = CASE WHEN @intModFieldBitmap & POWER(2, 13) > 0 AND @bitLastLogoutDate = 1 THEN GETDATE() ELSE LastLogoutDate END
		, @dt2LastLoginDate = LastLoginDate = CASE WHEN @intModFieldBitmap & POWER(2, 14) > 0 AND @bitLastLoginDate = 1 THEN GETDATE() ELSE LastLoginDate END
		, @dt2LastMapEntranceDateAfter = LastMapEntranceDate  = 
			CASE @intModFieldBitmap & POWER(2, 3)
				WHEN 0 THEN LastMapEntranceDate
				ELSE
					CASE LastMapID
						WHEN @intLastMapID THEN LastMapEntranceDate
						ELSE @dt2Now
					END
			END
		, LastQuestCheckDate = CASE WHEN @intModFieldBitmap & POWER(2, 15) > 0 THEN @sdtLastQuestCheckDate ELSE LastQuestCheckDate END
		, CurrentSkillCode = CASE WHEN @intModFieldBitmap & POWER(2, 16) > 0 THEN @inyCurrentSkillCode ELSE CurrentSkillCode END
	WHERE CharacterID = @inbCharacterID;

	/**_# ???? ???? ??? 103102? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103102;
		GOTO ErrorHandler;
	END

	SET @dt2LastMapEntranceDateBefore = CASE WHEN @dt2LastLoginDate > @dt2LastMapEntranceDateBefore THEN @dt2LastLoginDate ELSE @dt2LastMapEntranceDateBefore END;

	IF @intModFieldBitmap & POWER(2, 3) > 0 AND @dt2LastMapEntranceDateBefore < @dt2LastMapEntranceDateAfter AND @intLastMapIDBefore <> @intLastMapID
	BEGIN
		EXEC @intReturnValue = dbo.P_AddMapStayLog
			  @dt2ExitDate = @dt2Now
			, @intMapID = @intLastMapIDBefore
			, @inbCharacterID = @inbCharacterID
			, @inyJobCode = @inyJobCode
			, @inyCharacterLevel = @inyCharacterLevel
			, @dt2EntranceDate = @dt2LastMapEntranceDateBefore;
	END
END TRY
BEGIN CATCH
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
