USE [dngsm]
GO
/****** Object:  StoredProcedure [dbo].[P_AddGMCopyLog]    Script Date: 11/14/2026 8:47:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
	
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-02-19
description : ?? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddGMCopyLog]
	  @dtmLogDate datetime --// 
	, @inbSessionID bigint --// 
	, @nvcRestrictType nvarchar(50) --// 
	, @inbInitCoin bigint --// 
	, @inbCurrentCoin bigint --// 
	, @intCharacterLevel int --// 
	, @inbBanAmount bigint --// 
	, @inbIllegalAmount bigint --// 
	, @vchIP varchar(15) --// 
	, @intGMCopyLogID int = NULL OUTPUT --// 
WITH EXECUTE AS 'SPExecutor'
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
	/**_# [GMCopyLog] ???? INSERT???.*/
	INSERT dbo.GMCopyLog (LogDate, SessionID, RestrictType, InitCoin, CurrentCoin, CharacterLevel, BanAmount, IllegalAmount, IP)
	VALUES (@dtmLogDate, @inbSessionID, @nvcRestrictType, @inbInitCoin, @inbCurrentCoin, @intCharacterLevel, @inbBanAmount, @inbIllegalAmount, dbo.FN_IP2IPNumber(@vchIP));
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
