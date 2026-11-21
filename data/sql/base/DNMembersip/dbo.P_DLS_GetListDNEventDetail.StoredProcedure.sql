USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetListDNEventDetail]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_GetListDNEventDetail]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_GetListDNEventDetail] AS' 
END
GO
/**
version : 8
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ?? ??
result set :
	DNEventDetailID / int
	EventName / nvarchar(50)
	EventType1 / tinyint
	EventType2 / tinyint
	EventType3 / tinyint
	EventAtt1 / int
	EventAtt2 / int
	EventAtt3 / int
	EventAtt4 / int
	EventAtt5 / int
	MapIDs / nvarchar(100)
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	7 = ?? ??
**/

ALTER PROCEDURE [dbo].[P_DLS_GetListDNEventDetail]
	  @intDNEventDetailID int = NULL --// ??? ?? ID (NULL : ???? ??? ?? ??? ?? ??)
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

	/**_# ??? ?? ?? ?? */
	IF @intDNEventDetailID IS NULL
	BEGIN
		SELECT DNEventDetailID, EventName, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5, MapIDs
		  FROM dbo.DNEventDetails WITH (NOLOCK)
		 WHERE DeleteFlag = CONVERT(bit, 0);
	END
	ELSE
	BEGIN
		SELECT DNEventDetailID, EventName, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5, MapIDs
		  FROM dbo.DNEventDetails WITH (NOLOCK)
		 WHERE DNEventDetailID = @intDNEventDetailID;
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
