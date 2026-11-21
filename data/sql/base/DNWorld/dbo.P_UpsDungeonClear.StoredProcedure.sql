USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsDungeonClear]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 6
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ?? ??? ??? UPSERT???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
4 = ??? NestFlag ??? ????? ?? at 2010-07-23
5 = ?? ?? at 2010-07-27
6 = ??? ??? ???? ??? ???? ?? at 2010-07-27
**/

CREATE PROCEDURE [dbo].[P_UpsDungeonClear]
	  @inbCharacterID bigint --// ??? ID
	, @intMapID int --// ???? ? ID
	, @inyDifficultyCode tinyint --// 0, 1, 2
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
	/**_# [DungeonClear] ???? UPSERT???.*/
	UPDATE dbo.DungeonClear
	SET ClearCount += 1
	WHERE CharacterID = @inbCharacterID AND MapID = @intMapID AND DifficultyCode = @inyDifficultyCode;

	IF @@ROWCOUNT = 0
		INSERT dbo.DungeonClear (CharacterID, MapID, DifficultyCode, ClearCount)
		VALUES (@inbCharacterID, @intMapID, @inyDifficultyCode, 1);
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_DungeonClear_CharacterID%'
		SET @intReturnValue = 103102;

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
