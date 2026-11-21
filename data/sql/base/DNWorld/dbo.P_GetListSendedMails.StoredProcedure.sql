USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListSendedMails]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2011-09-23

description :
	?? ?? ?? ?? ??
result set : 
	CharacterID / bigint
	MailScheduleID / int
	SendDate / datetime2(0)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	3 = ?? ??
**/

CREATE PROCEDURE [dbo].[P_GetListSendedMails]
	  @intMailScheduleID int --// ?? ?? ID
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

	SELECT C.CharacterID, C.CharacterName, S.SendDate AS ReceiveDate
	  FROM dbo.SendedMails S WITH (NOLOCK)
	       INNER JOIN dbo.Characters C WITH (NOLOCK) ON (S.CharacterID = C.CharacterID)
	 WHERE MailScheduleID = @intMailScheduleID;

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
