USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddFieldForCharacter]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-05-27
description : ?? ???? ??? ??
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103282 = ?? ???? ??????.
103284 = ??? ? ?????.
history :
4 = TF_Split(), TF_NSplit() ??? ????? ?? by ??? at 2011-11-01
**/

CREATE PROCEDURE [dbo].[P_AddFieldForCharacter]
	  @inbCharacterID bigint --// ??? ID
	, @insFieldIndex smallint --// ?? ???
	, @intItemID int --// ?? ???? ??? ID
	, @intElapsedTime int --// ??? ?? ?? ?? ?? (?)
	, @vchAttachedItemIDs varchar(max) --// ,? ?????? ??? ?? ???? ??? ???? ??? ID ???
	, @vchAttachedItemCounts varchar(max) --// ,? ?????? ??? ?? ???? ??? ???? ?? ???
	, @intMaxFieldCount int = 100 --// ?? ??? ? ?? ??? ? (?? ??? ??)
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
	IF (SELECT COUNT(*) FROM dbo.FieldsForCharacter WITH (NOLOCK) WHERE CharacterID = @inbCharacterID) >= @intMaxFieldCount
	BEGIN
		SET @intReturnValue = 103284;
		GOTO ErrorHandler;
	END

	/**_# ?? ???? ??? ?? ????? 103282? ?????.*/
	INSERT dbo.FieldsForCharacter (CharacterID, FieldIndex, ItemID, PlantDate, ElapsedTime, LastUpdateDate)
	VALUES (@inbCharacterID, @insFieldIndex, @intItemID, GETDATE(), @intElapsedTime, GETDATE());

	IF @vchAttachedItemIDs > ''
		INSERT dbo.FieldForCharacterAttachments (CharacterID, FieldIndex, AttachedItemID, AttachedItemCount)
		SELECT @inbCharacterID, @insFieldIndex, CAST(A.string AS int), CAST(B.string AS int)
		FROM dbo.TF_Split(@vchAttachedItemIDs, ',') A
			INNER JOIN dbo.TF_Split(@vchAttachedItemCounts, ',') B ON A.seq = B.seq
		OPTION (KEEPFIXED PLAN);

END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 2627 AND ERROR_MESSAGE() LIKE N'%PK_FieldsForCharacter%'
		SET @intReturnValue = 103282;

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
