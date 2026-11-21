USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPoint]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 8
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-03-04
description : ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
5 = ??? ?? ?? at 2011-03-21
6 = ??? ??? ?? ?? at 2011-03-29
7 = input parameter ?? ??     by 20110412.wgkim
**/

CREATE PROCEDURE [dbo].[P_AddPoint]
	  @inbCharacterID bigint --// ??? ID
	, @inyPointCode tinyint --// 1=PVP ?? ???, 2=?? ?? ???, 3=?? ?? ???, 4=?? ?? ???, 5=??? ?? ???, 6=??
	, @inbPoint bigint --// ??? ??? ?
	, @intMapID int --// ? ID
	, @vchIP varchar(15) --// IP ??
	, @inbRemainPoint bigint OUTPUT --// ?? ? ?? ??? ?
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @intLogTableSuffix int, @dt2Now datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	IF @inbPoint < 0
		RETURN 0;

	/**_# [Points] ???? UPSERT???.*/
	UPDATE dbo.Points
	SET @inbRemainPoint = Point += @inbPoint
	WHERE CharacterID = @inbCharacterID AND PointCode = @inyPointCode;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.Points (CharacterID, PointCode, Point)
		VALUES (@inbCharacterID, @inyPointCode, @inbPoint);

		SET @inbRemainPoint = @inbPoint;
	END

	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2Now);

	IF @intLogTableSuffix = 0
		INSERT dbo.PointLogs0 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, @inyPointCode, @inbRemainPoint - @inbPoint, @inbRemainPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PointLogs1 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, @inyPointCode, @inbRemainPoint - @inbPoint, @inbRemainPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP));

	ELSE
		INSERT dbo.PointLogs2 (LogDate, CharacterID, PointCode, PointBefore, PointAfter, MapID, IP)
		VALUES (@dt2Now, @inbCharacterID, @inyPointCode, @inbRemainPoint - @inbPoint, @inbRemainPoint, @intMapID, dbo.FN_IP2IPNumber(@vchIP));

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
