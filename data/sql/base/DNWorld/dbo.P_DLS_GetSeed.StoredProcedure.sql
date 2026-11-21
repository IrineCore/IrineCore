USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_GetSeed]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-02-14
description : ??? ??? ?????.
return value : Seed/bigint
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_GetSeed]
	  @inbCharacterID bigint = NULL --// ??? ID
	, @nvcCharacterName nvarchar(50) = NULL --// ??? ??
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
	IF @inbCharacterID IS NOT NULL
		SELECT Point AS Seed
		FROM dbo.Points WITH(NOLOCK, INDEX(PK_Points), FORCESEEK)
		WHERE CharacterID = @inbCharacterID
		  AND PointCode = 6;
	ELSE IF @nvcCharacterName IS NOT NULL
		SELECT P.Point AS Seed
		FROM dbo.Characters C  WITH(NOLOCK, INDEX(IX_UNF_Characters_CharacterName), FORCESEEK)
			INNER JOIN dbo.Points P  WITH(NOLOCK, INDEX(PK_Points), FORCESEEK) ON C.CharacterID = P.CharacterID
		WHERE C.CharacterName = @nvcCharacterName AND C.DeleteFlag = 0
		  AND P.PointCode = 6;
	
END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
