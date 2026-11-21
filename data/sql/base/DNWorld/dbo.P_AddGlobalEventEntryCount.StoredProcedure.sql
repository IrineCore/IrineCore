USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGlobalEventEntryCount]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-02-24

description :
	??? ??? ??
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGlobalEventEntryCount]
	  @inbCharacterID bigint --// ???ID
	, @intGlobalEventScheduleID int --// ????????ID
	, @intEntryCount int --// ????
	, @intEntryCountSum int = NULL OUTPUT --// ?? ????
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

	-- ??? ??? ????? ?? ? ??
	UPDATE dbo.GlobalEventEntryCount
	   SET EntryCount = EntryCount + @intEntryCount
	 WHERE (CharacterID = @inbCharacterID)
	   AND (GlobalEventScheduleID = @intGlobalEventScheduleID);

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.GlobalEventEntryCount (CharacterID, GlobalEventScheduleID, EntryCount)
		VALUES (@inbCharacterID, @intGlobalEventScheduleID, @intEntryCount);
	END

	-- ?? ??? ???? ?? ????? ??
	SELECT @intEntryCountSum = EntryCountSum
	  FROM dbo.VIX_GlobalEventEntryCount WITH (NOLOCK, NOEXPAND)
	 WHERE (GlobalEventScheduleID = @intGlobalEventScheduleID);

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
