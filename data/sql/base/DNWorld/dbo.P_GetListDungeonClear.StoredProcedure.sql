USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListDungeonClear]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 5
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-13
description : ???? ?? ??? ??? ?????. (???? ?? ?? ???? ?????.)
result set : MapID/int, DifficultyCode/tinyint
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103109 = ???? ??? ????.
history :
5 = NEST? ???? ????? ?? at 2010-07-23
6 = ??? ??? ???? ??? ???? ?? at 2010-07-27
**/

CREATE PROCEDURE [dbo].[P_GetListDungeonClear]
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
	/**_# [DungeonClear] ???? SELECT???.*/
	;WITH C1 AS (
		SELECT ROW_NUMBER() OVER(PARTITION BY MapID ORDER BY DifficultyCode DESC) AS RowNumber, MapID, DifficultyCode
		FROM dbo.DungeonClear WITH (NOLOCK)
		WHERE CharacterID = @inbCharacterID
	)
	SELECT MapID, DifficultyCode
	FROM C1
	WHERE RowNumber = 1;

	/**_# ???? ??? ??? 103109? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103109;
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
