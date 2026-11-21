USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListFieldByCharacter]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-13
description : ???? ??? ??? ?????.
result set : FarmID/int, FieldIndex/smallint, ItemID/int, ElapsedTime/int, AttachedItemIDs/varchar, AttachedItemCounts/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103281 = ???? ????.
history :
2 = AttachedItemCounts ?? ?? ?? at 2010-10-28
**/

CREATE PROCEDURE [dbo].[P_GetListFieldByCharacter]
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
	;WITH C AS (
		SELECT FA.FarmID, FA.FieldIndex, FA.AttachedItemID, COUNT(*) AS AttachedItemCount
		FROM dbo.Fields F WITH (NOLOCK, INDEX(PK_Fields), FORCESEEK)
			INNER JOIN dbo.FieldAttachments FA WITH (NOLOCK, INDEX(IX_NN_FieldAttachments_FarmID_FieldIndex), FORCESEEK) ON F.FarmID = FA.FarmID AND F.FieldIndex = FA.FieldIndex
		WHERE F.CharacterID = @inbCharacterID
		GROUP BY FA.FarmID, FA.FieldIndex, FA.AttachedItemID
	)
	SELECT FL.FarmID, FL.FieldIndex, FL.ItemID, FL.ElapsedTime
		, STUFF((
			SELECT ',' + CAST(AttachedItemID AS varchar(10))
			FROM C
			WHERE FarmID = FL.FarmID AND FieldIndex = FL.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemIDs
		, STUFF((
			SELECT ',' + CAST(AttachedItemCount AS varchar(10))
			FROM C
			WHERE FarmID = FL.FarmID AND FieldIndex = FL.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemCounts
	FROM dbo.Fields FL WITH (NOLOCK, INDEX(IX_NN_Fields_CharacterID), FORCESEEK)
	WHERE FL.CharacterID = @inbCharacterID
	ORDER BY FL.FarmID, FL.FieldIndex
	OPTION (KEEPFIXED PLAN);

	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103281;
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
