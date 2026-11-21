USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_UpsGainMissionBin]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-15
description : ?? ??? ?? ???? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103102 = ???? ???? ????.
history :
3 = (59407) param. @binGainMissionBin? ???? 625 -> 1250?? ?? by ??? at 2012-06-12
**/

CREATE PROCEDURE [dbo].[P_UpsGainMissionBin]
	  @inbCharacterID bigint --// ??? ID
	, @binGainMissionBin binary(1250) --// ?? ?? Binary
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
	/**_# [MissionBinaries] ???? UPSERT???.*/
	UPDATE dbo.MissionBinaries
	SET GainMissionBin = @binGainMissionBin
	WHERE CharacterID = @inbCharacterID;

	IF @@ROWCOUNT = 0
	BEGIN
		INSERT dbo.MissionBinaries (CharacterID, GainMissionBin)
		VALUES (@inbCharacterID, @binGainMissionBin);
	END
END TRY
BEGIN CATCH
	IF ERROR_NUMBER() = 547 AND ERROR_MESSAGE() LIKE N'%FK_Characters_MissionBinaries_CharacterID%'
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
