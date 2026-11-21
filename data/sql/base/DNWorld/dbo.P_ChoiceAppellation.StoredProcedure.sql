USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ChoiceAppellation]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
103113 = ?? ??? ?????.
history :
2 = input param. @inyAppellationKind ?? at 2011-01-13
**/

CREATE PROCEDURE [dbo].[P_ChoiceAppellation]
	  @inbCharacterID bigint --// ??? ID
	, @intAppellationID int --// ?? ID
	, @inyAppellationKind tinyint = 1 --// 1=??? ??, 2=?? ??
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
	IF @inyAppellationKind = 1
	BEGIN
		UPDATE dbo.Appellations
		SET ChoiceFlag = 0
		WHERE CharacterID = @inbCharacterID AND ChoiceFlag = CAST(1 AS bit);

		UPDATE dbo.Appellations
		SET ChoiceFlag = 1
		WHERE CharacterID = @inbCharacterID AND AppellationID = @intAppellationID;
	END

	ELSE
	BEGIN
		UPDATE dbo.Appellations
		SET CoverChoiceFlag = 0
		WHERE CharacterID = @inbCharacterID AND CoverChoiceFlag = CAST(1 AS bit);

		UPDATE dbo.Appellations
		SET CoverChoiceFlag = 1
		WHERE CharacterID = @inbCharacterID AND AppellationID = @intAppellationID;
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
