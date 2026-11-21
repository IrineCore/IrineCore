USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListBlockedCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-18
description : ??? ?? ??? ?????.
result set : 
	version = 1 : BlockedCharacterID/bigint, BlockedCharacterName/nvarchar
	version = 2 : BlockedCharacterID/bigint, BlockedCharacterName/nvarchar, BlockReason/int
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103141 = ??? ???? ????.
history :
4 = Version ?? ? ?? ?? ????? ?? by ??? at 2013-06-24
**/

CREATE PROCEDURE [dbo].[P_GetListBlockedCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @intVersion int = 1 --// ??? SP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @i int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @intVersion IS NULL OR @intVersion = 1
		SELECT BCH.BlockedCharacterID, CH.CharacterName AS BlockedCharacterName
		FROM dbo.BlockedCharacters BCH WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON BCH.BlockedCharacterID = CH.CharacterID
		WHERE BCH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);
	ELSE IF @intVersion = 2
		SELECT BCH.BlockedCharacterID, CH.CharacterName AS BlockedCharacterName, BlockReason
		FROM dbo.BlockedCharacters BCH WITH (NOLOCK)
			INNER JOIN dbo.Characters CH WITH (NOLOCK) ON BCH.BlockedCharacterID = CH.CharacterID
		WHERE BCH.CharacterID = @inbCharacterID AND CH.DeleteFlag = CAST(0 AS bit);

	IF @i <> @@ROWCOUNT
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
