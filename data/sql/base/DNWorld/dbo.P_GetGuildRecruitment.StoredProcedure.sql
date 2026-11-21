USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetGuildRecruitment]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 13
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-11-18

description :
	?? ??? ??? ??? ?? ??? ?????.
result set :
	@intVersion < 6 : GuildIntroduction/nvarchar(max), MinLevel/int, MaxLevel/int, CharacterClassCode/tinyint
	@intVersion = 6 : GuildIntroduction/nvarchar(max), MinLevel/int, MaxLevel/int, CharacterClassCode/tinyint, PurposeCode/tinyint, GuildHomepageURLFlag/bit
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	6 = result set ?? (????, ????)     by ??? at 2012-05-08
	7 = GuildRecruitments ???? MinLevel, MaxLevel ?? ??  by ??? at 2012-05-10
	8 = ??? ???? ?? ??   by ??? at 2012-05-10
	9 = ?? ???? ?? ??? ?? ??     by ??? at 2012-05-24
	10 = input parameter ?? (SP ??)     by ??? at 2012-05-24
	11 = 10 ?? ??     by ??? at 2012-05-24
	12 = result set ?? (Homepage)     by ??? at 2012-05-24
	13 = GuildRecruitmentOptions ??? ??? ?? ??     by ??? at 2013-06-25
**/

CREATE PROCEDURE [dbo].[P_GetGuildRecruitment]
	  @intGuildID int --// ?? ID
	, @intVersion int = 5 --// SP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intVersion < 6
	BEGIN
		SELECT GuildIntroduction, MinLevel, MaxLevel, B.n AS CharacterClassCode
		FROM dbo.GuildRecruitments A WITH(NOLOCK, INDEX(PK_GuildRecruitments), FORCESEEK)
			CROSS JOIN dbo.Tally B
		WHERE A.GuildID = @intGuildID
          AND B.n BETWEEN 1 AND 10
          AND A.CharacterClassCodeBitmap & POWER(2,n-1) > 0
		OPTION (KEEPFIXED PLAN);
	END
	ELSE IF @intVersion = 6
	BEGIN
		SELECT A.GuildIntroduction, A.MinLevel, A.MaxLevel, B.n AS CharacterClassCode, A.PurposeCode, A.GuildHomepageURLFlag
		FROM dbo.GuildRecruitments A WITH(NOLOCK, INDEX(PK_GuildRecruitments), FORCESEEK)
			CROSS JOIN dbo.Tally B
		WHERE A.GuildID = @intGuildID
          AND B.n BETWEEN 1 AND 10
          AND A.CharacterClassCodeBitmap & POWER(2,n-1) > 0
		OPTION (KEEPFIXED PLAN);
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
