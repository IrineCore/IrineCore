USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddStageClearBest]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2012-12-12
description : ???? CP ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AddStageClearBest]
	  @intMapID int --// ?ID
	, @inbCharacterID bigint --// ???ID
	, @inyClearGradeCode tinyint --// 10=SSS, 11=SS, 12=S, 13=A, 14=B, 15=C, 16=D
	, @intClearPoint int --// ??? ???
	, @inyRecommendedCharacterLevel tinyint --// ? ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	  , @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [StageClearBest] ???? MERGE???.*/
	;WITH T AS(
		SELECT MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate
		FROM dbo.StageClearBest
		WHERE MapID = @intMapID
	), S (MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint) AS (
		SELECT @intMapID, @inbCharacterID, C.CharacterName, @inyClearGradeCode, @intClearPoint
		FROM dbo.Characters C WITH(NOLOCK)
			INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK) ON C.CharacterID = CS.CharacterID
		WHERE C.CharacterID = @inbCharacterID
          AND CS.CharacterLevel <= @inyRecommendedCharacterLevel
	)
	MERGE T
	USING S
	ON T.MapID = S.MapID
	WHEN MATCHED THEN
		UPDATE 
		SET CharacterID = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.CharacterID ELSE T.CharacterID END
		  , CharacterName = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.CharacterName ELSE T.CharacterName END
		  , ClearGradeCode = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearGradeCode ELSE T.ClearGradeCode END
		  , ClearPoint = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearPoint ELSE T.ClearPoint END
		  , RegisterDate = CASE WHEN T.ClearPoint < S.ClearPoint THEN @dt2Now ELSE T.RegisterDate END
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate) 
		VALUES (S.MapID, S.CharacterID, S.CharacterName, S.ClearGradeCode, S.ClearPoint, @dt2Now);

	/**_# [StageClearPersonalBest] ???? MERGE???.*/
	WITH T AS(
		SELECT MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate
		FROM dbo.StageClearPersonalBest
		WHERE CharacterID = @inbCharacterID 
	      AND MapID = @intMapID
	), S (CharacterID, MapID, CharacterName, ClearGradeCode, ClearPoint) AS (
		SELECT @inbCharacterID, @intMapID, C.CharacterName, @inyClearGradeCode, @intClearPoint
		FROM dbo.Characters C WITH(NOLOCK)
			INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK) ON C.CharacterID = CS.CharacterID
		WHERE C.CharacterID = @inbCharacterID
	)
	MERGE T
	USING S
	ON T.CharacterID = S.CharacterID AND T.MapID = S.MapID
	WHEN MATCHED THEN
		UPDATE 
		SET ClearGradeCode = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearGradeCode ELSE T.ClearGradeCode END
		  , ClearPoint = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearPoint ELSE T.ClearPoint END
		  , RegisterDate = CASE WHEN T.ClearPoint < S.ClearPoint THEN @dt2Now ELSE T.RegisterDate END
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate) 
		VALUES (S.MapID, S.CharacterID, S.CharacterName, S.ClearGradeCode, S.ClearPoint, @dt2Now);

	/**_# [StageClearMonthlyBest] ???? MERGE???.*/
	WITH T AS(
		SELECT MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate
		FROM dbo.StageClearMonthlyBest
		WHERE MapID = @intMapID
	), S (MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint) AS (
		SELECT @intMapID, @inbCharacterID, C.CharacterName, @inyClearGradeCode, @intClearPoint
		FROM dbo.Characters C WITH(NOLOCK)
			INNER JOIN dbo.CharacterStatus CS WITH(NOLOCK) ON C.CharacterID = CS.CharacterID
		WHERE C.CharacterID = @inbCharacterID
          AND CS.CharacterLevel <= @inyRecommendedCharacterLevel
	)
	MERGE T
	USING S
	ON T.MapID = S.MapID
	WHEN MATCHED THEN
		UPDATE 
		SET CharacterID = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.CharacterID ELSE T.CharacterID END
		  , CharacterName = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.CharacterName ELSE T.CharacterName END
		  , ClearGradeCode = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearGradeCode ELSE T.ClearGradeCode END
		  , ClearPoint = CASE WHEN T.ClearPoint < S.ClearPoint THEN S.ClearPoint ELSE T.ClearPoint END
		  , RegisterDate = CASE WHEN T.ClearPoint < S.ClearPoint THEN @dt2Now ELSE T.RegisterDate END
	WHEN NOT MATCHED BY TARGET THEN
		INSERT (MapID, CharacterID, CharacterName, ClearGradeCode, ClearPoint, RegisterDate) 
		VALUES (S.MapID, S.CharacterID, S.CharacterName, S.ClearGradeCode, S.ClearPoint, @dt2Now);


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
