USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGuildRecruitment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 12
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-18

description :
	??? ????? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103238 = ??? ???? ????.
	103319 = ?? ?? ??? ?? ???? ?????.
history :
	8 = input parameter ?? (????, ????)     by ??? at 2012-05-06
	9~10 = GuildRecruitments ???? MinLevel, MaxLevel ?? ??  by ??? at 2012-05-10
	11 = ?? ???? ?? ??? ?? ??     by ??? at 2012-05-24
	12 = GuildRecruitments ? GuildRecruitmentOptions ??? ??     by ??? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_AddGuildRecruitment]
	  @intGuildID int --// ?? ID
	, @nvcGuildIntroduction nvarchar(max) --// ?? ???
	, @vchCharacterClassCodes varchar(100) = '' --// ,? ?????? CharacterClassCode ???
	, @inyMinLevel tinyint = 0 --//?? ??
	, @inyMaxLevel tinyint = 0 --// ?? ??
	, @inyPurposeCode tinyint = 1 --// ???? (1=??, 2=??, 3=???, 4=????)
	, @bitGuildHomepageURLFlag bit = 0 --// ?? ???? ?? ??? ?? (0=????, 1=???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @inyCharacterClassCode tinyint = 0
	  , @dt2RegisterDate datetime2(0) = GETDATE()
	  , @intCharacterClassCodeBitmap int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT @intCharacterClassCodeBitmap = SUM(POWER(2,CAST(string AS tinyint)-1))
    FROM dbo.TF_Split(@vchCharacterClassCodes, ',')
	OPTION (KEEPFIXED PLAN);

	/**_# [GuildRecruitment] ???? INSERT???.*/
	INSERT dbo.GuildRecruitments (GuildID, GuildIntroduction, PurposeCode, MinLevel, MaxLevel, GuildHomepageURLFlag, CharacterClassCodeBitmap, GuildName, GuildLevel)
	SELECT GuildID, @nvcGuildIntroduction, @inyPurposeCode, @inyMinLevel, @inyMaxLevel, @bitGuildHomepageURLFlag, @intCharacterClassCodeBitmap, GuildName, GuildLevel
	FROM dbo.Guilds WITH(UPDLOCK, INDEX(PK_Guilds), FORCESEEK)
	WHERE GuildID = @intGuildID AND CloseFlag = CAST(0 AS bit)
	OPTION (KEEPFIXED PLAN);
	
	/**_## ?? ??? ???? ??? 103238? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
	END
	
	/**_# ?? ??? ?????.*/
	INSERT dbo.GuildHistory (GuildID, GuildHistoryCode, CharacterID, CharacterLevel, RegisterDate)
	VALUES (@intGuildID, 12, NULL, NULL, @dt2RegisterDate)
	OPTION (KEEPFIXED PLAN);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_GuildRecruitments%'
		SET @intReturnValue = 103319;

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
