USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFatigueLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-14
description : ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
4 = @inyFatigueTypeCode ????? ?? ?? at 2010-06-11
**/

CREATE PROCEDURE [dbo].[P_AddFatigueLog]
	  @dt2LogDate datetime2(0) --// ?? ??
	, @inbCharacterID bigint --// ??? ID
	, @inyFatigueTypeCode tinyint --// 1=PC? ???, 2=?? ???, 3=?? ???, 4=??? ???, 5=VIP ???
	, @insCurrentFatigue smallint --// ?? ???
	, @insChangedFatigue smallint --// ??? ???
	, @intChannelID int --// ???? ??? ?? ID
	, @intMapID int --// ???? ??? ? ID
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
	IF @insChangedFatigue = 0
		RETURN 0;

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);

	/**_# [FatigueLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.FatigueLogs0 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
		VALUES (@dt2LogDate, @inbCharacterID, @inyFatigueTypeCode, @insCurrentFatigue, @insChangedFatigue, @intChannelID, @intMapID);

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.FatigueLogs1 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
		VALUES (@dt2LogDate, @inbCharacterID, @inyFatigueTypeCode, @insCurrentFatigue, @insChangedFatigue, @intChannelID, @intMapID);

	ELSE
		INSERT dbo.FatigueLogs2 (LogDate, CharacterID, FatigueTypeCode, CurrentFatigue, ChangedFatigue, ChannelID, MapID)
		VALUES (@dt2LogDate, @inbCharacterID, @inyFatigueTypeCode, @insCurrentFatigue, @insChangedFatigue, @intChannelID, @intMapID);

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
