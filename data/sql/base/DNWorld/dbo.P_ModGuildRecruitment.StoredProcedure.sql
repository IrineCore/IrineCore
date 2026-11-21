USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModGuildRecruitment]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 9
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-18

description :
	?? ?? ??? ?????.
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
	103238 = ??? ???? ????.
	103295 = ?? ?? ??? ?? ???? ?????.
history :
	6 = input parameter ?? (????, ????)     by ??? at 2012-05-06
	7 = GuildRecruitments ???? MinLevel, MaxLevel ?? ??  by ??? at 2012-05-10
	8 = ?? ???? ?? ??? ?? ??     by ??? at 2012-05-24
	9 = GuildRecruitmentOptions ???? ??? ??      by ??? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_ModGuildRecruitment]
	  @intGuildID int --// ?? ID
	, @nvcGuildIntroduction nvarchar(max) --// ?? ???
	, @vchCharacterClassCodes varchar(100) = '' --// ,? ?????? JobCode ???
	, @inyMinLevel tinyint = 0 --// ?? ??
	, @inyMaxLevel tinyint = 0 --// ?? ??
	, @inyPurposeCode tinyint = NULL --// ???? (1=??, 2=??, 3=???, 4=????)
	, @bitGuildHomepageURLFlag bit = 0 --// ?? ???? ?? ??? ?? (0=????, 1=???)
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;
DECLARE @inyCharacterClassCode tinyint = 0
	  , @intCharacterClassCodeBitmap int;

DECLARE @tblCharacterClassCode table (
	CharacterClassCode tinyint 		
);

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

	/**_# [GuildRecruitment] ???? UPDATE???. (?? ??? ??)*/
	UPDATE dbo.GuildRecruitments
	SET GuildIntroduction = @nvcGuildIntroduction
		, PurposeCode = ISNULL(@inyPurposeCode, PurposeCode)
		, MinLevel = @inyMinLevel
		, MaxLevel = @inyMaxLevel
		, GuildHomepageURLFlag = @bitGuildHomepageURLFlag
		, CharacterClassCodeBitmap = @intCharacterClassCodeBitmap
	FROM dbo.GuildRecruitments WITH (INDEX(PK_GuildRecruitments), FORCESEEK)
	WHERE GuildID = @intGuildID
	OPTION (KEEPFIXED PLAN);
	
	/**_## ?? ??? ???? ??? 103238? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103238;
		GOTO ErrorHandler;
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
