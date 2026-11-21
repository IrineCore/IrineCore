USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetFieldCountByCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-28
description : ???? ??? ??? ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetFieldCountByCharacter]
	  @inbCharacterID bigint --// ???? ?? ??? ID
	, @intFarmID int --// ?? ID (?? ??? ???? ???? ????? ?? NULL? ?????.)
	, @intFieldCount int OUTPUT --// ??? ?
	, @intFieldForCharacterCount int = 0 OUTPUT --// ?? ??? ?
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
	IF @intFarmID IS NOT NULL
		SELECT @intFieldCount = FieldCount
		FROM dbo.VIX_FieldCountByCharacter WITH (NOLOCK, NOEXPAND)
		WHERE CharacterID = @inbCharacterID AND FarmID = @intFarmID;

	ELSE
		SELECT @intFieldCount = SUM(FieldCount)
		FROM dbo.VIX_FieldCountByCharacter WITH (NOLOCK, NOEXPAND)
		WHERE CharacterID = @inbCharacterID;

	IF @intFieldCount IS NULL
		SET @intFieldCount = 0; 

	SELECT @intFieldForCharacterCount = COUNT(*)
	FROM dbo.FieldsForCharacter WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;
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
