USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetMissionBinary]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-03-15
description : ?? ????? ?????.
result set : 
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = (59407) input param. @intVersion ?? by ??? at 2012-06-12
4 = (59407) param. @binGainMissionBin? ???? 625 -> 1250?? ?? by ??? at 2012-06-12
**/

CREATE PROCEDURE [dbo].[P_GetMissionBinary]
	  @inbCharacterID bigint --// ??? ID
	, @binGainMissionBin binary(1250) OUTPUT --// ?? ?? Binary
	, @binAchieveMissionBin binary(625) OUTPUT --// ?? ?? Binary
	, @intVersion int = 1 --// ??? SP ??
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
	/**_# ??? ???? 1?? ?? SELECT ???.*/
	SELECT @binGainMissionBin = GainMissionBin
	FROM dbo.MissionBinaries WITH (NOLOCK)
	WHERE CharacterID = @inbCharacterID;

	IF @intVersion = 1
		SET @binAchieveMissionBin = dbo.FN_AchieveMissionBin(@inbCharacterID);
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
