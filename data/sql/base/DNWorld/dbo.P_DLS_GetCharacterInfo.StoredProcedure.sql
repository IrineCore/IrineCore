USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetCharacterInfo]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : dailykim@eyedentitygames.com
created date : 2013-04-25
description : ??? ??? ?????.
result set : CharacterID/bigint, AccountID/int, AccountName/nvarchar, AccountLevelCode/tinyint, CharacterName/nvarchar, CharacterClassCode/tinyint, DeleteFlag/bit, DeleteDate/datetime, WorldID/tinyyint, CreateDate/datetime, MobileAuthenticationFlag/bit, PrivateChannelID/int, CharacterLevel/tinyint, JobCode/tinyint, CharacterExp/int, Coin/bigint, WarehouseCoin/bigint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetCharacterInfo]
	  @inbCharacterID bigint = NULL --// ??? ID
	, @nvcCharacterName nvarchar(30) = NULL --// ??? ??	
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
	/**_# [Characters] ???? SELECT???.*/
	IF @inbCharacterID IS NULL AND @nvcCharacterName IS NOT NULL
		SELECT CH.CharacterID, CH.AccountID, CH.AccountName, CH.AccountLevelCode, CH.CharacterName, CH.CharacterClassCode, CH.DeleteFlag, CH.DeleteDate, CH.WorldID, CH.CreateDate
             , CH.MobileAuthenticationFlag, CH.PrivateChannelID, CS.CharacterLevel, CS.JobCode, CS.CharacterExp, CS.Coin, CS.WarehouseCoin
		FROM dbo.Characters CH WITH (NOLOCK) INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		WHERE CH.CharacterName = @nvcCharacterName;
	ELSE IF @inbCharacterID IS NOT NULL AND @nvcCharacterName IS NULL	
		SELECT CH.CharacterID, CH.AccountID, CH.AccountName, CH.AccountLevelCode, CH.CharacterName, CH.CharacterClassCode, CH.DeleteFlag, CH.DeleteDate, CH.WorldID, CH.CreateDate
             , CH.MobileAuthenticationFlag, CH.PrivateChannelID, CS.CharacterLevel, CS.JobCode, CS.CharacterExp, CS.Coin, CS.WarehouseCoin
		FROM dbo.Characters CH WITH (NOLOCK) INNER JOIN dbo.CharacterStatus CS WITH (NOLOCK) ON CH.CharacterID = CS.CharacterID
		WHERE CH.CharacterID = @inbCharacterID;
END TRY
BEGIN CATCH	
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
