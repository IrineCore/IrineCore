USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_InitNestClearCount]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 15
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-23
description : ??? ?? ?? ??? ??? ??????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = ??? ??? ???? ??? ???? ?? at 2010-07-27
3 = ??????? @intMapID ?? : ??? ?? ??? ?? at 2010-09-13
4 = @inbCharacterID? NULL? ??? ?? ???? ?? ?????? ?? at 2010-10-04
11 = ?? ???? ?? ????? ?? ??? 500?? ????? ?? at 2010-10-25
12 = (43652) PC? ?? ??? ?? ?? ?? by ??? at 2011-12-13
**/

CREATE PROCEDURE [dbo].[P_InitNestClearCount]
	  @inbCharacterID bigint = NULL --// ??? ID. NULL ??? ?? ???? ??????.
	, @intMapID int = NULL   --// ? ID. NULL ??? ?? ?? ??????.
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
	/**_# [NestClear] ???? UPDATE???.*/
	IF @inbCharacterID > 0
		UPDATE dbo.NestClear
		SET ClearCount = 0, PCRoomClearCount = 0
		FROM dbo.NestClear WITH (INDEX(PK_NestClear), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
			AND MapID = CASE WHEN @intMapID IS NULL THEN MapID ELSE @intMapID END
		OPTION (KEEPFIXED PLAN);

	ELSE
	BEGIN
		CREATE TABLE #Temp (CharacterID bigint NOT NULL, MapID int NOT NULL);
		SET @inbCharacterID = 0;

		WHILE 1 = 1
		BEGIN
			TRUNCATE TABLE #Temp;

			INSERT #Temp (CharacterID, MapID)
			SELECT TOP (500) CharacterID, MapID
			FROM dbo.NestClear WITH (NOLOCK)
			WHERE CharacterID > @inbCharacterID AND MapID = COALESCE(@intMapID, MapID) AND ClearCount > 0
			ORDER BY CharacterID;

			IF @@ROWCOUNT = 0
				BREAK;

			UPDATE dbo.NestClear
			SET ClearCount = 0, PCRoomClearCount = 0
			FROM dbo.NestClear NC
				INNER JOIN #Temp T ON NC.CharacterID = T.CharacterID AND NC.MapID = T.MapID;
		END

		DROP TABLE #Temp;
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
