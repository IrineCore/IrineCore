USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListBlockedCharacterForGSM]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-18
description : ?? ??? ??? ?????.
result set : AccountID/int, AccountName/nvarchar, CharacterID/bigint, CharacterName/nvarchar, JobCode/tinyint, CharacterLevel/tinyint, BlockReason/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103141 = ??? ???? ????.
history:
3 = ?? ?? ????? ?? by ??? at 2013-06-24
**/

CREATE PROCEDURE [dbo].[P_GetListBlockedCharacterForGSM]
	  @inbCharacterID bigint --// ??? ID
	, @inyListType tinyint --// 1=?? ???? ??? ???, 2= ?? ???? ??? ???
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
	IF @inyListType = 1
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CH.JobCode, CH.CharacterLevel, BC.BlockReason
		FROM dbo.BlockedCharacters BC WITH (NOLOCK)
			INNER JOIN dbo.V_Characters CH WITH (NOLOCK) ON BC.BlockedCharacterID = CH.CharacterID
		WHERE BC.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);

	ELSE IF @inyListType = 2
		SELECT CH.AccountID, CH.AccountName, CH.CharacterID, CH.CharacterName, CH.JobCode, CH.CharacterLevel, BC.BlockReason
		FROM dbo.BlockedCharacters BC WITH (NOLOCK)
			INNER JOIN dbo.V_Characters CH WITH (NOLOCK) ON BC.CharacterID = CH.CharacterID
		WHERE BC.BlockedCharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103141;
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
