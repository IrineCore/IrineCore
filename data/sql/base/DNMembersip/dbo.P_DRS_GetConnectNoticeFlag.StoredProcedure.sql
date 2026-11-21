USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_DRS_GetConnectNoticeFlag]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_DRS_GetConnectNoticeFlag]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_DRS_GetConnectNoticeFlag] AS' 
END
GO
/**
version : 9
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2012-10-31
description : ?? ?? ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
1030000016 = ??? ???? ????.
**/

ALTER PROCEDURE [dbo].[P_DRS_GetConnectNoticeFlag]
	  @intAccountID int --// ??? ?? ID	
	, @bitConnectNoticeFlag bit = NULL OUTPUT  --// ??????????	
	, @dt2LastLoginDate datetime2(0) = NULL OUTPUT  --// ??? ??? ??
	, @bitConnectFlag bit = NULL OUTPUT  --// ?? ??
	, @inbConnectedCharacterID bigint = NULL OUTPUT  --// ?? ??? ID
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
	/**_# [Accounts] ???? SELECT ???.*/
	SELECT @bitConnectNoticeFlag = ISNULL(AC.ConnectNoticeFlag, CAST(0 AS bit)), @dt2LastLoginDate = AC.LastLoginDate,
		   @bitConnectFlag = CASE WHEN AU.CertifyingStep = 0 OR AU.CertifyingStep IS NULL THEN CAST(0 AS bit) ELSE CAST(1 AS bit) END,
		   @inbConnectedCharacterID = ISNULL(AU.CharacterDBID, 0)
	  FROM dbo.Accounts AC WITH (NOLOCK) LEFT OUTER JOIN dbo.DNAuth AU WITH (NOLOCK) ON AC.AccountID = AU.AccountDBID
	 WHERE AC.AccountID = @intAccountID;

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 1030000016;
		GOTO ErrorHandler;
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
