USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_DLS_ModDNGirlSchedule]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-01
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103409 = ?? ??? ???? ????.
**/

CREATE PROCEDURE [dbo].[P_DLS_ModDNGirlSchedule]
	  @intDNGirlSeasonID int --// ?? ??
	, @dtmSeasonStartDate datetime = NULL --// ?? ?? ??
	, @dtmSeasonEndDate datetime = NULL  --// ?? ?? ??
	, @dtmCeremonyEndDate datetime = NULL  --// ???? ?? ??	
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
	/**_# [DNGirlSeasonSchedules] ???? UPDATE???.*/
	UPDATE dbo.DNGirlSeasonSchedules
	SET SeasonStartDate = COALESCE(@dtmSeasonStartDate, SeasonStartDate)
	  , SeasonEndDate = COALESCE(@dtmSeasonEndDate, SeasonEndDate)
	  , CeremonyEndDate = COALESCE(@dtmCeremonyEndDate, CeremonyEndDate)	  
	WHERE DNGirlSeasonID = @intDNGirlSeasonID;
	
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103409;
		GOTO ErrorHandler;
	END
END TRY
BEGIN CATCH

	IF XACT_STATE() <> 0
		ROLLBACK TRANSACTION;

	IF @intReturnValue IS NULL OR @intReturnValue = 0
		EXEC @intReturnValue = dbo.P_AddErrorLog @intPROCID = @@PROCID;

	ELSE
		GOTO ErrorHandler;

	RETURN @intReturnValue;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

RETURN @intReturnValue;
GO
