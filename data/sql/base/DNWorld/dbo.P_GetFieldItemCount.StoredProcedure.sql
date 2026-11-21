USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetFieldItemCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-02-22
description : ?? ???? ?? ?? ?? Item? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetFieldItemCount]
	  @inbCharacterID bigint --// ???? ?? ??? ID
	, @intItemID int --// ?? ???? ??? ID
	, @intItemCount int OUTPUT --// ?? ??? ??
	, @intItemForCharacterCount int = 0 OUTPUT --// ?? ???? ?? ??? ??
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
	SELECT @intItemCount = COUNT(*)
	FROM dbo.Fields WITH (NOLOCK, INDEX(IX_NN_Fields_CharacterID), FORCESEEK)
	WHERE CharacterID = @inbCharacterID AND ItemID = @intItemID
	OPTION (KEEPFIXED PLAN);

	SELECT @intItemForCharacterCount = COUNT(*)
	FROM dbo.FieldsForCharacter WITH (NOLOCK, INDEX(PK_FieldsForCharacter), FORCESEEK)
	WHERE CharacterID = @inbCharacterID AND ItemID = @intItemID
	OPTION (KEEPFIXED PLAN);
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
