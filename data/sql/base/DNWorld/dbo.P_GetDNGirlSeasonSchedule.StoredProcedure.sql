USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_GetDNGirlSeasonSchedule]    Script Date: 11/14/2026 8:54:38 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 1
author : dailykim
e-mail : dailykim@eyedentitygames.com
created date : 2013-08-05
description : ??? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
**/

CREATE PROCEDURE [dbo].[P_GetDNGirlSeasonSchedule]
	  @intDNGirlSeasonID int OUTPUT --// ??? ?? ID
	, @dtmSeasonStartDate datetime OUTPUT --// ?? ?? ??
	, @dtmSeasonEndDate datetime OUTPUT --// ?? ?? ??
	, @dtmCeremonyEndDate datetime OUTPUT --// ???? ?? ??	 
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
	/**_# ???? ???? ?? ?? ?? / ??? ???? ?? ?? ??? ?????.*/
	
	SELECT TOP 1 @intDNGirlSeasonID = DNGirlSeasonID, @dtmSeasonStartDate = SeasonStartDate, @dtmSeasonEndDate = SeasonEndDate, @dtmCeremonyEndDate = CeremonyEndDate
	FROM dbo.DNGirlSeasonSchedules WITH (NOLOCK)
	WHERE CeremonyEndDate >= GETDATE()
	ORDER BY CeremonyEndDate;
	
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
