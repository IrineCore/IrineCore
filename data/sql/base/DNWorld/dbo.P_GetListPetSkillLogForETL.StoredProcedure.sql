USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetListPetSkillLogForETL]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-09-26
description : ? ?? ??? ?????.. DNStaging ???
result set : BasisDate, PetItemSerial, SkillID, WorldID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetListPetSkillLogForETL]
	  @dtmBasisDate datetime --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int = dbo.FN_LogTableSuffix(@dtmBasisDate);
DECLARE @intWorldID int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SELECT TOP (1) @intWorldID = WorldID FROM dbo.WorldCoverage WITH (NOLOCK) ORDER BY WorldID;

	IF @intLogTableSuffix = 0
		SELECT BasisDate, PetItemSerial, SkillID, @intWorldID AS WorldID
		FROM dbo.PetSkillLogs0 WITH (NOLOCK)
		WHERE BasisDate >= CAST(@dtmBasisDate AS datetime2(0)) AND BasisDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE IF @intLogTableSuffix = 1
		SELECT BasisDate, PetItemSerial, SkillID, @intWorldID AS WorldID
		FROM dbo.PetSkillLogs1 WITH (NOLOCK)		
		WHERE BasisDate >= CAST(@dtmBasisDate AS datetime2(0)) AND BasisDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

	ELSE
		SELECT BasisDate, PetItemSerial, SkillID, @intWorldID AS WorldID
		FROM dbo.PetSkillLogs2 WITH (NOLOCK)
		WHERE BasisDate >= CAST(@dtmBasisDate AS datetime2(0)) AND BasisDate < CAST(@dtmBasisDate + 1 AS datetime2(0))
		OPTION (MAXDOP 1);

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
