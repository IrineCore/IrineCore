USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListMailLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-04-21
description : ETL? ?? ??? ?????.
result set : SendDate, MailID, MailTypeCode, DeliveryTypeCode, SenderCharacterID, SenderName, ReceiverCharacterID, Subject, AttachCoin, AttachItemFlag, WorldID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListMailLogForETL]
	  @dtmBasisDate datetime
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	SELECT SendDate, MailID, MailTypeCode, DeliveryTypeCode, SenderCharacterID, CAST(LEFT(SenderName, 30) AS nvarchar(30)) AS SenderName, ReceiverCharacterID, [Subject], AttachCoin
		, CAST(CASE WHEN EXISTS (SELECT * FROM dbo.MailAttachItems WITH (NOLOCK) WHERE MailID = M.MailID) THEN 1 ELSE 0 END AS bit) AS AttachItemFlag, @intWorldID AS WorldID
	FROM dbo.Mails M WITH (NOLOCK)
	WHERE SendDate >= CAST(@dtmBasisDate AS datetime2(0)) AND SendDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
	OPTION (MAXDOP 1);
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
