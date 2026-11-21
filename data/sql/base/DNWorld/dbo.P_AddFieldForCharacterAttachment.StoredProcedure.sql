USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFieldForCharacterAttachment]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-27
description : ?? ????? ???? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddFieldForCharacterAttachment]
	  @inbCharacterID int --// ??? ID
	, @insFieldIndex smallint --// ?? ID
	, @intAttachedItemID int --// ??? ??? ID
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
	UPDATE dbo.FieldForCharacterAttachments
	SET AttachedItemCount += 1
	WHERE CharacterID = @inbCharacterID AND FieldIndex = @insFieldIndex AND AttachedItemID = @intAttachedItemID;

	IF @@ROWCOUNT = 0
		INSERT dbo.FieldForCharacterAttachments (CharacterID, FieldIndex, AttachedItemID, AttachedItemCount)
		VALUES (@inbCharacterID, @insFieldIndex, @intAttachedItemID, 1);
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
