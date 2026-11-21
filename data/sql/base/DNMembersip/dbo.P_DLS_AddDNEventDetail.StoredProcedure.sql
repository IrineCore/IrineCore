USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_AddDNEventDetail]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DLS_AddDNEventDetail]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DLS_AddDNEventDetail] AS' 
END
GO
/**
version : 11
author : ???
e-mail : wgkim@eyedentitygames.com
created date : 2012-11-08

description :
	??? ?? ??
result set :
	@intDNEventDetailID / int
return value :
	0 = ??? ????.
	1 = ????? Commit? ? ?? ?????. ????? Rollback???.
	100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
	10 = ????
	11 = ?ID ??
**/

ALTER PROCEDURE [dbo].[P_DLS_AddDNEventDetail]
	  @nvcEventName nvarchar(50) --// ??? ??
	, @inyEventType1 tinyint --// ??? ??1 (1:????, 2:???)
	, @inyEventType2 tinyint = NULL --// ??? ??2 (0:??, 1:???, 2:??, 3:????, 4:???, 5:????, 6:??)
	, @inyEventType3 tinyint = NULL --// ??? ??3
	, @intEventAtt1 int --// ??? ??1
	, @intEventAtt2 int = NULL --// ??? ??2
	, @intEventAtt3 int = NULL --// ??? ??3
	, @intEventAtt4 int = NULL --// ??? ??4
	, @intEventAtt5 int = NULL --// ??? ??5
	, @intDNEventDetailID int = NULL OUTPUT --// ??? ?? ID
	, @nvcMapIDs nvarchar(100) = NULL --// ?? ?ID ex)1000,1001
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

	/**_# ??? ?? ?? */
	INSERT dbo.DNEventDetails (EventName, EventType1, EventType2, EventType3, EventAtt1, EventAtt2, EventAtt3, EventAtt4, EventAtt5, DeleteFlag, MapIDs)
	VALUES (@nvcEventName, @inyEventType1, @inyEventType2, @inyEventType3, @intEventAtt1, @intEventAtt2, @intEventAtt3, @intEventAtt4, @intEventAtt5, CONVERT(bit, 0), @nvcMapIDs);

	SET @intDNEventDetailID = SCOPE_IDENTITY();

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
