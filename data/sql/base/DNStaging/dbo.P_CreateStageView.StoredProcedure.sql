USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_CreateStageView]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-06-16
description : V_Stages ?? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_CreateStageView]
	  @vchStageIDs varchar(max) --// ,? ?????? ???? ID ???
	, @vchNestStageIDs varchar(max) --// ,? ?????? "???" ???? ID ???
WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @vchStmt varchar(max);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF OBJECT_ID(N'dbo.V_Stages') IS NOT NULL
		SET @vchStmt = N'ALTER';

	ELSE
		SET @vchStmt = 'CREATE';

	SET @vchStmt += ' VIEW dbo.V_Stages
AS
SELECT StageID, StageName, RecommendedCharacterLevel,
	CASE
		WHEN StageID IN (' + @vchNestStageIDs + ') THEN CAST(1 AS bit)
		ELSE CAST(0 AS bit)
	END AS RaidFlag
FROM dbo.DimStages WITH (NOLOCK)
WHERE StageID IN (' + @vchStageIDs + ');'

	EXEC (@vchStmt);
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
