USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddSkillLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ??, ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
6 = input param. @inySkillCode ?? at 2011-04-06
**/

CREATE PROCEDURE [dbo].[P_AddSkillLog]
	  @dt2LogDate datetime2(0) --// ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @inySkillChangeCode tinyint --// 2=?? ?? ??, 3=????? ??, 4=?? ?? (???), 5=?? ?? (?? ??), 6=?? ?? (???), 7=?? ?? (???), 8=?? ?? (by ???), 9=?? ?? (??), 10=?? ?? (???), 11=?? ?? (??? ??), 12=?? ?? (by ???), 13=???? ?? (by ???), 14=?? ?? (??)
	, @intSkillID int --// ?? ID
	, @inySkillLevelBefore tinyint --// ?? ? ?? ??
	, @inySkillLevelAfter tinyint --// ?? ? ?? ??
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
	, @inySkillCode tinyint = 1 --// 1=1? ??, 2=2? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int;
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inySkillLevelBefore = @inySkillLevelAfter
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [SkillLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.SkillLogs0 (LogDate, CharacterID, SkillChangeCode, SkillID, SkillLevelBefore, SkillLevelAfter, ChannelID, MapID, SkillCode)
		VALUES (@dt2LogDate, @inbCharacterID, @inySkillChangeCode, @intSkillID, @inySkillLevelBefore, @inySkillLevelAfter, @intChannelID, @intMapID, @inySkillCode);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.SkillLogs1 (LogDate, CharacterID, SkillChangeCode, SkillID, SkillLevelBefore, SkillLevelAfter, ChannelID, MapID, SkillCode)
		VALUES (@dt2LogDate, @inbCharacterID, @inySkillChangeCode, @intSkillID, @inySkillLevelBefore, @inySkillLevelAfter, @intChannelID, @intMapID, @inySkillCode);

	ELSE
		INSERT dbo.SkillLogs2 (LogDate, CharacterID, SkillChangeCode, SkillID, SkillLevelBefore, SkillLevelAfter, ChannelID, MapID, SkillCode)
		VALUES (@dt2LogDate, @inbCharacterID, @inySkillChangeCode, @intSkillID, @inySkillLevelBefore, @inySkillLevelAfter, @intChannelID, @intMapID, @inySkillCode);

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
