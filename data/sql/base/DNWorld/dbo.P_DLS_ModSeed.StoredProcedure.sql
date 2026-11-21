USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModSeed]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2013-02-14
description : ??? ?? ? ??? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModSeed]
	  @inbCharacterID bigint --// ??? ID
	, @inbSeed bigint --// ?? ?(+/-??) 
	, @vchIP varchar(15) --// IP ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE(), @inbRemainSeed bigint ;


/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [Points] ???? UPSERT???.*/
	UPDATE dbo.Points
	SET @inbRemainSeed = Point = CASE WHEN Point + @inbSeed < 0 THEN 0 ELSE Point + @inbSeed END
	WHERE CharacterID = @inbCharacterID
      AND PointCode = 6
	
	IF @@ROWCOUNT = 0 AND @inbSeed > 0
	BEGIN
		INSERT dbo.Points (CharacterID, PointCode, Point)
		VALUES (@inbCharacterID, 6, @inbSeed);
		
		SET @inbRemainSeed = @inbSeed;
	END

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @inbRemainSeed IS NOT NULL
	BEGIN
		IF @intLogTableSuffix = 0
			INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 6, @inbRemainSeed - @inbSeed, @inbRemainSeed, 0, dbo.FN_IP2IPNumber(@vchIP));

		ELSE IF @intLogTableSuffix = 1
			INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 6, @inbRemainSeed - @inbSeed, @inbRemainSeed, 0, dbo.FN_IP2IPNumber(@vchIP));

		ELSE
			INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
			VALUES (@dt2Now, @inbCharacterID, 6, @inbRemainSeed - @inbSeed, @inbRemainSeed, 0, dbo.FN_IP2IPNumber(@vchIP));
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
	EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

RETURN @intReturnValue;
GO
