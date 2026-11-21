USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AllCharacterSeedExtinction]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-02-14
description : ?????? ??? ??????.(??!! ?????? ??!!) 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_AllCharacterSeedExtinction]
	  @inyExtinctionCode tinyint--// 1=???? ??, 2=?????? ??
	, @inbSeed bigint = NULL--// ??? ?? ? 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE(), @inbRemainSeed bigint;

DECLARE @tblLogs TABLE(CharacterID bigint, PointBefore int, PointAfter int)

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Points] ???? UPDATE???.*/
	UPDATE dbo.Points
	SET Point = CASE WHEN @inyExtinctionCode = 1 THEN 0 ELSE CASE WHEN Point - @inbSeed < 0 THEN 0 ELSE Point - @inbSeed END END
	OUTPUT deleted.CharacterID, deleted.Point, inserted.Point
	INTO @tblLogs (CharacterID, PointBefore, PointAfter)
    WHERE PointCode = 6
      AND Point > 0

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 6, PointBefore, PointAfter, 0, dbo.FN_IP2IPNumber('0.0.0.0')
		FROM @tblLogs;

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 6, PointBefore, PointAfter, 0, dbo.FN_IP2IPNumber('0.0.0.0')
		FROM @tblLogs;

	ELSE
		INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		SELECT @dt2Now, CharacterID, 6, PointBefore, PointAfter, 0, dbo.FN_IP2IPNumber('0.0.0.0')
		FROM @tblLogs;

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
