USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListGiftSendScheduleReceive]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_GetListGiftSendScheduleReceive]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_GetListGiftSendScheduleReceive] AS' 
END
GO
/**
version : 2
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-23

description :
	?? ?? ?? ?? ??
result set : 
	GiftSendScheduleID / int
	CharacterID / bigint
	ReceiveDate / datetime
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	2 = ?? ??
**/

ALTER PROCEDURE [dbo].[P_GetListGiftSendScheduleReceive]
	  @intGiftSendScheduleID int --// ?? ?? ID
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

	SELECT C.CharacterID, C.CharacterName, S.ReceiveDate
	  FROM dbo.GiftSendScheduleReceive S WITH (NOLOCK)
	       INNER JOIN dbo.Characters C WITH (NOLOCK) ON (S.CharacterID = C.CharacterID)
	 WHERE GiftSendScheduleID = @intGiftSendScheduleID;

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
