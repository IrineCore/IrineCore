USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListField]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-10-13
description : ??? ??? ??? ?????.
result set : FieldIndex/smallint, CharacterID/bigint, CharacterName/nvarchar, CharacterLevel/tinyint, JobCode/tinyint, ItemID/int, ElapsedTime/int, AttachedItemIDs/varchar, AttachedItemCounts/varchar
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103281 = ???? ????.
history :
2 = join ?? ?? at 2010-10-27
3 = AttachedItemCounts ?? ?? ?? at 2010-10-28
**/

CREATE PROCEDURE [dbo].[P_GetListField]
	  @intFarmID int --// ?? ID
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
	SELECT F.FieldIndex, CH.CharacterID, CH.CharacterName, CHS.CharacterLevel, CHS.JobCode
		, F.ItemID, F.ElapsedTime
		, STUFF((
			SELECT ',' + CAST(AttachedItemID AS varchar(10))
			FROM dbo.VIX_FieldAttachmentCount C WITH (NOLOCK, NOEXPAND)
			WHERE FarmID = F.FarmID AND FieldIndex = F.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemIDs
		, STUFF((
			SELECT ',' + CAST(AttachedItemCount AS varchar(10))
			FROM dbo.VIX_FieldAttachmentCount C WITH (NOLOCK, NOEXPAND)
			WHERE FarmID = F.FarmID AND FieldIndex = F.FieldIndex
			FOR XML PATH('')
		), 1, 1, '') AS AttachedItemCounts
	FROM dbo.Fields F WITH (NOLOCK)
		INNER JOIN dbo.Characters CH WITH (NOLOCK) ON F.CharacterID = CH.CharacterID
		INNER JOIN dbo.CharacterStatus CHS WITH (NOLOCK) ON CH.CharacterID = CHS.CharacterID
	WHERE FarmID = @intFarmID;

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
