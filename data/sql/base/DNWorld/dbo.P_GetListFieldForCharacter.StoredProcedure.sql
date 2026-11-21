USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldForCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-27
description : ?? ??? ?? (??? ??)
result set : FieldIndex/smallint, ItemID/int, ElapsedTime/int, LastUpdateDate/datetime2, AttachedItemIDs/varchar, AttachedItemCounts/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListFieldForCharacter]
	  @inbCharacterID bigint --// ??? ID
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
	SELECT FL.FieldIndex, FL.ItemID, FL.ElapsedTime, FL.LastUpdateDate
		, STUFF((
			SELECT ',' + CAST(AttachedItemID AS varchar(10))
			FROM dbo.FieldForCharacterAttachments WITH (NOLOCK, FORCESEEK)
			WHERE CharacterID = @inbCharacterID AND FieldIndex = FL.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemIDs
		, STUFF((
			SELECT ',' + CAST(AttachedItemCount AS varchar(10))
			FROM dbo.FieldForCharacterAttachments WITH (NOLOCK, FORCESEEK)
			WHERE CharacterID = @inbCharacterID AND FieldIndex = FL.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemCounts
	FROM dbo.FieldsForCharacter FL WITH (NOLOCK, FORCESEEK)
	WHERE FL.CharacterID = @inbCharacterID
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
