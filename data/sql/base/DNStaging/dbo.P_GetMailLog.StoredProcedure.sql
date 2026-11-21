USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetMailLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-03-28
description : ?? ??? ?????.
result set : SendDate/datetime2, MailTypeCode/tinyint, SenderCharacterID/bigint, ReceiverCharacterID/bigint, Subject/nvarchar, AttachCoin/bigint, AttachItemFlag/bit
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3~4 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
**/

CREATE PROCEDURE [dbo].[P_GetMailLog]
	  @dt2SendDate datetime2(0) = NULL --// ?? ?? ??
	, @dt2ReceiveDate datetime2(0) = NULL --// ?? ?? ??
	, @inbMailID bigint --// ?? ID
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@nvcStmt nvarchar(max),
	@nvcDBName nvarchar(50) = N'DNStaging',
	@nvcTblName nvarchar(50) = N'MailLogs',
	@nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), COALESCE(@dt2SendDate, DATEADD(DAY, -7, @dt2ReceiveDate), GETDATE()), 112),
	@nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), COALESCE(@dt2SendDate, @dt2ReceiveDate, GETDATE()), 112),
	@nvcHint nvarchar(50) = N'NOLOCK',
	@nvcStmtOUT nvarchar(max) = N'';
	
/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;	
	
	/**_# ?? ??? ?????.*/
	SET @nvcStmt = N'
SELECT SendDate, MailTypeCode, SenderCharacterID, ReceiverCharacterID, Subject, AttachCoin, AttachItemFlag 
FROM (' + @nvcStmtOUT + N') V
WHERE MailID = @inbMailID' +
	CASE
		WHEN @dt2SendDate IS NOT NULL THEN N' AND SendDate >= ''' + CONVERT(nvarchar(19), @dt2SendDate, 121) + N''''
		WHEN @dt2ReceiveDate IS NOT NULL THEN N' AND SendDate < ''' + CONVERT(nvarchar(19), DATEADD(DAY, 1, @dt2ReceiveDate), 121) + N''' AND SendDate > ''' + CONVERT(nvarchar(19), DATEADD(DAY, -7, @dt2ReceiveDate), 121) + N''''
	END;

	EXEC sys.sp_executesql @nvcStmt, N'@inbMailID bigint', @inbMailID = @inbMailID;
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
